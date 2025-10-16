#include "SADXModLoader.h"
#include "Keys.h"
#include "dirent.h"
#include "ObjectImport.h"

#include <string>

Sint32 _pno;


enum {
	COLLI_TYPE_NONE,
	COLLI_TYPE_FIXED,
	COLLI_TYPE_DYNAMIC,
	COLLI_TYPE_MOV_S,
	COLLI_TYPE_MOV_M,
	COLLI_TYPE_MOV_L,
	COLLI_TYPE_MOV_SW,
	COLLI_TYPE_MAX
};

std::string colli_string[] = {
	"NONE",
	"FIXED BOX",
	"DYNAMIC/COLLISION FILE",
	"HOLDABLE",
	"PUSH/PULL",
	"LIFTABLE BY BIG",
	"CHAO GARDEN TREE",
};

enum {
	PROP_MODE_INIT,
	PROP_MODE_NORMAL,
	PROP_MODE_MOV_S,
	PROP_MODE_MOV_M,
	PROP_MODE_MOV_L,
	PROP_MODE_MOV_SW,
	PROP_MODE_MAX
};

static CCL_INFO fixed_colli[] = {
	{  0, CI_FORM_RECTANGLE, 0x77, 0, 0x2400, { 0.0f, 5.0f, 0.0f }, 5.0f, 5.0f, 5.0f, 0.0f, 0, 0, 0 }
};

static CCL_INFO pickup_colli[] = {
	{ 0, CI_FORM_CYLINDER, 0x70, 0, 0x2400, { 0.0f, 3.0f, 0.0f }, 5.5f, 3.0f, 0.0f, 0.0f, 0, 0, 0 },
	{ 0, CI_FORM_CYLINDER, 0x77, (char)0xE0, 0x2000, { 0.0f, 3.0f, 0.0f }, 2.0f, 3.0f, 0.0f, 0.0f, 0, 0, 0 },
	{ 0, CI_FORM_CYLINDER, 0x70, 0, 0, { 0.0f, 5.0f, 0.0f }, 18.2f, 6.0f, 0.0f, 0.0f, 0, 0, 0 },
	{ 0, CI_FORM_CYLINDER, 0x77, 0, 0, { 0.0f, 4.5f, 0.0f }, 2.6f, 6.0f, 0.0f, 0.0f, 0, 0, 0 },
	{ 0, CI_FORM_CYLINDER, 0x70, 0, 0, { 0.0f, 15.0f, 0.0f }, 22.0f, 15.0f, 0.0f, 0.0f, 0, 0, 0 },
	{ 0, CI_FORM_CYLINDER, 0x77, 0, 0, { 0.0f, 15.0f, 0.0f }, 12.6f, 15.0f, 0.0f, 0.0f, 0, 0, 0 },
	{ 0, CI_FORM_CYLINDER, 0x70, 0, 0, { 0.0f, 20.0f, 0.0f }, 1.2f, 20.0f, 0.0f, 0.0f, 0, 0, 0 },
	{ 0, CI_FORM_CYLINDER, 0x77, 0, 0, { 0.0f, 20.0f, 0.0f }, 0.1f, 20.0f, 0.0f, 0.0f, 0, 0, 0 }
};

static Sint32 propID;
static Sint32 prop_EditID = 1;

//Prop limit
#define PROP_LIMIT 255

//RegisterCollisionEntry issues.
#define DYNCOLLI_LIMIT 50

std::string PropFolderPath;
std::string selectedPropFolder;
LPVOID* data[PROP_LIMIT];
LPVOID* coldata[PROP_LIMIT];
LPVOID* animdata[PROP_LIMIT];

//I should have probably made a struct but whatever lol
NJS_MOTION* mtn_list[PROP_LIMIT];
NJS_OBJECT* prop_objList[PROP_LIMIT];
NJS_OBJECT* col_objList[PROP_LIMIT];
NJS_TEXLIST* prop_texlists[PROP_LIMIT];
task* prop_taskList[PROP_LIMIT];
Sint32 obj_num;
Sint32 texlist_num;


