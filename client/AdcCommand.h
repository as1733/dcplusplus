/* 
 * Copyright (C) 2001-2003 Jacek Sieka, j_s@telia.com
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

class Command {
public:
	template<u_int32_t T>
	struct Type {
		static const u_int32_t type = T;
	};

	static const char TYPE_ACTIVE = 'A';
	static const char TYPE_BROADCAST = 'B';
	static const char TYPE_CLIENT = 'C';
	static const char TYPE_DIRECT = 'D';
	static const char TYPE_INFO = 'I';
	static const char TYPE_HUB = 'H';
	static const char TYPE_PASSIVE = 'P';
	static const char TYPE_UDP = 'U';

#define CMD(n, a, b, c) static const u_int32_t CMD_##n = (((u_int32_t)a) | (((u_int32_t)b)<<8) | (((u_int32_t)c)<<16)); typedef Type<CMD_##n> n;
	CMD(SUP, 'S','U','P');
	CMD(STA, 'S','T','A');
	CMD(INF, 'I','N','F');
	CMD(MSG, 'M','S','G');
	CMD(SCH, 'S','C','H');
	CMD(RES, 'R','E','S');
	CMD(CTM, 'C','T','M');
	CMD(GPA, 'G','P','A');
	CMD(PAS, 'P','A','S');
	CMD(QUI, 'Q','U','I');
	CMD(DSC, 'D','S','C');
	CMD(GET, 'G','E','T');
	CMD(GFI, 'G','F','I');
	CMD(SND, 'S','N','D');
	CMD(NTD, 'N','T','D');
#undef CMD

	Command(const string& aLine) : cmdInt(0), type(0) {
		parse(aLine);
	}

	void parse(const string& aLine);

	u_int32_t getCommand() const { return cmdInt; }
	char getType() const { return type; }

	StringList& getParameters() { return parameters; }
	const StringList& getParameters() const { return parameters; }

	operator ==(u_int32_t aCmd) { return cmdInt == aCmd; }

	static string escape(const string& str) {
		string tmp = str;
		string::size_type i = 0;
		while( (i = tmp.find_first_of(" \n\\", i)) != string::npos) {
			tmp.insert(i, 1, '\\');
			i+=2;
		}
		return tmp;
	}
private:

	StringList parameters;
	union {
		u_int8_t cmd[4];
		u_int32_t cmdInt;
	};
	char type;
	CID to;

};

template<class T>
class CommandHandler {
public:
	void dispatch(const string& aLine) {
		Command c(aLine);

#define CMD(n) case Command::CMD_##n: ((T*)this)->handle(c, Command::n()); break;
		switch(c.getCommand()) {
			CMD(SUP);
			CMD(STA);
			CMD(INF);
			CMD(MSG);
			CMD(SCH);
			CMD(RES);
			CMD(CTM);
			CMD(PAS);
			CMD(QUI);
			CMD(DSC);
			CMD(GET);
			CMD(GFI);
			CMD(SND);
			CMD(NTD);
		default: break;
		}
	}
};

/**
* @file
* $Id: AdcCommand.h,v 1.1 2004/04/04 12:11:51 arnetheduck Exp $
*/
