#include "SADXModLoader.h"
#include "Keys.h"

Float movementSpeed = 5.0f;
Float movementSpeedRate = 0.1f;
Bool playerFreeze[8];

static Sint32 md_mtn = 3;
static Sint32 moveID;

struct playerCoords {
	Uint16 anim;
	NJS_POINT3 pos;
	Angle3 ang;
	NJS_POINT3 scl;
};

playerCoords savedCoords[8];


static const std::string mtnList[] = {
	"Always Loop (MD_MTN_LOOP)",
	"Proceed if next exists (MD_MTN_NEXT)",
	"Stop on final frame (MD_MTN_STOP)",
	"",
	"Stop on first frame (MD_MTN_POTS)",
};

/// <summary>
/// HUD for Free Move
/// </summary>
/// <param name="col">Initial Column</param>
/// <param name="pno">Player No</param>
void displayFreeMoveInfo(Sint32 col, Sint32 pno) {
	njPrintC(NJM_LOCATION(1, col++), "Mode: Free Movement");
	col++;
	njPrintC(NJM_LOCATION(2, col++), "Move the analog to freely control the character.");
	njPrintC(NJM_LOCATION(2, col++), "Y+Analog = Y Axis");
	njPrintC(NJM_LOCATION(2, col++), "Left Analog = Position");
	njPrintC(NJM_LOCATION(2, col++), "Right Analog = Rotation");

	njPrintC(NJM_LOCATION(2, col++), "D-Pad = Animation");
	njPrintC(NJM_LOCATION(2, col++), "Y (Held) = Fast List");


	//njPrintC(NJM_LOCATION(2, col++), "< > = Load/Save Coordinate Info");
	//njPrintC(NJM_LOCATION(2, col++), "- + = Change Movement Speed");
	//njPrintC(NJM_LOCATION(2, col++), "[ ] = Change Movement Speed Adjust Rate");
	//njPrintC(NJM_LOCATION(2, col++), "B N = Change Motion Mode");
	//njPrint(NJM_LOCATION(2, col++), "F  = Keep player frozen on exit (Currently %s)", playerFreeze[pno] ? "ON " : "OFF");
	njPrintC(NJM_LOCATION(2, col++), "A Button = Quick Exit (will force unfreeze)");
	njPrintC(NJM_LOCATION(2, col++), "B Button = Switch Modes (while frozen)");

	col++;
	col++;

	Sint32 infoCol = col;
	njPrintC(NJM_LOCATION(2, col++), "CURRENT INFO");

	Sint32 colArrow = col + moveID;
	njPrintC(NJM_LOCATION(1, colArrow), ">");

	//Check for errors
	Sint32 errorcol = col;
	for (Float* i = &playertwp[pno]->pos.x; i < (Float*)&playertwp[pno]->cwp; ++i) {
		if (njAbs(*i) > 1000000.0f) {
			njPrintColor(0xFFFF0000);
			njPrintC(NJM_LOCATION(37, errorcol++), "What the fuck are you doing");
			njPrintColor(0xFFFFFFFF);
		}

		if (*i != *i) {
			njPrintColor(0xFFFF0000);
			njPrintC(NJM_LOCATION(37, errorcol++), "NaN.");
			if (playertwp[pno] && playertwp[pno]->counter.b[1] == PLNO_KNUCKLES)
				njPrintC(NJM_LOCATION(37, errorcol++), "TIP: Knuckles needs a death plane");
				njPrintC(NJM_LOCATION(37, errorcol++), "     below him to stay in the universe.");
			njPrintColor(0xFFFFFFFF);
			break;
		}
	}

	njPrint(NJM_LOCATION(2, col++), "Animation = %d", playerpwp[pno]->mj.reqaction);
	njPrint(NJM_LOCATION(2, col++), "POS X = %.2f", playertwp[pno]->pos.x);
	njPrint(NJM_LOCATION(2, col++), "POS Y = %.2f", playertwp[pno]->pos.y);
	njPrint(NJM_LOCATION(2, col++), "POS Z = %.2f", playertwp[pno]->pos.z);
	njPrint(NJM_LOCATION(2, col++), "ANG X = %04X", (Uint16)playertwp[pno]->ang.x);
	njPrint(NJM_LOCATION(2, col++), "ANG Y = %04X", (Uint16)playertwp[pno]->ang.y);
	njPrint(NJM_LOCATION(2, col++), "ANG Z = %04X", (Uint16)playertwp[pno]->ang.z);
	njPrint(NJM_LOCATION(2, col++), "SCL X = %.2f", playertwp[pno]->scl.x);
	njPrint(NJM_LOCATION(2, col++), "SCL Y = %.2f", playertwp[pno]->scl.y);
	njPrint(NJM_LOCATION(2, col++), "SCL Z = %.2f", playertwp[pno]->scl.z);
	njPrint(NJM_LOCATION(2, col++), "FACING ANG Y = %04X", (Uint16)(0x4000 - (Uint16)playertwp[pno]->ang.y));
	col++;
	njPrintC(NJM_LOCATION(21, infoCol++), "SAVED INFO");
	njPrint(NJM_LOCATION(21, infoCol++), "Animation = %d", savedCoords[pno].anim);
	njPrint(NJM_LOCATION(21, infoCol++), "POS X = %.2f", savedCoords[pno].pos.x);
	njPrint(NJM_LOCATION(21, infoCol++), "POS Y = %.2f", savedCoords[pno].pos.y);
	njPrint(NJM_LOCATION(21, infoCol++), "POS Z = %.2f", savedCoords[pno].pos.z);
	njPrint(NJM_LOCATION(21, infoCol++), "ANG X = %04X", (Uint16)savedCoords[pno].ang.x);
	njPrint(NJM_LOCATION(21, infoCol++), "ANG Y = %04X", (Uint16)savedCoords[pno].ang.y);
	njPrint(NJM_LOCATION(21, infoCol++), "ANG Z = %04X", (Uint16)savedCoords[pno].ang.z);
	njPrint(NJM_LOCATION(21, infoCol++), "SCL X = %.2f", savedCoords[pno].scl.x);
	njPrint(NJM_LOCATION(21, infoCol++), "SCL Y = %.2f", savedCoords[pno].scl.y);
	njPrint(NJM_LOCATION(21, infoCol++), "SCL Z = %.2f", savedCoords[pno].scl.z);

	njPrint(NJM_LOCATION(2, col++), "Freeze Player On Exit: %s", playerFreeze[pno] ? "ON " : "OFF");
	njPrint(NJM_LOCATION(2, col++), "Movement Speed = %.3f", movementSpeed);
	njPrint(NJM_LOCATION(2, col++), "Adjust Rate = %.3f", movementSpeedRate);
	njPrint(NJM_LOCATION(2, col++), "Motion Mode: %s", mtnList[md_mtn - 3].c_str());
	njPrintC(NJM_LOCATION(2, col++), "<- LOAD / SAVE ->");
	njPrintC(NJM_LOCATION(2, col++), "BACK");
}