static Float shadow_scale = 1.0f;
static Sint32 colli_type;
static Sint32 colli_count;
static Sint32 anim_no = 0;
static NJS_POINT3 prop_scale = { 1.0f, 1.0f, 1.0f };

/// <summary>
/// HUD for Prop Spawner
/// </summary>
/// 
void displayPropInfo(Sint32 col, Sint32 pno) {
	njPrintC(NJM_LOCATION(1, col++), "Mode: Prop Spawner");
	col++;
	njPrintC(NJM_LOCATION(2, col++), "Spawn a custom model as a prop.");
	njPrintC(NJM_LOCATION(2, col++), "Z Button (RB) = Spawn");
	njPrintC(NJM_LOCATION(2, col++), "Y+Z Button (RB) = Delete");
	njPrintC(NJM_LOCATION(2, col++), "/ = Clear All");
	col++;
	njPrint(NJM_LOCATION(2, col++), "Props spawned: %d", obj_num, PROP_LIMIT);
	col++;
	Sint32 propArrow = col + prop_EditID;
	njPrintC(NJM_LOCATION(1, propArrow), ">");
	njPrintC(NJM_LOCATION(2, col++), "BACK");
	njPrint(NJM_LOCATION(2, col++), "Prop name: %s", selectedPropFolder.c_str());
	njPrint(NJM_LOCATION(2, col++), "Shadow scale: %.2f", shadow_scale);
	njPrint(NJM_LOCATION(2, col++), "Collision Type: %s", colli_string[colli_type].c_str());
	njPrint(NJM_LOCATION(2, col++), "Animation (if has): %s", anim_no ? std::to_string(anim_no).c_str() : "NONE");
	njPrint(NJM_LOCATION(2, col++), "SCL X: %.2f", prop_scale.x);
	njPrint(NJM_LOCATION(2, col++), "SCL Y: %.2f", prop_scale.y);
	njPrint(NJM_LOCATION(2, col++), "SCL Z: %.2f", prop_scale.z);
}

/// <summary>
/// Sets the prop folder location
/// </summary>
/// <param name="folderpath"></param>
void setPropFolder(std::string folderpath) {
	PropFolderPath = folderpath;
}

/// <summary>
/// Dest for instance of prop task
/// </summary>
/// <param name="tp"></param>
static void propObject_dest(task* tp) {
	taskwk* twp = tp->twp;
	prop_taskList[twp->btimer] = 0;

	//It's safer to destroy the file data here
	if (data[twp->btimer]) {
		syFree(*data[twp->btimer]);
		data[twp->btimer] = 0;
	}

	if (animdata[twp->btimer]) {
		syFree(*animdata[twp->btimer]);
		animdata[twp->btimer] = 0;
	}

	if (mtn_list[twp->btimer])
		mtn_list[twp->btimer] = 0;
	if (prop_objList) {
		prop_objList[twp->btimer] = 0;
	}

	if (prop_texlists[twp->btimer]) {
		Free(prop_texlists[twp->btimer]);
		prop_texlists[twp->btimer] = 0;
	}


	if (twp->counter.ptr)
	{
		WithdrawCollisionEntry(tp, (NJS_OBJECT*)twp->counter.ptr);
		ReleaseMobileLandObject((NJS_OBJECT*)twp->counter.ptr);

		if (coldata[twp->btimer]) {
			syFree(*coldata[twp->btimer]);
			coldata[twp->btimer] = 0;
		}

		if (col_objList) {
			col_objList[twp->btimer] = 0;
		}

		--colli_count;
	}

	--obj_num;


}

