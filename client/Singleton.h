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

#if !defined(AFX_SINGLETON_H__3B62D311_53B4_42E9_8522_D890E70BDFE3__INCLUDED_)
#define AFX_SINGLETON_H__3B62D311_53B4_42E9_8522_D890E70BDFE3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

template<typename T>
class Singleton {
public:
	static T* getInstance() {
		dcassert(instance);
		return instance;
	}
	
	static void newInstance() {
		if(instance)
			delete instance;
		
		instance = new T();
	}
	
	static void deleteInstance() {
		if(instance)
			delete instance;
		instance = NULL;
	}
protected:
	static T* instance;
};

#endif // !defined(AFX_SINGLETON_H__3B62D311_53B4_42E9_8522_D890E70BDFE3__INCLUDED_)

/**
 * @file Singleton.h
 * $Id: Singleton.h,v 1.1 2002/04/16 16:46:21 arnetheduck Exp $
 */
