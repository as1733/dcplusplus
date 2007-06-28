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

#include "stdafx.h"
#include <client/DCPlusPlus.h>

#include "AboutDlg.h"

#include <client/SimpleXML.h>
#include <client/ResourceManager.h>
#include <client/version.h>
#include "WinUtil.h"

static const char thanks[] = "Big thanks to all donators and people who have contributed with ideas "
"and code! Thanks go out to sourceforge for hosting the project. This application uses bzip2 (www.bzip.org), "
"thanks to Julian Seward and team for providing it. Thiz application uses zlib (www.zlib.net), "
"thanks to Jean-loup Gailly and Mark Adler for providing it. "
"This product includes GeoIP data created by MaxMind, available from http://maxmind.com/. "
"This product uses yassl from www.yassl.com, thanks to Todd Ouska and Larry Stefonic."
"The following people have contributed code to "
"DC++ (I hope I haven't missed someone, they're in roughly chronological order...=):\r\n"
"geoff, carxor, luca rota, dan kline, mike, anton, zc, sarf, farcry, kyrre aalerud, opera, "
"patbateman, xeroc, fusbar, vladimir marko, kenneth skovhede, ondrea, todd pederzani, who, "
"sedulus, sandos, henrik engstr\303\266m, dwomac, robert777, saurod, atomicjo, bzbetty, orkblutt, "
"distiller, citruz, dan fulger, cologic, christer palm, twink, ilkka sepp\303\244l\303\244, johnny, ciber, "
"theparanoidone, gadget, naga, tremor, joakim tosteberg, pofis, psf8500, lauris ievins, "
"defr, ullner, fleetcommand, liny, xan, olle svensson, mark gillespie, jeremy huddleston, "
"bsod, sulan, jonathan stone, tim burton, izzzo, guitarm, paka, nils maier, jens oknelid, yoji, "
"krzysztof tyszecki, poison, pothead, pur, bigmuscle, martin, jove, bart vullings, "
"steven sheehy, tobias nygren, poy, dorian, stephan hohe, mafa_45, mikael eman. "
"Keep it coming!";

AboutDlg::AboutDlg(SmartWin::Widget* parent) : SmartWin::Widget(parent) {
	onInitDialog(&AboutDlg::handleInitDialog);
	onSpeaker(&AboutDlg::spoken);
}

AboutDlg::~AboutDlg() {
}

bool AboutDlg::handleInitDialog() {
	::SetDlgItemText(handle(), IDC_VERSION, Text::toT("DC++ " VERSIONSTRING "\n(c) Copyright 2001-2006 Jacek Sieka\nEx-codeveloper: Per Lind\303\251n\nGraphics: Martin Skogevall et al.\nDC++ is licenced under GPL\nhttp://dcplusplus.sourceforge.net/").c_str());
	::SetDlgItemText(handle(), IDC_TTH, WinUtil::tth.c_str());
	::SetDlgItemText(handle(), IDC_THANKS, Text::toT(thanks).c_str());
	::SetDlgItemText(handle(), IDC_TOTALS, Text::toT("Upload: " + Util::formatBytes(SETTING(TOTAL_UPLOAD)) + ", Download: " + Util::formatBytes(SETTING(TOTAL_DOWNLOAD))).c_str());
	if(SETTING(TOTAL_DOWNLOAD) > 0) {
		char buf[64];
		sprintf(buf, "Ratio (up/down): %.2f", ((double)SETTING(TOTAL_UPLOAD)) / ((double)SETTING(TOTAL_DOWNLOAD)));
		::SetDlgItemText(handle(), IDC_RATIO, Text::toT(buf).c_str());
	}
	::SetDlgItemText(handle(), IDC_LATEST, CTSTRING(DOWNLOADING));

	subclassButton(IDOK)->onClicked(&AboutDlg::handleOKClicked);

#ifdef PORT_ME
	CenterWindow(GetParent());
#endif

	c.addListener(this);
	c.downloadFile("http://dcplusplus.sourceforge.net/version.xml");

	return false;
}

HRESULT AboutDlg::spoken(LPARAM lParam, WPARAM wParam) {
 	if(wParam == SPEAK_VERSIONDATA) {
		tstring* x = (tstring*)lParam;
		::SetDlgItemText(handle(), IDC_LATEST, x->c_str());
		delete x;
	}
	return 0;
}

void AboutDlg::handleOKClicked(WidgetButtonPtr) {
	endDialog(IDOK);
}

void AboutDlg::on(HttpConnectionListener::Data, HttpConnection* /*conn*/, const uint8_t* buf, size_t len) throw() {
	downBuf.append((char*)buf, len);
}

void AboutDlg::on(HttpConnectionListener::Complete, HttpConnection* conn, const string&) throw() {
	if(!downBuf.empty()) {
		SimpleXML xml;
		xml.fromXML(downBuf);
		if(xml.findChild("DCUpdate")) {
			xml.stepIn();
			if(xml.findChild("Version")) {
				tstring* x = new tstring(Text::toT(xml.getChildData()));
				speak((LPARAM)x, SPEAK_VERSIONDATA);
			}
		}
	}
	conn->removeListener(this);
}

void AboutDlg::on(HttpConnectionListener::Failed, HttpConnection* conn, const string& aLine) throw() {
	tstring* x = new tstring(Text::toT(aLine));
	speak((LPARAM)x, SPEAK_VERSIONDATA);
	conn->removeListener(this);
}