/// <summary>
/// Disp for instance of prop task
/// </summary>
/// <param name="tp"></param>
static void propObject_disp(task* tp) {
	taskwk* twp = tp->twp;

	if (!loop_count) {
		njSetTexture((NJS_TEXLIST*)twp->timer.ptr);
		njPushMatrix(0);

		//Model
		njPushMatrix(0);
		njTranslateV(0, &twp->pos);
		if (twp->ang.z)
			njRotateZ(0, (Uint16)twp->ang.z);
		if (twp->ang.y)
			njRotateY(0, (Uint16)twp->ang.y);
		if (twp->ang.x)
			njRotateX(0, (Uint16)twp->ang.x);
		njScaleV(0, &twp->scl);

		//Animation
		if (mtn_list[twp->btimer]) {

			if (++twp->wtimer >= mtn_list[twp->btimer]->nbFrame)
				twp->wtimer = 0;

			dsDrawMotion(prop_objList[twp->btimer], mtn_list[twp->btimer], (Float)twp->wtimer);
		}
		else {
			dsDrawObject(prop_objList[twp->btimer]);
		}
		njPopMatrix(1);

		//Shadow model
		njPushMatrix(0);
		SetRegularTexture();
		NJS_POINT3 shadowpos = twp->pos;
		Angle3 ang;
		shadowpos.y = GetShadowPos(twp->pos.x, twp->pos.y + 1.0f, twp->pos.z, &ang) + 0.3f;
		njTranslateV(0, &shadowpos);
		if (ang.z)
			njRotateZ(0, (Uint16)ang.z);
		if (ang.x)
			njRotateX(0, (Uint16)ang.x);

		njScale(0, twp->value.f, twp->value.f, twp->value.f);

		late_DrawShadowObject(object_shadow, 1.0f);
		njPopMatrix(1);

		njPopMatrix(1);
	}
}

/// <summary>
/// Exec for instance of prop task
/// </summary>
/// <param name="tp"></param>
static void propObject_exec(task* tp) {
	taskwk* twp = tp->twp;

	switch (twp->mode) {
	case PROP_MODE_NORMAL:
		EntryColliList(twp);
		break;
	case PROP_MODE_MOV_S:
		ObjectMovableSRegularExecute(tp);

		if (twp->flag & 0x1000) {

			for (Sint32 i = 0; i < 8; ++i) {
				if (playerpwp[i] && playerpwp[i]->htp == tp) {
					twp->ang.y = 0x4000 - playertwp[i]->ang.y;
					twp->ang.x = playertwp[i]->ang.x;
					twp->ang.z = playertwp[i]->ang.z;
				}
			}

			
		}
		break;
	case PROP_MODE_MOV_M:
	case PROP_MODE_MOV_L:
		ObjectMovableMRegularExecute(tp);
		break;
	case PROP_MODE_MOV_SW:
		ObjectSwingableRegularExecute(tp);
		break;
	}

	propObject_disp(tp);
}

