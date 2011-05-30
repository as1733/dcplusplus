/*
  DC++ Widget Toolkit

  Copyright (c) 2007-2011, Jacek Sieka

  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met:

      * Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright notice,
        this list of conditions and the following disclaimer in the documentation
        and/or other materials provided with the distribution.
      * Neither the name of the DWT nor the names of its contributors
        may be used to endorse or promote products derived from this software
        without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <dwt/widgets/Notification.h>

#include <dwt/Application.h>
#include <dwt/util/win32/Version.h>

namespace dwt {

const UINT Notification::message = ::RegisterWindowMessage(_T("dwt::Notification"));

static const UINT taskbarMsg = ::RegisterWindowMessage(_T("TaskbarCreated"));

/* the following dance adds the hBalloonIcon member to NOTIFYICONDATA without requiring a global
switch of WINVER / _WIN32_WINNT / etc to Vista values. */
typedef NOTIFYICONDATA legacyNOTIFYICONDATA;
#if(_WIN32_WINNT < 0x600)
struct NOTIFYICONDATA_ : NOTIFYICONDATA {
	HICON hBalloonIcon;
	NOTIFYICONDATA_(const NOTIFYICONDATA& nid) : NOTIFYICONDATA(nid), hBalloonIcon(0) { }
};
#define NOTIFYICONDATA NOTIFYICONDATA_
#define NIF_SHOWTIP 0x80
#endif

legacyNOTIFYICONDATA Notification::makeNID() const {
	bool vista = util::win32::ensureVersion(util::win32::VISTA);
	legacyNOTIFYICONDATA nid = { vista ? sizeof(NOTIFYICONDATA) : sizeof(legacyNOTIFYICONDATA), parent->handle() };
	if(vista)
		nid.uFlags |= NIF_SHOWTIP;
	return nid;
}

Notification::Notification(WindowPtr parent_) :
parent(parent_),
visible(false),
onlyBalloons(false),
lastTick(0)
{
}

Notification::~Notification() {
	setVisible(false);
}

void Notification::create(const Notification::Seed& seed) {
	icon = seed.icon;
	tip = seed.tip;

	// TODO Allow more than one icon per window
	parent->setCallback(Message(message), [this](const MSG& msg, LRESULT&) { return trayHandler(msg); });
	parent->setCallback(Message(taskbarMsg), [this](const MSG&, LRESULT&) { return redisplay(); });
}

void Notification::setIcon(const IconPtr& icon_) {
	icon = icon_;
	redisplay();
}

void Notification::setVisible(bool visible_) {
	if(visible == visible_) {
		if(visible && onlyBalloons)
			onlyBalloons = false;
		return;
	}

	visible = visible_;

	NOTIFYICONDATA nid = makeNID();

	if(visible) {
		nid.uFlags |= NIF_MESSAGE;
		nid.uCallbackMessage = message;

		if(!tip.empty()) {
			nid.uFlags |= NIF_TIP;
			tip.copy(nid.szTip, sizeof(nid.szTip) / sizeof(nid.szTip[0]) - 1);
		}

		if(icon) {
			nid.uFlags |= NIF_ICON;
			nid.hIcon = icon->handle();
		}

		::Shell_NotifyIcon(NIM_ADD, &nid);

	} else {
		::Shell_NotifyIcon(NIM_DELETE, &nid);
	}
}

void Notification::setTooltip(const tstring& tip_) {
	tip = tip_;
	lastTick = ::GetTickCount();

	if(visible) {
		NOTIFYICONDATA nid = makeNID();
		nid.uFlags |= NIF_TIP;
		tip.copy(nid.szTip, sizeof(nid.szTip) / sizeof(nid.szTip[0]) - 1);
		::Shell_NotifyIcon(NIM_MODIFY, &nid);
	}
}

void Notification::addMessage(const tstring& title, const tstring& message, const Callback& callback, const IconPtr& balloonIcon) {
	if(!visible) {
		setVisible(true);
		onlyBalloons = true;
	}

	balloons.push_back(std::make_pair(callback, balloonIcon));

	NOTIFYICONDATA nid = makeNID();
	nid.uFlags |= NIF_INFO;

	message.copy(nid.szInfo, sizeof(nid.szInfo) / sizeof(nid.szInfo[0]) - 1);

	title.copy(nid.szInfoTitle, sizeof(nid.szInfoTitle) / sizeof(nid.szInfoTitle[0]) - 1);

	if(balloonIcon && util::win32::ensureVersion(util::win32::VISTA)) {
		nid.dwInfoFlags = NIIF_USER;
		nid.hBalloonIcon = balloonIcon->handle();
	} else {
		nid.dwInfoFlags = NIIF_INFO;
	}

	::Shell_NotifyIcon(NIM_MODIFY, &nid);
}

bool Notification::redisplay() {
	if(visible) {
		setVisible(false);
		setVisible(true);
	}
	return false;
}

bool Notification::trayHandler(const MSG& msg) {
	switch(LOWORD(msg.lParam)) {

	case WM_LBUTTONUP:
		{
			if(iconClicked) {
				iconClicked();
			}
			break;
		}

	case WM_RBUTTONUP:
		{
			if(contextMenu) {
				// Work-around for windows bug (KB135788)
				::SetForegroundWindow(parent->handle());
				contextMenu();
				parent->postMessage(WM_NULL);
			}
			break;
		}

	case WM_MOUSEMOVE:
		{
			if(updateTip) {
				DWORD now = ::GetTickCount();
				if(now - 1000 > lastTick) {
					updateTip();
					lastTick = now;
				}
			}
			break;
		}

	case NIN_BALLOONUSERCLICK:
		{
			balloons.front().first();
		} // fall through
	case NIN_BALLOONHIDE: // fall through
	case NIN_BALLOONTIMEOUT:
		{
			balloons.pop_front();
			if(onlyBalloons && balloons.empty()) {
				setVisible(false);
			}
			break;
		}
	}

	return true;
}

}
