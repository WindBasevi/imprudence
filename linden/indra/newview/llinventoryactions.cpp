/** 
 * @file llinventoryactions.cpp
 * @brief Implementation of the actions associated with menu items.
 *
 * Copyright (c) 2001-2007, Linden Research, Inc.
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

#include "llviewerprecompiledheaders.h"

#include <utility> // for std::pair<>

#include "llinventoryview.h"
#include "llpanelinventory.h"
#include "llinventorybridge.h"

#include "message.h"

#include "llagent.h"
#include "llcallingcard.h"
#include "llcheckboxctrl.h"		// for radio buttons
#include "llradiogroup.h"
#include "llspinctrl.h"
#include "lltextbox.h"
#include "llui.h"

#include "llviewercontrol.h"
#include "llfirstuse.h"
#include "llfloateravatarinfo.h"
#include "llfloaterchat.h"
#include "llfloatercustomize.h"
#include "llfloaterproperties.h"
#include "llfocusmgr.h"
#include "llfolderview.h"
#include "llgesturemgr.h"
#include "lliconctrl.h"
#include "llinventorymodel.h"
#include "llinventoryclipboard.h"
#include "lllineeditor.h"
#include "llmenugl.h"
#include "llpreviewanim.h"
#include "llpreviewgesture.h"
#include "llpreviewlandmark.h"
#include "llpreviewnotecard.h"
#include "llpreviewscript.h"
#include "llpreviewsound.h"
#include "llpreviewtexture.h"
#include "llresmgr.h"
#include "llscrollcontainer.h"
#include "llimview.h"
#include "lltooldraganddrop.h"
#include "llviewerimagelist.h"
#include "llviewerinventory.h"
#include "llviewerobjectlist.h"
#include "llviewerwindow.h"
#include "llwearable.h"
#include "llwearablelist.h"
#include "viewer.h"
#include "llviewermessage.h" 
#include "llviewerregion.h"
#include "lltabcontainer.h"
#include "llvieweruictrlfactory.h"
#include "llselectmgr.h"

typedef LLMemberListener<LLPanelInventory> object_inventory_listener_t;
typedef LLMemberListener<LLInventoryView> inventory_listener_t;
typedef LLMemberListener<LLInventoryPanel> inventory_panel_listener_t;

bool doToSelected(LLFolderView* folder, LLString action)
{
	LLInventoryModel* model = &gInventory;
	if ("rename" == action)
	{
		folder->startRenamingSelectedItem();
		return true;
	}
	if ("delete" == action)
	{
		folder->removeSelectedItems();
		return true;
	}

	if ("copy" == action)
	{	
		LLInventoryClipboard::instance().reset();
	}

	std::set<LLUUID> selected_items;
	folder->getSelectionList(selected_items);

	LLMultiPreview* multi_previewp = NULL;
	LLMultiProperties* multi_propertiesp = NULL;

	if (("task_open" == action  || "open" == action) && selected_items.size() > 1)
	{
		S32 left, top;
		gFloaterView->getNewFloaterPosition(&left, &top);

		multi_previewp = new LLMultiPreview(LLRect(left, top, left + 300, top - 100));
		gFloaterView->addChild(multi_previewp);

		LLFloater::setFloaterHost(multi_previewp);
	
	}
	else if (("task_properties" == action || "properties" == action) && selected_items.size() > 1)
	{
		S32 left, top;
		gFloaterView->getNewFloaterPosition(&left, &top);

		multi_propertiesp = new LLMultiProperties(LLRect(left, top, left + 100, top - 100));
		gFloaterView->addChild(multi_propertiesp);

		LLFloater::setFloaterHost(multi_propertiesp);
	}

	std::set<LLUUID>::iterator set_iter;

	for (set_iter = selected_items.begin(); set_iter != selected_items.end(); ++set_iter)
	{
		LLFolderViewItem* folder_item = folder->getItemByID(*set_iter);
		if(!folder_item) continue;
		LLInvFVBridge* bridge = (LLInvFVBridge*)folder_item->getListener();
		if(!bridge) continue;

		bridge->performAction(folder, model, action);
	}

	LLFloater::setFloaterHost(NULL);
	if (multi_previewp)
	{
		multi_previewp->open();
	}
	else if (multi_propertiesp)
	{
		multi_propertiesp->open();		/*Flawfinder: ignore*/
	}

	return true;
}

