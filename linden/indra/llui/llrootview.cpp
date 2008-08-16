/** 
 * @file llrootview.cpp
 * @brief Container for all other views in an application
 *
 * Copyright (c) 2006-2007, Linden Research, Inc.
 * 
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlife.com/developers/opensource/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at http://secondlife.com/developers/opensource/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 */

#include "llrootview.h"

LLRootView::LLRootView(const LLString& name, const LLRect& rect, BOOL mouse_opaque, U32 follows)
: LLView(name,rect,mouse_opaque,follows) 
{ }

// virtual
EWidgetType LLRootView::getWidgetType() const
{
	return WIDGET_TYPE_ROOT_VIEW;
}

// virtual
LLString LLRootView::getWidgetTag() const 
{ 
	return LL_ROOT_VIEW_TAG; 
}
