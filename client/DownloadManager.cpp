/* 
 * Copyright (C) 2001 Jacek Sieka, jacek@creatio.se
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "stdafx.h"
#include "DCPlusPlus.h"

#include "DownloadManager.h"
#include "ConnectionManager.h"
#include "Client.h"
#include "User.h"
#include "ClientManager.h"

DownloadManager* DownloadManager::instance = NULL;

/**
 * Each minute we check whether any of the users in the download queue have gone offline or connected
 */
void DownloadManager::onTimerMinute(DWORD aTick) {
	cs.enter();

	Download::List offline;

	for(Download::Iter i = queue.begin(); i != queue.end(); ++i) {
		Download*d = *i;

		if(d->isSet(Download::RUNNING)) {
			continue;
		}

		// Check if we've got a user pointer at all
		if(!d->getUser()) {
			d->setUser(ClientManager::getInstance()->findUser(d->getLastNick()));
	
			if(!d->getUser()) {
				// Still no user, go on to the next one...
				continue;
			}

		}

		// Check if the user is still online
		if(!d->getUser()->isOnline()) {
			bool found = false;
			for(UserConnection::Iter i = connections.begin(); i != connections.end(); ++i) {
				if((*i)->getUser() == d->getUser()) {
					found = true;
				}
			}
			if(!found) {
				// Damn, we've lost him...
				d->setUser(User::nuser);
				offline.push_back(d);
			}
			continue;
		}

		// Alright, we've made it this far, add the user to the waiting queue (unless he's there already...)
		map<User::Ptr, DWORD>::iterator i = waiting.find(d->getUser());
		if(i == waiting.end()) {
			waiting[d->getUser()] = 0;
		}
	}
	cs.leave();

	for(Download::Iter j = offline.begin(); j != offline.end(); ++j) {
		fireFailed(*j, "User is offline");
	}

}

void DownloadManager::onTimerSecond(DWORD aTick) {
	cs.enter();

	map<User::Ptr, DWORD>::iterator i = waiting.begin();

	// Check on the users we're waiting for...
	while(i != waiting.end()) {
		// Check if something's happened the last 60 seconds...
		if(i->second + 60*1000 < aTick) {
			// Update the timer
			i->second = aTick;

			Download* d = getNextDownload(i->first);
			if(d && d->getUser()->isOnline()) {
				int status = ConnectionManager::getInstance()->getDownloadConnection(d->getUser());
				if(status==UserConnection::CONNECTING) {
					fireConnecting(d);
				} else if(status == UserConnection::FREE) {
					// Alright, the connection was reused, so the waiting pool might have changed...try again...
					i = waiting.begin();
					continue;
				}
				
			} else {
				// Duuh...no downloads for this user...remove him/her from the waiting queue...
				i = waiting.erase(i);
				continue;
			}
		}
		++i;
	}

	for(Download::MapIter m = running.begin(); m != running.end(); ++m) {
		if(m->second->getPos() > 0) {
			fireTick(m->second);
		}
	}
	cs.leave();
	
}

void DownloadManager::connectFailed(const User::Ptr& aUser) {
	cs.enter();
	map<User::Ptr, DWORD>::iterator i = waiting.find(aUser);
	if(i != waiting.end()) {
		Download* d = getNextDownload(aUser);
		if(d) {
			if(!aUser->isOnline()) {
				waiting.erase(i);
				d->setUser(User::nuser);
				cs.leave();
				fireFailed(d, "Connection timeout, user disconnected");

			} else {
				cs.leave();
				fireFailed(d, "Connection timeout, user not responding");
			}
		} else {
			cs.leave();
		}
	} else {
		cs.leave();
	}
}

/**
 * Add a file to the download queue. When added, a connection attempt will automatically be
 * made, unless there is an existing connection to the user specified that is busy.
 * Note; make sure that there is a point in asking for a download, i.e. if the target file
 * has the same size as a download, the other client will complain that there's nothing to
 * send...
 * @param aFile Filename and path at server.
 * @param aSize Size of file, set to -1 if unknown.
 * @param aUser Pointer to a _connected_ user.
 * @param aTarget Target location of a file.
 * @param aResume Try to resume download if possible (not recommended for MyList.DcLst).
 */
