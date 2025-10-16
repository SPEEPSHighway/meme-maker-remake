#include "SADXModLoader.h"
#include "Keys.h"
#include "FunctionHook.h"
#include "FreeCamera.h"

Sint8 faceNo[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };
task* playerheads[8];

Float movementSpeedHead = 10.0f;
Float movementSpeedRateHead = 0.1f;
Float movementSpeedPoint = 3.0f;
Float movementSpeedRatePoint = 0.1f;
Sint32 lookMode;
Sint32 lookObjType;
Sint32 lookPlayerID;
Sint32 lookObjectID;
static Bool isEditPoint;
Bool isEventFaceSpeed;
FACETBL facetbl2[faceEntries.size()];
Bool eyeSelection;

static Sint32 cursorID = 1;

static NJS_POINT3 pointCoords;

static const std::string objLookList[] = {
	"CAMERA",
	"PLAYER",
	"SET LIST OBJECT",
};

TaskFunc(MilesDirectAhead, 0x45DAD0);
//TaskFunc(MilesDirectAhead2, 0x45DE20);
TaskFunc(KnucklesDirectAhead, 0x475260);
TaskFunc(AmyDirectAhead, 0x486410);
TaskFunc(TikalDirectAhead, 0x7B3880);

static FunctionHook<void, task*> SonicDirectAhead_hook(SonicDirectAhead);
static FunctionHook<void, task*> TikalDirectAhead_hook(TikalDirectAhead);
static FunctionHook<void, task*> BigDirectAhead_hook(BigDirectAhead);
static FunctionHook<void, task*> AmyDirectAhead_hook(AmyDirectAhead);
static FunctionHook<void, task*> KnucklesDirectAhead_hook(KnucklesDirectAhead);
static FunctionHook<void, task*> MilesDirectAhead_hook(MilesDirectAhead);
//static FunctionHook<void, task*> MilesDirectAhead2_hook(MilesDirectAhead2);

/// <summary>
/// Store pointer to player's head task.
/// </summary>
/// <param name="tp"></param>
void getDirectAheadTask(task* tp) {
	playerheads[tp->twp->counter.b[0]] = tp;

	if (playerheads[tp->twp->counter.b[0]]->twp->mode != 9) { //9 means custom head controls are active, so don't run normal head code.
		if (playertwp[tp->twp->counter.b[0]]) {
			switch (playertwp[tp->twp->counter.b[0]]->counter.b[1]) {
			case PLNO_SONIC:
				SonicDirectAhead_hook.Original(tp);
				break;
			case PLNO_TAILS:
				MilesDirectAhead_hook.Original(tp);
				break;
			case PLNO_KNUCKLES:
				KnucklesDirectAhead_hook.Original(tp);
				break;
			case PLNO_TIKAL:
				TikalDirectAhead_hook.Original(tp);
				break;
			case PLNO_AMY:
				AmyDirectAhead_hook.Original(tp);
				break;
			case PLNO_BIG:
				BigDirectAhead_hook.Original(tp);
				break;
			}
		}
	}
}

/// <summary>
/// Used in Init to hook DirectAhead functions.
/// </summary>
void hookHead() {
	SonicDirectAhead_hook.Hook(getDirectAheadTask);
	MilesDirectAhead_hook.Hook(getDirectAheadTask);
	KnucklesDirectAhead_hook.Hook(getDirectAheadTask);
	TikalDirectAhead_hook.Hook(getDirectAheadTask);
	AmyDirectAhead_hook.Hook(getDirectAheadTask);
	BigDirectAhead_hook.Hook(getDirectAheadTask);
}

/// <summary>
/// Back up the original face anim frame data.
/// </summary>
void initFaceTableBackup() {
	for (Uint32 i = 0; i < faceEntries.size(); i++) facetbl2[i].nbFrame = faceEntries[i].nbFrame;
}

