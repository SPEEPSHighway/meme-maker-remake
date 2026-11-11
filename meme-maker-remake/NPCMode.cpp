#include "NPCMode.h"
#include <math.h>

//This one is mainly unchanged from the original mod.
//I changed out old names ie. CharObj2Ptrs and edited the display.

Bool npcMode[8];
Sint32 npcIndex[8][3];
Sint32 npcAnim[8];
Sint32 waitAnimType[8][3];

float npcSpeed = 0.4f;
Bool npcAnimOn[8];
Bool setNewAction;
Bool changedStandAnim;
Bool punch;
Bool justPunched;

#define NPC_MAX 28

#define GET_NPC_INDEX npcIndex[pno][field[pno]]

static const std::string npcFieldList[] = {
	"STATION SQUARE",
	"MYSTIC RUINS",
	"PAST"
};

Sint32 field[8];

//NPC model+texture+type
struct _people_model_data
{
	NJS_OBJECT* obj_p;
	NJS_TEXLIST* tex_p;
	sspmodel_type type;
};

DataArray(_people_model_data, SSpeople_model_table, 0x3C82EF0, 26);
DataArray(_people_model_data, MRpeople_model_table, 0x7FC738, 3);
DataArray(_people_model_data, PASTpeople_model_table, 0x3C638EC, 3);

Sint32 model_table_max[3] = { 25, 2, 2 };


//NPC height + animations
struct _people_motion_data
{
	NJS_MOTION* mot_p;
	float play_rate;
};

struct _SSpeople_type_data
{
	_people_motion_data motion_data_tbl[7];
	Float y_offset;
	Float eye_height;
	Float walk_step;
	CCL_INFO colli[1];
};

struct _MRpeople_type_data
{
	_people_motion_data motion_data_tbl[4];
	Float y_offset;
	Float eye_height;
	Float walk_step;
	CCL_INFO colli[1];
};

DataArray(_SSpeople_type_data, SSpeople_type_table, 0x3C82C38, 7);
DataArray(_MRpeople_type_data, MRpeople_type_table, 0x111CB90, 2);
DataArray(_MRpeople_type_data, PASTpeople_type_table, 0x3C637D8, 3);

player_parameter backup[8] = {
	playerwk_default[0],
	playerwk_default[1],
	playerwk_default[2],
	playerwk_default[3],
	playerwk_default[4],
	playerwk_default[5],
	playerwk_default[6],
	playerwk_default[7],
};

static Sint32 cursor = 1;

enum NPCCURSOR {
	NPCCURSOR_BACK,
	NPCCURSOR_FIELD,
	NPCCURSOR_TYPE,
	NPCCURSOR_WAITTYPE,
	NPCCURSOR_MAX
};


enum {
	ANIMTYPE_WAIT,
	ANIMTYPE_WAIT2,
	ANIMTYPE_WALK,
	ANIMTYPE_TALK1,
	ANIMTYPE_TALK2,
	ANIMTYPE_HIT,
};

HMODULE DCCONVERSION = GetModuleHandle(L"DCMods_Main");

void displayNPCInfo(Sint32 col, Sint32 pno) {
	njPrintC(NJM_LOCATION(1, col++), "Mode: NPC Mode");
	njPrintC(NJM_LOCATION(2, col++), "Turns the player into an NPC.");
	njPrintC(NJM_LOCATION(2, col++), "Z Button (RB) = Select");
	col++;
	Sint32 colArrow = col + cursor;
	njPrintC(NJM_LOCATION(1, colArrow), ">");
	njPrintC(NJM_LOCATION(2, col++), "BACK");
	njPrint(NJM_LOCATION(2, col++), "Field: %s", npcFieldList[field[pno]].c_str());
	njPrint(NJM_LOCATION(2, col++), "Currently selected NPC: %d", GET_NPC_INDEX);
	njPrint(NJM_LOCATION(2, col++), "Idle animation: %d", waitAnimType[pno][field[pno]]);
	col++;

	if (playertwp[pno] && playertwp[pno]->counter.b[1] == PLNO_TIKAL) {
		njPrintC(NJM_LOCATION(2, col++), "NOTE: They won't walk if they're over Tikal.");
	}

	if (field[pno] == 1 && DCCONVERSION) {
		njPrintC(NJM_LOCATION(2, col++), "NOTE: DC Conversion's Mystic Ruins code causes the explorers");
		njPrintC(NJM_LOCATION(2, col++), "      to have broken textures unless MR has been loaded at least once.");
	}
}

Bool isNPC(Sint32 pno) {
	return npcMode[pno];
}

DataPointer(NJS_TEXLIST, _texlist_tanken, 0x111C604);

