#include "SADXModLoader.h"
#include "FreeCamera.h"
#include "FreeMove.h"
#include "Keys.h"

static Float cam_movementSpeed = 5.0f;
static Float cam_movementSpeedRate = 0.1f;
//static Bool isFreeCamMove;
//static Bool isCamChange;
static Sint16 camMd = -1;
static Sint8 camAdj;
static Uint16 stgAct = 0xFFFF; //Transitions break freecam
static Sint32 cam_EditID = 1;
static Sint32 cam_mode;
static Sint32 persp = 0x31C7;

static Bool objSpawnRange;

struct camCoords {
	NJS_POINT3 pos;
	Angle3 ang;
	Angle persp;
};

camCoords savedCoords_cam;


void resetFreeCamera() {

	if (cam_mode) {
		cameraSystemWork.G_scCameraDirect = 1;
		CameraReleaseCollisionCamera();
	}

	cam_mode = 0;
}


/// <summary>
/// HUD for Free Move
/// HUD for Free Move
/// </summary>
/// <param name="col">Initial Column</param>
/// <param name="pno">Player No</param>
void displayCameraInfo(Sint32 col, Sint32 pno) {
	njPrintC(NJM_LOCATION(1, col++), "Mode: Camera Options");
	col++;
	njPrintC(NJM_LOCATION(2, col++), "Change Settings for the current Camera.");
	Sint32 propArrow = col + cam_EditID;
	njPrintC(NJM_LOCATION(1, propArrow), ">");
	njPrintC(NJM_LOCATION(2, col++), "BACK");
	njPrintC(NJM_LOCATION(2, col++), "C = Change Camera");
	njPrintC(NJM_LOCATION(2, col++), "V = Free Movement");
	njPrint(NJM_LOCATION(2, col++), "Object Spawning: %s", objSpawnRange ? "NEAR CAMERA" : "NEAR PLAYER");
	col++;

	if (cam_mode == 1) {
		njPrintC(NJM_LOCATION(2, col++), "CAMERA CHANGE");
		//njPrintC(NJM_LOCATION(2, col++), "D-Left/Right = Camera Mode");
		//njPrintC(NJM_LOCATION(2, col++), "D-Up/Down = Camera Adjust");
		njPrintC(NJM_LOCATION(2, col++), "Z Button = Use Camera");
		njPrintC(NJM_LOCATION(2, col++), "/ = Reset Camera");
		col++;
		njPrint(NJM_LOCATION(2, col++), "Active Mode: %s", camMDList[GetCameraMode()].c_str());
		njPrint(NJM_LOCATION(2, col++), "Active Adjust: %s", camADJList[GetAdjustMode()].c_str());
		col++;
		njPrintC(NJM_LOCATION(2, col++), "Use these to select a camera type:");
		col++;
		njPrint(NJM_LOCATION(2, col++), "Camera Mode: %s", camMd != -1 ? camMDList[camMd].c_str() : "NONE");
		njPrint(NJM_LOCATION(2, col++), "Camera Adjust: %s", camADJList[camAdj].c_str());

		//Errors
		if (camMd >= CAMMD_CHAOS && camMd <= CAMMD_EGM3 || camMd == CAMMD_BACK || camMd == CAMMD_BACK2) {
			njPrintColor(0xFFFF0000);
			njPrintC(NJM_LOCATION(2, col++), "WARNING: Mode requires specific actors!");
			njPrintColor(0xFFFFFFFF);
		}
		else if (camMd == CAMMD_EDITOR) {
			njPrintC(NJM_LOCATION(2, col++), "Uses P3 and Start. P3 controls will be repointed to yours");
			njPrintC(NJM_LOCATION(2, col++), "and pausing will be disabled while this camera is active.");
		}
		col++;
	}

	if (cam_mode == 2) {
		njPrintC(NJM_LOCATION(2, col++), "CAMERA FREE MOVE");
		njPrintC(NJM_LOCATION(2, col++), "Z (Held) = Spin");
		njPrintC(NJM_LOCATION(2, col++), "L/R = Y Axis");
		njPrintC(NJM_LOCATION(2, col++), "Left Analog = Position");
		njPrintC(NJM_LOCATION(2, col++), "Right Analog = Rotation");
		//njPrintC(NJM_LOCATION(2, col++), "< > = Save/Load Coordinate Info");
		//njPrintC(NJM_LOCATION(2, col++), "- + = Change Movement Speed");
		//njPrintC(NJM_LOCATION(2, col++), "[ ] = Change Movement Speed Adjust Rate");
		col++;

		Sint32 infoCol = col;
		njPrintC(NJM_LOCATION(2, col++), "CURRENT INFO");
		njPrint(NJM_LOCATION(2, col++), "POS X = %.2f", camera_twp->pos.x);
		njPrint(NJM_LOCATION(2, col++), "POS Y = %.2f", camera_twp->pos.y);
		njPrint(NJM_LOCATION(2, col++), "POS Z = %.2f", camera_twp->pos.z);
		njPrint(NJM_LOCATION(2, col++), "ANG X = %04X", (Uint16)camera_twp->ang.x);
		njPrint(NJM_LOCATION(2, col++), "ANG Y = %04X", (Uint16)camera_twp->ang.y);
		njPrint(NJM_LOCATION(2, col++), "ANG Z = %04X", (Uint16)camera_twp->ang.z);
		njPrint(NJM_LOCATION(2, col++), "PERSP = %04X", persp);
		col++;
		njPrintC(NJM_LOCATION(21, infoCol++), "SAVED INFO");
		njPrint(NJM_LOCATION(21, infoCol++), "POS X = %.2f", savedCoords_cam.pos.x);
		njPrint(NJM_LOCATION(21, infoCol++), "POS Y = %.2f", savedCoords_cam.pos.y);
		njPrint(NJM_LOCATION(21, infoCol++), "POS Z = %.2f", savedCoords_cam.pos.z);
		njPrint(NJM_LOCATION(21, infoCol++), "ANG X = %04X", (Uint16)savedCoords_cam.ang.x);
		njPrint(NJM_LOCATION(21, infoCol++), "ANG Y = %04X", (Uint16)savedCoords_cam.ang.y);
		njPrint(NJM_LOCATION(21, infoCol++), "ANG Z = %04X", (Uint16)savedCoords_cam.ang.z);
		njPrint(NJM_LOCATION(21, infoCol++), "PERSP = %04X", savedCoords_cam.persp);

		DispSphere(&savedCoords_cam.pos, 1.0f);

		njPrint(NJM_LOCATION(2, col++), "Movement Speed = %.3f", cam_movementSpeed);
		njPrint(NJM_LOCATION(2, col++), "Adjust Rate = %.3f", cam_movementSpeedRate);
		njPrintC(NJM_LOCATION(2, col++), "<- LOAD / SAVE ->");
	}
}