/// <summary>
/// Get length of plactptr for each character
/// </summary>
/// <param name="pno"></param>
/// <returns></returns>
static Sint32 getAnimLimit(Sint32 pno) {
	if (playertwp[pno]) {
		switch (playertwp[pno]->counter.b[1]) {
		case PLNO_SONIC: return playerpwp[pno]->equipment & 0x8000 ? 146 : 133;
		case PLNO_EGGMAN: return 23;
		case PLNO_TAILS: return 135;
		case PLNO_KNUCKLES: return 104;
		case PLNO_TIKAL: return 8;
		case PLNO_AMY: return 101;
		case PLNO_E102: return 90;
		case PLNO_BIG: return 104;
		default: return 0;
		}
	}
	return 0;
}

void movementKeyboardCtrls(Sint32 pno) {

	//Movement Speed
	if (KeyGetOn(KEYS_MINUS)) {
		movementSpeed = NJM_MAX(0.0f, movementSpeed - movementSpeedRate);
	}
	else if (KeyGetOn(KEYS_PLUS)) {
		movementSpeed = NJM_MIN(10000.0f, movementSpeed + movementSpeedRate);
	}

	//Adjust Rate
	if (KeyGetOn(KEYS_LBRACKET)) {
		movementSpeedRate = NJM_MAX(0.0f, movementSpeedRate - 0.001f);
	}
	else if (KeyGetOn(KEYS_RBRACKET)) {
		movementSpeedRate = NJM_MIN(1.0f, movementSpeedRate + 0.001f);
	}

	//Save & Load
	if (KeyGetPress(KEYS_RARROW)) {
		if (playertwp[pno]) {
			savedCoords[pno].anim = playerpwp[pno]->mj.reqaction;
			savedCoords[pno].pos = playertwp[0]->pos;
			savedCoords[pno].ang = playertwp[0]->ang;
			savedCoords[pno].scl = playertwp[0]->scl;
		}
	}
	else if (KeyGetPress(KEYS_LARROW)) {
		if (playertwp[pno]) {
			playerpwp[pno]->mj.reqaction = savedCoords[pno].anim;
			playertwp[pno]->pos = savedCoords[pno].pos;
			playertwp[pno]->ang = savedCoords[pno].ang;
			playertwp[pno]->scl = savedCoords[pno].scl;
		}
	}


	//Motion Mode
	if (KeyGetPress(KEYS_B)) {
		md_mtn = NJM_MAX(3, md_mtn - 1);
		if (md_mtn == 6) { //Don't do MD_MTN_TXEN, it resets to 0 a lot so navigating the list is a pain
			--md_mtn;
		}
	}
	else if (KeyGetPress(KEYS_N)) {
		md_mtn = NJM_MIN(7, md_mtn + 1);
		if (md_mtn == 6) { //Don't do MD_MTN_TXEN, it resets to 0 a lot so navigating the list is a pain
			++md_mtn;
		}
	}
}