void DownloadManager::download(const string& aFile, LONGLONG aSize, User::Ptr& aUser, const string& aTarget, bool aResume /* = true /*/) {
	Download* d = NULL;

	cs.enter();
	
	for(Download::Iter i = queue.begin(); i != queue.end(); ++i) {
		// First, search the queue for the same download...
		Download* dd = *i;
		if(dd->getTarget() == aTarget) {
			// Same download it seems, check it it's running...
			for(Download::MapIter j = running.begin(); j != running.end(); ++j) {
				if(j->second == dd) {
					// Yes, it's running, ignore it...
					cs.leave();
					return;
				}
			}
			d = dd;
		}
	}

	if(d == NULL) {
		// No such download, check if the target file is smaller than the one being downloaded...
		if(aResume && (aSize != -1) ) {
			if(Util::getFileSize(aTarget) >= aSize) {
				cs.leave();
				return;
			}
		}

		d = new Download();
	
		if(aFile.find('\\')) {
			d->setFileName(aFile.substr(aFile.rfind('\\')+1));
			d->setLast(aUser->getNick(), aFile.substr(0, aFile.rfind('\\')+1));
		} else {
			d->setFileName(aFile);
			d->setLast(aUser->getNick(), "");
		}
		if(d->getFileName().find(".DcLst") != string::npos)
			d->setFlag(Download::USER_LIST); 
		d->setUser(aUser);
		d->setTarget(aTarget);
		d->setSize(aSize);
		d->setResume(aResume);

		queue.push_back(d);
		cs.leave();

		fireAdded(d);

		cs.enter();
	} 

	if(waiting.find(aUser) == waiting.end()) {
		waiting[d->getUser()] = 0;
	}
	
	cs.leave();
	
}

/**
 * Add a file to the download queue. Useful if it is not certain that the user is connected.
 * If he/she is, it will be added as usual, otherwise it will be put on the queue and the 
 * DownloadManager will hopefully continue downloading it later on...
 * @param aFile Filename and path at server.
 * @param aSize Size of file, set to -1 if unknown.
 * @param aUser Pointer to a _connected_ user.
 * @param aTarget Target location of a file.
 * @param aResume Try to resume download if possible (not recommended for MyList.DcLst).
 */
void DownloadManager::download(const string& aFile, LONGLONG aSize, const string& aUser, const string& aTarget, bool aResume /* = true /*/) {
	Download* d = NULL;

	User::Ptr& user = ClientManager::getInstance()->findUser(aUser);
	if(user) {
		download(aFile, aSize, user, aTarget, aResume);
		return;
	}

	// We don't know who this user is, so we just add it to the list...
	cs.enter();
	
	for(Download::Iter i = queue.begin(); i != queue.end(); ++i) {
		// First, search the queue for the same download...
		Download* dd = *i;
		if(dd->getTarget() == aTarget) {
			// Same download it seems, remove it...todo: something clever
			cs.leave();
			return;
		}
	}

	d = new Download();

	if(aFile.find('\\')) {
		d->setFileName(aFile.substr(aFile.rfind('\\')+1));
		d->setLast(aUser, aFile.substr(0, aFile.rfind('\\')+1));
	} else {
		d->setFileName(aFile);
		d->setLast(aUser, "");
	}

	d->setTarget(aTarget);
	d->setSize(aSize);
	d->setResume(aResume);

	queue.push_back(d);
	cs.leave();

	fireAdded(d);
	fireFailed(d, d->getLastNick() + " has gone offline");
}

void DownloadManager::downloadList(User::Ptr& aUser) {
	string file = Settings::getAppPath() + aUser->getNick() + ".DcLst";
	download("MyList.DcLst", -1, aUser, file, false);
	userLists.push_back(file);
}

void DownloadManager::downloadList(const string& aUser) {
	string file = Settings::getAppPath() + aUser + ".DcLst";
	download("MyList.DcLst", -1, aUser, file, false);
	userLists.push_back(file);
}

void DownloadManager::removeDownload(Download* aDownload) {
	cs.enter();

	// Check the running downloads...
	for(Download::MapIter j = running.begin(); j != running.end(); ++j) {
		if(j->second == aDownload) {
			// This is worse, we have to abort the download...
			UserConnection* conn = j->first;
			running.erase(j);
			removeConnection(conn);
		}
	}
	
	// Search the queue
	for(Download::Iter i = queue.begin(); i != queue.end(); ++i) {
		if(*i == aDownload) {
			// Good! It's in the queue, we can simply remove it...
			queue.erase(i);
			delete aDownload;
			cs.leave();
			return;
		}
	}

	dcassert(0);
	// Not found...
	cs.leave();
}

