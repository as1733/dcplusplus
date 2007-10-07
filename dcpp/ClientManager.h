/*
 * Copyright (C) 2001-2007 Jacek Sieka, arnetheduck on gmail point com
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

#if !defined(CLIENT_MANAGER_H)
#define CLIENT_MANAGER_H

#include "TimerManager.h"

#include "Client.h"
#include "Singleton.h"
#include "SettingsManager.h"
#include "User.h"

#include "ClientManagerListener.h"

namespace dcpp {

class UserCommand;

class ClientManager : public Speaker<ClientManagerListener>,
	private ClientListener, public Singleton<ClientManager>,
	private TimerManagerListener, private SettingsManagerListener
{
public:
	Client* getClient(const string& aHubURL);
	void putClient(Client* aClient);

	size_t getUserCount() const;
	int64_t getAvailable() const;
	StringList getHubs(const CID& cid) const;
	StringList getHubNames(const CID& cid) const;
	StringList getNicks(const CID& cid) const;
	string getConnection(const CID& cid) const;

	bool isConnected(const string& aUrl) const;

	void search(int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken);
	void search(StringList& who, int aSizeMode, int64_t aSize, int aFileType, const string& aString, const string& aToken);
	void infoUpdated();

	UserPtr getUser(const string& aNick, const string& aHubUrl) throw();
	UserPtr getUser(const CID& cid) throw();

	string findHub(const string& ipPort) const;
	string findHubEncoding(const string& aUrl) const;

	UserPtr findUser(const string& aNick, const string& aHubUrl) const throw() { return findUser(makeCid(aNick, aHubUrl)); }
	UserPtr findUser(const CID& cid) const throw();
	UserPtr findLegacyUser(const string& aNick) const throw();

	bool isOnline(const UserPtr& aUser) const {
		Lock l(cs);
		return onlineUsers.find(aUser->getCID()) != onlineUsers.end();
	}

	bool isOp(const UserPtr& aUser, const string& aHubUrl) const;

	/** Constructs a synthetic, hopefully unique CID */
	CID makeCid(const string& nick, const string& hubUrl) const throw();

	void putOnline(OnlineUser* ou) throw();
	void putOffline(OnlineUser* ou) throw();

	UserPtr& getMe();

	void connect(const UserPtr& p, const string& token);
	void send(AdcCommand& c, const CID& to);
	void privateMessage(const UserPtr& p, const string& msg);

	void userCommand(const UserPtr& p, const UserCommand& uc, StringMap& params, bool compatibility);

	bool isActive() { return SETTING(INCOMING_CONNECTIONS) != SettingsManager::INCOMING_FIREWALL_PASSIVE; }

	void lock() throw() { cs.enter(); }
	void unlock() throw() { cs.leave(); }

	Client::List& getClients() { return clients; }

	string getCachedIp() const { Lock l(cs); return cachedIp; }

	CID getMyCID();
	const CID& getMyPID();

private:
	typedef unordered_map<string, UserPtr> LegacyMap;
	typedef LegacyMap::iterator LegacyIter;

	typedef unordered_map<CID, UserPtr, CID::Hash> UserMap;
	typedef UserMap::iterator UserIter;

	typedef unordered_multimap<CID, OnlineUser*, CID::Hash> OnlineMap;
	typedef OnlineMap::iterator OnlineIter;
	typedef OnlineMap::const_iterator OnlineIterC;
	typedef pair<OnlineIter, OnlineIter> OnlinePair;
	typedef pair<OnlineIterC, OnlineIterC> OnlinePairC;

	Client::List clients;
	mutable CriticalSection cs;

	UserMap users;
	OnlineMap onlineUsers;

	UserPtr me;

	Socket udp;

	string cachedIp;
	CID pid;

	friend class Singleton<ClientManager>;

	ClientManager() {
		TimerManager::getInstance()->addListener(this);
		SettingsManager::getInstance()->addListener(this);
	}

	virtual ~ClientManager() throw() {
		SettingsManager::getInstance()->removeListener(this);
		TimerManager::getInstance()->removeListener(this);
	}

	void updateCachedIp();

	// SettingsManagerListener
	virtual void on(Load, SimpleXML&) throw();

	// ClientListener
	virtual void on(Connected, Client* c) throw() { fire(ClientManagerListener::ClientConnected(), c); }
	virtual void on(UserUpdated, Client*, const OnlineUser& user) throw() { fire(ClientManagerListener::UserUpdated(), user); }
	virtual void on(UsersUpdated, Client* c, const UserList&) throw() { fire(ClientManagerListener::ClientUpdated(), c); }
	virtual void on(Failed, Client*, const string&) throw();
	virtual void on(HubUpdated, Client* c) throw() { fire(ClientManagerListener::ClientUpdated(), c); }
	virtual void on(HubUserCommand, Client*, int, int, const string&, const string&) throw();
	virtual void on(NmdcSearch, Client* aClient, const string& aSeeker, int aSearchType, int64_t aSize,
		int aFileType, const string& aString) throw();
	virtual void on(AdcSearch, Client* c, const AdcCommand& adc, const CID& from) throw();
	// TimerManagerListener
	virtual void on(TimerManagerListener::Minute, uint32_t aTick) throw();
};

} // namespace dcpp

#endif // !defined(CLIENT_MANAGER_H)