/// <summary>
/// Controls for Camera Movement
/// </summary>
/// <param name="pno"></param>
void cameraFreeMove(Sint32 pno) {
	//Get analog data
	Float analogLeftX = (Float)(per[0]->x1 << 8);
	Float analogLeftY = (Float)(per[0]->y1 << 8);
	Float analogRightX = (Float)(per[0]->x2 << 8);
	Float analogRightY = (Float)(per[0]->y2 << 8);

	//Movement Speed (keys)
	if (KeyGetOn(KEYS_MINUS)) {
		cam_movementSpeed = NJM_MAX(0.0f, cam_movementSpeed - cam_movementSpeedRate);
	}
	else if (KeyGetOn(KEYS_PLUS)) {
		cam_movementSpeed = NJM_MIN(10000.0f, cam_movementSpeed + cam_movementSpeedRate);
	}

	//Adjust Rate
	if (KeyGetOn(KEYS_LBRACKET)) {
		cam_movementSpeedRate = NJM_MAX(0.0f, cam_movementSpeedRate - 0.001f);
	}
	else if (KeyGetOn(KEYS_RBRACKET)) {
		cam_movementSpeedRate = NJM_MIN(1.0f, cam_movementSpeedRate + 0.001f);
	}

	//Save & Load
	if (KeyGetPress(KEYS_RARROW) || (per[0]->press & Buttons_Right && cam_EditID == 22)) {
		if (camera_twp) {
			savedCoords_cam.pos = camera_twp->pos;
			savedCoords_cam.ang = camera_twp->ang;
			savedCoords_cam.persp = persp;
		}
	}
	else if (KeyGetPress(KEYS_LARROW) || (per[0]->press & Buttons_Left && cam_EditID == 22)) {
		if (camera_twp) {
			camera_twp->pos = savedCoords_cam.pos;
			camera_twp->ang = savedCoords_cam.ang;
			njSetPerspective(savedCoords_cam.persp);
		}
	}

	if (per[0]->on & Buttons_R) {
		camera_twp->pos.y -= 1.0f * (cam_movementSpeed * (Float)persp / (Float)0x31C7 / 2.0f) * ((Float)per[0]->r) / 255.0f;
	}
	if (per[0]->on & Buttons_L) {
		camera_twp->pos.y += 1.0f * (cam_movementSpeed * (Float)persp / (Float)0x31C7 / 2.0f) * (Float)per[0]->l / 255.0f;
	}


	if (analogLeftX > 3072.0f || analogLeftX < -3072.0f || analogLeftY > 3072.0f || analogLeftY < -3072.0f) {
		//Game already records left analog angle/magnitude
		camera_twp->pos.x += njCos(input_data[pno].angle) * (Float)persp / (Float)0x31C7 * cam_movementSpeed / 2.0f * input_data[pno].stroke;
		camera_twp->pos.z += njSin(input_data[pno].angle) * (Float)persp / (Float)0x31C7 * cam_movementSpeed / 2.0f * input_data[pno].stroke;
	}


	if (per[0]->on & Buttons_Z) {
		//Y Position
		//if (analogLeftY > 3072.0f || analogLeftY < -3072.0f)
		//	camera_twp->pos.y -= analogLeftY / njSqrt(analogLeftY * analogLeftY) * (cam_movementSpeed / 2.0f);

		//Y Rotation
		if (analogRightX > 3072.0f)
			camera_twp->ang.z -= (Uint16)((cam_movementSpeed * (Float)persp / (Float)0x31C7 * 100.0f) * (analogRightX / 32768.0f));
		if (analogRightX < -3072.0f)
			camera_twp->ang.z += (Uint16)((cam_movementSpeed * (Float)persp / (Float)0x31C7 * 100.0f) * (analogRightX / -32768.0f));

		if (analogRightY > 3072.0f) {
			persp = NJM_MIN(0x9000, persp + (Angle)cam_movementSpeed * 10);
			njSetPerspective(persp);
			perspective_value = persp;
		}
		if (analogRightY < -3072.0f) {
			persp = NJM_MAX(1, persp - (Angle)cam_movementSpeed * 10);
			njSetPerspective(persp);
			perspective_value = persp;
		}
	//	if (analogRightY > 3072.0f)
		//	njSetPerspective(ds_perspective_value - 1);

	}
	else {

		persp = perspective_value;
		if (analogRightY > 3072.0f)
			camera_twp->ang.x -= (Uint16)((cam_movementSpeed * (Float)persp / (Float)0x31C7 * 75.0f) * ((analogRightY / 32768.0f) - 0.08));
		if (analogRightY < -3072.0f)
			camera_twp->ang.x += (Uint16)((cam_movementSpeed * (Float)persp / (Float)0x31C7 * 75.0f) * ((analogRightY / -32768.0f) - 0.08));
		if (analogRightX > 3072.0f)
			camera_twp->ang.y -= (Uint16)((cam_movementSpeed * (Float)persp / (Float)0x31C7 * 75.0f) * ((analogRightX / 32768.0f) - 0.08));
		if (analogRightX < -3072.0f)
			camera_twp->ang.y += (Uint16)((cam_movementSpeed * (Float)persp / (Float)0x31C7 * 75.0f) * ((analogRightX / -32768.0f) - 0.08));
	}
}