class LLDoToSelectedPanel : public object_inventory_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLString action = userdata.asString();
		LLPanelInventory *panel = mPtr;
		LLFolderView* folder = panel->getRootFolder();
		if(!folder) return true;

		return doToSelected(folder, action);
	}
};

class LLDoToSelectedFloater : public inventory_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLString action = userdata.asString();
		LLInventoryPanel *panel = mPtr->getPanel();
		LLFolderView* folder = panel->getRootFolder();
		if(!folder) return true;

		return doToSelected(folder, action);
	}
};

class LLDoToSelected : public inventory_panel_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLString action = userdata.asString();
		LLInventoryPanel *panel = mPtr;
		LLFolderView* folder = panel->getRootFolder();
		if(!folder) return true;

		return doToSelected(folder, action);
	}
};

class LLNewWindow : public inventory_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLRect rect(gSavedSettings.getRect("FloaterInventoryRect"));
		S32 left = 0 , top = 0;
		gFloaterView->getNewFloaterPosition(&left, &top);
		rect.setLeftTopAndSize(left, top, rect.getWidth(), rect.getHeight());
		LLInventoryView* iv = new LLInventoryView("Inventory",
												rect,
												mPtr->getActivePanel()->getModel());
		iv->getActivePanel()->setFilterTypes(mPtr->getActivePanel()->getFilterTypes());
		iv->getActivePanel()->setFilterSubString(mPtr->getActivePanel()->getFilterSubString());
		iv->open();		/*Flawfinder: ignore*/

		// force onscreen
		gFloaterView->adjustToFitScreen(iv, FALSE);
		return true;
	}
};

class LLShowFilters : public inventory_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		mPtr->toggleFindOptions();
		return true;
	}
};

class LLResetFilter : public inventory_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLInventoryViewFinder *finder = mPtr->getFinder();
		mPtr->getActivePanel()->getFilter()->resetDefault();
		if (finder)
		{
			finder->updateElementsFromFilter();
		}

		mPtr->setFilterTextFromFilter();
		return true;
	}
};

class LLCloseAllFolders : public inventory_panel_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		mPtr->closeAllFolders();
		return true;
	}
};

class LLCloseAllFoldersFloater : public inventory_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		mPtr->getPanel()->closeAllFolders();
		return true;
	}
};

class LLEmptyTrash : public inventory_panel_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLInventoryModel* model = mPtr->getModel();
		if(!model) return false;
		gViewerWindow->alertXml("ConfirmEmptyTrash", callback_empty_trash, this);
		return true;
	}

	static void callback_empty_trash(S32 option, void* userdata)
	{
		LLEmptyTrash* empty_trash = (LLEmptyTrash*)userdata;
		if (option == 0) // YES
		{
			LLInventoryModel* model = empty_trash->mPtr->getModel();
			LLUUID trash_id = model->findCategoryUUIDForType(LLAssetType::AT_TRASH);
			model->purgeDescendentsOf(trash_id);
			model->notifyObservers();
		}
	}
};

class LLEmptyLostAndFound : public inventory_panel_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLInventoryModel* model = mPtr->getModel();
		if(!model) return false;
		gViewerWindow->alertXml("ConfirmEmptyLostAndFound", callback_empty_lost_and_found, this);
		return true;
	}

	static void callback_empty_lost_and_found(S32 option, void* userdata)
	{
		LLEmptyLostAndFound* empty_lost_and_found = (LLEmptyLostAndFound*)userdata;
		if (option == 0) // YES
		{
			LLInventoryModel* model = empty_lost_and_found->mPtr->getModel();
			LLUUID lost_and_found_id = model->findCategoryUUIDForType(LLAssetType::AT_LOST_AND_FOUND);
			model->purgeDescendentsOf(lost_and_found_id);
			model->notifyObservers();
		}
	}
};

class LLEmptyTrashFloater : public inventory_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLInventoryModel* model = mPtr->getPanel()->getModel();
		if(!model) return false;
		LLUUID trash_id = model->findCategoryUUIDForType(LLAssetType::AT_TRASH);
		model->purgeDescendentsOf(trash_id);
		model->notifyObservers();
		return true;
	}
};

