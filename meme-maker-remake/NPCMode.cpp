#include "NPCMode.h"
#include <math.h>

//This one is mainly unchanged from the original mod.
//I changed out old names ie. CharObj2Ptrs and edited the display.

Bool npcMode[8] = { FALSE };
Sint32 npcIndex[8] = { 0 };
Sint32 npcAnim[8] = { 0 };
float npcSpeed = 0.4f;
Sint32 animType = 0;
Bool npcAnimOn[8];
Bool setNewAction = FALSE;

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

void displayNPCInfo(Sint32 col, Sint32 pno) {
	njPrintC(NJM_LOCATION(1, col++), "Mode: NPC Mode");
	njPrintC(NJM_LOCATION(2, col++), "Turns the player into an NPC.");
	njPrintC(NJM_LOCATION(2, col++), "Z Button (RB) = Select");
	col++;
	Sint32 colArrow = col + cursor;
	njPrintC(NJM_LOCATION(1, colArrow), ">");
	njPrintC(NJM_LOCATION(2, col++), "BACK");
	njPrint(NJM_LOCATION(2, col++), "Currently selected NPC: %d", npcIndex[pno]);
	col++;

	if (playertwp[pno] && playertwp[pno]->counter.b[1] == PLNO_TIKAL) {
		njPrintC(NJM_LOCATION(2, col++), "NOTE: They won't walk if they're over Tikal.");
	}
}

Sint32 getNPCIndex(Sint32 pno) {
	return npcIndex[pno];
}

Bool isNPC(Sint32 pno) {
	return npcMode[pno];
}

void doNPCModeDisplay(Sint32 pno) {

	if (per[0]->press & Buttons_Y && cursor == 0) {
		cursor = 1;
		maker_mode = 0;
		return;
	}

	switch (per[pno]->press) {
	case Buttons_Right:
		npcMode[pno] = FALSE;
		if (npcIndex[pno] < 25) npcIndex[pno] += 1;
		else npcIndex[pno] = 0;
		npcAnim[pno] = 0;
		break;
	case Buttons_Left:
		npcMode[pno] = FALSE;
		if (npcIndex[pno] > 0) npcIndex[pno] -= 1;
		else npcIndex[pno] = 25;
		npcAnim[pno] = 0;
		break;
	case Buttons_Up:
		cursor = 0;
		break;
	case Buttons_Down:
		cursor = 1;
		break;
	case Buttons_Z:
		EV_ClrAction(EV_GetPlayer(pno));
		if (!npcMode[pno]) {
			njLoadTexturePvmFile("SS_PEOPLE", ADV00_TEXLISTS[6]);
			njLoadTexturePvmFile("SS_BURGER", ADV00_TEXLISTS[9]);
			njLoadTexturePvmFile("SS_EKIIN", ADV00_TEXLISTS[7]);
			njLoadTexturePvmFile("SS_MIZUGI", ADV00_TEXLISTS[8]);
			npcMode[pno] = TRUE;
		}
		else npcMode[pno] = FALSE;
		break;
	}
}

Sint32 getNPCMotion(Sint32 index, Sint32 anim) {
	switch (anim) {
	case 0:
		if (index <= 3 || index >= 24) return 1;
		if (index >= 4 && index <= 7) return 7;
		if (index >= 8 && index <= 11) return 14;
		if ((index >= 12 && index <= 13) || (index >= 15 && index <= 16)) return 21;
		if (index == 14 || (index >= 17 && index <= 19)) return 25;
		if (index >= 20 && index <= 23) return 7;
		break;
	case 1:
		if (index <= 3 || index >= 24) return 2;
		if (index >= 4 && index <= 7) return 6;
		if (index >= 8 && index <= 11) return 12;
		if ((index >= 12 && index <= 13) || (index >= 15 && index <= 16)) return 18;
		if (index == 14 || (index >= 17 && index <= 19)) return 28;
		if (index >= 20 && index <= 23) return 6;
		break;
	case 2:
		if (index <= 3 || index >= 24) return 0;
		if (index >= 4 && index <= 7) return 5;
		if (index >= 8 && index <= 11) return 11;
		if ((index >= 12 && index <= 13) || (index >= 15 && index <= 16)) return 20;
		if (index == 14 || (index >= 17 && index <= 19)) return 24;
		if (index >= 20 && index <= 23) return 5;
		break;
	case 3:
		if (index <= 3 || index >= 24) return 0;
		if (index >= 4 && index <= 7) return 1;
		if (index >= 8 && index <= 11) return 2;
		if ((index >= 12 && index <= 13) || (index >= 15 && index <= 16)) return 3;
		if (index == 14 || (index >= 17 && index <= 19)) return 4;
		if (index >= 20 && index <= 23) return 1;
		break;
	default:
		break;
	}
	return 0;
}