void DownloadManager::removeConnection(UserConnection::Ptr aConn, bool reuse /* = false */) {
	cs.enter();
	for(UserConnection::Iter i = connections.begin(); i != connections.end(); ++i) {
		if(*i == aConn) {
			aConn->removeListener(this);
			connections.erase(i);
			ConnectionManager::getInstance()->putDownloadConnection(aConn, reuse);
			break;
		}
	}
	cs.leave();
}

void DownloadManager::removeConnections() {
	cs.enter();

	for(UserConnection::Iter i = connections.begin(); i != connections.end(); ++i) {
		UserConnection* c = *i;
		c->removeListener(this);
		ConnectionManager::getInstance()->putDownloadConnection(c);
	}
	connections.clear();
	cs.leave();
}

void DownloadManager::checkDownloads(UserConnection* aConn) {
	cs.enter();
	dcdebug("Checking downloads...");
	Download* d = getNextDownload(aConn->getUser());

	if(d) {
		running[aConn] = d;
		
		if(d->getResume()) {
			LONGLONG x = Util::getFileSize(d->getTarget());
			d->setPos( (x == -1) ? 0 : x);
		} else {
			d->setPos(0);
		}
		d->setFlag(Download::RUNNING);
		
		aConn->get(d->getLastPath()+d->getFileName(), d->getPos());
		dcdebug("Found!\n");
		cs.leave();
		return;
	}
	// Connection not needed any more, return it to the ConnectionManager...
	dcdebug("Not found!\n");

	// No more downloads for this user, make sure we're not waiting for a connection...
	waiting.erase(aConn->getUser());

	removeConnection(aConn, true);
	cs.leave();
}

void DownloadManager::onData(UserConnection* aSource, const BYTE* aData, int aLen) {
	cs.enter();
	dcassert(running.find(aSource) != running.end());
	DWORD len;
	Download* d = running[aSource];
	cs.leave();
	
	WriteFile(d->getFile(), aData, aLen, &len, NULL);
	d->addPos(len);
}

