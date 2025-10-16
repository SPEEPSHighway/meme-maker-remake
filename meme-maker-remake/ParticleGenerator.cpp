#include "SADXModLoader.h"
#include "FreeCamera.h"
#include "Keys.h"

static Sint32 particle_editID = 1;
static Sint32 particleID;
static Sint32 particleSlot;
static Sint32 particleInterval = 10;
static NJS_POINT3 pos;
static NJS_POINT3 velo = { 0.0f, 1.0f, 0.0f };
static Float scl = 1.0f;
static Bool isEdit;
static NJS_POINT3 pointCoords;

Float movementSpeedParticle = 3.0f;
Float movementSpeedRateParticle = 0.1f;

struct maker_particle {
	Sint8 mode;
	Sint8 pl_id;
	Sint32 id;
	Sint32 interval;
	Sint32 timer;
	NJS_POINT3 pos;
	NJS_POINT3 velo;
	Float scl;
};

#define PARTICLE_SLOTS_NUM 500
maker_particle particles[PARTICLE_SLOTS_NUM];

enum {
	P_NONE,
	P_BOMB,
	P_FIRE,
	//	P_HITMARK,
	P_LIGHTNING,
	P_SNOW,
	P_SMOKE,
	P_WATER,
	P_RIPPLE,
	P_SPARK,
	P_STAR,
	P_MAX
};

static const std::string particleStr[] = {
	"NONE",
	"BOMB",
	"FIRE",
	//	"HITMARK",
	"LIGHTNING",
	"SNOW",
	"SMOKE",
	"WATER",
	"RIPPLE",
	"SPARK",
	"STAR",
};


/// <summary>
/// HUD for Particle Generator
/// </summary>
void displayParticleInfo(Sint32 col, Sint32 pno) {
	njPrintC(NJM_LOCATION(1, col++), "Mode: Particle Generator");
	col++;
	njPrintC(NJM_LOCATION(2, col++), "Spawn Particles.");
	njPrintC(NJM_LOCATION(2, col++), "D-Pad Left/Right = Change Particle Type");
	njPrintC(NJM_LOCATION(2, col++), "D-Pad Up/Down = Change Slot");
	njPrintC(NJM_LOCATION(2, col++), "Z Button (RB/M) = Spawn Particle");
	//njPrintC(NJM_LOCATION(2, col++), "C = Follow Player");
	//njPrintC(NJM_LOCATION(2, col++), "V = Fixed Point");
	//njPrintC(NJM_LOCATION(2, col++), "+/- = Scale");
	//njPrintC(NJM_LOCATION(2, col++), "[ ] = Interval");
	njPrintC(NJM_LOCATION(2, col++), "/ = Reset");
	col++;
	switch (particles[particleSlot].mode) {
	case 0:
		njPrintC(NJM_LOCATION(2, col++), "PLAYER MODE");
		njPrintC(NJM_LOCATION(2, col++), "Y + L/R = Change Player");
		njPrint(NJM_LOCATION(2, col++), "Currently: PLAYER %d", particles[particleSlot].pl_id + 1);
		break;
	case 1:
		njPrintC(NJM_LOCATION(2, col++), "POINT MODE");
		njPrintC(NJM_LOCATION(2, col++), "F = Edit Point");
		col++;
		break;
	}

	col++;
	njPrint(NJM_LOCATION(2, col++), "Slot currently playing: %s", particleStr[particles[particleSlot].id].c_str());
	col++;
	Sint32 colArrow = col + particle_editID;
	njPrintC(NJM_LOCATION(1, colArrow), ">");
	njPrintC(NJM_LOCATION(2, col++), "BACK");
	njPrint(NJM_LOCATION(2, col++), "Slot: %d", particleSlot + 1);
	njPrint(NJM_LOCATION(2, col++), "Effect: %s", particleStr[particleID].c_str());
	njPrint(NJM_LOCATION(2, col++), "X Velo: %.2f", velo.x);
	njPrint(NJM_LOCATION(2, col++), "Y Velo: %.2f", velo.y);
	njPrint(NJM_LOCATION(2, col++), "Z Velo: %.2f", velo.z);

	njPrint(NJM_LOCATION(2, col++), "Scale: %.2f", scl);
	njPrint(NJM_LOCATION(2, col++), "Interval: %d", particleInterval);
	njPrint(NJM_LOCATION(2, col++), "Mode: %s", particles[particleSlot].mode ? "FIXED POINT" : "FOLLOW PLAYER");
	col++;
	if (isEdit) {
		njPrintC(NJM_LOCATION(2, col++), "Start + Analog = Move Camera");
		njPrintC(NJM_LOCATION(2, col++), "L R = Move Up/Down");
		njPrintC(NJM_LOCATION(2, col++), "- + = Change Movement Speed");
		njPrintC(NJM_LOCATION(2, col++), "[ ] = Change Movement Speed Adjust Rate");
		DispSphere(&pos, scl);
	}

	//Display sphere at the point looking at
	for (Sint32 i = 0; i < PARTICLE_SLOTS_NUM; ++i) {
		DispSphere(&particles[i].pos, particles[i].scl);
	}
}