/// <summary>
/// HUD for Head Controls
/// </summary>
/// <param name="col">Initial Column</param>
/// <param name="pno">Player No</param>
void displayHeadInfo(Sint32 col, Sint32 pno) {
	njPrintC(NJM_LOCATION(1, col++), "Mode: Head Options");
	col++;

	if (!playerheads[pno]) {
		njPrintC(NJM_LOCATION(2, col++), "This character doesn't have a head object.");
		return;
	}

	njPrintC(NJM_LOCATION(2, col++), "Change Settings for the player's Head.");
	njPrintC(NJM_LOCATION(2, col++), "NOTE: Some animations don't support head rotation/faces.");
	njPrintC(NJM_LOCATION(2, col++), "Z Button/RSHIFT = Read from face.txt.");
	njPrintC(NJM_LOCATION(2, col++), "/ = Reset Head");
	col++;

	Sint32 colArrow = col + cursorID;
	njPrintC(NJM_LOCATION(1, colArrow), ">");
	njPrintC(NJM_LOCATION(2, col++), "BACK");

	if(faceNo[pno] == -1)
		njPrintC(NJM_LOCATION(2, col++), "Face: NORMAL");
	else
		njPrint(NJM_LOCATION(2, col++), "Face: %d", faceNo[pno]);
	njPrint(NJM_LOCATION(2, col++), "G = Change Face Animation Speed (Currently: %s)", isEventFaceSpeed ? "EVENTS" : "IN-GAME");
	if (lookMode == 4) {
		col++;
		njPrintC(NJM_LOCATION(2, col++), "CUSTOM HEAD CONTROLS MODE");
		njPrintC(NJM_LOCATION(2, col++), "Unlimited Head/Eye rotation");
		njPrintC(NJM_LOCATION(2, col++), "Other Head modes disabled.");
		col++;
		njPrintC(NJM_LOCATION(2, col++), "Right Analog = Rotate Head");
		njPrintC(NJM_LOCATION(2, col++), "Y Button + Left Analog = Move Eye");
		col++;
		col++;
		//njPrintC(NJM_LOCATION(2, col++), "D-Up = Right Eye");
		//njPrintC(NJM_LOCATION(2, col++), "D-Down = Left Eye");
		njPrint(NJM_LOCATION(2, col++), "Current Eye: %s", eyeSelection ? "RIGHT" : "LEFT");
		njPrint(NJM_LOCATION(2, col++), "Movement Speed = %.3f", movementSpeedHead);
		njPrint(NJM_LOCATION(2, col++), "Adjust Rate = %.3f", movementSpeedRateHead);
		njPrintC(NJM_LOCATION(2, col++), "Back to Head Menu");
		//	njPrintC(NJM_LOCATION(2, col++), "- + = Change Movement Speed");
		//	njPrintC(NJM_LOCATION(2, col++), "[ ] = Change Movement Speed Adjust Rate");

	}
	else {
		njPrintC(NJM_LOCATION(2, col++), "C = Angle");
		njPrintC(NJM_LOCATION(2, col++), "V = Point");
		njPrintC(NJM_LOCATION(2, col++), "B = Object");

		//Big's head movement is limited to his eyes, so there's no reason for him to need custom controls.
		if (playertwp[pno] && playertwp[pno]->counter.b[1] != PLNO_BIG)
			njPrintC(NJM_LOCATION(2, col++), "N = Custom Head Controls");
	}
	col++;
	switch (lookMode) {
	case LOOK_MODE_ANGLE:
	{
		njPrintC(NJM_LOCATION(2, col++), "ANGLE MODE");
		pheadeyewk* phewp = (pheadeyewk*)playerheads[pno]->mwp;
		if (phewp) {
			njPrint(NJM_LOCATION(2, col++), "ANG Y = %04X", (Uint16)phewp->angy_head);
			njPrint(NJM_LOCATION(2, col++), "ANG Z = %04X", (Uint16)phewp->angz_head);
			njPrintC(NJM_LOCATION(2, col++), "Move the right analog to control the character's head.");
		}
		col++;
		njPrint(NJM_LOCATION(2, col++), "Movement Speed = %.3f", movementSpeedHead);
		njPrint(NJM_LOCATION(2, col++), "Adjust Rate = %.3f", movementSpeedRateHead);
		//	njPrintC(NJM_LOCATION(2, col++), "- + = Change Movement Speed");
		//	njPrintC(NJM_LOCATION(2, col++), "[ ] = Change Movement Speed Adjust Rate");
		break;
	}
	case LOOK_MODE_POINT:
	{
		njPrintC(NJM_LOCATION(2, col++), "POINT MODE");
		njPrintC(NJM_LOCATION(2, col++), "Player is now looking at a coordinate.");
		//	njPrintC(NJM_LOCATION(2, col++), "< > = Save/Load Coordinate Info");
		njPrintC(NJM_LOCATION(2, col++), "/   = Reset Point to Player");

		col++;
		Sint32 infoCol = col;
		njPrintC(NJM_LOCATION(2, col++), "CURRENT INFO");
		njPrint(NJM_LOCATION(2, col++), "POS X = %.2f", playertwp[pno]->ewp->look.pos.x);
		njPrint(NJM_LOCATION(2, col++), "POS Y = %.2f", playertwp[pno]->ewp->look.pos.y);
		njPrint(NJM_LOCATION(2, col++), "POS Z = %.2f", playertwp[pno]->ewp->look.pos.z);
		col++;
		njPrintC(NJM_LOCATION(21, infoCol++), "SAVED INFO");
		njPrint(NJM_LOCATION(21, infoCol++), "POS X = %.2f", pointCoords.x);
		njPrint(NJM_LOCATION(21, infoCol++), "POS Y = %.2f", pointCoords.y);
		njPrint(NJM_LOCATION(21, infoCol++), "POS Z = %.2f", pointCoords.z);
		njPrintC(NJM_LOCATION(2, col++), "<- LOAD / SAVE ->");
		njPrintC(NJM_LOCATION(2, col++), "F = Move Point (NOTE: This will change the camera)");
		if (isEditPoint) {
			col++;
			njPrintC(NJM_LOCATION(2, col++), "Start + Analog = Move Camera");
			njPrintC(NJM_LOCATION(2, col++), "L R = Move Up/Down");
			njPrintC(NJM_LOCATION(2, col++), "- + = Change Movement Speed");
			njPrintC(NJM_LOCATION(2, col++), "[ ] = Change Movement Speed Adjust Rate");
		}

		//Display sphere at the point looking at
		DispSphere(&playertwp[pno]->ewp->look.pos, 5.0f);
		break;
	}
	case LOOK_MODE_OBJECT:
		njPrintC(NJM_LOCATION(2, col++), "OBJECT MODE");
		njPrintC(NJM_LOCATION(2, col++), "Player is now looking at an object.");
		col++;
		njPrint(NJM_LOCATION(2, col++), "Type: %s", objLookList[lookObjType].c_str());

		switch (lookObjType) {
		case 0:
			njPrintC(NJM_LOCATION(2, col++), "Currently looking at the active camera.");
			break;
		case 1:
		{
			//Get the player character's name
			const char* playerName;
			if (playertwp[lookPlayerID]) {

				if (lookPlayerID == 0 && gu8flgPlayingMetalSonic)
					playerName = metalSonicName;
				else
					playerName = playerNames[playertwp[lookPlayerID]->counter.b[1]];
			}
			else {
				playerName = "NOT FOUND";
			}

			//Print the player looking at.
			njPrint(NJM_LOCATION(2, col++), "Looking at: Player %d (%s)", lookPlayerID + 1, playerName);
			if (pno == lookPlayerID)
				njPrintC(NJM_LOCATION(2, col++), "NOTE: THIS IS YOU (Looks weird)");
			break;
		}
		case 2: //Object
		{
			njPrintC(NJM_LOCATION(2, col++), "Currently looking at a SET object.");
			njPrintC(NJM_LOCATION(2, col++), "This is better in smaller maps.");

			col++;
			njPrint(NJM_LOCATION(2, col++), "ID: %d", lookObjectID);
			if (objStatusEntry[lookObjectID].pTask) {
				if ((objStatusEntry[lookObjectID].pObjEditEntry->usID & 0xFFF) < pObjItemTable->ssCount)
				{
					njPrint(NJM_LOCATION(2, col++), "Name: %s", pObjItemTable->pObjItemEntry[objStatusEntry[lookObjectID].pObjEditEntry->usID & 0xFFF].strObjName);
				}
				else {
					njPrint(NJM_LOCATION(2, col++), "Name: NON-LIST OBJECT");
				}
				if (objStatusEntry[lookObjectID].pTask->twp) {
					Sint32 infoCol = col;
					NJS_POINT3 objPos = objStatusEntry[lookObjectID].pTask->twp->pos;
					njPrintC(NJM_LOCATION(2, col++), "OBJ:");
					njPrint(NJM_LOCATION(2, col++), "POS X = %.2f", objPos.x);
					njPrint(NJM_LOCATION(2, col++), "POS Y = %.2f", objPos.y);
					njPrint(NJM_LOCATION(2, col++), "POS Z = %.2f", objPos.z);
					njPrintC(NJM_LOCATION(21, infoCol++), "YOU:");
					njPrint(NJM_LOCATION(21, infoCol++), "POS X = %.2f", playertwp[pno]->pos.x);
					njPrint(NJM_LOCATION(21, infoCol++), "POS Y = %.2f", playertwp[pno]->pos.y);
					njPrint(NJM_LOCATION(21, infoCol++), "POS Z = %.2f", playertwp[pno]->pos.z);
				}
			}
			else {
				njPrintColor(0xFFFF0000);
				njPrintC(NJM_LOCATION(2, col++), "ERROR: OBJECT NOT FOUND (MAY BE OUT OF RANGE)");
				njPrintColor(0xFFFFFFFF);
			}
			break;
		}
		}
	}
	col++;
	njPrint(NJM_LOCATION(2, col++), "Player Head Pointer: %04X", playerheads[pno]);
}