void do_create(LLInventoryModel *model, LLInventoryPanel *ptr, LLString type, LLFolderBridge *self = NULL)
{
	if ("category" == type)
	{
		if (self)
		{
			model->createNewCategory(self->getUUID(), LLAssetType::AT_NONE, NULL);
		}
		else
		{
			model->createNewCategory(gAgent.getInventoryRootID(),
									LLAssetType::AT_NONE, NULL);
		}
		model->notifyObservers();
	}
	else if ("lsl" == type)
	{
		LLUUID parent_id = self ? self->getUUID() : model->findCategoryUUIDForType(LLAssetType::AT_LSL_TEXT);
		ptr->createNewItem(NEW_LSL_NAME,
							parent_id,
							LLAssetType::AT_LSL_TEXT,
							LLInventoryType::IT_LSL,
							PERM_MOVE | PERM_TRANSFER);
	}
	else if ("notecard" == type)
	{
		LLUUID parent_id = self ? self->getUUID() : model->findCategoryUUIDForType(LLAssetType::AT_NOTECARD);
		ptr->createNewItem(NEW_NOTECARD_NAME,
							parent_id,
							LLAssetType::AT_NOTECARD,
							LLInventoryType::IT_NOTECARD,
							PERM_ALL);
	}
	else if ("gesture" == type)
	{
		LLUUID parent_id = self ? self->getUUID() : model->findCategoryUUIDForType(LLAssetType::AT_GESTURE);
		ptr->createNewItem(NEW_GESTURE_NAME,
							parent_id,
							LLAssetType::AT_GESTURE,
							LLInventoryType::IT_GESTURE,
							PERM_ALL);
	}
	else if ("shirt" == type)
	{
		LLUUID parent_id = self ? self->getUUID() : gInventory.findCategoryUUIDForType(LLAssetType::AT_CLOTHING);
		LLFolderBridge::createWearable(parent_id, WT_SHIRT);
	}
	else if ("pants" == type)
	{
		LLUUID parent_id = self ? self->getUUID() : gInventory.findCategoryUUIDForType(LLAssetType::AT_CLOTHING);
		LLFolderBridge::createWearable(parent_id, WT_PANTS);
	}
	else if ("shoes" == type)
	{
		LLUUID parent_id = self ? self->getUUID() : gInventory.findCategoryUUIDForType(LLAssetType::AT_CLOTHING);
		LLFolderBridge::createWearable(parent_id, WT_SHOES);
	}
	else if ("socks" == type)
	{
		LLUUID parent_id = self ? self->getUUID() : gInventory.findCategoryUUIDForType(LLAssetType::AT_CLOTHING);
		LLFolderBridge::createWearable(parent_id, WT_SOCKS);
	}
	else if ("jacket" == type)
	{
		LLUUID parent_id = self ? self->getUUID() : gInventory.findCategoryUUIDForType(LLAssetType::AT_CLOTHING);
		LLFolderBridge::createWearable(parent_id, WT_JACKET);
	}
	else if ("skirt" == type)
	{
		LLUUID parent_id = self ? self->getUUID() : gInventory.findCategoryUUIDForType(LLAssetType::AT_CLOTHING);
		LLFolderBridge::createWearable(parent_id, WT_SKIRT);
	}
	else if ("gloves" == type)
	{
		LLUUID parent_id = self ? self->getUUID() : gInventory.findCategoryUUIDForType(LLAssetType::AT_CLOTHING);
		LLFolderBridge::createWearable(parent_id, WT_GLOVES);
	}
	else if ("undershirt" == type)
	{
		LLUUID parent_id = self ? self->getUUID() : gInventory.findCategoryUUIDForType(LLAssetType::AT_CLOTHING);
		LLFolderBridge::createWearable(parent_id, WT_UNDERSHIRT);
	}
	else if ("underpants" == type)
	{
		LLUUID parent_id = self ? self->getUUID() : gInventory.findCategoryUUIDForType(LLAssetType::AT_CLOTHING);
		LLFolderBridge::createWearable(parent_id, WT_UNDERPANTS);
	}
	else if ("shape" == type)
	{
		LLUUID parent_id = self ? self->getUUID() : gInventory.findCategoryUUIDForType(LLAssetType::AT_BODYPART);
		LLFolderBridge::createWearable(parent_id, WT_SHAPE);
	}
	else if ("skin" == type)
	{
		LLUUID parent_id = self ? self->getUUID() : gInventory.findCategoryUUIDForType(LLAssetType::AT_BODYPART);
		LLFolderBridge::createWearable(parent_id, WT_SKIN);
	}
	else if ("hair" == type)
	{
		LLUUID parent_id = self ? self->getUUID() : gInventory.findCategoryUUIDForType(LLAssetType::AT_BODYPART);
		LLFolderBridge::createWearable(parent_id, WT_HAIR);
	}
	else if ("eyes" == type)
	{
		LLUUID parent_id = self ? self->getUUID() : gInventory.findCategoryUUIDForType(LLAssetType::AT_BODYPART);
		LLFolderBridge::createWearable(parent_id, WT_EYES);
	}
}