void setParticle(Sint32 id, Sint32 pno) {

	//Stop playing if one is spawned.
	if (particles[particleSlot].id) {
		particles[particleSlot].id = P_NONE;
		particles[particleSlot].interval = 0;
		particles[particleSlot].timer = 0;
		return;
	}
	else {
		//Start playing
		particles[particleSlot].id = id;
		particles[particleSlot].interval = particleInterval;
		particles[particleSlot].timer = 0;
		particles[particleSlot].pl_id = pno;
		particles[particleSlot].pos = pos;
		particles[particleSlot].velo = velo;
		particles[particleSlot].scl = scl;
	}
}


void playParticles() {
	for (Sint32 i = 0; i < PARTICLE_SLOTS_NUM; ++i) {
		if (++particles[i].timer >= particles[i].interval) {

			//Update pos if assigned to follow a task
			if (particles[i].mode == 0 && playertwp[particles[i].pl_id]) {
				particles[i].pos = playertwp[particles[i].pl_id]->pos;
			}

			//Play the particle effect
			switch (particles[i].id) {
			case P_SNOW:
				CreateSnow(&particles[i].pos, &particles[i].velo, particles[i].scl);
				break;
			case P_SMOKE:
				CreateSmoke(&particles[i].pos, &particles[i].velo, particles[i].scl);
				break;
			case P_WATER:
				CreateWater(&particles[i].pos, &particles[i].velo, particles[i].scl);
				break;
			case P_FIRE:
				CreateFire(&particles[i].pos, &particles[i].velo, particles[i].scl);
				break;
				//	case P_HITMARK:
				//		CreateHitmark(&particles[i].pos, particles[i].scl + 20.0f);
				//		break;
			case P_BOMB:
				CreateBomb(&particles[i].pos, particles[i].scl);
				break;
			case P_LIGHTNING:
				CreateLightning(&particles[i].pos, particles[i].scl);
				break;
			case P_RIPPLE:
				CreateWaterripple(&particles[i].pos, &particles[i].velo, particles[i].scl);
				break;
			case P_SPARK:
				CreateSpark(&particles[i].pos, &particles[i].velo);
				break;
			case P_STAR:
				CreateKiranR(&particles[i].pos, &particles[i].velo, particles[i].scl, 0x1000);
				break;
			default:
				break;
			}
			particles[i].timer = 0;
		}
	}
}

/// <summary>
/// Temporary empty object when editing pos
/// </summary>
/// <param name="tp">Task</param>
static void tempPointObject(task* tp) {
};