/// <summary>
/// Use eventwk's LOOK_MODE_ANGLE to set player head/eye angle.
/// </summary>
/// <param name="pno">Player No</param>
void setLookingAngle(Sint32 pno) {
	pheadeyewk* phewp = (pheadeyewk*)playerheads[pno]->mwp;
	if (!phewp) {
		PrintDebug("\nERROR: PHEWP not found.");
		return;
	}
	Float analogLeftX = (Float)(per[0]->x1 << 8);
	Float analogLeftY = (Float)(per[0]->y1 << 8);
	Float analogRightX = (Float)(per[0]->x2 << 8);
	Float analogRightY = (Float)(per[0]->y2 << 8);

	//Movement Speed
	if (KeyGetOn(KEYS_MINUS)) {
		movementSpeedHead = NJM_MAX(0.0f, movementSpeedHead - movementSpeedRateHead);
	}
	else if (KeyGetOn(KEYS_PLUS)) {
		movementSpeedHead = NJM_MIN(10000.0f, movementSpeedHead + movementSpeedRateHead);
	}

	//Adjust Rate
	if (KeyGetOn(KEYS_LBRACKET)) {
		movementSpeedRateHead = NJM_MAX(0.0f, movementSpeedRateHead - 0.001f);
	}
	else if (KeyGetOn(KEYS_RBRACKET)) {
		movementSpeedRateHead = NJM_MIN(1.0f, movementSpeedRateHead + 0.001f);
	}

	playertwp[pno]->ewp->look.mode = LOOK_MODE_ANGLE;

	//Invert X-Axis if the player is facing the camera
	Bool invertX = FALSE;
	Uint16 facingAngle = (Uint16)SubAngle(camera_twp->ang.y + 0x4000, -playertwp[0]->ang.y);
	if (facingAngle > 0xB500 || facingAngle < 0x4500) {
		invertX = TRUE;
	}


	Angle dir = -camera_twp->ang.y + njArcTan2(analogRightY, analogRightX);
	Float mag = analogRightY * analogRightY + analogRightX * analogRightX;
	mag = njSqrt(mag) * mag * 3.918702724E-14f;

	//Up & Down
	if (analogRightY > 3072.0f) playertwp[pno]->ewp->look.ang.z = NJM_MIN(phewp->angz_head_max + phewp->angz_reye_max, playertwp[pno]->ewp->look.ang.z + (Sint16)(movementSpeedHead * 100.0f * mag));
	if (analogRightY < -3072.0f) playertwp[pno]->ewp->look.ang.z = NJM_MAX(phewp->angz_head_min + phewp->angz_reye_min, playertwp[pno]->ewp->look.ang.z - (Sint16)(movementSpeedHead * 100.0f * mag));

	//Left & Right
	if (invertX) {
		if (analogRightX > 3072.0f) playertwp[pno]->ewp->look.ang.y = NJM_MIN(phewp->angy_head_max + phewp->angy_reye_max, playertwp[pno]->ewp->look.ang.y + (Sint16)(movementSpeedHead * 100.0f * mag));
		if (analogRightX < -3072.0f) playertwp[pno]->ewp->look.ang.y = NJM_MAX(phewp->angy_head_min + phewp->angy_leye_min, playertwp[pno]->ewp->look.ang.y - (Sint16)(movementSpeedHead * 100.0f * mag));
	}
	else {
		if (analogRightX < 3072.0f) playertwp[pno]->ewp->look.ang.y = NJM_MIN(phewp->angy_head_max + phewp->angy_reye_max, playertwp[pno]->ewp->look.ang.y + (Sint16)(movementSpeedHead * 100.0f * mag));
		if (analogRightX > -3072.0f) playertwp[pno]->ewp->look.ang.y = NJM_MAX(phewp->angy_head_min + phewp->angy_leye_min, playertwp[pno]->ewp->look.ang.y - (Sint16)(movementSpeedHead * 100.0f * mag));
	}
}

