/* 
 * Copyright (C) 2001 Jacek Sieka, j_s@telia.com
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

#if !defined(AFX_USER_H__26AA222C_500B_4AD2_A5AA_A594E1A6D639__INCLUDED_)
#define AFX_USER_H__26AA222C_500B_4AD2_A5AA_A594E1A6D639__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Util.h"
#include "Pointer.h"

class Client;
class SocketException;

/**
 * A user connected to a hubs.
 */
class User : public PointerBase, public Flags
{
public:
	enum {
		OP = 0x01,
		ONLINE = 0x02,
		DCPLUSPLUS = 0x04,
		PASSIVE = 0x08
	};
	typedef Pointer<User> Ptr;
	typedef vector<Ptr> List;
	typedef List::iterator Iter;
	typedef HASH_MAP<string,Ptr> NickMap;
	typedef NickMap::iterator NickIter;

	User(const string& aNick) throw() : nick(aNick), client(NULL), sharingLong(0) { };
	virtual ~User() throw() { };

	void setClient(Client* aClient);
	void connect();
	string getClientNick();
	void update();
	string getClientName();
	void privateMessage(const string& aMsg);
	void clientMessage(const string& aMsg);
	void kick(const string& aMsg);
	void redirect(const string& aTarget, const string& aReason);
	bool isClientOp();
	
	int64_t getBytesShared() const { return sharingLong; };
	const string& getBytesSharedString() const { return sharing; };
	void setBytesShared(int64_t aSharing) { sharing = Util::toString(aSharing); sharingLong = aSharing; };
	void setBytesShared(const string& aSharing) { sharing = aSharing; sharingLong = Util::toInt64(aSharing); };

	bool isOnline() const { return isSet(ONLINE); };
	bool isClient(Client* aClient) const { return client == aClient; };
	
	static void updated(User::Ptr& aUser);
	
	GETSETREF(string, connection, Connection);
	GETSETREF(string, nick, Nick);
	GETSETREF(string, email, Email);
	GETSETREF(string, description, Description);
	GETSETREF(string, lastHubIp, LastHubIp);
	GETSETREF(string, lastHubName, LastHubName)
private:
	RWLock cs;
	
	Client* client;
	string sharing;
	int64_t sharingLong;		// Cache this...requested very frequently...
	
};

#endif // !defined(AFX_USER_H__26AA222C_500B_4AD2_A5AA_A594E1A6D639__INCLUDED_)

/**
 * @file User.cpp
 * $Id: User.h,v 1.17 2002/04/13 12:57:23 arnetheduck Exp $
 */

