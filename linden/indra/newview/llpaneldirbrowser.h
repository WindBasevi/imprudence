/** 
 * @file llpaneldirbrowser.h
 * @brief LLPanelDirBrowser class definition
 *
 * $LicenseInfo:firstyear=2001&license=viewergpl$
 * 
 * Copyright (c) 2001-2008, Linden Research, Inc.
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

// Base class for the various search panels/results browsers
// in the Find floater.  For example, Find > Popular Places
// is derived from this.

#ifndef LL_LLPANELDIRBROWSER_H
#define LL_LLPANELDIRBROWSER_H

#include "llpanel.h"

#include "lluuid.h"
#include "llframetimer.h"
#include "llmap.h"

class LLMessageSystem;
class LLFloaterDirectory;

class LLPanelDirBrowser: public LLPanel
{
public:
	LLPanelDirBrowser(const std::string& name, LLFloaterDirectory* floater);
	virtual ~LLPanelDirBrowser();

	virtual BOOL postBuild();

	// Use to get periodic updates.
	virtual void draw();

	virtual void onVisibilityChange(BOOL curVisibilityIn);

	// Redo your search for the prev/next page of results
	virtual void prevPage();
	virtual void nextPage();
	void resetSearchStart();
	// Do the current query (used by prevPage/nextPage)
	virtual void performQuery() {};

	const LLUUID& getSearchID() const { return mSearchID; }

	// Select the line in the scroll list control with this ID,
	// either now or when data arrives from the server.
	void selectByUUID(const LLUUID& id);

	void selectEventByID(S32 event_id);

	U32 getSelectedEventID() const;
	void getSelectedInfo(LLUUID* id, S32 *type);

	void showDetailPanel(S32 type, LLSD item_id);
		// type is EVENT_CODE, PLACE_CODE, etc. from below.
		// item_id is integer for events, UUID for all others.

	// from llpaneldirbase
	void setupNewSearch();

	// default handler for clicking the search button resets the
	// next/prev state and performs the query.
	// Expects a pointer to an LLPanelDirBrowser object.
	static void onClickSearchCore(void* userdata);

	// query_start indicates the first result row to
	// return, usually 0 or 100 or 200 because the searches
	// return a max of 100 rows
	static void sendDirFindQuery(
		LLMessageSystem* msg,
		const LLUUID& query_id,
		const LLString& text,
		U32 flags,
		S32 query_start);

	// Put an item in the first line of the results list.
	void addHelpText(const char* text);

	void newClassified();

	void showEvent(const U32 event_id);

	void renameClassified(const LLUUID& classified_id, const char* name);

	// Prev/Next page buttons
	static void onClickNext(void* data);
	static void onClickPrev(void* data);

	static void onCommitList(LLUICtrl* ctrl, void* data);

	static void processDirPeopleReply(LLMessageSystem* msg, void**);
	static void processDirPlacesReply(LLMessageSystem* msg, void**);
	static void processDirEventsReply(LLMessageSystem* msg, void**);
	static void processDirGroupsReply(LLMessageSystem* msg, void**);
	static void processDirClassifiedReply(LLMessageSystem* msg, void**);
	static void processDirPopularReply(LLMessageSystem* msg, void**);
	static void processDirLandReply(LLMessageSystem *msg, void**);

protected:
	void updateResultCount();

	void addClassified(LLCtrlListInterface *list, const LLUUID& classified_id, const char* name, const U32 creation_date, const S32 price_for_listing);
	LLSD createLandSale(const LLUUID& parcel_id, BOOL is_auction, BOOL is_for_sale, const LLString& name, S32 *type);

	static void onKeystrokeName(LLLineEditor* line, void* data);

	// If this is a search for a panel like "people_panel" (and not the "all" panel)
	// optionally show the "Next" button.  Return the actual number of
	// rows to display.
	S32 showNextButton(S32 rows);

protected:
	LLUUID			mSearchID;		// Unique ID for a pending search
	LLUUID			mWantSelectID;	// scroll item to select on arrival
	LLString        mCurrentSortColumn;
	BOOL            mCurrentSortAscending;
	// Some searches return a max of 100 items per page, so we can
	// start the search from the 100th item rather than the 0th, etc.
	S32				mSearchStart;
	// Places is 100 per page, events is 200 per page
	S32				mResultsPerPage;
	S32				mResultsReceived;

	U32				mMinSearchChars;

	LLSD			mResultsContents;

	BOOL			mHaveSearchResults;
	BOOL			mDidAutoSelect;
	LLFrameTimer	mLastResultTimer;

	LLFloaterDirectory* mFloaterDirectory;
};

// Codes used for sorting by type.
const S32 INVALID_CODE = -1;
const S32 EVENT_CODE = 0;
const S32 PLACE_CODE = 1;
// We no longer show online vs. offline in search result icons.
//const S32 ONLINE_CODE = 2;
//const S32 OFFLINE_CODE = 3;
const S32 AVATAR_CODE = 3;
const S32 GROUP_CODE = 4;
const S32 CLASSIFIED_CODE = 5;	
const S32 FOR_SALE_CODE = 6;	// for sale place
const S32 AUCTION_CODE = 7;		// for auction place
const S32 POPULAR_CODE = 8;		// popular by dwell

extern LLMap< const LLUUID, LLPanelDirBrowser* > gDirBrowserInstances;

#endif // LL_LLPANELDIRBROWSER_H