/// <summary>
/// Temporary empty object when editing point coordinate
/// </summary>
/// <param name="tp">Task</param>
static void tempPointObject(task* tp) {
};

/// <summary>
/// Use eventwk's LOOK_MODE_POINT to set point for player to look.
/// </summary>
/// <param name="pno">Player No</param>
void setLookingPoint(Sint32 pno) {
	playertwp[pno]->ewp->look.mode = LOOK_MODE_POINT;

	task* point_tp = NULL;
	if (KeyGetPress(KEYS_F) || (cursorID == 18 && per[0]->press & Buttons_Y)) {
		isEditPoint = isEditPoint ? FALSE : TRUE; //VS hates our Bool type and kept trying to recommend bitwise with !showDisplay lol

		if (isEditPoint) {
			//Create a temp task so we can use the editor camera
			point_tp = CreateElementalTask(2, LEV_2, tempPointObject);
			point_tp->twp->pos = playertwp[pno]->ewp->look.pos;
			pTaskWorkEditor = point_tp->twp;
			cameraSystemWork.G_scCameraLevel = 1;
			CameraSetEventCamera(CAMMD_EDITOR, CAMADJ_NONE);
			cameraSystemWork.G_scCameraLevel = 4;
		}
		else {
			cameraSystemWork.G_scCameraLevel = 2;
			CameraReleaseEventCamera();
			pTaskWorkEditor = NULL;
			FreeTask(point_tp);
			point_tp = 0;
		}

		editorCameraCheck(pno);
	}

	//Save & Load
	if (KeyGetPress(KEYS_RARROW) || (cursorID == 17 && per[0]->press & Buttons_Right)) {
		if (playertwp[pno]) {
			pointCoords = playertwp[pno]->ewp->look.pos;
		}
	}
	else if (KeyGetPress(KEYS_LARROW) || (cursorID == 17 && per[0]->press & Buttons_Left)) {
		if (playertwp[pno]) {
			if (isEditPoint && pTaskWorkEditor) { //Account for being in the editor
				pTaskWorkEditor->pos = pointCoords;
			}
			playertwp[pno]->ewp->look.pos = pointCoords;
		}
	}

	if (KeyGetOn(KEYS_FSLASH)) {
		if (playertwp[pno]) {
			if (isEditPoint && pTaskWorkEditor) {
				pTaskWorkEditor->pos = playertwp[0]->pos;
			}
			playertwp[pno]->ewp->look.pos = playertwp[0]->pos;
		}
	}

	if (isEditPoint) {
		//Get analog data
		Float analogLeftX = (Float)(per[0]->x1 << 8);
		Float analogLeftY = (Float)(per[0]->y1 << 8);
		Float analogRightX = (Float)(per[0]->x2 << 8);
		Float analogRightY = (Float)(per[0]->y2 << 8);

		//Movement Speed
		if (KeyGetOn(KEYS_MINUS)) {
			movementSpeedPoint = NJM_MAX(0.0f, movementSpeedPoint - movementSpeedRatePoint);
		}
		else if (KeyGetOn(KEYS_PLUS)) {
			movementSpeedPoint = NJM_MIN(10000.0f, movementSpeedPoint + movementSpeedRatePoint);
		}

		//Adjust Rate
		if (KeyGetOn(KEYS_LBRACKET)) {
			movementSpeedRatePoint = NJM_MAX(0.0f, movementSpeedRatePoint - 0.001f);
		}
		else if (KeyGetOn(KEYS_RBRACKET)) {
			movementSpeedRatePoint = NJM_MIN(1.0f, movementSpeedRatePoint + 0.001f);
		}

		//AKA Shamelessly copy the debug movement code lol
		if (!(per[0]->on & Buttons_Start)) {
			if (per[0]->on & Buttons_R) {
				pTaskWorkEditor->pos.y -= movementSpeedPoint * (Float)per[0]->r / 255.0f;
			}
			if (per[0]->on & Buttons_L) {
				pTaskWorkEditor->pos.y += movementSpeedPoint * (Float)per[0]->l / 255.0f;
			}

			if (analogLeftX > 3072.0f || analogLeftX < -3072.0f || analogLeftY > 3072.0f || analogLeftY < -3072.0f) {
				pTaskWorkEditor->pos.x += njCos(input_data[pno].angle) * movementSpeedPoint * input_data[pno].stroke;
				pTaskWorkEditor->pos.z += njSin(input_data[pno].angle) * movementSpeedPoint * input_data[pno].stroke;
			}
			if (analogRightX > 3072.0f || analogRightX < -3072.0f || analogRightY > 3072.0f || analogRightY < -3072.0f) {
				Angle dir = -camera_twp->ang.y + njArcTan2(analogRightY, analogRightX);
				Float mag = analogRightY * analogRightY + analogRightX * analogRightX;
				mag = njSqrt(mag) * mag * 3.918702724E-14f;

				pTaskWorkEditor->ang.x += (Uint16)(njSin(dir) * movementSpeedPoint * 50.0f * mag);
				pTaskWorkEditor->ang.z += (Uint16)(njCos(dir) * movementSpeedPoint * -50.0f * mag);
			}
		}

		//Update looking point if in the editor
		if (pTaskWorkEditor)
			playertwp[pno]->ewp->look.pos = pTaskWorkEditor->pos;

		//Stop the player moving
		playerpwp[pno]->nocontimer = 10;
	}
}