/// <summary>
/// View animations in the character's plactptr (in-game animation list).
/// </summary>
/// <param name="pno">Player No</param>
void doBasicAnimation(Sint32 pno) {
	//Animation
	Uint16 runningAnim = playerpwp[pno]->mj.reqaction;

	//Selection Arrow
	Uint32 isPressorOn = per[0]->press;
	if (per[0]->on & Buttons_Y) {
		isPressorOn = per[0]->on & ~Buttons_Y;
	}

	switch (isPressorOn) {
	case Buttons_Right:
		switch (moveID) {
		case 0:
			if (runningAnim == getAnimLimit(pno)) runningAnim = 0; else runningAnim += 1;
			break;
		case 12:
			playerFreeze[pno] = playerFreeze[pno] != 1;
			break;
		case 13:
			movementSpeed = NJM_MIN(10000.0f, movementSpeed + movementSpeedRate);
			break;
		case 14:
			movementSpeedRate = NJM_MIN(1.0f, movementSpeedRate + 0.001f);
			break;
		case 15:
			md_mtn = NJM_MIN(7, md_mtn + 1);
			if (md_mtn == 6) { //Don't do MD_MTN_TXEN, it resets to 0 a lot so navigating the list is a pain
				++md_mtn;
				break;
			}
			break;
		}
		break;
	case Buttons_Left:
		switch (moveID) {
		case 0:
			runningAnim = NJM_MAX(0, runningAnim - 1);
			break;
		case 12:
			playerFreeze[pno] = playerFreeze[pno] != 1;
			break;
		case 13:
			movementSpeed = NJM_MAX(0.0f, movementSpeed - movementSpeedRate);
			break;
		case 14:
			movementSpeedRate = NJM_MAX(0.0f, movementSpeedRate - 0.001f);
			break;
		case 15:
			md_mtn = NJM_MAX(3, md_mtn - 1);
			if (md_mtn == 6) { //Don't do MD_MTN_TXEN, it resets to 0 a lot so navigating the list is a pain
				--md_mtn;
			}
			break;
		}
		break;
	case Buttons_Up:
		moveID = NJM_MAX(0, moveID - 1);
		if (moveID < 12) {
			moveID = 0;
		}
		break;
	case Buttons_Down:
		moveID = NJM_MIN(17, moveID + 1);

		if (moveID > 0 && moveID < 12) {
			moveID = 12;
		}
		break;
	}

	if (moveID == 16) {
		switch (per[0]->press) {
		case Buttons_Left:
			if (playertwp[pno]) {
				playerpwp[pno]->mj.reqaction = savedCoords[pno].anim;
				playertwp[pno]->pos = savedCoords[pno].pos;
				playertwp[pno]->ang = savedCoords[pno].ang;
				playertwp[pno]->scl = savedCoords[pno].scl;
			}
			break;
		case Buttons_Right:
			if (playertwp[pno]) {
				savedCoords[pno].anim = playerpwp[pno]->mj.reqaction;
				savedCoords[pno].pos = playertwp[0]->pos;
				savedCoords[pno].ang = playertwp[0]->ang;
				savedCoords[pno].scl = playertwp[0]->scl;
			}
			break;
		}
	}

	if (per[0]->press & Buttons_Y && moveID == 17) {
		if (playertwp[pno] && playerpwp[pno]) {
			if (!playerFreeze[pno]) {
				if (playerpwp[pno]->equipment & 0x8000)
					playertwp[pno]->mode = 75;
				else
					playertwp[0]->mode = 1;
			}
		}
		moveID = 0;
		maker_mode = 0;
		return;
	}


	movementKeyboardCtrls(pno);

	playerpwp[pno]->mj.reqaction = runningAnim;

	if(playerpwp[pno]->mj.mtnmode > 2) //Don't interfere with MD_MTN_INIT/SET/CHNG, that's very bad
	playerpwp[pno]->mj.mtnmode = md_mtn;
}

