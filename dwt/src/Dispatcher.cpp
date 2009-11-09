/*
  DC++ Widget Toolkit

  Copyright (c) 2007-2009, Jacek Sieka

  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met:

      * Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright notice,
        this list of conditions and the following disclaimer in the documentation
        and/or other materials provided with the distribution.
      * Neither the name of the DWT nor SmartWin++ nor the names of its contributors
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

#include <dwt/Dispatcher.h>
#include <dwt/util/check.h>
#include <dwt/Widget.h>
#include <dwt/DWTException.h>

namespace dwt {

LRESULT CALLBACK WindowProc::initProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	MSG msg = { hwnd, uMsg, wParam, lParam };

	Widget* w = getInitWidget(msg);
	if(w) {
		w->setHandle(hwnd);

		return wndProc(hwnd, uMsg, wParam, lParam);
	}

	return returnUnknown(msg);
}

LRESULT CALLBACK WindowProc::wndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	MSG msg = { hwnd, uMsg, wParam, lParam };

	// We dispatch certain messages back to the child widget, so that their
	// potential callbacks will die along with the child when the time comes
	HWND handler = getHandler(msg);

	// Try to get the this pointer
	Widget* w = hwnd_cast<Widget*>(handler);

	if(w) {
		if(uMsg == WM_NCDESTROY) {
			w->kill();
			return returnUnknown(msg);
		}

		LRESULT res = 0;
		if(w->handleMessage(msg, res)) {
			return res;
		}
	}

	if(handler != hwnd) {
		w = hwnd_cast<Widget*>(hwnd);
	}

	// If this fails there's something wrong
	dwtassert(w, "Expected to get a pointer to a widget - something's wrong");

	if(!w) {
		return returnUnknown(msg);
	}

	return w->getDispatcher().chain(msg);
}

Widget* WindowProc::getInitWidget(const MSG& msg) {
	if(msg.message == WM_NCCREATE) {
		return reinterpret_cast<Widget*>(reinterpret_cast<CREATESTRUCT*>(msg.lParam)->lpCreateParams);
	}
	return 0;
}

LRESULT WindowProc::returnUnknown(const MSG& msg) {
	return ::DefWindowProc(msg.hwnd, msg.message, msg.wParam, msg.lParam);
}

HWND WindowProc::getHandler(const MSG& msg) {
	HWND handler;

	// Check who should handle the message - parent or child
	switch(msg.message) {
	case WM_CTLCOLORBTN:
	case WM_CTLCOLORDLG:
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORLISTBOX:
	case WM_CTLCOLORMSGBOX:
	case WM_CTLCOLORSCROLLBAR:
	case WM_CTLCOLORSTATIC:
		{
			handler = reinterpret_cast<HWND>(msg.lParam);
			break;
		}

	case WM_NOTIFY :
		{
			NMHDR* nmhdr = reinterpret_cast<NMHDR*>(msg.lParam);
			handler = nmhdr->hwndFrom;
			break;
		}

	case WM_COMMAND:
		{
			if(msg.lParam != 0) {
				handler = reinterpret_cast<HWND>(msg.lParam);
			} else {
				handler = msg.hwnd;
			}
			break;
		}

	default:
		{
			// By default, widgets handle their own messages
			handler = msg.hwnd;
			break;
		}
	}

	return handler;
}

#ifdef PORT_ME /// @todo for MDI, make MDIChildProc derive from WindowProc and MDIFrameDispatcher from Dispatcher

LRESULT MDIChildProc::returnUnknown(const MSG& msg) {
	return ::DefMDIChildProc(msg.hwnd, msg.message, msg.wParam, msg.lParam);
}

Widget* MDIChildProc::getInitWidget(const MSG& msg) {
	if(msg.message == WM_NCCREATE) {
		return reinterpret_cast<Widget*>(reinterpret_cast<MDICREATESTRUCT*>(
			reinterpret_cast<CREATESTRUCT*>(msg.lParam)->lpCreateParams)->lParam);
	}
}

LRESULT MDIFrameDispatcher::chain(const MSG& msg) {
	if(getMDIParent()) {
		return ::DefFrameProc(hWnd, getMDIParent()->handle(), msg, wPar, lPar);
	}
	return NormalDispatcher::chain(msg);
}

#endif

Dispatcher::Dispatcher(WNDCLASSEX& cls) : atom(0) {
	atom = ::RegisterClassEx(&cls);
	if(!atom) {
		throw Win32Exception("Unable to register class");
	}
}

Dispatcher::~Dispatcher() {
	if(getClassName()) {
		::UnregisterClass(getClassName(), ::GetModuleHandle(NULL));
	}
}

HCURSOR Dispatcher::getDefaultCursor() {
	static HCURSOR cursor(::LoadCursor(0, IDC_ARROW));
	return cursor;
}

HBRUSH Dispatcher::getDefaultBackground() {
	static HBRUSH background(reinterpret_cast<HBRUSH>(COLOR_3DFACE + 1));
	return background;
}

WNDCLASSEX Dispatcher::makeWndClass(LPCTSTR className) {
	WNDCLASSEX cls = { sizeof(WNDCLASSEX) };
	fillWndClass(cls, className);
	return cls;
}

void Dispatcher::fillWndClass(WNDCLASSEX& cls, LPCTSTR className) {
	cls.style = CS_DBLCLKS;
	cls.lpfnWndProc = WindowProc::initProc;
	cls.hInstance = ::GetModuleHandle(NULL);
	cls.hCursor = getDefaultCursor();
	cls.hbrBackground = getDefaultBackground();
	cls.lpszMenuName = 0;
	cls.lpszClassName = className;
}

NormalDispatcher::NormalDispatcher(WNDCLASSEX& cls) :
Dispatcher(cls)
{ }

Dispatcher& NormalDispatcher::getDefault() {
	WNDCLASSEX cls = makeWndClass(className<NormalDispatcher>().c_str());
	static NormalDispatcher dispatcher(cls);
	return dispatcher;
}

LRESULT NormalDispatcher::chain(const MSG& msg) {
	return ::DefWindowProc(msg.hwnd, msg.message, msg.wParam, msg.lParam);
}

ChainingDispatcher::ChainingDispatcher(WNDCLASSEX& cls, WNDPROC wndProc_) :
Dispatcher(cls),
wndProc(wndProc_)
{ }

LRESULT ChainingDispatcher::chain(const MSG& msg) {
	return ::CallWindowProc(wndProc, msg.hwnd, msg.message, msg.wParam, msg.lParam);
}

std::auto_ptr<Dispatcher> ChainingDispatcher::superClass(LPCTSTR original, LPCTSTR newName) {
	WNDCLASSEX orgClass = { sizeof(WNDCLASSEX) };

	if(!::GetClassInfoEx(::GetModuleHandle(NULL), original, &orgClass)) {
		throw Win32Exception("Unable to find information for class");
	}

	WNDPROC proc = orgClass.lpfnWndProc;

	fillWndClass(orgClass, newName);

	return std::auto_ptr<Dispatcher>(new ChainingDispatcher(orgClass, proc));
}

}