/// <summary>
/// Use eventwk's LOOK_MODE_POINT to set object's pos for player to look. (because LOOK_MODE_OBJECT sucks)
/// </summary>
/// <param name="pno">Player No</param>
void setLookingObject(Sint32 pno) {
	switch (lookObjType) {
	case 0: //Camera
		//Need to do this one slightly differently because we don't have camera's task pointer
		//and DirectAhead will override playerheads[pno]->twp->mode before we can use the old Z button code
		if (playerheads[pno]) {
			playertwp[pno]->ewp->look.mode = LOOK_MODE_POINT;
			playertwp[pno]->ewp->look.pos = camera_twp->pos;
		}
		break;
	case 1: //Player
		//Vanilla object head looks weird
		if (playertp[lookPlayerID]) {
			NJS_POINT3 lookpos = playertwp[lookPlayerID]->pos;
			lookpos.y += playerpwp[lookPlayerID]->p.eyes_height;
			playertwp[pno]->ewp->look.mode = LOOK_MODE_POINT;
			playertwp[pno]->ewp->look.pos = lookpos;
		}
		else {
			playertwp[pno]->ewp->look.mode = LOOK_MODE_NORMAL;
			playertwp[pno]->ewp->look.obj = NULL;
		}
		break;
	case 2: //SET Object
		NJS_POINT3 lookpos{};

		if (objStatusEntry[lookObjectID].pTask && objStatusEntry[lookObjectID].pTask->twp) {
			lookpos = objStatusEntry[lookObjectID].pTask->twp->pos;
			//PrintDebug("\n%d %s", objStatusEntry[0].pObjEditEntry->usID, pObjItemTable->pObjItemEntry[objStatusEntry[0].pObjEditEntry->usID].strObjName);
			playertwp[pno]->ewp->look.mode = LOOK_MODE_POINT;
			playertwp[pno]->ewp->look.pos = lookpos;
		}
		break;
	}
}