PDS_PERIPHERAL* old_per3;


void editorCameraCheck(Sint32 pno) {
	//Editor camera reads Player 3's controller and uses Start, so need to disable pausing.
	if (GetCameraMode() == CAMMD_EDITOR) {
		if (per[0]->on & Buttons_Start) {
			playerpwp[pno]->nocontimer = 10;
		}
		DisablePause();
		per[3] = per[0];
	}
	else {
		if (old_per3) {
			per[3] = old_per3;
			per[3] = NULL;
		}

		EnablePause();
	}
}

/// <summary>
/// Mode for selecting and changing the current camera type.
/// </summary>
/// <param name="pno"></param>
void changeCamera(Sint32 pno) {

	editorCameraCheck(pno);

	if (per[0]->press & Buttons_Z || (per[0]->press & Buttons_Y && cam_EditID == 14)) {
		if (camMd == CAMMD_EDITOR) {
			if (playertwp[pno]) {
				pTaskWorkEditor = playertwp[pno];
			}
			else {
				PrintDebug("\nMEME MAKER 2: Player not found for Editor Camera");
				return;
			}
		}

		//Set up as a collision camera - the highest priority, usually reserved for first person/back camera.
		if(camMd != -1)
			CameraSetCollisionCamera(camMd, camAdj);
		else
			CameraReleaseCollisionCamera();
	}

	if (KeyGetOn(KEYS_FSLASH)) {
		CameraReleaseCollisionCamera();
	}
}