class LLDoCreate : public inventory_panel_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLInventoryModel* model = mPtr->getModel();
		if(!model) return false;
		LLString type = userdata.asString();
		do_create(model, mPtr, type, LLFolderBridge::sSelf);
		return true;
	}
};

class LLDoCreateFloater : public inventory_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLInventoryModel* model = mPtr->getPanel()->getModel();
		if(!model) return false;
		LLString type = userdata.asString();
		do_create(model, mPtr->getPanel(), type);
		return true;
	}
};

class LLSetSortBy : public inventory_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLString sort_field = userdata.asString();
		if (sort_field == "name")
		{
			U32 order = mPtr->getActivePanel()->getSortOrder();
			mPtr->getActivePanel()->setSortOrder( order & ~LLInventoryFilter::SO_DATE );
			
			mPtr->getControl("Inventory.SortByName")->setValue( TRUE );
			mPtr->getControl("Inventory.SortByDate")->setValue( FALSE );
		}
		else if (sort_field == "date")
		{
			U32 order = mPtr->getActivePanel()->getSortOrder();
			mPtr->getActivePanel()->setSortOrder( order | LLInventoryFilter::SO_DATE );

			mPtr->getControl("Inventory.SortByName")->setValue( FALSE );
			mPtr->getControl("Inventory.SortByDate")->setValue( TRUE );
		}
		else if (sort_field == "foldersalwaysbyname")
		{
			U32 order = mPtr->getActivePanel()->getSortOrder();
			if ( order & LLInventoryFilter::SO_FOLDERS_BY_NAME )
			{
				order &= ~LLInventoryFilter::SO_FOLDERS_BY_NAME;

				mPtr->getControl("Inventory.FoldersAlwaysByName")->setValue( FALSE );
			}
			else
			{
				order |= LLInventoryFilter::SO_FOLDERS_BY_NAME;

				mPtr->getControl("Inventory.FoldersAlwaysByName")->setValue( TRUE );
			}
			mPtr->getActivePanel()->setSortOrder( order );
		}

		return true;
	}
};

class LLBeginIMSession : public inventory_panel_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLString who = userdata.asString();
		bool only_online = false;
		if ("everyone" == who)
		{
			only_online = false;
		}
		else // "online"
		{
			only_online = true;
		}
		LLInventoryPanel *panel = mPtr;
		LLInventoryModel* model = panel->getModel();
		if(!model) return true;
		std::set<LLUUID> selected_items;
		panel->getRootFolder()->getSelectionList(selected_items);
		LLUUID item = *selected_items.begin();
		LLFolderViewItem* folder_item = panel->getRootFolder()->getItemByID(item);
		if(!folder_item) return true;
		LLFolderBridge* bridge = (LLFolderBridge*)folder_item->getListener();
		if(!bridge) return true;
		LLViewerInventoryCategory* cat = bridge->getCategory();
		if(!cat) return true;
		LLUniqueBuddyCollector is_buddy;
		LLInventoryModel::cat_array_t cat_array;
		LLInventoryModel::item_array_t item_array;
		model->collectDescendentsIf(bridge->getUUID(),
									cat_array,
									item_array,
									LLInventoryModel::EXCLUDE_TRASH,
									is_buddy);
		S32 count = item_array.count();
		if(count > 0)
		{
			// create the session
			gIMView->setFloaterOpen(TRUE);
			LLDynamicArray<LLUUID> members;
			//members.put(gAgent.getID());
			S32 i;
			EInstantMessage type = IM_SESSION_ADD;
			if(only_online)
			{
				LLAvatarTracker& at = LLAvatarTracker::instance();
				LLUUID id;
				for(i = 0; i < count; ++i)
				{
					id = item_array.get(i)->getCreatorUUID();
					if(at.isBuddyOnline(id))
					{
						members.put(id);
					}
				}
			}
			else
			{
				type = IM_SESSION_OFFLINE_ADD;
				for(i = 0; i < count; ++i)
				{
					members.put(item_array.get(i)->getCreatorUUID());
				}
			}
			// the session_id is always the item_id of the inventory folder
			gIMView->addSession(cat->getName(),
								type,
								bridge->getUUID(),
								members);
		}
		return true;
	}
};

