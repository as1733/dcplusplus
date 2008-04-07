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

#include <dwt/widgets/WidgetMenu.h>

#include <dwt/resources/Brush.h>
#include <dwt/resources/Pen.h>

namespace dwt {

const int WidgetMenu::borderGap = 3;
const int WidgetMenu::pointerGap = 5;
const int WidgetMenu::textIconGap = 8;
const int WidgetMenu::textBorderGap = 4;
const int WidgetMenu::separatorHeight = 8;
const int WidgetMenu::minSysMenuItemWidth = 130;
Point WidgetMenu::defaultImageSize = Point( 16, 16 );

WidgetMenu::Seed::Seed(bool ownerDrawn_, const MenuColorInfo& colorInfo_, FontPtr font_) :
popup(true),
ownerDrawn(ownerDrawn_),
colorInfo(colorInfo_),
font(font_)
{
}

WidgetMenu::WidgetMenu( dwt::Widget* parent ) :
isSysMenu(false),
itsParent(parent),
drawSidebar(false)
{
	xAssert(itsParent != NULL, _T("A WidgetMenu must have a parent"));
}

void WidgetMenu::createHelper(const Seed& cs) {
	// save settings provided through the Seed
	ownerDrawn = cs.ownerDrawn;
	itsColorInfo = cs.colorInfo;

	if(ownerDrawn) {
		if(cs.font)
			font = cs.font;
		else
			font = new Font(DefaultGuiFont);

		{
			LOGFONT lf;
			::GetObject(font->handle(), sizeof(lf), &lf);
			lf.lfWeight = FW_BOLD;
			itsTitleFont = FontPtr(new Font(::CreateFontIndirect(&lf), true));
		}

		// set default drawing
		itsParent->setCallback(Message(WM_DRAWITEM), DrawItemDispatcher(std::tr1::bind(&WidgetMenu::handleDrawItem, this, _1, _2)));
		itsParent->setCallback(Message(WM_MEASUREITEM), MeasureItemDispatcher(std::tr1::bind(&WidgetMenu::handleMeasureItem, this, _1)));
	}
}

void WidgetMenu::create(const Seed& cs) {
	createHelper(cs);

	if(cs.popup)
		itsHandle = ::CreatePopupMenu();
	else
		itsHandle = ::CreateMenu();
	if ( !itsHandle )
	{
		xCeption x( _T( "CreateMenu in WidgetMenu::create fizzled..." ) );
		throw x;
	}
}

void WidgetMenu::attach(HMENU hMenu, const Seed& cs) {
	createHelper(cs);

	itsHandle = hMenu;

	if(ownerDrawn) {
		// update all current items to be owner-drawn
		// @todo update sub-menus too...
		const int count = getCount();
		for(int i = 0; i < count; ++i) {
			// init structure for items
			MENUITEMINFO info;
			memset( & info, 0, sizeof( info ) );
			info.cbSize = sizeof( MENUITEMINFO );

			// set flags
			info.fMask = MIIM_FTYPE | MIIM_DATA;

			if(::GetMenuItemInfo(itsHandle, i, TRUE, &info)) {
				info.fMask |= MIIM_DATA;
				info.fType |= MFT_OWNERDRAW;

				// create item data wrapper
				ItemDataWrapper * wrapper = new ItemDataWrapper( this, i, MenuItemDataPtr( new MenuItemData() ) );
				info.dwItemData = reinterpret_cast< ULONG_PTR >( wrapper );

				if(::SetMenuItemInfo(itsHandle, i, TRUE, &info))
					itsItemData.push_back( wrapper );
				else
					throw xCeption( _T( "SetMenuItemInfo in WidgetMenu::attach fizzled..." ) );
			} else
				throw xCeption( _T( "GetMenuItemInfo in WidgetMenu::attach fizzled..." ) );
		}
	}
}

void WidgetMenu::setMenu() {
	addCommands();
	if ( ::SetMenu( itsParent->handle(), itsHandle ) == FALSE )
		throw xCeption( _T( "SetMenu in WidgetMenu::setMenu fizzled..." ) );
}

WidgetMenu::ObjectType WidgetMenu::appendPopup( const SmartUtil::tstring & text, MenuItemDataPtr itemData )
{
	// create popup menu pointer
	ObjectType retVal ( new WidgetMenu(itsParent) );
	retVal->create( Seed(ownerDrawn, itsColorInfo, font) );

	// init structure for new item
	MENUITEMINFO info;
	memset( & info, 0, sizeof( info ) );
	info.cbSize = sizeof( MENUITEMINFO );

	// set flags
	info.fMask = MIIM_SUBMENU | MIIM_CHECKMARKS | MIIM_STRING;

	// set item text
	info.dwTypeData = const_cast< LPTSTR >( text.c_str() );

	// set sub menu
	info.hSubMenu = retVal->handle();

	// get position to insert
	int position = getCount();

	ItemDataWrapper * wrapper = NULL;
	if(ownerDrawn) {
		info.fMask |= MIIM_DATA | MIIM_FTYPE;

		info.fType = MFT_OWNERDRAW;

		// create item data
		wrapper = new ItemDataWrapper( this, position, itemData );
		info.dwItemData = reinterpret_cast< ULONG_PTR >( wrapper );
	}

	// append to this menu at the end
	if ( ::InsertMenuItem( itsHandle, position, TRUE, & info ) )
	{
		if(ownerDrawn)
			itsItemData.push_back( wrapper );
		itsChildren.push_back( retVal );
	}
	return retVal;
}

#ifdef PORT_ME
WidgetMenu::ObjectType WidgetMenu::getSystemMenu()
{
	// get system menu for the utmost parent
	HMENU handle = ::GetSystemMenu( itsParent->handle(), FALSE );

	// create pointer to system menu
	ObjectType sysMenu( new WidgetMenu( itsParent->handle() ) );

	// create(take) system menu
	sysMenu->isSysMenu = true;
	sysMenu->create( handle, false );

	// We're assuming that the system menu has the same lifespan as the "this" menu, we must keep a reference to te system menu
	// otherwise it will be "lost", therefore we add it up as a child to the "this" menu...
	itsChildren.push_back( sysMenu );

	return sysMenu;
}
#endif

void WidgetMenu::addCommands() {
	for(CallbackMap::iterator i = callbacks.begin(); i != callbacks.end(); ++i) {
		itsParent->setCallback(Message(WM_COMMAND, i->first), i->second);
	}
	for(std::vector< ObjectType >::iterator i = itsChildren.begin(); i != itsChildren.end(); ++i) {
		(*i)->addCommands();
	}
}

int WidgetMenu::getItemIndex( unsigned int id )
{
	int index = 0;
	const int itemCount = getCount();

	for ( index = 0; index < itemCount; ++index )
		if ( ::GetMenuItemID( itsHandle, index ) == id ) // exit the loop if found
			return index;

	return - 1;
}

MenuItemDataPtr WidgetMenu::getData( int itemIndex )
{
	for(size_t i = 0; i < itsItemData.size(); ++i)
		if(itsItemData[i]->index == itemIndex)
			return itsItemData[i]->data;

	return MenuItemDataPtr();
}

WidgetMenu::~WidgetMenu()
{
	// Destroy this menu
	::DestroyMenu( handle() );
	std::for_each( itsItemData.begin(), itsItemData.end(), destroyItemDataWrapper );
}

void WidgetMenu::destroyItemDataWrapper( ItemDataWrapper * wrapper )
{
	if ( 0 != wrapper )
		delete wrapper;

	wrapper = 0;
}

void WidgetMenu::setTitleFont( FontPtr font )
{
	itsTitleFont = font;
	setTitle( itsTitle, drawSidebar ); // Easy for now, should be refactored...
}

void WidgetMenu::clearTitle( bool clearSidebar /* = false */)
{
	if(!ownerDrawn)
		return;

	if ( !clearSidebar && !itsTitle.empty() )
		removeItem( 0 );

	// clear title text
	itsTitle.clear();
}

void WidgetMenu::checkItem( unsigned id, bool value )
{
	::CheckMenuItem( handle(), id, value ? MF_CHECKED : MF_UNCHECKED );
}

void WidgetMenu::setItemEnabled( unsigned id, bool byPosition, bool value )
{
	if ( ::EnableMenuItem( handle(), id, (byPosition ? MF_BYPOSITION : MF_BYCOMMAND) | (value ? MF_ENABLED : MF_GRAYED) ) == - 1 )
	{
		xCeption x( _T( "Couldn't enable/disable the menu item, item doesn't exist" ) );
		throw x;
	}
}

UINT WidgetMenu::getMenuState( UINT id, bool byPosition )
{
	return ::GetMenuState(handle(), id, byPosition ? MF_BYPOSITION : MF_BYCOMMAND); 
}

bool WidgetMenu::isSeparator( UINT id, bool byPosition )
{
	return (getMenuState(id, byPosition) & MF_SEPARATOR) == MF_SEPARATOR; 
}

bool WidgetMenu::isChecked( UINT id, bool byPosition )
{
	return (getMenuState(id, byPosition) & MF_CHECKED) == MF_CHECKED; 
}

bool WidgetMenu::isPopup( UINT id, bool byPosition )
{
	return (getMenuState(id, byPosition) & MF_POPUP) == MF_POPUP; 
}

bool WidgetMenu::isEnabled( UINT id, bool byPosition )
{
	return !(getMenuState(id, byPosition) & (MF_DISABLED | MF_GRAYED)); 
}

void WidgetMenu::setDefaultItem( UINT id, bool byPosition )
{
	::SetMenuDefaultItem(handle(), id, byPosition);
}

SmartUtil::tstring WidgetMenu::getText( unsigned id, bool byPosition )
{
	MENUITEMINFO mi = { sizeof(MENUITEMINFO) };

	// set flag
	mi.fMask = MIIM_STRING;

	if ( ::GetMenuItemInfo( itsHandle, id, byPosition, & mi ) == FALSE )
		throw xCeption( _T( "Couldn't get item info in WidgetMenu::getText" ) );

	boost::scoped_array< TCHAR > buffer( new TCHAR[++mi.cch] );
	mi.dwTypeData = buffer.get();
	if ( ::GetMenuItemInfo( itsHandle, id, byPosition, & mi ) == FALSE )
		throw xCeption( _T( "Couldn't get item info in WidgetMenu::getText" ) );
	return mi.dwTypeData;
}

void WidgetMenu::setText( unsigned id, const SmartUtil::tstring& text )
{
	MENUITEMINFO mi = { sizeof(MENUITEMINFO) };

	// set flag
	mi.fMask = MIIM_STRING;
	mi.dwTypeData = (TCHAR*) text.c_str();

	if ( ::SetMenuItemInfo( itsHandle, id, FALSE, & mi ) == FALSE )
		throw xCeption( _T( "Couldn't set item info in WidgetMenu::setText" ) );
}

void WidgetMenu::setTitle( const SmartUtil::tstring & title, bool drawSidebar /* = false */)
{
	if(!ownerDrawn)
		return;

	this->drawSidebar = drawSidebar;
	const bool hasTitle = !itsTitle.empty();

	// set its title
	itsTitle = title;

	if ( !drawSidebar )
	{
		// init struct for title info
		MENUITEMINFO info;
		memset( & info, 0, sizeof( MENUITEMINFO ) );
		info.cbSize = sizeof( MENUITEMINFO );

		// set flags
		info.fMask = MIIM_STATE | MIIM_STRING | MIIM_FTYPE | MIIM_DATA;
		info.fType = MFT_OWNERDRAW;
		info.fState = MF_DISABLED;

		// set title text
		info.dwTypeData = const_cast< LPTSTR >( title.c_str() );

		// created info for title item
		ItemDataWrapper * wrapper = new ItemDataWrapper( this, 0, MenuItemDataPtr( new MenuItemData() ), true );

		// set item data
		info.dwItemData = reinterpret_cast< ULONG_PTR >( wrapper );

		if ( ( !hasTitle && ::InsertMenuItem( itsHandle, 0, TRUE, & info ) ) ||
			( hasTitle && ::SetMenuItemInfo( itsHandle, 0, TRUE, & info ) ) )
		{
			// adjust item data wrappers for all existing items
			for(size_t i = 0; i < itsItemData.size(); ++i)
				if(itsItemData[i])
					++itsItemData[i]->index;

			// push back title
			itsItemData.push_back( wrapper );
		}
	}
}

bool WidgetMenu::handleDrawItem(int id, LPDRAWITEMSTRUCT drawInfo) {
	if ( ( id != 0 ) || ( drawInfo->CtlType != ODT_MENU ) ) // if not intended for us
		return false;

	// get item data wrapper
	ItemDataWrapper * wrapper = reinterpret_cast< ItemDataWrapper * >( drawInfo->itemData );
	xAssert( wrapper != 0, _T( "Unsupported menu item in drawItem()" ) );

	// if processing menu bar
	const bool isMenuBar = ::GetMenu( wrapper->menu->getParent()->handle() ) == wrapper->menu->handle();

	// init struct for menu item info
	MENUITEMINFO info;
	memset( & info, 0, sizeof( MENUITEMINFO ) );
	info.cbSize = sizeof( MENUITEMINFO );

	// set flags
	info.fMask = MIIM_CHECKMARKS | MIIM_FTYPE | MIIM_DATA | MIIM_STATE | MIIM_STRING;

	if ( ::GetMenuItemInfo( wrapper->menu->handle(), wrapper->index, TRUE, & info ) == FALSE )
		throw xCeption ( _T( "Couldn't get menu item info in drawItem()" ) );

	// check if item is owner drawn
	xAssert( ( info.fType & MFT_OWNERDRAW ) != 0, _T( "Not a owner - drawn item in drawItem()" ) );

	// get item data
	MenuItemDataPtr data ( wrapper->data );
	xAssert( data != 0, _T( "Couldn't find item data in drawItem()" ) );

	// get state info
	bool isGrayed = ( drawInfo->itemState & ODS_GRAYED ) == ODS_GRAYED;
	bool isChecked = ( drawInfo->itemState & ODS_CHECKED ) == ODS_CHECKED;
	bool isDisabled = ( drawInfo->itemState & ODS_DISABLED ) == ODS_DISABLED;
	bool isSelected = ( drawInfo->itemState & ODS_SELECTED ) == ODS_SELECTED;
	bool isHighlighted = ( drawInfo->itemState & ODS_HOTLIGHT ) == ODS_HOTLIGHT;

	// this will contain item image
	HBITMAP image = NULL;

	// if checked/unchecked image is avaiable
	if ( ( info.hbmpChecked != NULL ) && ( info.hbmpUnchecked != NULL ) )
		image = isChecked ? info.hbmpChecked : info.hbmpUnchecked;
	else // get normal image
		image = data->Image->getBitmap();

	// this will contain image size
	Point imageSize = data->Image->getBitmapSize();

	if ( ( imageSize.x == 0 ) && ( imageSize.y == 0 ) ) // no image
		imageSize = defaultImageSize; // set default image size

	// compute strip width
	int stripWidth = imageSize.x + textIconGap;

	// prepare item rectangle
	Rectangle itemRectangle( drawInfo->rcItem.left, drawInfo->rcItem.top, // position
		drawInfo->rcItem.right - drawInfo->rcItem.left, // width
		drawInfo->rcItem.bottom - drawInfo->rcItem.top ); // height

	// setup buffered canvas
	BufferedCanvas< FreeCanvas > canvas( reinterpret_cast<HWND>(wrapper->menu->handle()), drawInfo->hDC );

	// this will contain adjusted sidebar width
	int sidebarWidth = 0;

	// this will contain adjusted(rotated) title font for sidebar
	HFONT titleFont = NULL;

	// get title font info and adjust item rectangle
	if ( wrapper->menu->drawSidebar )
	{
		// get title font
		FontPtr font = wrapper->menu->itsTitleFont;

		// get logical info for title font
		LOGFONT lf;
		::GetObject(font->handle(), sizeof(lf), &lf);

		// 90 degree rotation and bold
		lf.lfOrientation = lf.lfEscapement = 900;

		// create title font from logical info
		titleFont = ::CreateFontIndirect( & lf );

		// get title text size
		SIZE textSize;
		memset( & textSize, 0, sizeof( SIZE ) );

		HGDIOBJ oldFont = ::SelectObject( canvas.handle(), titleFont );
		::GetTextExtentPoint32( canvas.handle(), wrapper->menu->itsTitle.c_str(), ( int ) wrapper->menu->itsTitle.size(), & textSize );
		::SelectObject( canvas.handle(), oldFont );

		// set sidebar width to text height
		sidebarWidth = textSize.cy;

		// adjust item rectangle and item background
		itemRectangle.pos.x += sidebarWidth;
		itemRectangle.size.x -= sidebarWidth;
	}

	const MenuColorInfo& colorInfo = wrapper->menu->itsColorInfo;

	// draw sidebar with menu title
	if ( ( drawInfo->itemAction & ODA_DRAWENTIRE ) && ( wrapper->menu->drawSidebar ) && !wrapper->menu->itsTitle.empty() )
	{
		// select title font and color
		HGDIOBJ oldFont = ::SelectObject ( canvas.handle(), titleFont );
		COLORREF oldColor = canvas.setTextColor( colorInfo.colorTitleText );

		// set background mode to transparent
		bool oldMode = canvas.setBkMode( true );

		// get rect for sidebar
		RECT rect;
		::GetClipBox( drawInfo->hDC, & rect );
		//rect.left -= borderGap;

		// set title rectangle
		Rectangle textRectangle( 0, 0, sidebarWidth, rect.bottom - rect.top );

		{
			// draw background
			Brush brush(colorInfo.colorStrip);
			canvas.fillRectangle(textRectangle, brush);
		}

		// draw title
		textRectangle.pos.y += 10;
		canvas.drawText( wrapper->menu->itsTitle, textRectangle, DT_BOTTOM | DT_SINGLELINE );

		// clear
		canvas.setTextColor( oldColor );
		canvas.setBkMode( oldMode );

		// set back old font
		::SelectObject( canvas.handle(), oldFont );
	}

	// destroy title font
	::DeleteObject( titleFont );

	bool highlight = (isSelected || isHighlighted) && !isDisabled;

	{
		// set item background
		Brush brush(highlight ? colorInfo.colorHighlight : (wrapper->isMenuTitleItem || isMenuBar) ? colorInfo.colorStrip : colorInfo.colorMenu);
		canvas.fillRectangle(itemRectangle, brush);
	}

	if(!highlight && !isMenuBar && !wrapper->isMenuTitleItem) // strip bar (on the left, where bitmaps go)
	{
		// create rectangle for strip bar
		Rectangle stripRectangle ( itemRectangle );
		stripRectangle.size.x = stripWidth;

		// draw strip bar
		Brush brush(colorInfo.colorStrip);
		canvas.fillRectangle(stripRectangle, brush);
	}

	if ( !isMenuBar && info.fType & MFT_SEPARATOR ) // draw separator
	{
		// set up separator rectangle
		Rectangle rectangle ( itemRectangle );

		// center in the item rectangle
		rectangle.pos.x += stripWidth + textIconGap;
		rectangle.pos.y += rectangle.height() / 2 - 1;

		// select color
		Canvas::Selector select(canvas, *PenPtr(new Pen(::GetSysColor( COLOR_GRAYTEXT ))));

		// draw separator
		canvas.moveTo( rectangle.left(), rectangle.top() );
		canvas.lineTo( rectangle.width(), rectangle.top() );
	} // end if
	else // not a seperator, then draw item text and icon
	{
		// get item text
		const int length = info.cch + 1;
		std::vector< TCHAR > buffer( length );
		int count = ::GetMenuString( wrapper->menu->handle(), wrapper->index, & buffer[0], length, MF_BYPOSITION );
		SmartUtil::tstring itemText( buffer.begin(), buffer.begin() + count );

		// index will contain accelerator position
		size_t index = itemText.find_last_of( _T( '\t' ) );

		// split item text to draw accelerator correctly
		SmartUtil::tstring text = itemText.substr( 0, index );

		// get accelerator
		SmartUtil::tstring accelerator;

		if ( index != itemText.npos )
			accelerator = itemText.substr( index + 1 );

		// set mode to transparent
		bool oldMode = canvas.setBkMode( true );

		// select item text color
		canvas.setTextColor( isGrayed ? ::GetSysColor( COLOR_GRAYTEXT ) : wrapper->isMenuTitleItem ? colorInfo.colorTitleText : data->TextColor );

		// Select item font
		FontPtr font =
			(wrapper->isMenuTitleItem || (static_cast<int>(::GetMenuDefaultItem(wrapper->menu->handle(), TRUE, GMDI_USEDISABLED)) == wrapper->index))
			? wrapper->menu->itsTitleFont
			: wrapper->menu->font;

		HGDIOBJ oldFont = ::SelectObject( canvas.handle(), font->handle() );

		unsigned drawTextFormat = DT_VCENTER | DT_SINGLELINE;
		if((drawInfo->itemState & ODS_NOACCEL) == ODS_NOACCEL)
			drawTextFormat |= DT_HIDEPREFIX;

		if ( !isMenuBar && !wrapper->isMenuTitleItem && !itemText.empty() ) // if menu item
		{
			// compute text rectangle
			Rectangle textRectangle( itemRectangle );

			// adjust rectangle
			textRectangle.pos.x += stripWidth + textIconGap;
			textRectangle.size.x -= stripWidth + textIconGap + borderGap;

			canvas.drawText( text, textRectangle, DT_LEFT | drawTextFormat );

			// draw accelerator
			if ( !accelerator.empty() )
				canvas.drawText( accelerator, textRectangle, DT_RIGHT | drawTextFormat );
		} // end if
		else if ( !itemText.empty() ) // draw menu bar item text
		{
			Rectangle textRectangle( itemRectangle );

			if ( image != NULL ) // has icon
				textRectangle.pos.x += textIconGap;

			canvas.drawText( text, textRectangle, DT_CENTER | drawTextFormat );
		} // end if

		// set back old font
		::SelectObject( canvas.handle(), oldFont );

		// reset old mode
		canvas.setBkMode( oldMode );

		// set up image rectangle
		Rectangle imageRectangle( itemRectangle.pos, imageSize );

		// adjust icon rectangle
		imageRectangle.pos.x += ( stripWidth - imageSize.x ) / 2;
		imageRectangle.pos.y += ( itemRectangle.height() - imageSize.y ) / 2;

		if ( image == NULL ) // drawing item without icon
		{
			if ( isChecked ) // needs checkmark
			{
				// draw the check mark or radio bullet
				// prepare background
				Brush brush( colorInfo.colorStrip );
				canvas.fillRectangle( imageRectangle, brush );

				// create memory DC and set bitmap on it
				HDC memoryDC = ::CreateCompatibleDC( canvas.handle() );
				HGDIOBJ old = ::SelectObject( memoryDC, ::CreateCompatibleBitmap( canvas.handle(), imageSize.x, imageSize.y ) );

				// draw into memory
				RECT rc( Rectangle( 0, 0, imageSize.x, imageSize.y ) );
				::DrawFrameControl( memoryDC, & rc, DFC_MENU, ( info.fType & MFT_RADIOCHECK ) == 0 ? DFCS_MENUCHECK : DFCS_MENUBULLET );

				const int adjustment = 2; // adjustment for mark to be in the center

				// bit - blast into out canvas
				::BitBlt( canvas.handle(), imageRectangle.left() + adjustment, imageRectangle.top(), imageSize.x, imageSize.y, memoryDC, 0, 0, SRCAND );

				// delete memory dc
				::DeleteObject( ::SelectObject( memoryDC, old ) );
				::DeleteDC( memoryDC );
			}
		}
		else // drawing item with icon
		{
			if ( isSelected && !isDisabled ) // if selected and active, then imitate icon shadow
			{
				// adjust icon position for later drawing
				imageRectangle.pos.x--;
				imageRectangle.pos.y--;

				// setup brush for shadow emulation
				Brush brush( ColorUtilities::darkenColor( colorInfo.colorStrip, 0.2 ) );

				// draw the icon shadow
				Rectangle shadowRectangle( imageRectangle );
				shadowRectangle.pos.x++;
				shadowRectangle.pos.y++;
				canvas.drawBitmap( image, shadowRectangle, colorInfo.colorImageBackground, true );
			}

			// draw normal icon
			canvas.drawBitmap( image, imageRectangle, colorInfo.colorImageBackground, isGrayed );
		}

		if ( isChecked ) // draw surrounding rectangle for checked items
		{
			/*if ( image != NULL ) // adjust for icon
			iconRectangle = iconRectangle.shrink( 1.20 );*/

			// draw the surrounding rectangle
			Canvas::Selector select(canvas, *PenPtr(new Pen(colorInfo.colorHighlight)));
			canvas.line( imageRectangle );
		}
	}

	// blast buffer into screen
	if ( ( drawInfo->itemAction & ODA_DRAWENTIRE ) && wrapper->menu->drawSidebar ) // adjustment for sidebar
	{
		itemRectangle.pos.x -= sidebarWidth;
		itemRectangle.size.x += sidebarWidth;
	}

	canvas.blast( itemRectangle );
	return true;
}

bool WidgetMenu::handleMeasureItem(LPMEASUREITEMSTRUCT measureInfo) {
	if ( measureInfo->CtlType != ODT_MENU ) // if not intended for us
		return false;

	// get item data wrapper
	ItemDataWrapper * wrapper = reinterpret_cast< ItemDataWrapper * >( measureInfo->itemData );
	xAssert( wrapper != 0, _T( "Unsupported menu item type in measureItem()" ) );

	// this will contain item size
	UINT & itemWidth = measureInfo->itemWidth;
	UINT & itemHeight = measureInfo->itemHeight;

	// init struct for item info
	MENUITEMINFO info;
	memset( & info, 0, sizeof( MENUITEMINFO ) );
	info.cbSize = sizeof( MENUITEMINFO );

	// set up flags
	info.fMask = MIIM_FTYPE | MIIM_DATA | MIIM_CHECKMARKS | MIIM_STRING;

	// try to get item info
	if ( ::GetMenuItemInfo( wrapper->menu->handle(), wrapper->index, TRUE, & info ) == FALSE )
		throw xCeption ( _T( "Couldn't get item info in measureItem()" ) );

	// check if item is owner drawn
	xAssert( ( info.fType & MFT_OWNERDRAW ) != 0, _T( "Not owner - drawn item encountered in measureItem()" ) );

	// check if separator
	if ( info.fType & MFT_SEPARATOR )
	{
		itemWidth = 60;
		itemHeight = separatorHeight;
		return true;
	}

	// are we processing menu bar ?
	const bool isMenuBar = ::GetMenu( wrapper->menu->getParent()->handle() ) == wrapper->menu->handle();

	// compute text width and height by simulating write to dc
	// get its DC
	HDC hdc = ::GetDC( wrapper->menu->getParent()->handle() );

	// get the item data
	MenuItemDataPtr data = wrapper->data;
	xAssert( data != 0, _T( "Couldn't find item data in measureItem()" ) );

	// get item text
	const int length = info.cch + 1;
	std::vector< TCHAR > buffer ( length );
	int count = ::GetMenuString( wrapper->menu->handle(), wrapper->index, & buffer[0], length, MF_BYPOSITION );
	SmartUtil::tstring itemText ( buffer.begin(), buffer.begin() + count );

	// now get text extents
	SIZE textSize;
	memset( & textSize, 0, sizeof( SIZE ) );

	HGDIOBJ oldFont = ::SelectObject( hdc, wrapper->menu->font->handle() );
	::GetTextExtentPoint32( hdc, itemText.c_str(), ( int ) itemText.size(), & textSize );
	::SelectObject( hdc, oldFont );

	// release DC
	::ReleaseDC( reinterpret_cast<HWND>(wrapper->menu->handle()), hdc );

	// adjust item size
	itemWidth = textSize.cx + borderGap;
	itemHeight = textSize.cy + borderGap;

	// check to see if item has an image
	Point imageSize = data->Image->getBitmapSize();

	// this will contain checked/unchecked image size
	Point checkImageSize;

	// if item has check/unchecked state images, then get their sizes
	if ( ( info.hbmpChecked != NULL ) && ( info.hbmpUnchecked != NULL ) )
		checkImageSize = ( info.fState & MFS_CHECKED ) == 0
		? Bitmap::getBitmapSize( info.hbmpUnchecked )
		: Bitmap::getBitmapSize( info.hbmpChecked );

	// take the maximum of all available images or set default image size
	imageSize.x = (std::max)( imageSize.x, checkImageSize.x );
	imageSize.y = (std::max)( imageSize.y, checkImageSize.y );

	bool hasImage = ( imageSize.x != 0 ) && ( imageSize.y != 0 );

	// set default image size if no image is available
	if ( !hasImage )
	{
		imageSize.x = (std::max)( defaultImageSize.x, (std::max)( imageSize.x, checkImageSize.x ) );
		imageSize.y = (std::max)( defaultImageSize.y, (std::max)( imageSize.y, checkImageSize.y ) );
	}

	// adjust default image size
	defaultImageSize.x = (std::max)( defaultImageSize.x, imageSize.x );
	defaultImageSize.y = (std::max)( defaultImageSize.y, imageSize.y );

	// adjust width
	if ( !isMenuBar || // if not menu bar item
		( isMenuBar && hasImage ) ) // or menu bar item with image
	{
		// adjust item width
		itemWidth += imageSize.x + textIconGap + pointerGap;

		// adjust item height
		itemHeight = (std::max)( itemHeight, ( UINT ) imageSize.y + borderGap );
	}

	// adjust width for system menu items
	if ( wrapper->menu->isSysMenu )
		itemWidth = (std::max)( ( UINT ) minSysMenuItemWidth, itemWidth );

	// adjust width for sidebar
	if ( wrapper->menu->drawSidebar )
	{
		// get title text extents
		SIZE textSize;
		memset( & textSize, 0, sizeof( SIZE ) );

		::GetTextExtentPoint32( hdc, wrapper->menu->itsTitle.c_str(), ( int ) wrapper->menu->itsTitle.size(), & textSize );

		itemWidth += textSize.cy;
	}

	// adjust item height
	itemHeight = (std::max)( itemHeight, ( UINT )::GetSystemMetrics( SM_CYMENU ) );
	return true;
}

void WidgetMenu::appendSeparatorItem()
{
	// init structure for new item
	MENUITEMINFO itemInfo;
	memset( & itemInfo, 0, sizeof( itemInfo ) );
	itemInfo.cbSize = sizeof( MENUITEMINFO );

	// set flags
	itemInfo.fMask = MIIM_FTYPE;
	itemInfo.fType = MFT_SEPARATOR;

	// get position to insert
	int position = getCount();

	ItemDataWrapper * wrapper = NULL;
	if(ownerDrawn) {
		itemInfo.fMask |= MIIM_DATA;
		itemInfo.fType |= MFT_OWNERDRAW;

		// create item data wrapper
		wrapper = new ItemDataWrapper( this, position, MenuItemDataPtr( new MenuItemData() ) );
		itemInfo.dwItemData = reinterpret_cast< ULONG_PTR >( wrapper );
	}

	if ( ::InsertMenuItem( itsHandle, position, TRUE, & itemInfo ) && ownerDrawn )
		itsItemData.push_back( wrapper );
}

void WidgetMenu::removeItem( unsigned itemIndex )
{
	// has sub menus ?
	HMENU popup = ::GetSubMenu( itsHandle, itemIndex );

	// try to remove item
	if ( ::RemoveMenu( itsHandle, itemIndex, MF_BYPOSITION ) )
	{
		if(ownerDrawn) {
			ItemDataWrapper * wrapper = 0;
			int itemRemoved = -1;

			for(size_t i = 0; i < itsItemData.size(); ++i) {
				// get current data wrapper
				wrapper = itsItemData[i];

				if ( wrapper->index == int(itemIndex) ) // if found
				{
					itemRemoved = int(i);
					delete wrapper;
					itsItemData[i] = 0;
				}
				else if ( wrapper->index > int(itemIndex) )
					--wrapper->index; // adjust succeeding item indices
			}

			if( itemRemoved != -1 )
				itsItemData.erase( itsItemData.begin() + itemRemoved );
		}

		// remove sub menus if any
		if(popup)
			for(size_t i = 0; i < itsChildren.size(); ++i)
				if(itsChildren[i]->handle() == popup)
					itsChildren[i].reset();
	}
	else
		throw xCeption( _T( "Couldn't remove item in removeItem()" ) );
}

void WidgetMenu::removeAllItems()
{
	//must be backwards, since bigger indexes change on remove
	for( int i = getCount() - 1; i >= 0; i-- )
	{
		removeItem( i );
	}
}

int WidgetMenu::getCount() const
{
	int count = ::GetMenuItemCount( itsHandle );
	if( count == -1 )
		throw xCeption( _T( "Couldn't get item count in getCount()" ) );
	return count;
}

void WidgetMenu::appendItem(unsigned int id, const SmartUtil::tstring & text, MenuItemDataPtr itemData)
{
	// init structure for new item
	MENUITEMINFO info;
	memset( & info, 0, sizeof( info ) );
	info.cbSize = sizeof( MENUITEMINFO );

	// set flags
	info.fMask = MIIM_DATA | MIIM_FTYPE | MIIM_CHECKMARKS | MIIM_ID | MIIM_STRING;

	// set fields
	xAssert( !isSysMenu || id < SC_SIZE, _T( "Can't add sysmenu item with that high value, value can not be higher then SC_SIZE - 1" ) );
	info.wID = id;

	// set text
	info.dwTypeData = const_cast< LPTSTR >( text.c_str() );

	// find item index
	int index = getItemIndex( id );

	// set position to insert
	bool itemExists = index != - 1;
	index = itemExists ? index : getCount();

	ItemDataWrapper * wrapper = NULL;
	if(ownerDrawn) {
		info.fMask |= MIIM_DATA | MIIM_FTYPE;
		info.fType = MFT_OWNERDRAW;

		// set item data
		wrapper = new ItemDataWrapper( this, index, itemData );
		info.dwItemData = reinterpret_cast< ULONG_PTR >( wrapper );
	}

	if ( ( !itemExists && ::InsertMenuItem( itsHandle, id, FALSE, & info ) ) ||
		( itemExists && ::SetMenuItemInfo( itsHandle, id, FALSE, & info ) ) )
	{
		if(ownerDrawn)
			itsItemData.push_back( wrapper );
	}
	else
		throw xCeption( _T( "Couldn't insert/update item in WidgetMenu::appendItem" ) );
}

void WidgetMenu::appendItem(unsigned int id, const SmartUtil::tstring & text, BitmapPtr image)
{
	MenuItemDataPtr itemData(new MenuItemData());
	if(ownerDrawn)
		itemData->Image = image;
	appendItem(id, text, itemData);
}

unsigned WidgetMenu::trackPopupMenu( const ScreenCoordinate& sc, unsigned flags )
{
	addCommands();

	long x = sc.getPoint().x, y = sc.getPoint().y;

	if ( x == - 1 && y == - 1 )
	{
		DWORD pos = ::GetMessagePos();
		x = LOWORD( pos );
		y = HIWORD( pos );
	}

	int retVal = ::TrackPopupMenu(itsHandle, flags, x, y, 0, itsParent->handle(), 0 );
	return retVal;
}

WidgetMenu::ObjectType WidgetMenu::getChild( unsigned position ) {
	HMENU h = ::GetSubMenu(handle(), position);
	for(size_t i = 0; i < itsChildren.size(); ++i) {
		ObjectType& menu = itsChildren[i];
		if(menu->handle() == h) {
			return menu;
		}
	}
	return ObjectType();
}

}

#endif