/// <summary>
/// Instance of prop task
/// </summary>
/// <param name="tp"></param>
void propObject(task* tp) {
	taskwk* twp = tp->twp;

	switch (twp->mode) {
	case PROP_MODE_INIT:
	{
		NJS_OBJECT* propObj = prop_objList[twp->btimer];
		twp->pos = playertwp[_pno]->pos;
		twp->ang.x = (Uint16)propObj->ang[0] + (Uint16)playertwp[_pno]->ang.x;
		twp->ang.y = (Uint16)propObj->ang[1] + 0x4000 - (Uint16)playertwp[_pno]->ang.y;
		twp->ang.z = (Uint16)propObj->ang[2] + (Uint16)playertwp[_pno]->ang.z;
		twp->scl = *(NJS_POINT3*)propObj->scl;
		twp->scl.x *= prop_scale.x;
		twp->scl.y *= prop_scale.y;
		twp->scl.z *= prop_scale.z;
		twp->timer.ptr = prop_texlists[twp->btimer]; //Store which ID in the list this task is.
		twp->value.f = shadow_scale;

		switch (colli_type) {
		case COLLI_TYPE_NONE:
			//No Collision
			twp->mode = 1;
			break;
		case COLLI_TYPE_FIXED:
			if (!OnEdit(tp)) {
				CCL_Init(tp, fixed_colli, 1, CID_OBJECT);
			}
			twp->mode = 1;
			break;
		case COLLI_TYPE_DYNAMIC:
		{
			if (!twp->counter.ptr)
			{
				NJS_OBJECT* colObj = col_objList[twp->btimer];
				NJS_OBJECT* obj = GetMobileLandObject();
				if (obj) {
					twp->counter.ptr = obj;
					obj->evalflags = propObj->evalflags;

					obj->scl[0] = colObj->scl[0];
					obj->scl[1] = colObj->scl[1];
					obj->scl[2] = colObj->scl[2];
					obj->ang[0] = twp->ang.x;
					obj->ang[1] = twp->ang.y;
					obj->ang[2] = twp->ang.z;
					obj->pos[0] = twp->pos.x + colObj->pos[0];
					obj->pos[1] = twp->pos.y + colObj->pos[1];
					obj->pos[2] = twp->pos.z + colObj->pos[2];
					obj->model = colObj->model;
					obj->child = colObj->child;
					obj->sibling = colObj->sibling;
					twp->flag |= 0x100;

					if (obj->model && obj_num < 50) {
						NJS_MODEL* mdl = (NJS_MODEL*)obj->model;
						mdl->r = 999999.0f;
						RegisterCollisionEntry(0x1, tp, obj);
						playertwp[_pno]->pos.y += colObj->pos[1]; //Move the player on top to make sure the player doesn't get pushed under the floor.
						++colli_count;
					}
				}
			}
			//Collision File
			twp->mode = 1;
			break;
		}
		case COLLI_TYPE_MOV_S:
		case COLLI_TYPE_MOV_M:
		case COLLI_TYPE_MOV_L:
		case COLLI_TYPE_MOV_SW:
		{
			//Pickup Object
			motionwk* mwp = tp->mwp;

			//We can reuse this code for all movable types, so use mode to determine which one it is.
			Sint32 movType = colli_type - COLLI_TYPE_MOV_S; 

			ObjectMovableInitialize(twp, mwp, 4 + movType );
			twp->mode = PROP_MODE_MOV_S + movType;
			if (!OnEdit(tp)) {
				CCL_Init(tp, &pickup_colli[0 + (movType * 2)], 2, CID_OBJECT);
			}
			break;
		}
		}
		tp->disp = propObject_disp;
		tp->exec = propObject_exec;
		tp->dest = propObject_dest;
		break;
	}
	case PROP_MODE_NORMAL:
		twp->flag |= 0x100; //Keeps its collision enabled.
		LoopTaskC(tp);
		break;
	default:
		break;
	}
}


/// <summary>
/// Spawns a prop from various files
/// </summary>
/// <param name="propID"></param>
/// <param name="pno"></param>
void setProp(Sint32 propI, Sint32 pno) {

	std::string animstr = "props\\" + selectedPropFolder + "\\anim" + std::to_string(anim_no) + ".saanim";
	std::string colstr = "props\\" + selectedPropFolder + "\\colli.sa1mdl";
	std::string namestr = "props\\" + selectedPropFolder + "\\model.sa1mdl";
	std::string texstr = "props\\" + selectedPropFolder + "\\TEXTURES";
	std::string tlsstr = "props\\" + selectedPropFolder + "\\texlist.tls";

	//Create the imported object task.
	NJS_OBJECT* importedObj = importSA1MDL(data[obj_num], namestr.c_str(), FALSE);

	if (importedObj) {

		NJS_TEXLIST* texlist_importedObj = importTLS(tlsstr.c_str());
		prop_texlists[obj_num] = texlist_importedObj;

		if (prop_texlists[obj_num]) {
			//Load the PVM
			texLoadTexturePvmFile(texstr.c_str(), texlist_importedObj);
		}

		prop_objList[obj_num] = importedObj;
		prop_taskList[obj_num] = CreateElementalTask(3, 3, propObject);
		
		if (prop_taskList[obj_num]) {
			prop_taskList[obj_num]->twp->btimer = obj_num;

			//Create Collision
			if (colli_type == COLLI_TYPE_DYNAMIC) {

				/* If there's no model in the root node, this is probably a multi-noded model.
					   Check if there's a collision model for it and load it if there is. */
				if (!importedObj->model)
					col_objList[obj_num] = importSA1MDL(&coldata[obj_num], colstr.c_str(), TRUE);
				else
					col_objList[obj_num] = importedObj;
			}



			//Load Animation
			if (anim_no) {
				mtn_list[obj_num] = importSAANIM(&animdata[obj_num], animstr.c_str());
			}
		}
		++obj_num;
	}
}

