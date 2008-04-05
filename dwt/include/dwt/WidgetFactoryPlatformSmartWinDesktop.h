/*
  DC++ Widget Toolkit

  Copyright (c) 2007-2008, Jacek Sieka

  SmartWin++

  Copyright (c) 2005 Thomas Hansen

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

#ifndef WINCE
#ifndef DWT_WidgetFactoryPlatformSmartWinDesktop_h
#define DWT_WidgetFactoryPlatformSmartWinDesktop_h

#include "WidgetFactoryPlatformCommon.h"
#include "widgets/FontDialog.h"
#include "widgets/WidgetMenu.h"
#include "WidgetCreator.h"

namespace dwt {

/// Class for WidgetFactoryCode which only exists in Desktop version of Windows API
/** Desktop version dependant functions which does not exist in Windows CE version of
  * Windows API will be here
  */
template< typename ContainerWidgetType >
class WidgetFactoryPlatformImplementation< ContainerWidgetType,  dwtDesktop >
	: public ContainerWidgetType
{
public:
	/// Menu class type.
	typedef dwt::WidgetMenu WidgetMenu;

	/// Menu object type.
	typedef typename WidgetMenu::ObjectType WidgetMenuPtr;

	/// ChooseFont class and object type.
	typedef dwt::FontDialog FontDialog;

	/// Constructor taking a pointer to it's parent.
	/** If you for instance create a WidgetChildWindow then use this Constructor
	  * since it explicitly sets the parent of the Widget.
	  */
	WidgetFactoryPlatformImplementation( Widget * parent )
		: ContainerWidgetType( parent )
	{}

	/// Creates a FontDialog and returns it.
	/** Usable to let user choose font from the system installed fonts.
	  */
	FontDialog createFontDialog() {
		return FontDialog( this );
	}

	/// Creates a Menu
	/** The returned object is of type std::tr1::shared_ptr< WidgetMenu >, but
	  * you should use the typedef WidgetMenuPtr and not <br>
	  * the shared_ptr itself since this may change in future releases.
	  */
	WidgetMenuPtr createMenu(const typename WidgetMenu::Seed& cs = WidgetMenu::Seed())
	{
		return WidgetCreator< WidgetMenu >::create( this, cs );
	}

	WidgetMenuPtr attachMenu(HMENU hMenu, const typename WidgetMenu::Seed& cs = WidgetMenu::Seed())
	{
		return WidgetCreator< WidgetMenu >::attach( this, cs, hMenu );
	}
};

}

#endif //! WidgetFactoryPlatformSmartWinDesktop_h
#endif //! WINCE
