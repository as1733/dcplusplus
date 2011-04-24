/*
 * Copyright (C) 2001-2011 Jacek Sieka, arnetheduck on gmail point com
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

#include "resource.h"

#include "NotificationsPage.h"

#include <dwt/widgets/LoadDialog.h>

#include <dcpp/SettingsManager.h>
#include "MainWindow.h"
#include "WinUtil.h"

NotificationsPage::Option NotificationsPage::options[] = {
	{ N_("Download finished"), SettingsManager::SOUND_FINISHED_DL, Util::emptyStringT,
	SettingsManager::BALLOON_FINISHED_DL, false, IDH_SETTINGS_NOTIFICATIONS_FINISHED_DL },
	{ N_("File list downloaded"), SettingsManager::SOUND_FINISHED_FL, Util::emptyStringT,
	SettingsManager::BALLOON_FINISHED_FL, false, IDH_SETTINGS_NOTIFICATIONS_FINISHED_FL },
	{ N_("Main chat message received"), SettingsManager::SOUND_MAIN_CHAT, Util::emptyStringT,
	SettingsManager::BALLOON_MAIN_CHAT, false, IDH_SETTINGS_NOTIFICATIONS_MAIN_CHAT },
	{ N_("Private message received"), SettingsManager::SOUND_PM, Util::emptyStringT,
	SettingsManager::BALLOON_PM, false, IDH_SETTINGS_NOTIFICATIONS_PM },
	{ N_("Private message window opened"), SettingsManager::SOUND_PM_WINDOW, Util::emptyStringT,
	SettingsManager::BALLOON_PM_WINDOW, false, IDH_SETTINGS_NOTIFICATIONS_PM_WINDOW }
};

static const ColumnInfo columns[] = {
	{ "", 0, false },
	{ N_("Event"), 100, false },
	{ N_("Sound"), 100, false },
	{ N_("Balloon"), 100, false },
};

NotificationsPage::NotificationsPage(dwt::Widget* parent) :
PropPage(parent, 5, 1),
table(0),
sound(0),
balloon(0),
beepGroup(0),
beepFile(0),
prevSelection(-1)
{
	setHelpId(IDH_NOTIFICATIONSPAGE);

	grid->column(0).mode = GridInfo::FILL;
	grid->row(0).mode = GridInfo::FILL;
	grid->row(0).align = GridInfo::STRETCH;

	{
		auto cur = grid->addChild(GroupBox::Seed(T_("Notifications")))->addChild(Grid::Seed(2, 1));
		cur->column(0).mode = GridInfo::FILL;
		cur->row(0).mode = GridInfo::FILL;
		cur->row(0).align = GridInfo::STRETCH;
		cur->setSpacing(grid->getSpacing());

		auto seed = WinUtil::Seeds::Dialog::table;
		seed.style |= LVS_SINGLESEL;
		seed.lvStyle |= LVS_EX_SUBITEMIMAGES;
		table = cur->addChild(seed);

		const dwt::Point size(16, 16);
		dwt::ImageListPtr images(new dwt::ImageList(size));
		images->add(dwt::Icon(IDI_CANCEL, size));
		images->add(dwt::Icon(IDI_SOUND, size));
		images->add(dwt::Icon(IDI_BALLOON, size));
		table->setSmallImageList(images);

		cur = cur->addChild(Grid::Seed(1, 2));
		cur->column(0).mode = GridInfo::FILL;
		cur->column(0).align = GridInfo::BOTTOM_RIGHT;
		cur->column(1).align = GridInfo::BOTTOM_RIGHT;
		cur->setSpacing(grid->getSpacing());

		sound = cur->addChild(CheckBox::Seed(T_("Play a sound")));
		sound->setHelpId(IDH_SETTINGS_NOTIFICATIONS_SOUND);
		sound->onClicked([this] { handleSoundClicked(); });

		balloon = cur->addChild(CheckBox::Seed(T_("Display a balloon popup")));
		balloon->setHelpId(IDH_SETTINGS_NOTIFICATIONS_BALLOON);
		balloon->onClicked([this] { handleBalloonClicked(); });
	}

	{
		beepGroup = grid->addChild(GroupBox::Seed(T_("Notification sound")));
		beepGroup->setHelpId(IDH_SETTINGS_NOTIFICATIONS_BEEPFILE);

		auto cur = beepGroup->addChild(Grid::Seed(1, 3));
		cur->column(1).mode = GridInfo::FILL;

		auto button = cur->addChild(Button::Seed(T_("Play")));
		button->setImage(WinUtil::buttonIcon(IDI_SOUND));
		button->onClicked([this] { handlePlayClicked(); });

		beepFile = cur->addChild(WinUtil::Seeds::Dialog::textBox);

		cur->addChild(Button::Seed(T_("&Browse...")))->onClicked([this] { handleBrowseClicked(); });
	}

	{
		auto button = grid->addChild(Grid::Seed(1, 1))->addChild(Button::Seed(T_("Fire a balloon popup example")));
		button->setHelpId(IDH_SETTINGS_NOTIFICATIONS_BALLOON_EXAMPLE);
		button->setImage(WinUtil::buttonIcon(IDI_BALLOON));
		button->onClicked([this] { handleExampleClicked(); });
	}

	WinUtil::makeColumns(table, columns, COLUMN_LAST);

	for(size_t i = 0, n = sizeof(options) / sizeof(Option); i < n; ++i) {
		options[i].sound = Text::toT(SettingsManager::getInstance()->get((SettingsManager::StrSetting)options[i].soundSetting));
		options[i].balloon = SettingsManager::getInstance()->getBool((SettingsManager::IntSetting)options[i].balloonSetting);

		TStringList row(COLUMN_LAST);
		row[COLUMN_TEXT] = T_(options[i].text);
		table->insert(row);

		update(i, COLUMN_SOUND, !options[i].sound.empty());
		update(i, COLUMN_BALLOON, options[i].balloon);
	}

	handleSelectionChanged();

	table->onSelectionChanged([this] { handleSelectionChanged(); });
	table->onDblClicked([this] { handleDblClicked(); });

	table->onHelp([this](Widget*, unsigned id) { handleTableHelp(id); });
	table->setHelpId([this](unsigned id) { handleTableHelpId(id); });
}

NotificationsPage::~NotificationsPage() {
}

void NotificationsPage::layout() {
	PropPage::layout();

	table->setColumnWidth(COLUMN_TEXT, table->getWindowSize().x - 220);
}

void NotificationsPage::write() {
	table->clearSelection(); // to save the current sound file

	SettingsManager* settings = SettingsManager::getInstance();
	for(size_t i = 0, n = sizeof(options) / sizeof(Option); i < n; ++i) {
		settings->set((SettingsManager::StrSetting)options[i].soundSetting, Text::fromT(options[i].sound));
		settings->set((SettingsManager::IntSetting)options[i].balloonSetting, options[i].balloon);
	}
}

void NotificationsPage::handleTableHelp(unsigned id) {
	// same as PropPage::handleListHelp
	int item =
		isKeyPressed(VK_F1) ? table->getSelected() :
		table->hitTest(dwt::ScreenCoordinate(dwt::Point::fromLParam(::GetMessagePos()))).first;
	if(item >= 0 && options[item].helpId)
		id = options[item].helpId;
	WinUtil::help(table, id);
}

void NotificationsPage::handleTableHelpId(unsigned& id) {
	// same as PropPage::handleListHelpId
	int item = table->getSelected();
	if(item >= 0 && options[item].helpId)
		id = options[item].helpId;
}

void NotificationsPage::handleSelectionChanged() {
	if(prevSelection >= 0) {
		auto& s = options[prevSelection].sound;
		if(s == _T("beep") && beepFile->length() > 0)
			s = beepFile->getText();
	}

	int sel = table->getSelected();
	if(sel >= 0) {
		auto& s = options[sel].sound;
		beepGroup->setEnabled(!s.empty());
		beepFile->setText((s == _T("beep")) ? Util::emptyStringT : s);

		sound->setChecked(!s.empty());
		sound->setEnabled(true);

		balloon->setChecked(options[sel].balloon);
		balloon->setEnabled(true);

	} else {
		beepGroup->setEnabled(false);
		beepFile->setText(Util::emptyStringT);

		sound->setChecked(false);
		sound->setEnabled(false);

		balloon->setChecked(false);
		balloon->setEnabled(false);
	}

	prevSelection = sel;
}

void NotificationsPage::handleDblClicked() {
	auto item = table->hitTest(dwt::ScreenCoordinate(dwt::Point::fromLParam(::GetMessagePos())));
	if(item.first == -1 || item.second == -1) {
		return;
	}

	switch(item.second) {
	case COLUMN_SOUND: handleSoundClicked(); break;
	case COLUMN_BALLOON: handleBalloonClicked(); break;
	}
}

void NotificationsPage::handleSoundClicked() {
	auto sel = table->getSelected();
	if(sel >= 0) {
		auto& s = options[sel].sound;
		if(s.empty())
			s = _T("beep");
		else
			s.clear();
		update(sel, COLUMN_SOUND, !s.empty());
		handleSelectionChanged();
	}
}

void NotificationsPage::handleBalloonClicked() {
	auto sel = table->getSelected();
	if(sel >= 0) {
		auto& b = options[sel].balloon;
		b = !b;
		update(sel, COLUMN_BALLOON, b);
		handleSelectionChanged();
	}
}

void NotificationsPage::handlePlayClicked() {
	auto s = beepFile->getText();
	WinUtil::playSound(s.empty() ? _T("beep") : s);
}

void NotificationsPage::handleBrowseClicked() {
	tstring x = beepFile->getText();
	if(LoadDialog(this).open(x)) {
		beepFile->setText(x);
	}
}

void NotificationsPage::handleExampleClicked() {
	WinUtil::mainWindow->notify(T_("Balloon popup example"), T_("This is an example of a balloon popup notification!"));
}

void NotificationsPage::update(size_t row, size_t column, bool enable) {
	table->setIcon(row, column, enable ? (column - COLUMN_TEXT) : 0);
	table->setText(row, column, enable ? T_("Yes") : T_("No"));
}