/// <summary>
/// Classic Meme Maker Head Movement
/// </summary>
/// <param name="pno">Player No</param>
void setLookingNoLimits(Sint32 pno) {
	pheadeyewk* phewp = (pheadeyewk*)playerheads[pno]->mwp;
	Float analogLeftX = (Float)(per[0]->x1 << 8);
	Float analogLeftY = (Float)(per[0]->y1 << 8);
	Float analogRightX = (Float)(per[0]->x2 << 8);
	Float analogRightY = (Float)(per[0]->y2 << 8);

	//Movement Speed
	if (KeyGetOn(KEYS_MINUS)) {
		movementSpeedHead = NJM_MAX(0.0f, movementSpeedHead - movementSpeedRateHead);
	}
	else if (KeyGetOn(KEYS_PLUS)) {
		movementSpeedHead = NJM_MIN(10000.0f, movementSpeedHead + movementSpeedRateHead);
	}

	//Adjust Rate
	if (KeyGetOn(KEYS_LBRACKET)) {
		movementSpeedRateHead = NJM_MAX(0.0f, movementSpeedRateHead - 0.001f);
	}
	else if (KeyGetOn(KEYS_RBRACKET)) {
		movementSpeedRateHead = NJM_MIN(1.0f, movementSpeedRateHead + 0.001f);
	}

	if (movementSpeedHead) { //Head won't move below 1.0

		//Invert X-Axis if the player is facing the camera
		Bool invertX = FALSE;
		Uint16 facingAngle = (Uint16)SubAngle(camera_twp->ang.y + 0x4000, -playertwp[0]->ang.y);
		if (facingAngle > 0xB500 || facingAngle < 0x4500) {
			invertX = TRUE;
		}

		Angle dir = -camera_twp->ang.y + njArcTan2(analogRightY, analogRightX);
		Float mag = analogRightY * analogRightY + analogRightX * analogRightX;
		mag = njSqrt(mag) * mag * 3.918702724E-14f;

		if (invertX) {
			if (analogRightX > 3072.0f) phewp->headobjptr->ang[1] -= (Sint16)(movementSpeedHead * 100.0f * mag);
			if (analogRightX < -3072.0f) phewp->headobjptr->ang[1] += (Sint16)(movementSpeedHead * 100.0f * mag);
		}
		else {
			if (analogRightX > 3072.0f) phewp->headobjptr->ang[1] += (Sint16)(movementSpeedHead * 100.0f * mag);
			if (analogRightX < -3072.0f) phewp->headobjptr->ang[1] -= (Sint16)(movementSpeedHead * 100.0f * mag);
		}

		if (analogRightY > 3072.0f)  phewp->headobjptr->ang[2] += (Sint16)(movementSpeedHead * 100.0f * mag);
		if (analogRightY < -3072.0f) phewp->headobjptr->ang[2] -= (Sint16)(movementSpeedHead * 100.0f * mag);

		if (per[0]->on & Buttons_Y) {
			playerpwp[pno]->nocontimer = 10;
			NJS_OBJECT* eyeptr = eyeSelection ? phewp->reyeobjptr : phewp->leyeobjptr;

			if (analogLeftY > 3072.0f) eyeptr->ang[2] += (Sint16)(movementSpeedHead * 100.0f * input_data[pno].stroke);
			if (analogLeftY < -3072.0f)eyeptr->ang[2] -= (Sint16)(movementSpeedHead * 100.0f * input_data[pno].stroke);

			if (invertX) {
				if (analogLeftX > 3072.0f)  eyeptr->ang[1] += (Sint16)(movementSpeedHead * 100.0f * input_data[pno].stroke);
				if (analogLeftX < -3072.0f) eyeptr->ang[1] -= (Sint16)(movementSpeedHead * 100.0f * input_data[pno].stroke);
			}
			else {
				if (analogLeftX > 3072.0f) eyeptr->ang[1] -= (Sint16)(movementSpeedHead * 100.0f * input_data[pno].stroke);
				if (analogLeftX < -3072.0f) eyeptr->ang[1] += (Sint16)(movementSpeedHead * 100.0f * input_data[pno].stroke);
			}
		}
	}

}