void doNPCModeDisplay(Sint32 pno) {
	if (per[0]->press & Buttons_Y && cursor == NPCCURSOR_BACK) {
		cursor = 1;
		maker_mode = 0;
		return;
	}

	Sint32 wait_max = field[pno] ? 1 : 4;


	switch (per[pno]->press) {
	case Buttons_Right:

		if (cursor != NPCCURSOR_WAITTYPE) {
			npcMode[pno] = FALSE;
			npcAnim[pno] = 0;
		}

		switch (cursor) {
		case NPCCURSOR_FIELD:
			field[pno] = NJM_MIN(field[pno] + 1, 2);
			break;
		case NPCCURSOR_TYPE:
			if (GET_NPC_INDEX < model_table_max[field[pno]]) GET_NPC_INDEX += 1;
			else GET_NPC_INDEX = 0;
			break;
		case NPCCURSOR_WAITTYPE:
			if (waitAnimType[pno][field[pno]] < wait_max) waitAnimType[pno][field[pno]] += 1;
			else waitAnimType[pno][field[pno]] = 0;

			setNewAction = TRUE;

			break;
		default:
			break;
		};

		break;
	case Buttons_Left:
		if (cursor != NPCCURSOR_WAITTYPE) {
			npcMode[pno] = FALSE;
			npcAnim[pno] = 0;
		}

		switch (cursor) {
		case NPCCURSOR_FIELD:
			field[pno] = NJM_MAX(field[pno] - 1, 0);
			break;
		case NPCCURSOR_TYPE:
			if (GET_NPC_INDEX > 0) GET_NPC_INDEX -= 1;
			else GET_NPC_INDEX = model_table_max[field[pno]];
			break;
		case NPCCURSOR_WAITTYPE:
			if (waitAnimType[pno][field[pno]] > 0) waitAnimType[pno][field[pno]] -= 1;
			else waitAnimType[pno][field[pno]] = wait_max;

			setNewAction = TRUE;
			changedStandAnim = TRUE;
			break;
		default:
			break;
		};
		break;
	case Buttons_Up:
		cursor = NJM_MAX(cursor - 1, NPCCURSOR_BACK);
		break;
	case Buttons_Down:
		cursor = NJM_MIN(cursor + 1, NPCCURSOR_MAX - 1);
		break;

	case Buttons_Y:
		setNewAction = TRUE;
		punch = TRUE;
		break;

	case Buttons_Z:

		//Start & Stop NPC mode
		EV_ClrAction(EV_GetPlayer(pno));
		if (!npcMode[pno]) {

			//Load the field texture files
			switch (field[pno]) {
			case 0:
				njLoadTexturePvmFile("SS_PEOPLE", ADV00_TEXLISTS[6]);
				njLoadTexturePvmFile("SS_BURGER", ADV00_TEXLISTS[9]);
				njLoadTexturePvmFile("SS_EKIIN", ADV00_TEXLISTS[7]);
				njLoadTexturePvmFile("SS_MIZUGI", ADV00_TEXLISTS[8]);
				break;
			case 1:
				njLoadTexturePvmFile("TANKEN", &_texlist_tanken);
				break;
			case 2:
				njLoadTexturePvmFile("PAST_KN_FAM", ADV03_TEXLISTS[3]);
				break;
			default:
				break;
			}

			npcMode[pno] = TRUE;
		}
		else npcMode[pno] = FALSE;
		break;
	}
}

Sint32 findNPCAnim(Sint32 pno, Sint32 anim) {

	if (punch) {
		return field[pno] ? 3 : 6;
	}

	switch (npcAnim[pno]) {
	case 0: // Stand
	case 1: // Stand
	case 2: // Stand
	case 5: // Stand
	case 8: // Stand
		return waitAnimType[pno][field[pno]];
	case 9: // Walk
	case 10: // Walk
	case 11:
	case 12:
	case 13:
	case 14:
	case 18:
		return field[pno] ? 2 : 5;
	default:
		return 0;
	}
}

Float findNPCSpeed(Sint32 pno, Sint32 anim) {
	switch (npcAnim[pno]) {
	case 9: // Walk
	case 10:
		return 0.5f;
	case 5:
	case 8:
	case 11:
		return 1.0f;
	case 12:
		return 1.8f;
	case 13:
		return 2.4f;
	case 14:
	case 18:
		return 0.0f;
	default:
		switch (field[pno]) {
		case 0:
			return SSpeople_type_table[SSpeople_model_table[GET_NPC_INDEX].type].motion_data_tbl[anim].play_rate;
		case 1:
			return MRpeople_type_table[MRpeople_model_table[GET_NPC_INDEX].type].motion_data_tbl[anim].play_rate;
		case 2:
			return PASTpeople_type_table[PASTpeople_model_table[GET_NPC_INDEX].type].motion_data_tbl[anim].play_rate;
		default:
			break;
		}
	}
}


