/** 
 * @file llsdappservices.h
 * @author Phoenix
 * @date 2006-09-12
 * @brief Header file to declare the /app common web services.
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

#ifndef LL_LLSDAPPSERVICES_H
#define LL_LLSDAPPSERVICES_H

/** 
 * @class LLSDAppServices
 * @brief This class forces a link to llsdappservices if the static
 * method is called which declares the /app web services.
 */
class LLSDAppServices
{
public:
	/**
	 * @brief Call this method to declare the /app common web services.
	 *
	 * This will register:
	 *  /app/config
	 *  /app/config/runtime-override
	 *  /app/config/runtime-override/<option-name>
	 *  /app/config/command-line
	 *  /app/config/specific
	 *  /app/config/general
	 *  /app/config/default
	 *  /app/config/live
	 *  /app/config/live/<option-name>
	 */
	static void useServices();
};


#endif // LL_LLSDAPPSERVICES_H