void npcModeFunction(Sint32 pno) {
	NJS_TEXLIST* npcTex[8];

	if (npcIndex[pno] == 23 || npcIndex[pno] == 25) npcTex[pno] = ADV00_TEXLISTS[9];
	if (npcIndex[pno] == 24) npcTex[pno] = ADV00_TEXLISTS[7];
	if (npcIndex[pno] >= 20 && npcIndex[pno] <= 22) npcTex[pno] = ADV00_TEXLISTS[8];
	if (npcIndex[pno] < 20) npcTex[pno] = ADV00_TEXLISTS[6];

	//For some reason Sonic uses center_height rather than height.
	if (playertwp[pno]->counter.b[1] == PLNO_SONIC || playertwp[pno]->counter.b[1] == PLNO_TIKAL) {
		switch (getNPCMotion(npcIndex[pno], 3)) {
		case 0: playerpwp[pno]->p.center_height = 10; break;
		case 1: playerpwp[pno]->p.center_height = 11; break;
		case 2: playerpwp[pno]->p.center_height = 6; break;
		case 3: playerpwp[pno]->p.center_height = 8; break;
		case 4: playerpwp[pno]->p.center_height = 6; break;
		}
	}
	else {
		switch (getNPCMotion(npcIndex[pno], 3)) {
		case 0:  playerpwp[pno]->p.height = 20; break;
		case 1:  playerpwp[pno]->p.height = 22; break;
		case 2:  playerpwp[pno]->p.height = 12; break;
		case 3:  playerpwp[pno]->p.height = 17; break;
		case 4:  playerpwp[pno]->p.height = 12; break;
		}
	}
	switch (playerpwp[pno]->mj.action) {
	case 0: case 1: case 2:
		if (npcAnim[pno] != 0) {
			npcSpeed = 0.4f;
			animType = 1;
			setNewAction = TRUE;
			npcAnim[pno] = 0;
		}
		break;
	case 9: case 10:
		if (npcAnim[pno] != 9) {
			npcSpeed = 0.5f;
			animType = 2;
			setNewAction = TRUE;
			npcAnim[pno] = 9;
		}
		break;
	case 11:
		if (npcAnim[pno] != 11) {
			npcSpeed = 1.0f;
			animType = 2;
			setNewAction = TRUE;
			npcAnim[pno] = 11;
		}
		break;
	case 12:
		if (npcAnim[pno] != 12) {
			npcSpeed = 1.8f;
			animType = 2;
			setNewAction = TRUE;
			npcAnim[pno] = 12;
		}
		break;
	case 13:
		if (npcAnim[pno] != 13) {
			npcSpeed = 2.4f;
			animType = 2;
			setNewAction = TRUE;
			npcAnim[pno] = 13;
		}
		break;
	case 14: case 18:
		if (npcAnim[pno] != 14) {
			npcSpeed = 0.0f;
			animType = 2;
			setNewAction = TRUE;
			npcAnim[pno] = 14;
		}
		break;
	case 5: case 8:
		if (npcAnim[pno] != 5) {
			npcSpeed = 1.0f;
			animType = 0;
			setNewAction = TRUE;
			npcAnim[pno] = 5;
		}
		break;
	default:
		break;
	}

	if (setNewAction) {
		EV_ClrAction(EV_GetPlayer(pno));
		EV_SetMotion(EV_GetPlayer(pno), MODEL_SS_PEOPLE_OBJECTS[npcIndex[pno]], MODEL_SS_PEOPLE_MOTIONS[getNPCMotion(npcIndex[pno], animType)], npcTex[pno], npcSpeed, 1, 10);
		setNewAction = FALSE;
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
	//	WriteData<1>((Sint32*)0x413DA4, 0xFF); //Disable the HUD so the timer doesn't overlap the display.
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