void npcModeFunction(Sint32 pno) {

	//Change player height. For some reason Sonic uses center_height rather than height.
	if (playertwp[pno]->counter.b[1] == PLNO_SONIC || playertwp[pno]->counter.b[1] == PLNO_TIKAL) {
		switch (field[pno]) {
		case 0:
			playerpwp[pno]->p.center_height = SSpeople_type_table[SSpeople_model_table[GET_NPC_INDEX].type].y_offset;
			playerpwp[pno]->p.eyes_height = SSpeople_type_table[SSpeople_model_table[GET_NPC_INDEX].type].eye_height;
			break;
		case 1:
			playerpwp[pno]->p.center_height = MRpeople_type_table[MRpeople_model_table[GET_NPC_INDEX].type].y_offset;
			playerpwp[pno]->p.eyes_height = MRpeople_type_table[MRpeople_model_table[GET_NPC_INDEX].type].eye_height;
			break;
		case 2:
			playerpwp[pno]->p.center_height = PASTpeople_type_table[PASTpeople_model_table[GET_NPC_INDEX].type].y_offset;
			playerpwp[pno]->p.eyes_height = PASTpeople_type_table[PASTpeople_model_table[GET_NPC_INDEX].type].eye_height;
			break;
		default:
			break;
		}
	}
	else {
		switch (field[pno]) {
		case 0:
			playerpwp[pno]->p.height = SSpeople_type_table[SSpeople_model_table[GET_NPC_INDEX].type].y_offset * 2.0f;
			playerpwp[pno]->p.eyes_height = SSpeople_type_table[SSpeople_model_table[GET_NPC_INDEX].type].eye_height;
			break;
		case 1:
			playerpwp[pno]->p.height = MRpeople_type_table[MRpeople_model_table[GET_NPC_INDEX].type].y_offset * 2.0f;
			playerpwp[pno]->p.eyes_height = MRpeople_type_table[MRpeople_model_table[GET_NPC_INDEX].type].eye_height;
			break;
		case 2:
			playerpwp[pno]->p.height = PASTpeople_type_table[PASTpeople_model_table[GET_NPC_INDEX].type].y_offset * 2.0f;
			playerpwp[pno]->p.eyes_height = PASTpeople_type_table[PASTpeople_model_table[GET_NPC_INDEX].type].eye_height;
			break;
		default:
			break;
		}
		
	}

	//Set NPC actions depending on character
	Sint32 walkID = field[pno] ? 2 : 5;
	Sint32 hitID = field[pno] ? 3 : 6;

	if (playerpwp[pno]->mj.action != npcAnim[pno] && findNPCAnim(pno, playerpwp[pno]->mj.action) != -1) {

		//Looks weird at low speed so skip changing for 9/10
		if (!((playerpwp[pno]->mj.action == 9 || playerpwp[pno]->mj.action == 10) && (npcAnim[pno] == 9 || npcAnim[pno] == 10))) {
			setNewAction = TRUE;
			npcAnim[pno] = playerpwp[pno]->mj.action;
		}
	}

	if (setNewAction) {

		if (!changedStandAnim) {
			EV_ClrAction(EV_GetPlayer(pno));
		}

		changedStandAnim = FALSE;

		Sint32 anim = findNPCAnim(pno, npcAnim[pno]);
		switch (field[pno]) {
		case 0:
			EV_SetMotion(EV_GetPlayer(pno),
				SSpeople_model_table[GET_NPC_INDEX].obj_p,
				SSpeople_type_table[SSpeople_model_table[GET_NPC_INDEX].type].motion_data_tbl[anim].mot_p,
				SSpeople_model_table[GET_NPC_INDEX].tex_p,
				findNPCSpeed(pno, npcAnim[pno]),
				1,
				justPunched ? 0 : 10);
			break;
		case 1:
			EV_SetMotion(EV_GetPlayer(pno),
				MRpeople_model_table[GET_NPC_INDEX].obj_p,
				MRpeople_type_table[MRpeople_model_table[GET_NPC_INDEX].type].motion_data_tbl[anim].mot_p,
				MRpeople_model_table[GET_NPC_INDEX].tex_p,
				findNPCSpeed(pno, npcAnim[pno]),
				1,
				justPunched ? 0 : 10);
			break;
		case 2:
			EV_SetMotion(EV_GetPlayer(pno),
				PASTpeople_model_table[GET_NPC_INDEX].obj_p,
				PASTpeople_type_table[PASTpeople_model_table[GET_NPC_INDEX].type].motion_data_tbl[anim].mot_p,
				PASTpeople_model_table[GET_NPC_INDEX].tex_p,
				findNPCSpeed(pno, npcAnim[pno]),
				1,
				justPunched ? 0 : 10);
			break;
		default:
			break;
		}

		if (punch) {
			punch = FALSE;
			changedStandAnim = TRUE;
			justPunched = TRUE;
		}
		else {
			justPunched = FALSE;
			setNewAction = FALSE;
		}
	}
}

void checkNPCMode(Sint32 pno) {
	if (!playertp[pno])
		return;

	if (npcMode[pno] && !DebugMode) {
		if (!npcAnimOn[pno]) {
			EV_ClrAction(EV_GetPlayer(pno));
			setNewAction = TRUE;
		}
		npcAnimOn[pno] = TRUE;
		npcModeFunction(pno);

	}
	else {
		if (npcAnimOn[pno]) {
			npcMode[pno] = FALSE;
			EV_ClrAction(EV_GetPlayer(pno));
			npcAnimOn[pno] = FALSE;
			playerpwp[pno]->p.height = backup[playertwp[pno]->counter.b[1]].height;
			playerpwp[pno]->p.center_height = backup[playertwp[pno]->counter.b[1]].center_height;
		}

	}
}