void DownloadManager::onFileLength(UserConnection* aSource, const string& aFileLength) {

	cs.enter();
	Download::MapIter i = running.find(aSource);
	dcassert(i != running.end());
	Download* d = i->second;

	HANDLE file;
	Util::ensureDirectory(d->getTarget());
	if(d->getResume())
		file = CreateFile(d->getTarget().c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	else
		file = CreateFile(d->getTarget().c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	
	if(file == INVALID_HANDLE_VALUE) {
		running.erase(i);
		d->unsetFlag(Download::RUNNING);
		cs.leave();
		removeConnection(aSource);

		fireFailed(d, "Could not open target file");
		return;
	}
	
	d->setFile(file, true);
	d->setPos(d->getSize(), true);
	d->setSize(aFileLength);

	if(d->getSize() == d->getPos()) {
		for(Download::Iter j = queue.begin(); j != queue.end(); ++j) {
			if(*j == d) {
				queue.erase(j);
				break;
			}
		}
		cs.leave();
		removeConnection(aSource);

		// We're done...and this connection is broken...
		fireComplete(d);
		delete d;
		
	} else {
		cs.leave();
		fireStarting(d);
		aSource->setDataMode(d->getSize() - d->getPos());
		aSource->startSend();
	}
}

/** Download finished! */
void DownloadManager::onModeChange(UserConnection* aSource, int aNewMode) {
	cs.enter();

	Download::MapIter i = running.find(aSource);
	dcassert(i != running.end());
	
	Download::Ptr p = i->second;
	running.erase(i);
	for(Download::Iter j = queue.begin(); j != queue.end(); ++j) {
		if(*j == p) {
			queue.erase(j);
			break;
		}
	}
	
	cs.leave();

	if(p->getPos() != p->getSize())
		dcdebug("Download incomplete??? : ");
	
	CloseHandle(p->getFile());
	p->setFile(NULL);

	dcdebug("Download finished: %s to %s, size %I64d\n", p->getFileName().c_str(), p->getTarget().c_str(), p->getSize());
	fireComplete(p);
	delete p;

	checkDownloads(aSource);
}

void DownloadManager::onMaxedOut(UserConnection* aSource) { 
	cs.enter();
	Download::MapIter i = running.find(aSource);
	dcassert(i != running.end());

	Download* d = i->second;
	
	running.erase(i);
	d->unsetFlag(Download::RUNNING);
	waiting[aSource->getUser()] = TimerManager::getTick();

	cs.leave();

	fireFailed(d, "No slots available");
	removeConnection(aSource);
}

void DownloadManager::onError(UserConnection* aSource, const string& aError) {
	cs.enter();
	Download::MapIter i = running.find(aSource);
	
	if(i == running.end()) {
		cs.leave();
		removeConnection(aSource);
		return;
	}

	Download* d = i->second;
	running.erase(i);
	d->unsetFlag(Download::RUNNING);
	cs.leave();
	
	if(d->getFile()) {
		CloseHandle(d->getFile());
		d->setFile(NULL);
	}
	fireFailed(d, aError);
	
	removeConnection(aSource);
}


/**
 * @file DownloadManger.cpp
 * $Id: DownloadManager.cpp,v 1.17 2001/12/21 23:52:30 arnetheduck Exp $
 * @if LOG
 * $Log: DownloadManager.cpp,v $
 * Revision 1.17  2001/12/21 23:52:30  arnetheduck
 * Last commit for five days
 *
 * Revision 1.16  2001/12/21 20:21:17  arnetheduck
 * Private messaging added, and a lot of other updates as well...
 *
 * Revision 1.15  2001/12/19 23:07:59  arnetheduck
 * Added directory downloading from the directory tree (although it hasn't been
 * tested at all) and password support.
 *
 * Revision 1.14  2001/12/18 12:32:18  arnetheduck
 * Stability fixes
 *
 * Revision 1.13  2001/12/16 19:47:48  arnetheduck
 * Reworked downloading and user handling some, and changed some small UI things
 *
 * Revision 1.12  2001/12/15 17:01:06  arnetheduck
 * Passive mode searching as well as some searching code added
 *
 * Revision 1.11  2001/12/13 19:21:57  arnetheduck
 * A lot of work done almost everywhere, mainly towards a friendlier UI
 * and less bugs...time to release 0.06...
 *
 * Revision 1.10  2001/12/11 01:10:29  arnetheduck
 * More bugfixes...I really have to change the bufferedsocket so that it only
 * uses one thread...or maybe even multiple sockets/thread...
 *
 * Revision 1.9  2001/12/10 10:48:40  arnetheduck
 * Ahh, finally found one bug that's been annoying me for days...=) the connections
 * in the pool were not reset correctly before being put back for later use...
 *
 * Revision 1.8  2001/12/07 20:03:06  arnetheduck
 * More work done towards application stability
 *
 * Revision 1.7  2001/12/05 14:27:35  arnetheduck
 * Premature disconnection bugs removed.
 *
 * Revision 1.6  2001/12/04 21:50:34  arnetheduck
 * Work done towards application stability...still a lot to do though...
 * a bit more and it's time for a new release.
 *
 * Revision 1.5  2001/12/02 23:47:35  arnetheduck
 * Added the framework for uploading and file sharing...although there's something strange about
 * the file lists...my client takes them, but not the original...
 *
 * Revision 1.4  2001/12/01 17:15:03  arnetheduck
 * Added a crappy version of huffman encoding, and some other minor changes...
 *
 * Revision 1.3  2001/11/29 19:10:54  arnetheduck
 * Refactored down/uploading and some other things completely.
 * Also added download indicators and download resuming, along
 * with some other stuff.
 *
 * Revision 1.2  2001/11/26 23:40:36  arnetheduck
 * Downloads!! Now downloads are possible, although the implementation is
 * likely to change in the future...more UI work (splitters...) and some bug
 * fixes. Only user file listings are downloadable, but at least it's something...
 *
 * Revision 1.1  2001/11/25 22:06:25  arnetheduck
 * Finally downloading is working! There are now a few quirks and bugs to be fixed
 * but what the heck....!
 *
 * @endif
 */