/// <summary>
/// Edited Free Movement/Noclip processing
/// </summary>
/// <param name="pno">Player No</param>
void doPlayerMovement(Sint32 pno) {
	//Get analog data
	Float analogLeftX = (Float)(per[0]->x1 << 8);
	Float analogLeftY = (Float)(per[0]->y1 << 8);
	Float analogRightX = (Float)(per[0]->x2 << 8);
	Float analogRightY = (Float)(per[0]->y2 << 8);


	//If driving a cart then get rid of it
	if (taskOfPlayerOn) {
		taskOfPlayerOn->twp->mode = 7; //Explode
		cart_data->ignor_collision = 30;
		playertwp[pno]->cwp->info->a = cart_data->player_colli_r;
		playertwp[pno]->smode = 28;
		taskOfPlayerOn = 0;
	}

	//AKA Shamelessly copy the debug movement code lol
	//Round 2: Copy from the old mod
	if (per[0]->on & Buttons_Y) {
		if (analogLeftY > 3072.0f || analogLeftY < -3072.0f)
			playertwp[pno]->pos.y -= analogLeftY / njSqrt(analogLeftY * analogLeftY) * movementSpeed * input_data[pno].stroke;

		//Y Rotation
		if (analogRightX > 3072.0f) playertwp[pno]->ang.y -= (Uint16)(movementSpeed * 100.0f * analogRightX / 32768.0f);
		if (analogRightX < -3072.0f) playertwp[pno]->ang.y += (Uint16)(movementSpeed * 100.0f * analogRightX / -32768.0f);
	}
	else {
		if (analogLeftX > 3072.0f || analogLeftX < -3072.0f || analogLeftY > 3072.0f || analogLeftY < -3072.0f) {
			playertwp[pno]->pos.x += njCos(input_data[pno].angle) * movementSpeed * input_data[pno].stroke;
			playertwp[pno]->pos.z += njSin(input_data[pno].angle) * movementSpeed * input_data[pno].stroke;
		}
		if (analogRightX > 3072.0f || analogRightX < -3072.0f || analogRightY > 3072.0f || analogRightY < -3072.0f) {
			Angle dir = -camera_twp->ang.y + njArcTan2(analogRightY, analogRightX);
			Float mag = analogRightY * analogRightY + analogRightX * analogRightX;
			mag = njSqrt(mag) * mag * 3.918702724E-14f;
			playertwp[pno]->ang.x += (Uint16)(njSin(dir) * movementSpeed * 50.0f * mag);
			playertwp[pno]->ang.z += (Uint16)(njCos(dir) * movementSpeed * -50.0f * mag);
		}
	}
}