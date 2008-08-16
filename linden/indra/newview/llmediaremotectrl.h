/** 
 * @file llmediaremotectrl.h
 * @brief A remote control for media (video and music)
 *
 * $LicenseInfo:firstyear=2005&license=viewergpl$
 * 
 * Copyright (c) 2005-2008, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
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
 * $/LicenseInfo$
 */

#ifndef LL_LLMEDIAREMOTECTRL_H
#define LL_LLMEDIAREMOTECTRL_H

#include "llpanel.h"

////////////////////////////////////////////////////////////////////////////////
//
class LLMediaRemoteCtrl : public LLPanel
{
public:
	LLMediaRemoteCtrl ( const LLString& name,
						const LLString& label,
						const LLRect& rect,
						const LLString& xml_file );
	
	virtual ~LLMediaRemoteCtrl ();
	virtual BOOL postBuild();

	virtual void draw();
	
	virtual EWidgetType getWidgetType() const;
	virtual LLString getWidgetTag() const;
};

#endif
