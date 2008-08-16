/** 
 * @file llvograss.cpp
 * @brief Not a blade, but a clump of grass
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

#include "llvograss.h"

#include "imageids.h"
#include "llviewercontrol.h"

#include "llagent.h"
#include "llviewerwindow.h"
#include "lldrawable.h"
#include "llface.h"
#include "llsky.h"
#include "llsurface.h"
#include "llsurfacepatch.h"
#include "llvosky.h"
#include "llviewercamera.h"
#include "llviewerimagelist.h"
#include "llviewerregion.h"
#include "pipeline.h"
#include "llworld.h"
#include "lldir.h"
#include "llxmltree.h"

const S32 GRASS_MAX_BLADES =	32;
const F32 GRASS_BLADE_BASE =	0.25f;			//  Width of grass at base
const F32 GRASS_BLADE_TOP =		0.25f;			//  Width of grass at top
const F32 GRASS_BLADE_HEIGHT =	0.5f;			// meters
const F32 GRASS_DISTRIBUTION_SD = 0.15f;		// empirically defined

F32 exp_x[GRASS_MAX_BLADES];
F32 exp_y[GRASS_MAX_BLADES];
F32 rot_x[GRASS_MAX_BLADES];
F32 rot_y[GRASS_MAX_BLADES];
F32 dz_x [GRASS_MAX_BLADES];
F32 dz_y [GRASS_MAX_BLADES];

F32 w_mod[GRASS_MAX_BLADES];					//  Factor to modulate wind movement by to randomize appearance

LLVOGrass::SpeciesMap LLVOGrass::sSpeciesTable;
S32 LLVOGrass::sMaxGrassSpecies = 0;


LLVOGrass::LLVOGrass(const LLUUID &id, const LLPCode pcode, LLViewerRegion *regionp)
:	LLViewerObject(id, pcode, regionp)
{
	mPatch               = NULL;
	mLastPatchUpdateTime = 0;
	mGrassVel.clearVec();
	mGrassBend.clearVec();
	mbCanSelect          = TRUE;

	mBladeWindAngle      = 35.f;
	mBWAOverlap          = 2.f;

	setNumTEs(1);

	setTEColor(0, LLColor4(1.0f, 1.0f, 1.0f, 1.f));
	mNumBlades = GRASS_MAX_BLADES;
}

LLVOGrass::~LLVOGrass()
{
}


void LLVOGrass::updateSpecies()
{
	mSpecies = mState;
	
	if (!sSpeciesTable.count(mSpecies))
	{
		llinfos << "Unknown grass type, substituting grass type." << llendl;
		SpeciesMap::const_iterator it = sSpeciesTable.begin();
		mSpecies = (*it).first;
	}
	setTEImage(0, gImageList.getImage(sSpeciesTable[mSpecies]->mTextureID));
}


void alert_done(S32 option, void* user_data)
{
	return;
}


void LLVOGrass::initClass()
{
	LLVector3 pos(0.0f, 0.0f, 0.0f);
	//  Create nifty list of exponential distribution 0-1
	F32 x = 0.f;
	F32 y = 0.f;
	F32 rot;
	
	std::string xml_filename = gDirUtilp->getExpandedFilename(LL_PATH_APP_SETTINGS,"grass.xml");
	
	LLXmlTree grass_def_grass;

	if (!grass_def_grass.parseFile(xml_filename))
	{
		llerrs << "Failed to parse grass file." << llendl;
		return;
	}

	LLXmlTreeNode* rootp = grass_def_grass.getRoot();

	for (LLXmlTreeNode* grass_def = rootp->getFirstChild();
		grass_def;
		grass_def = rootp->getNextChild())
	{
		if (!grass_def->hasName("grass"))
		{
			llwarns << "Invalid grass definition node " << grass_def->getName() << llendl;
			continue;
		}
		F32 F32_val;
		LLUUID id;

		BOOL success = TRUE;

		S32 species;
		static LLStdStringHandle species_id_string = LLXmlTree::addAttributeString("species_id");
		if (!grass_def->getFastAttributeS32(species_id_string, species))
		{
			llwarns << "No species id defined" << llendl;
			continue;
		}

		if (species < 0)
		{
			llwarns << "Invalid species id " << species << llendl;
			continue;
		}

		GrassSpeciesData* newGrass = new GrassSpeciesData();


		static LLStdStringHandle texture_id_string = LLXmlTree::addAttributeString("texture_id");
		grass_def->getFastAttributeUUID(texture_id_string, id);
		newGrass->mTextureID = id;

		if (newGrass->mTextureID.isNull())
		{
			LLString textureName;

			static LLStdStringHandle texture_name_string = LLXmlTree::addAttributeString("texture_name");
			success &= grass_def->getFastAttributeString(texture_name_string, textureName);
			newGrass->mTextureID.set( gViewerArt.getString(textureName) );
		}

		static LLStdStringHandle blade_sizex_string = LLXmlTree::addAttributeString("blade_size_x");
		success &= grass_def->getFastAttributeF32(blade_sizex_string, F32_val);
		newGrass->mBladeSizeX = F32_val;

		static LLStdStringHandle blade_sizey_string = LLXmlTree::addAttributeString("blade_size_y");
		success &= grass_def->getFastAttributeF32(blade_sizey_string, F32_val);
		newGrass->mBladeSizeY = F32_val;

		if (sSpeciesTable.count(species))
		{
			llinfos << "Grass species " << species << " already defined! Duplicate discarded." << llendl;
			delete newGrass;
			continue;
		}
		else
		{
			sSpeciesTable[species] = newGrass;
		}

		if (species >= sMaxGrassSpecies) sMaxGrassSpecies = species + 1;

		if (!success)
		{
			LLString name;
			static LLStdStringHandle name_string = LLXmlTree::addAttributeString("name");
			grass_def->getFastAttributeString(name_string, name);
			llwarns << "Incomplete definition of grass " << name << llendl;
		}
	}

	BOOL have_all_grass = TRUE;
	LLString err;
	char buffer[10];

	for (S32 i=0;i<sMaxGrassSpecies;++i)
	{
		if (!sSpeciesTable.count(i))
		{
			snprintf(buffer,10," %d",i);
			err.append(buffer);
			have_all_grass = FALSE;
		}
	}

	if (!have_all_grass) 
	{
		LLStringBase<char>::format_map_t args;
		args["[SPECIES]"] = err;
		gViewerWindow->alertXml("ErrorUndefinedGrasses", args, alert_done );
	}

	for (S32 i = 0; i < GRASS_MAX_BLADES; ++i)
	{
		if (1)   //(i%2 == 0)			Uncomment for X blading
		{
			F32 u = sqrt(-2.0f * log(ll_frand()));
			F32 v = 2.0f * F_PI * ll_frand();
			
			x = u * sin(v) * GRASS_DISTRIBUTION_SD;
			y = u * cos(v) * GRASS_DISTRIBUTION_SD;

			rot = ll_frand(F_PI);
		}
		else
		{
			rot += (F_PI*0.4f + ll_frand(0.2f*F_PI));
		}

		exp_x[i] = x;
		exp_y[i] = y;
		rot_x[i] = sin(rot);
		rot_y[i] = cos(rot);
		dz_x[i] = ll_frand(GRASS_BLADE_BASE * 0.25f);
		dz_y[i] = ll_frand(GRASS_BLADE_BASE * 0.25f);
		w_mod[i] = 0.5f + ll_frand();						//  Degree to which blade is moved by wind

	}
}

void LLVOGrass::cleanupClass()
{
	for_each(sSpeciesTable.begin(), sSpeciesTable.end(), DeletePairedPointer());
}

U32 LLVOGrass::processUpdateMessage(LLMessageSystem *mesgsys,
										  void **user_data,
										  U32 block_num,
										  const EObjectUpdateType update_type,
										  LLDataPacker *dp)
{
	// Do base class updates...
	U32 retval = LLViewerObject::processUpdateMessage(mesgsys, user_data, block_num, update_type, dp);

	updateSpecies();

	if (  (getVelocity().magVecSquared() > 0.f)
		||(getAcceleration().magVecSquared() > 0.f)
		||(getAngularVelocity().magVecSquared() > 0.f))
	{
		llinfos << "ACK! Moving grass!" << llendl;
		setVelocity(LLVector3::zero);
		setAcceleration(LLVector3::zero);
		setAngularVelocity(LLVector3::zero);
	}

	if (mDrawable)
	{
		gPipeline.markRebuild(mDrawable, LLDrawable::REBUILD_VOLUME, TRUE);
	}

	return retval;
}

BOOL LLVOGrass::isActive() const
{
	return TRUE;
}

BOOL LLVOGrass::idleUpdate(LLAgent &agent, LLWorld &world, const F64 &time)
{
 	if (mDead || !(gPipeline.hasRenderType(LLPipeline::RENDER_TYPE_GRASS)))
	{
		return TRUE;
	}
	
	if (!mDrawable)
	{
		// So drones work.
		return TRUE;
	}

	if (mPatch && (mLastPatchUpdateTime != mPatch->getLastUpdateTime()))
	{
		gPipeline.markRebuild(mDrawable, LLDrawable::REBUILD_VOLUME, TRUE);
	}

	return TRUE;
}


void LLVOGrass::setPixelAreaAndAngle(LLAgent &agent)
{
	// This should be the camera's center, as soon as we move to all region-local.
	LLVector3 relative_position = getPositionAgent() - agent.getCameraPositionAgent();
	F32 range = relative_position.magVec();				// ugh, square root

	F32 max_scale = getMaxScale();

	mAppAngle = (F32) atan2( max_scale, range) * RAD_TO_DEG;

	// Compute pixels per meter at the given range
	F32 pixels_per_meter = gCamera->getViewHeightInPixels() / 
						(tan(gCamera->getView()) * range);

	// Assume grass texture is a 1 meter by 1 meter sprite at the grass object's center
	mPixelArea = (pixels_per_meter) * (pixels_per_meter);
}


// BUG could speed this up by caching the relative_position and range calculations
void LLVOGrass::updateTextures(LLAgent &agent)
{
	// dot_product = A B cos T
	// BUT at_axis is unit, so dot_product = B cos T
	LLVector3 relative_position = getPositionAgent() - agent.getCameraPositionAgent();
	F32 dot_product = relative_position * agent.getFrameAgent().getAtAxis();
	F32 cos_angle = dot_product / relative_position.magVec();

	if (getTEImage(0))
	{
		getTEImage(0)->addTextureStats(mPixelArea*20.f, 1.f, cos_angle);
	}
}

BOOL LLVOGrass::updateLOD()
{
	F32 tan_angle = 0.f;
	S32 num_blades = 0;

	tan_angle = (mScale.mV[0]*mScale.mV[1])/mDrawable->mDistanceWRTCamera;
	num_blades = llmin(GRASS_MAX_BLADES, lltrunc(tan_angle * 5));
	num_blades = llmax(1, num_blades);
	if (num_blades >= (mNumBlades << 1))
	{
		while (mNumBlades < num_blades)
		{
			mNumBlades <<= 1;
		}

		gPipeline.markRebuild(mDrawable, LLDrawable::REBUILD_ALL, TRUE);
	}
	else if (num_blades <= (mNumBlades >> 1))
	{
		while (mNumBlades > num_blades)
		{
			mNumBlades >>=1;
		}

		gPipeline.markRebuild(mDrawable, LLDrawable::REBUILD_ALL, TRUE);
		return TRUE;
	}

	return FALSE;
}

LLDrawable* LLVOGrass::createDrawable(LLPipeline *pipeline)
{
	pipeline->allocDrawable(this);
// 	mDrawable->setLit(FALSE);
	mDrawable->setRenderType(LLPipeline::RENDER_TYPE_GRASS);

	LLDrawPool *pool = gPipeline.getPool(LLDrawPool::POOL_ALPHA);

	mDrawable->setNumFaces(1, pool, getTEImage(0));

	return mDrawable;
}

BOOL LLVOGrass::updateGeometry(LLDrawable *drawable)
{
	plantBlades();
	return TRUE;
}

void LLVOGrass::plantBlades()
{
	mPatch               = mRegionp->getLand().resolvePatchRegion(getPositionRegion());
	mLastPatchUpdateTime = mPatch->getLastUpdateTime();
	
	LLVector3 position;
	// Create random blades of grass with gaussian distribution
	F32 x,y,xf,yf,dzx,dzy;

	LLVector3 normal(0,0,1);
	LLColor4U color(0,0,0,1);

	LLFace *face = mDrawable->getFace(0);

	LLStrider<LLVector3> verticesp;
	LLStrider<LLVector3> normalsp;
	LLStrider<LLVector2> texCoordsp;
	LLStrider<LLColor4U> colorsp;

	U32 *indicesp;

	face->setPool(face->getPool(), getTEImage(0));
	face->setState(LLFace::GLOBAL);
	face->setSize(mNumBlades * 4, mNumBlades * 12);
	face->setPrimType(LLTriangles);
	S32 index_offset = face->getGeometryColors(verticesp,normalsp,texCoordsp,colorsp,indicesp);
	if (-1 == index_offset)
	{
		return;
	}

	// It is possible that the species of a grass is not defined
	// This is bad, but not the end of the world.
	if (!sSpeciesTable.count(mSpecies))
	{
		llinfos << "Unknown grass species " << mSpecies << llendl;
		return;
	}
	
	F32 width  = sSpeciesTable[mSpecies]->mBladeSizeX;
	F32 height = sSpeciesTable[mSpecies]->mBladeSizeY;

	for (S32 i = 0;  i < mNumBlades; i++)
	{
		x   = exp_x[i] * mScale.mV[VX];
		y   = exp_y[i] * mScale.mV[VY];
		xf  = rot_x[i] * GRASS_BLADE_BASE * width * w_mod[i];
		yf  = rot_y[i] * GRASS_BLADE_BASE * width * w_mod[i];
		dzx = dz_x [i];
		dzy = dz_y [i];

		F32 blade_height= GRASS_BLADE_HEIGHT * height * w_mod[i];

		*texCoordsp++   = LLVector2(0, 0);
		*texCoordsp++   = LLVector2(0, 0.98f);
		*texCoordsp++   = LLVector2(1, 0);
		*texCoordsp++   = LLVector2(1, 0.98f);

		position.mV[0]  = mPosition.mV[VX] + x + xf;
		position.mV[1]  = mPosition.mV[VY] + y + yf;
		position.mV[2]  = 0.f;
		position.mV[2]  = mRegionp->getLand().resolveHeightRegion(position);
		*verticesp++    = position + mRegionp->getOriginAgent();

		position.mV[0] += dzx;
		position.mV[1] += dzy;
		position.mV[2] += blade_height;
		*verticesp++    = position + mRegionp->getOriginAgent();

		position.mV[0]  = mPosition.mV[VX] + x - xf;
		position.mV[1]  = mPosition.mV[VY] + y - xf;
		position.mV[2]  = 0.f;
		position.mV[2]  = mRegionp->getLand().resolveHeightRegion(position);
		*verticesp++    = position + mRegionp->getOriginAgent();

		position.mV[0] += dzx;
		position.mV[1] += dzy;
		position.mV[2] += blade_height;
		*verticesp++    = position + mRegionp->getOriginAgent();

		*(normalsp++)   = normal;
		*(normalsp++)   = normal;
		*(normalsp++)   = normal;
		*(normalsp++)   = normal;

		*(colorsp++)   = color;
		*(colorsp++)   = color;
		*(colorsp++)   = color;
		*(colorsp++)   = color;
		
		*indicesp++     = index_offset + 0;
		*indicesp++     = index_offset + 1;
		*indicesp++     = index_offset + 2;

		*indicesp++     = index_offset + 1;
		*indicesp++     = index_offset + 3;
		*indicesp++     = index_offset + 2;

		*indicesp++     = index_offset + 0;
		*indicesp++     = index_offset + 2;
		*indicesp++     = index_offset + 1;

		*indicesp++     = index_offset + 1;
		*indicesp++     = index_offset + 2;
		*indicesp++     = index_offset + 3;
		index_offset   += 4;
	}

	LLPipeline::sCompiles++;

	face->mCenterLocal = mPosition;

}





