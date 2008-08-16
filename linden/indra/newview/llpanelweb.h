/** 
 * @file llpanelweb.h
 * @brief Web browser preferences panel
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

#ifndef LL_LLPANELWEB_H
#define LL_LLPANELWEB_H

#include "llpanel.h"
#include "llviewerthrottle.h"

class LLCheckBoxCtrl;
class LLButton;

class LLPanelWeb : public LLPanel
{
public:
	LLPanelWeb();
	virtual ~LLPanelWeb();

	virtual BOOL postBuild();
	virtual void refresh();
	virtual void apply();	// Apply the changed values.
	virtual void cancel();	// Cancel the changed values.

private:
	static void onClickClearCache(void*);
	static void onClickClearCookies(void*);
	static void callback_clear_browser_cache(S32 option, void* userdata);
	static void callback_clear_cookies(S32 option, void* userdata);
	static void onCommitCookies(LLUICtrl* ctrl, void* data);
	static void onCommitWebProxyEnabled(LLUICtrl* ctrl, void* data);
	static void onCommitWebProxyAddress(LLUICtrl *ctrl, void *userdata);
	static void onCommitWebProxyPort(LLUICtrl *ctrl, void *userdata);

private:
	BOOL mCookiesEnabled;
	BOOL mWebProxyEnabled;
	LLString mWebProxyAddress;
	S32 mWebProxyPort;
};

#endif
