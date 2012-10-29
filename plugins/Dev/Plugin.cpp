/*
 * Copyright (C) 2012 Jacek Sieka, arnetheduck on gmail point com
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
#include "Plugin.h"
#include "Util.h"

Plugin* Plugin::instance = nullptr;

const char* switchText = "Dev plugin: enable/disable";

Plugin::Plugin() {
}

Plugin::~Plugin() {
	clearHooks();

	if(ui) {
		ui->remove_command(switchText);
	}
}

Bool DCAPI Plugin::main(PluginState state, DCCorePtr core, dcptr_t) {
	switch(state) {
	case ON_INSTALL:
	case ON_LOAD:
		{
			Bool res = True;
			instance = new Plugin();
			instance->onLoad(core, (state == ON_INSTALL), res);
			return res;
		}

	case ON_UNINSTALL:
	case ON_UNLOAD:
		{
			delete instance;
			instance = nullptr;
			return True;
		}

	default:
		{
			return False;
		}
	}
}

void Plugin::dlgClosed() {
	instance->close();
}

void Plugin::addHooks() {
	events[HOOK_NETWORK_HUB_IN] = hooks->bind_hook(HOOK_NETWORK_HUB_IN, [](dcptr_t pObject, dcptr_t pData, dcptr_t, Bool*) {
		return instance->onHubDataIn(reinterpret_cast<HubDataPtr>(pObject), reinterpret_cast<char*>(pData)); }, nullptr);
	events[HOOK_NETWORK_HUB_OUT] = hooks->bind_hook(HOOK_NETWORK_HUB_OUT, [](dcptr_t pObject, dcptr_t pData, dcptr_t, Bool*) {
		return instance->onHubDataOut(reinterpret_cast<HubDataPtr>(pObject), reinterpret_cast<char*>(pData)); }, nullptr);
	events[HOOK_NETWORK_CONN_IN] = hooks->bind_hook(HOOK_NETWORK_CONN_IN, [](dcptr_t pObject, dcptr_t pData, dcptr_t, Bool*) {
		return instance->onConnectionDataIn(reinterpret_cast<ConnectionDataPtr>(pObject), reinterpret_cast<char*>(pData)); }, nullptr);
	events[HOOK_NETWORK_CONN_OUT] = hooks->bind_hook(HOOK_NETWORK_CONN_OUT, [](dcptr_t pObject, dcptr_t pData, dcptr_t, Bool*) {
		return instance->onConnectionDataOut(reinterpret_cast<ConnectionDataPtr>(pObject), reinterpret_cast<char*>(pData)); }, nullptr);
}

void Plugin::clearHooks() {
	for(auto& i: events)
		hooks->release_hook(i.second);
	events.clear();
}

void Plugin::start() {
	dialog.create();
	addHooks();
}

void Plugin::close() {
	clearHooks();
	dialog.close();
}

void Plugin::onLoad(DCCorePtr core, bool install, Bool& loadRes) {
	dcpp = core;
	hooks = reinterpret_cast<DCHooksPtr>(core->query_interface(DCINTF_HOOKS, DCINTF_HOOKS_VER));

	auto utils = reinterpret_cast<DCUtilsPtr>(core->query_interface(DCINTF_DCPP_UTILS, DCINTF_DCPP_UTILS_VER));
	auto config = reinterpret_cast<DCConfigPtr>(core->query_interface(DCINTF_CONFIG, DCINTF_CONFIG_VER));
	auto logger = reinterpret_cast<DCLogPtr>(core->query_interface(DCINTF_LOGGING, DCINTF_LOGGING_VER));
	ui = reinterpret_cast<DCUIPtr>(core->query_interface(DCINTF_DCPP_UI, DCINTF_DCPP_UI_VER));

	if(!utils || !config || !logger || !ui) {
		loadRes = False;
		return;
	}

	Util::initialize(core->host_name(), utils, config, logger);

	if(install) {
		/// @todo config enabled/disabled

		Util::logMessage("The dev plugin has been installed; check the \"" + string(switchText) + "\" command.");
	}

	start();
	ui->add_command(switchText, [] { instance->onSwitched(); });
}

void Plugin::onSwitched() {
	if(events.empty()) {
		start();
	} else {
		close();
	}
}

Bool Plugin::onHubDataIn(HubDataPtr hHub, const char* message) {
	dialog.write(true, false, hHub->ip, "Hub " + string(hHub->url), message);
	return False;
}

Bool Plugin::onHubDataOut(HubDataPtr hHub, const char* message) {
	dialog.write(true, true, hHub->ip, "Hub " + string(hHub->url), message);
	return False;
}

Bool Plugin::onConnectionDataIn(ConnectionDataPtr hConn, const char* message) {
	dialog.write(false, false, hConn->ip, "User" /** @todo get user's nick */, message);
	return False;
}

Bool Plugin::onConnectionDataOut(ConnectionDataPtr hConn, const char* message) {
	dialog.write(false, true, hConn->ip, "User" /** @todo get user's nick */, message);
	return False;
}