void doCameraSettngs(Sint32 pno) {

	//Check for stage transition
	if (cam_mode && stgAct != (Uint16)GetStageNumber()) {
		stgAct = 0xFFFF;
		cam_mode = 0;
	}


	//Selection Arrow
	Uint32 isPressorOn = per[0]->press;
	if (per[0]->on & Buttons_Y) {
		isPressorOn = per[0]->on & ~Buttons_Y;
	}

	if (per[0]->press & Buttons_Y && cam_EditID == 0) {
		cam_EditID = 1;
		maker_mode = 0;
		return;
	}


	switch (isPressorOn) {
	case Buttons_Up:
		cam_EditID = NJM_MAX(0, cam_EditID - 1);

		switch (cam_mode) {
		case 1:
			if (cam_EditID > 3 && cam_EditID < 14)
				cam_EditID = 3;
			break;
		case 2:
			if (cam_EditID > 3 && cam_EditID < 19)
				cam_EditID = 3;
			break;
		}
		break;
	case Buttons_Down:
		switch (cam_mode) {
		case 0:
			cam_EditID = NJM_MIN(3, cam_EditID + 1);
			break;
		case 1:
			cam_EditID = NJM_MIN(15, cam_EditID + 1);

			if (cam_EditID > 3 && cam_EditID < 14)
				cam_EditID = 14;
			break;
		case 2:
			cam_EditID = NJM_MIN(22, cam_EditID + 1);

			if (cam_EditID > 3 && cam_EditID < 20)
				cam_EditID = 20;
			break;
		}
		break;
	case Buttons_Left:
		switch (cam_EditID) {
		case 14:
			camMd = NJM_MAX(camMd - 1, -1);
			break;
		case 15:
			camAdj = NJM_MAX(camAdj - 1, 0);
			break;
		case 20: //Freecam Spd
			cam_movementSpeed = NJM_MAX(0.0f, cam_movementSpeed - cam_movementSpeedRate);
			break;
		case 21: //Freecam AdjRate
			cam_movementSpeedRate = NJM_MAX(0.0f, cam_movementSpeedRate - 0.001f);
			break;
		}
		break;
	case Buttons_Right:
		switch (cam_EditID) {
		case 14:
			camMd = NJM_MIN(camMd + 1, CameraModeTable.ssModeCount - 1);
			break;
		case 15:
			camAdj = NJM_MIN(camAdj + 1, CameraModeTable.ssAdjustCount - 1);
			break;
		case 20:
			cam_movementSpeed = NJM_MIN(10000.0f, cam_movementSpeed + cam_movementSpeedRate);
			break;
		case 21:
			cam_movementSpeedRate = NJM_MIN(1.0f, cam_movementSpeedRate + 0.001f);
			break;
		}
		break;
	}

	if (per[0]->press & Buttons_Y && cam_EditID == 3) {
		objSpawnRange = objSpawnRange != 1;

		if (objSpawnRange) {
			WriteData<1>((void*)0x46BA4A, 0xEB); //Make object spawning follow the camera for ProcessStatusTable1P
			WriteData<1>((void*)0x46B7BA, 0xEB); //Make object spawning follow the camera for ProcessStatusTable2P
		}
		else {
			WriteData<1>((void*)0x46BA4A, 0x74);  //Make object spawning follow the player for ProcessStatusTable1P
			WriteData<1>((void*)0x46B7BA, 0x74);  //Make object spawning follow the camera for ProcessStatusTable2P
		}
	}




	stgAct = (Uint16)GetStageNumber();

	//Process CAMERA CHANGE
	if (KeyGetPress(KEYS_C) || (cam_EditID == 1 && per[0]->press & Buttons_Y)) {
		if (cam_mode == 2) {
			cameraSystemWork.G_scCameraDirect = 1;
			CameraReleaseCollisionCamera();
		}
		cam_mode = cam_mode == 1 ? 0 : 1;
		cam_EditID = cam_mode == 1 ? 14 : 1;

		return;
	}

	//Run main camera change code
	if (cam_mode == 1) {
		changeCamera(pno);
	}


	//Process CAMERA MOVEMENT
	if (KeyGetPress(KEYS_V) || (cam_EditID == 2 && per[0]->press & Buttons_Y)) {
		cam_mode = cam_mode == 2 ? 0 : 2;
		cam_EditID = cam_mode == 2 ? 20 : 2;

		//Init & End Camera Movement
		if (cam_mode == 2) {
			free_camera_mode |= 4;
			CameraSetCollisionCamera(CAMMD_FOLLOW, CAMADJ_NONE);
			cameraSystemWork.G_scCameraAttribute = 2;
			cameraSystemWork.G_scCameraMode = 0;
			cameraSystemWork.G_scCameraDirect = 0;
		}
		else {
			persp = 0x31C7;
			njSetPerspective(0x31C7);
			stgAct = 0xFFFF;
			cameraSystemWork.G_scCameraDirect = 1;
			CameraReleaseCollisionCamera();
		}
		return;
	}

	//Camera Movement
	if (cam_mode == 2) {
		//Disable controls while moving the camera around
		playerpwp[pno]->nocontimer = 30;
		cameraFreeMove(pno);
	}
}