void rez_attachment(LLViewerInventoryItem* item, LLViewerJointAttachment* attachment);

class LLAttachObject : public inventory_panel_listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		LLInventoryPanel *panel = mPtr;
		LLFolderView* folder = panel->getRootFolder();
		if(!folder) return true;

		std::set<LLUUID> selected_items;
		folder->getSelectionList(selected_items);
		LLUUID id = *selected_items.begin();

		LLString joint_name = userdata.asString();
		LLVOAvatar *avatarp = gAgent.getAvatarObject();
		LLViewerJointAttachment* attachmentp = NULL;
		for (LLViewerJointAttachment* attachment = avatarp->mAttachmentPoints.getFirstData(); 
			attachment;
			attachment = gAgent.getAvatarObject()->mAttachmentPoints.getNextData())
		{
			if (attachment->getName() == joint_name)
			{
				attachmentp = attachment;
				break;
			}
		}
		if (attachmentp == NULL)
		{
			return true;
		}
		LLViewerInventoryItem* item = (LLViewerInventoryItem*)gInventory.getItem(id);

		if(item && gInventory.isObjectDescendentOf(id, gAgent.getInventoryRootID()))
		{
			rez_attachment(item, attachmentp);
		}
		else if(item && item->isComplete())
		{
			// must be in library. copy it to our inventory and put it on.
			LLPointer<LLInventoryCallback> cb = new RezAttachmentCallback(attachmentp);
			copy_inventory_item(
				gAgent.getID(),
				item->getPermissions().getOwner(),
				item->getUUID(),
				LLUUID::null,
				std::string(),
				cb);
		}
		gFocusMgr.setKeyboardFocus(NULL, NULL);

		return true;
	}
};

/*
class LL : public listener_t
{
	bool handleEvent(LLPointer<LLEvent> event, const LLSD& userdata)
	{
		return true;
	}
};
*/

void init_object_inventory_panel_actions(LLPanelInventory *panel)
{
	(new LLDoToSelectedPanel())->registerListener(panel, "Inventory.DoToSelected");
}

void init_inventory_actions(LLInventoryView *floater)
{
	(new LLDoToSelectedFloater())->registerListener(floater, "Inventory.DoToSelected");
	(new LLCloseAllFoldersFloater())->registerListener(floater, "Inventory.CloseAllFolders");
	(new LLEmptyTrashFloater())->registerListener(floater, "Inventory.EmptyTrash");
	(new LLDoCreateFloater())->registerListener(floater, "Inventory.DoCreate");

	(new LLNewWindow())->registerListener(floater, "Inventory.NewWindow");
	(new LLShowFilters())->registerListener(floater, "Inventory.ShowFilters");
	(new LLResetFilter())->registerListener(floater, "Inventory.ResetFilter");
	(new LLSetSortBy())->registerListener(floater, "Inventory.SetSortBy");
}

void init_inventory_panel_actions(LLInventoryPanel *panel)
{
	(new LLDoToSelected())->registerListener(panel, "Inventory.DoToSelected");
	(new LLAttachObject())->registerListener(panel, "Inventory.AttachObject");
	(new LLCloseAllFolders())->registerListener(panel, "Inventory.CloseAllFolders");
	(new LLEmptyTrash())->registerListener(panel, "Inventory.EmptyTrash");
	(new LLEmptyLostAndFound())->registerListener(panel, "Inventory.EmptyLostAndFound");
	(new LLDoCreate())->registerListener(panel, "Inventory.DoCreate");
	(new LLBeginIMSession())->registerListener(panel, "Inventory.BeginIMSession");
}