/// <summary>
/// Main code for Head Editor
/// </summary>
/// <param name="pno"></param>
void doHeadFunctions(Sint32 pno) {
	if (playerheads[pno]) {
		//Selection Arrow
		Uint32 isPressorOn = per[0]->press;
		if (lookMode != 1 && cursorID > 6 && per[0]->on & Buttons_Y) {
			isPressorOn = per[0]->on & ~Buttons_Y;
		}

		if (per[0]->press & Buttons_Y && cursorID == 0) {
			cursorID = 1;
			maker_mode = 0;
			return;
		}

		switch (isPressorOn) {
		case Buttons_Right:
			switch (cursorID) {
			case 1:  //Face
			{
				Sint32 maxFace = 19;
				if (playertwp[pno] && playertwp[pno]->counter.b[1] == PLNO_BIG)
					maxFace = 7;
				faceNo[pno] = NJM_MIN(faceNo[pno] + 1, maxFace);

				//Quick hack to set blinking face
				if (faceNo[pno] == 0) {
					EV_ClrFace(playertp[pno]);
					EV_SetFace(playertp[pno], "A");
				}
				break;
			}
			case 11: //Object type
				lookObjType = NJM_MIN(lookObjType + 1, 2);
				break;
			case 12: //Object player/Custom Eyes
				if (lookMode == LOOK_MODE_OBJECT)
					lookPlayerID = NJM_MIN(lookPlayerID + 1, 7);
				else
					eyeSelection = eyeSelection != 1;
				break;
			case 13: //Angle spd
				movementSpeedHead = NJM_MIN(10000.0f, movementSpeedHead + movementSpeedRateHead);
				break;
			case 14: //Angle spdrate
				movementSpeedRateHead = NJM_MIN(1.0f, movementSpeedRateHead + 0.001f);
				break;
			case 15: //Object SET
				if (lookMode == LOOK_MODE_OBJECT) {
					if (++lookObjectID >= 1024) {
						lookObjectID = 0;
					}
				}
				break;
			}
			break;
		case Buttons_Left:
			switch (cursorID) {
			case 1:  //Face
				faceNo[pno] = NJM_MAX(faceNo[pno] - 1, -1);

				//Quick hack to set normal and blinking face
				if (faceNo[pno] == -1) {
					playertwp[pno]->ewp->face.nbFrame = 0;
					playertwp[pno]->ewp->look.ang.y = 0;
					playertwp[pno]->ewp->look.ang.z = 0;
				}
				else if (faceNo[pno] == 0) {
					EV_ClrFace(playertp[pno]);
					EV_SetFace(playertp[pno], "A");
				}
				break;
			case 11: //Object type
				lookObjType = NJM_MAX(lookObjType - 1, 0);
				break;
			case 12: //Object player/Custom Eyes

				if (lookMode == LOOK_MODE_OBJECT)
					lookPlayerID = NJM_MAX(lookPlayerID - 1, 0);
				else
					eyeSelection = eyeSelection != 1;
				break;
			case 13: //Angle spd
				movementSpeedHead = NJM_MAX(0.0f, movementSpeedHead - movementSpeedRateHead);
				break;
			case 14: //Angle spdrate
				movementSpeedRateHead = NJM_MAX(0.0f, movementSpeedRateHead - 0.001f);
				break;
			case 15: //Object SET
				if (lookMode == LOOK_MODE_OBJECT) {
					if (--lookObjectID < 0) {
						lookObjectID = 1023;
					}
				}
				break;
			}
			break;
		case Buttons_Up:
			cursorID = NJM_MAX(0, cursorID - 1);

			switch (lookMode) {
			case 0:
				break;
			case 1:
				if (cursorID > 6 && cursorID < 13)
					cursorID = 6;
				break;
			case 2:
				if (cursorID > 6 && cursorID < 17)
					cursorID = 6;
				if (isEditPoint)
					cursorID = NJM_MAX(17, cursorID);
				break;
			case 3:
				if (cursorID > 6 && cursorID < 11)
					cursorID = 6;
				if (lookObjType == 2 && cursorID > 11 && cursorID < 15)
					cursorID = 11;
				break;
			case 4:
				if (cursorID > 2 && cursorID < 12)
					cursorID = 2;
				break;
			}

			break;
		case Buttons_Down:
			switch (lookMode) {
			case 0:
				cursorID = NJM_MIN(6, cursorID + 1);
				break;
			case 1:
				cursorID = NJM_MIN(14, cursorID + 1);
				if (cursorID > 6 && cursorID < 13)
					cursorID = 13;
				break;
			case 2:
				cursorID = NJM_MIN(18, cursorID + 1);
				if (cursorID > 6 && cursorID < 17)
					cursorID = 17;
				break;
			case 3:
				switch (lookObjType) {
				case 0:
					cursorID = NJM_MIN(11, cursorID + 1);
					break;
				case 1:
					cursorID = NJM_MIN(12, cursorID + 1);
					break;
				case 2:
					cursorID = NJM_MIN(15, cursorID + 1);
					if (cursorID > 11 && cursorID < 15)
						cursorID = 15;
					break;
				}

				if (cursorID > 6 && cursorID < 11)
					cursorID = 11;
				break;
			case 4:
				cursorID = NJM_MIN(15, cursorID + 1);

				if (cursorID > 2 && cursorID < 12)
					cursorID = 12;
				break;
			}
			break;
		}


		//Face change is done in mod.cpp to keep it intact outside face mode

		//Clear face
		if (KeyGetOn(KEYS_FSLASH)) {
			faceNo[pno] = 0;
			playertwp[pno]->ewp->face.nbFrame = 0;
			playertwp[pno]->ewp->look.ang.y = 0;
			playertwp[pno]->ewp->look.ang.z = 0;
		}


		if (KeyGetPress(KEYS_G) || (per[0]->press & Buttons_Y && cursorID == 2)) {
			isEventFaceSpeed = isEventFaceSpeed ? FALSE : TRUE;

			if (isEventFaceSpeed) {
				for (Uint32 i = 0; i < faceEntries.size(); i++) faceEntries[i].nbFrame = facetbl2[i].nbFrame * 2;
			}
			else {
				for (Uint32 i = 0; i < faceEntries.size(); i++) faceEntries[i].nbFrame = facetbl2[i].nbFrame;
			}
		}

		if (lookMode != 4) { //None of these work without DirectAhead's code so block using them when it's disabled.
			if (KeyGetPress(KEYS_C) || (per[0]->press & Buttons_Y && cursorID == 3)) {
				lookMode = lookMode == LOOK_MODE_ANGLE ? LOOK_MODE_NORMAL : LOOK_MODE_ANGLE;
				cursorID = lookMode == LOOK_MODE_ANGLE ? 13 : 0;
			}
			if (KeyGetPress(KEYS_V) || (per[0]->press & Buttons_Y && cursorID == 4)) {
				lookMode = lookMode == LOOK_MODE_POINT ? LOOK_MODE_NORMAL : LOOK_MODE_POINT;
				if (lookMode == LOOK_MODE_POINT) {
					playertwp[pno]->ewp->look.pos = playertwp[pno]->pos; //Init looking point.
				}
				cursorID = lookMode == LOOK_MODE_POINT ? 17 : 0;
			}
			if (KeyGetPress(KEYS_B) || (per[0]->press & Buttons_Y && cursorID == 5)) {
				lookMode = lookMode == LOOK_MODE_OBJECT ? LOOK_MODE_NORMAL : LOOK_MODE_OBJECT;
				cursorID = lookMode == LOOK_MODE_OBJECT ? 11 : 0;
			}
		}

		if (playertwp[pno] && playertwp[pno]->counter.b[1] != PLNO_BIG) {
			if (KeyGetPress(KEYS_N) || (per[0]->press & Buttons_Y && lookMode != 4 && cursorID == 6) || (lookMode == 4 && cursorID == 15 && per[0]->press & Buttons_Y)) {
				lookMode = lookMode == 4 ? LOOK_MODE_NORMAL : 4;

				if (lookMode == 4) {
					cursorID = 12;
					playerheads[pno]->twp->mode = 9; // Don't run DirectAhead code
				}
				else {
					cursorID = 6;
					playerheads[pno]->twp->mode = 1;
				}
			}
		}

		switch (lookMode) {
		case LOOK_MODE_ANGLE:
			setLookingAngle(pno);
			break;
		case LOOK_MODE_POINT:
			setLookingPoint(pno);
			break;
		case LOOK_MODE_OBJECT:
			setLookingObject(pno);
			break;
		case 4: //No Limits
			setLookingNoLimits(pno);
			break;
		default:
			break;
		}
	}
};