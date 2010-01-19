/*
 * Copyright (C) 2001-2008 Jacek Sieka, arnetheduck on gmail point com
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

#include "RichTextBox.h"

#include "WinUtil.h"
#include "ParamDlg.h"

RichTextBox::Seed::Seed() : 
	BaseType::Seed()
{
}

RichTextBox::RichTextBox( dwt::Widget * parent ) : BaseType(parent) {
	this->onLeftMouseDblClick(std::tr1::bind(&RichTextBox::handleLeftDblClick, this, _1));
}

void RichTextBox::handleCopy()
{
	WinUtil::setClipboard(getSelection());
}

void RichTextBox::handleFind()
{
	findText(findTextPopup());
}

bool RichTextBox::handleContextMenu(dwt::ScreenCoordinate pt)
{
	// This context menu is specialized for non-user-modifiable controls.
	dwt::MenuPtr menu(dwt::WidgetCreator<dwt::Menu>::create(this, WinUtil::Seeds::menu));
	menu->appendItem(T_("&Copy\tCtrl+C"), std::tr1::bind(&RichTextBox::handleCopy, this));
	menu->appendSeparator();
	menu->appendItem(T_("&Find...\tF3"), std::tr1::bind(&RichTextBox::handleFind, this));

	menu->open(pt, TPM_LEFTALIGN | TPM_RIGHTBUTTON);

	return true;
}

tstring RichTextBox::findTextPopup() {
	tstring param = Util::emptyStringT;
	ParamDlg lineFind(this, T_("Search"), T_("Specify search string"), Util::emptyStringT, false);
	if(lineFind.run() == IDOK) {
		param = lineFind.getValue();
	}
	return param;
}

void RichTextBox::findTextNext() {
	findText(currentNeedle.empty() ? findTextPopup() : currentNeedle);
}

bool RichTextBox::handleLeftDblClick(const dwt::MouseEvent& ev) {
	return WinUtil::parseDBLClick(textUnderCursor(ev.pos));
}
