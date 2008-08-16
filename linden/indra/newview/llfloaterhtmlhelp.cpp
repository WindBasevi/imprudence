/** 
 * @file llfloaterhtmlhelp.cpp
 * @brief HTML Help floater - uses embedded web browser control
 *
 * $LicenseInfo:firstyear=2006&license=viewergpl$
 * 
 * Copyright (c) 2006-2008, Linden Research, Inc.
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

#include "llviewerprecompiledheaders.h"

#include "llfloaterhtmlhelp.h"

#include "llvieweruictrlfactory.h"
#include "llwebbrowserctrl.h"
#include "llviewerwindow.h"
#include "llviewercontrol.h"
#include "llweb.h"
#include "llui.h"

#if LL_LIBXUL_ENABLED

LLViewerHtmlHelp gViewerHtmlHelp;

class LLFloaterHtmlHelp :
	public LLFloater,
	public LLWebBrowserCtrlObserver
{
public:
	LLFloaterHtmlHelp(std::string start_url, std::string title);
	virtual ~LLFloaterHtmlHelp();
	
	virtual void onClose( bool app_quitting );
	virtual void draw();
	
	static void show(std::string url, std::string title);
	static void onClickBack( void* data );
	static void onClickHome( void* data );
	static void onClickForward( void* data );
	static void onClickClose( void* data );
	
	// browser observer impls
	virtual void onStatusTextChange( const EventType& eventIn );
	virtual void onLocationChange( const EventType& eventIn );
	
	// used for some stats logging - will be removed at some point
	static BOOL sFloaterOpened;

	static void onClickF1HelpLoadURL(S32 option, void* userdata);

protected:
	LLWebBrowserCtrl* mWebBrowser;
	static LLFloaterHtmlHelp* sInstance;
	LLButton* mBackButton;
	LLButton* mForwardButton;
	LLButton* mCloseButton;
	LLTextBox* mStatusText;
	LLString mStatusTextContents;
	LLString mCurrentUrl;
};

LLFloaterHtmlHelp* LLFloaterHtmlHelp::sInstance = 0;

BOOL LLFloaterHtmlHelp::sFloaterOpened = FALSE;

////////////////////////////////////////////////////////////////////////////////
//
LLFloaterHtmlHelp::LLFloaterHtmlHelp(std::string start_url, std::string title)
:	LLFloater( "HTML Help" ),
	mWebBrowser( 0 ),
	mStatusTextContents( "" ),
	mCurrentUrl( "" )
{
	sInstance = this;
		
	// create floater from its XML definition
	gUICtrlFactory->buildFloater( this, "floater_html_help.xml" );
		
	childSetAction("back_btn", onClickBack, this);
	childSetAction("home_btn", onClickHome, this);
	childSetAction("forward_btn", onClickForward, this);
	
	if (!title.empty())
	{
		setTitle(title);
	}

	mWebBrowser = LLViewerUICtrlFactory::getWebBrowserByName(this,  "html_help_browser" );
	if ( mWebBrowser )
	{
		// observe browser control events
		mWebBrowser->addObserver( this );

		if (start_url != "")
		{
			mWebBrowser->navigateTo( start_url );
		}
		else
		{
			// if the last page we were at before the client was closed is valid, go there and
			// override what is in the XML file
			// (not when the window was closed - it's only ever hidden - not closed)
			LLString lastPageUrl = gSavedSettings.getString( "HtmlHelpLastPage" );
			if ( lastPageUrl != "" )
			{
				mWebBrowser->navigateTo( lastPageUrl );
			};
		}
	};
}

////////////////////////////////////////////////////////////////////////////////
//
LLFloaterHtmlHelp::~LLFloaterHtmlHelp()
{
	// stop observing browser events
	if ( mWebBrowser )
	{
		mWebBrowser->remObserver( this );
	};

	// save position of floater
	gSavedSettings.setRect( "HtmlHelpRect", mRect );

	// save the location we were at when SL closed 
	gSavedSettings.setString( "HtmlHelpLastPage", mCurrentUrl );

	sInstance = 0;
}

////////////////////////////////////////////////////////////////////////////////
// virtual 
void LLFloaterHtmlHelp::draw()
{
	// enable/disable buttons depending on state
	if ( mWebBrowser )
	{
		bool enable_back = mWebBrowser->canNavigateBack();	
		childSetEnabled( "back_btn", enable_back );

		bool enable_forward = mWebBrowser->canNavigateForward();	
		childSetEnabled( "forward_btn", enable_forward );
	};

	LLFloater::draw();
}

////////////////////////////////////////////////////////////////////////////////
//
void LLFloaterHtmlHelp::show(std::string url, std::string title)
{
    gViewerWindow->alertXml("ClickOpenF1Help", onClickF1HelpLoadURL, (void*) NULL);

	// switching this out for the moment - will come back later
	// want it still to be compiled so not using comments of #if 0
	if ( false )
	{
		sFloaterOpened = true;

		if ( sInstance )
		{
			if (sInstance->mWebBrowser)
			{
				sInstance->mWebBrowser->navigateTo(url);
			}
			sInstance->setVisibleAndFrontmost();
			return;
		}

		LLFloaterHtmlHelp* self = new LLFloaterHtmlHelp(url, title);

		// reposition floater from saved settings
		LLRect rect = gSavedSettings.getRect( "HtmlHelpRect" );
		self->reshape( rect.getWidth(), rect.getHeight(), FALSE );
		self->setRect( rect );
	};
}

// static 
void LLFloaterHtmlHelp::onClickF1HelpLoadURL(S32 option, void* userdata)
{
	if (option == 0)
	{
		// choose HELP url based on selected language - default to english language support page
		LLString lang = LLUI::sConfigGroup->getString("Language");

		// this sucks but there isn't a way to grab an arbitrary string from an XML file
		// (using llcontroldef strings causes problems if string don't exist)
		LLString help_url( "http://secondlife.com/support" );
		if ( lang == "ja" )
			help_url = "http://help.secondlife.com/jp";
		else
		if ( lang == "ko" )
			help_url = "http://help.secondlife.com/kr";
		else
		if ( lang == "pt" )
			help_url = "http://help.secondlife.com/pt";
		else
		if ( lang == "de" )
			help_url = "http://de.secondlife.com/support";

		LLWeb::loadURL( help_url );
	};
}

////////////////////////////////////////////////////////////////////////////////
//
void LLFloaterHtmlHelp::onClose( bool app_quitting )
{
	setVisible( false );
}

////////////////////////////////////////////////////////////////////////////////
//
void LLFloaterHtmlHelp::onClickClose( void* data )
{
	LLFloaterHtmlHelp* self = ( LLFloaterHtmlHelp* )data;

	self->setVisible( false );
}

////////////////////////////////////////////////////////////////////////////////
//
void LLFloaterHtmlHelp::onClickBack( void* data )
{
	LLFloaterHtmlHelp* self = ( LLFloaterHtmlHelp* )data;
	if ( self )
	{
		if ( self->mWebBrowser )
		{
			self->mWebBrowser->navigateBack();
		};
	};
}

////////////////////////////////////////////////////////////////////////////////
//
void LLFloaterHtmlHelp::onClickHome( void* data )
{
	LLFloaterHtmlHelp* self = ( LLFloaterHtmlHelp* )data;
	if ( self )
	{
		// get the home page URL (which can differ from the start URL) from XML and go there
		LLWebBrowserCtrl* web_browser = LLUICtrlFactory::getWebBrowserCtrlByName( self, "html_help_browser" );
		if ( web_browser )
		{
			web_browser->navigateHome();
		};
	};
}

////////////////////////////////////////////////////////////////////////////////
// 
void LLFloaterHtmlHelp::onClickForward( void* data )
{
	LLFloaterHtmlHelp* self = ( LLFloaterHtmlHelp* )data;
	if ( self )
	{
		if ( self->mWebBrowser )
		{
			self->mWebBrowser->navigateForward();
		};
	};
}

////////////////////////////////////////////////////////////////////////////////
//
void LLFloaterHtmlHelp::onStatusTextChange( const EventType& eventIn )
{
	mStatusTextContents = LLString( eventIn.getStringValue() );

	childSetText("status_text", mStatusTextContents);
}

////////////////////////////////////////////////////////////////////////////////
//
void LLFloaterHtmlHelp::onLocationChange( const EventType& eventIn )
{
	llinfos << "MOZ> Location changed to " << eventIn.getStringValue() << llendl;
	mCurrentUrl = LLString( eventIn.getStringValue() );
}

////////////////////////////////////////////////////////////////////////////////
//
LLViewerHtmlHelp::LLViewerHtmlHelp()
{
	LLUI::setHtmlHelp(this);
}

LLViewerHtmlHelp::~LLViewerHtmlHelp()
{
	LLUI::setHtmlHelp(NULL);
}

void LLViewerHtmlHelp::show(std::string url, std::string title)
{
	LLFloaterHtmlHelp::show(url, title);
}

BOOL LLViewerHtmlHelp::getFloaterOpened()
{
	return LLFloaterHtmlHelp::sFloaterOpened;
}

#endif	// LL_LIBXUL_ENABLED