//Deletes a prop. Deleting the file data is handled in the prop object's dest.
void deleteProp(Sint32 prop) {
	if (obj_num > 0 && prop_taskList[prop]) {
		FreeTask(prop_taskList[prop]);
		prop_taskList[prop]->dest;
		prop_taskList[prop] = 0;
	}
}

void propSpawnerMain(Sint32 pno) {

	//Update current prop selection
	getNameOfFile(&selectedPropFolder, PropFolderPath, propID, TRUE);

	/* Set the upper limit for propID.
	   If selectedPropFolder is empty (You deleted a folder) then adjust propID accordingly.*/
	if (selectedPropFolder == "" && propID > 0) {
		getNameOfFile(&selectedPropFolder, PropFolderPath, --propID, TRUE);
	}

	//ID controls
		//Selection Arrow
	Uint32 isPressorOn = per[0]->press;
	if (per[0]->on & Buttons_Y) {
		isPressorOn = per[0]->on & ~Buttons_Y;
	}

	if (prop_EditID == 0 && per[0]->press & Buttons_Y) {
		prop_EditID = 1;
		maker_mode = 0;
		return;
	}

	switch (isPressorOn) {
	case Buttons_Right:

		switch (prop_EditID) {
		case 1:
			++propID;
			break;
		case 2:
			shadow_scale = NJM_MIN(100.0f, shadow_scale + 0.1f);
			break;
		case 3:
			colli_type = NJM_MIN(COLLI_TYPE_MAX - 1, colli_type + 1);
			break;
		case 4:
			++anim_no;
			break;
		case 5:
			prop_scale.x = NJM_MIN(1000.0f, prop_scale.x + 0.1f);
			break;
		case 6:
			prop_scale.y = NJM_MIN(1000.0f, prop_scale.y + 0.1f);
			break;
		case 7:
			prop_scale.z = NJM_MIN(1000.0f, prop_scale.z + 0.1f);
			break;
		}
		break;
	case Buttons_Left:
		switch (prop_EditID) {
		case 1:
			propID = NJM_MAX(0, propID - 1);
			break;
		case 2:
			shadow_scale = NJM_MAX(0.0f, shadow_scale - 0.1f);
			break;
		case 3:
			colli_type = NJM_MAX(COLLI_TYPE_NONE, colli_type - 1);
			break;
		case 4:
			anim_no = NJM_MAX(0, anim_no - 1);
			break;
		case 5:
			prop_scale.x = NJM_MAX(0, prop_scale.x - 0.1f);
			break;
		case 6:
			prop_scale.y = NJM_MAX(0, prop_scale.y - 0.1f);
			break;
		case 7:
			prop_scale.z = NJM_MAX(0, prop_scale.z - 0.1f);
			break;
		}
		break;
	case Buttons_Up:
		prop_EditID = NJM_MAX(0, prop_EditID - 1);
		break;
	case Buttons_Down:
		prop_EditID = NJM_MIN(7, prop_EditID + 1);
		break;
	}

	// Place/Delete prop
	if (per[0]->press & Buttons_Z) {
		if (per[0]->on & Buttons_Y) {
			deleteProp(obj_num - 1);
		}
		else if (obj_num < PROP_LIMIT) {
			_pno = pno;
			setProp(propID, pno);
		}
	}



	//Delete All
	if (KeyGetPress(KEYS_FSLASH)) {
		for (Sint32 i = obj_num; i; --i)
			deleteProp(i - 1);
	}
}