void setParticlePoint(Sint32 pno) {
	task* point_tp = NULL;
	if (KeyGetPress(KEYS_F) || (particle_editID == 8 && per[0]->press & Buttons_Y)) {
		isEdit = isEdit != 1; //VS hates our Bool type and kept trying to recommend bitwise with !showDisplay lol

		if (isEdit) {
			particles[particleSlot].pl_id = pno;
			//Create a temp task so we can use the editor camera
			point_tp = CreateElementalTask(2, LEV_2, tempPointObject);
			pTaskWorkEditor = point_tp->twp;
			pTaskWorkEditor->pos = pos;
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
	if (KeyGetPress(KEYS_LARROW)) {
		if (playertwp[pno]) {
			pointCoords = pos;
		}
	}
	else if (KeyGetPress(KEYS_RARROW)) {
		if (playertwp[pno]) {
			if (isEdit && pTaskWorkEditor) { //Account for being in the editor
				pTaskWorkEditor->pos = pointCoords;
			}
			pos = pointCoords;
		}
	}


	//Reset
	if (KeyGetOn(KEYS_FSLASH)) {
		if (playertwp[pno]) {
			if (isEdit && pTaskWorkEditor) {
				pTaskWorkEditor->pos = playertwp[pno]->pos;
			}
			pos = playertwp[pno]->pos;
		}
	}


	//Editing pos of spawner
	if (isEdit) {
		//Get analog data
		Float analogLeftX = (Float)(per[0]->x1 << 8);
		Float analogLeftY = (Float)(per[0]->y1 << 8);
		Float analogRightX = (Float)(per[0]->x2 << 8);
		Float analogRightY = (Float)(per[0]->y2 << 8);

		//Movement Speed
		if (KeyGetOn(KEYS_MINUS)) {
			movementSpeedParticle = NJM_MAX(0.0f, movementSpeedParticle -= movementSpeedRateParticle);
		}
		else if (KeyGetOn(KEYS_PLUS)) {
			movementSpeedParticle = NJM_MIN(10000.0f, movementSpeedParticle += movementSpeedRateParticle);
		}

		//Adjust Rate
		if (KeyGetOn(KEYS_LBRACKET)) {
			movementSpeedRateParticle = NJM_MAX(0.0f, movementSpeedRateParticle -= 0.001f);
		}
		else if (KeyGetOn(KEYS_RBRACKET)) {
			movementSpeedRateParticle = NJM_MIN(1.0f, movementSpeedRateParticle += 0.001f);
		}

		//AKA Shamelessly copy the debug movement code lol
		if (!(per[0]->on & Buttons_Start)) {
			if (per[0]->on & Buttons_R) {
				pTaskWorkEditor->pos.y -= 1.0f * (movementSpeedParticle / 2.0f) * ((Float)per[0]->r) / 255.0f;
			}
			else if (per[0]->on & Buttons_L) {
				pTaskWorkEditor->pos.y += 1.0f * (movementSpeedParticle / 2.0f) * (Float)per[0]->l / 255.0f;
			}
			else {
				if (analogLeftX > 3072.0f || analogLeftX < -3072.0f || analogLeftY > 3072.0f || analogLeftY < -3072.0f) {
					//Game already records left analog angle/magnitude
					pTaskWorkEditor->pos.x += njCos(input_data[pno].angle) * movementSpeedParticle / 2.0f * input_data[pno].stroke;
					pTaskWorkEditor->pos.z += njSin(input_data[pno].angle) * movementSpeedParticle / 2.0f * input_data[pno].stroke;
				}
				if (analogRightX > 3072.0f || analogRightX < -3072.0f || analogRightY > 3072.0f || analogRightY < -3072.0f) {
					Angle dir = -camera_twp->ang.y + njArcTan2(analogRightY, analogRightX);
					pTaskWorkEditor->ang.x += (Uint16)(njSin(dir) * movementSpeedParticle * 50.0f);
					pTaskWorkEditor->ang.z += (Uint16)(njCos(dir) * movementSpeedParticle * -50.0f);
				}
			}
		}

		//Update point if in the editor
		if (pTaskWorkEditor) {
			pos = pTaskWorkEditor->pos;
			particles[particleSlot].pos = pos;
		}

		//Stop the player moving
		playerpwp[pno]->nocontimer = 10;
	}
}

void setPLID(Sint32 pno) {
	//Cycle through modes
	if (per[0]->on & Buttons_Y) {
		switch (per[0]->press) {
		case Buttons_R:
			particles[particleSlot].pl_id = NJM_MIN(8, particles[particleSlot].pl_id + 1);
			break;
		case Buttons_L:
			particles[particleSlot].pl_id = NJM_MAX(0, particles[particleSlot].pl_id - 1);
			break;
		}
	}
}


void doParticleGenerator(Sint32 pno) {
	//If not moving a particle's pos
	if (!isEdit) {
		//Selection Arrow
		Uint32 isPressorOn = per[0]->press;
		if (per[0]->on & Buttons_Y) {
			isPressorOn = per[0]->on & ~Buttons_Y;
		}

		if (per[0]->press & Buttons_Y && particle_editID == 0) {
			particle_editID = 1;
			maker_mode = 0;
			return;
		}

		switch (isPressorOn) {
		case Buttons_Up:
			particle_editID = NJM_MAX(0, particle_editID - 1);
			break;
		case Buttons_Down:
			particle_editID = NJM_MIN(8, particle_editID + 1);
			break;
		case Buttons_Left:
			switch (particle_editID) {
			case 1:
				particleSlot = NJM_MAX(0, particleSlot - 1);
				pos = particles[particleSlot].pos;
				scl = NJM_MAX(1.0f, particles[particleSlot].scl);
				break;
			case 2:
				particleID = NJM_MAX(0, particleID - 1);
				break;
			case 3:
				velo.x = NJM_MAX(-1000.0f, velo.x - 0.1f);
				break;
			case 4:
				velo.y = NJM_MAX(-1000.0f, velo.y - 0.1f);
				break;
			case 5:
				velo.z = NJM_MAX(-1000.0f, velo.z - 0.1f);
				break;
			case 6:
				scl = NJM_MAX(0.0f, scl - 0.01f);
				break;
			case 7:
				particleInterval = NJM_MAX(0, particleInterval - 1);
				break;
			case 8:
				particles[particleSlot].mode = 0;
				break;
			}
			break;
		case Buttons_Right:
			switch (particle_editID) {
			case 1:
				particleSlot = NJM_MIN(particleSlot + 1, PARTICLE_SLOTS_NUM - 1);
				pos = particles[particleSlot].pos;
				scl = NJM_MAX(1.0f, particles[particleSlot].scl);
				break;
			case 2:
				particleID = NJM_MIN(particleID + 1, P_MAX - 1);
				break;
			case 3:
				velo.x = NJM_MIN(1000.0f, velo.x + 0.1f);
				break;
			case 4:
				velo.y = NJM_MIN(1000.0f, velo.y + 0.1f);
				break;
			case 5:
				velo.z = NJM_MIN(1000.0f, velo.z + 0.1f);
				break;
			case 6:
				scl = NJM_MIN(10000.0f, scl + 0.01f);
				break;
			case 7:
				particleInterval = NJM_MIN(9999, particleInterval + 1);
				break;
			case 8:
				particles[particleSlot].mode = 1;
				particles[particleSlot].pos = playertwp[particles[particleSlot].pl_id]->pos;
				pos = particles[particleSlot].pos;
				break;
			}
			break;
		}

		if (per[0]->press & Buttons_Z) {
			setParticle(particleID, pno);
		}

		//Movement Speed
		if (KeyGetOn(KEYS_MINUS)) {
			scl = NJM_MAX(0.0f, scl - 0.1f);
		}
		else if (KeyGetOn(KEYS_PLUS)) {
			scl = NJM_MIN(10000.0f, scl + 0.1f);
		}

		//Interval
		if (KeyGetPress(KEYS_LBRACKET)) {
			particleInterval = NJM_MAX(0, particleInterval - 1);
		}
		else if (KeyGetPress(KEYS_RBRACKET)) {
			particleInterval = NJM_MIN(9999, particleInterval + 1);
		}

		//Modes
		if (KeyGetPress(KEYS_C)) {
			particles[particleSlot].mode = 0;
		}
		if (KeyGetPress(KEYS_V)) {
			particles[particleSlot].mode = 1;
			particles[particleSlot].pos = playertwp[pno]->pos;
			pos = particles[particleSlot].pos;
		}




	}

	//Main mode processor
	switch (particles[particleSlot].mode) {
	case 0:
		setPLID(pno);
		break;
	case 1:
		setParticlePoint(pno);
		break;
	}




	///Reset
	if (KeyGetPress(KEYS_FSLASH)) {
		for (Sint32 i = 0; i < PARTICLE_SLOTS_NUM; ++i) {
			particles[particleSlot].id = P_NONE;
			particles[particleSlot].interval = 10;
			particles[particleSlot].timer = 0;
			particles[particleSlot].pl_id = 0;
			particles[particleSlot].pos = { 0 };
			particles[particleSlot].velo = { 0.0f, 1.0f, 0.0f };
			particles[particleSlot].scl = 1.0f;

			scl = 1.0f;
			particleInterval = 10;
		}
	}
}