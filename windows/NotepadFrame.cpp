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

#include "stdafx.h"
#include "../client/DCPlusPlus.h"
#include "Resource.h"

#include "NotepadFrame.h"
#include "WinUtil.h"

#include "../client/SimpleXML.h"

NotepadFrame* NotepadFrame::frame = NULL;
string NotepadFrame::text;

LRESULT NotepadFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	
	CreateSimpleStatusBar(ATL_IDS_IDLEMESSAGE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP);
	ctrlStatus.Attach(m_hWndStatusBar);
	
	ctrlPad.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_VSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_NOHIDESEL, WS_EX_CLIENTEDGE);
	
	ctrlPad.LimitText(0);
	ctrlPad.SetFont(WinUtil::font);
	ctrlPad.SetWindowText(text.c_str());

	frame = this;
	
	bHandled = FALSE;
	return 1;
}

void NotepadFrame::load(SimpleXML* aXml) {
	if(aXml->findChild("Notepad")) {
		aXml->stepIn();
		if(aXml->findChild("Text")) {
			text = aXml->getChildData();
		}
	}
}

void NotepadFrame::save(SimpleXML* aXml) {
	aXml->addTag("Notepad");
	aXml->stepIn();
	aXml->addTag("Text", text);
	aXml->stepOut();

}

/**
 * @file NotepadFrame.cpp
 * $Id: NotepadFrame.cpp,v 1.2 2002/04/13 12:57:23 arnetheduck Exp $
 */


