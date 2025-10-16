#include "SADXModLoader.h"
#include "FreeCamera.h"
#include "Keys.h"

Sint8 funcID;

//RotaryEmerald
static task* rotEme[7];
static Sint32 rotEmeMode;
static Float rotRad = 7.0f;
static Sint32 rotSpd = 0x130;
static Sint8 rotTimer;

//Ichimaie
static Float IchimaiePri = -70.0f;
static task* ichiMaie_tp[8];

//Background image
static NJS_TEXNAME texture_icm_bg[] = { { (void*)"myBG", 0, 0 } };
static NJS_TEXLIST texlist_icm_bg = { texture_icm_bg, 1 };

//Skywalk
static task* skywalk_tp[8];

static Sint32 cursor;

//SphereBomb
static Bool sphbomb_follow = TRUE;
static task* sphbomb_tp;
static Sint32 sphbomb_editID;

struct sphbomb {
	NJS_POINT3 pos;
	Sint16 frame;
	Float scl_spd;
	Float sphere_radius;
	Float scl_reduce_acc;
	Float vscroll_spd;
};

static sphbomb sphereBomb = { { 0.0f, 0.0f, 0.0f }, 550, 2.0f, 350.0f, 0.8f, 10.0f };



//crushlight
struct CRUSHLIGHT_DATA_maker
{
	NJS_POINT3 pos;
	Sint32 num;
	Sint32 life;
	Float startWidth;
	Float endWidth;
	NJS_COLOR startCol;
	NJS_COLOR endCol;
};

static CRUSHLIGHT_DATA_maker crushlight = {
	{ 0.0f, 0.0f, 0.0f },
	3,
	10,
	0.4f,
	2.0f,
	0xFFFF64FF,
	0x00FF1EFF
};
static Bool crush_follow = TRUE;
static Sint32 crush_editID;
static Sint32 sphbomb_wait;

//Capture beam
static Bool useAngle = TRUE;
static Bool capture_follow = TRUE;
static task* capture_tp;

struct capturebeam {
	NJS_POINT3 pos;
	Angle3 ang;
	NJS_POINT3 scl;
	Float scl_large_spd;
	Float scl_small_spd;
	Angle rot_spd;
};

static capturebeam capture_data = {
	{ 0.0f, 0.0f, 0.0f },
	{ 0, 0, 0 },
	{ 1.0f, 1.0f, 1.0f },
	0.01f,
	0.01f,
	0x300
};

static Sint32 capture_editID;
static Sint32 capture_wait;

//Water Pillar
static Bool wpillar_follow = TRUE;
static NJS_POINT3 wpillar_pos;
static Float wpillar_spd = 0.5f;
static Float wpillar_scl = 0.7f;

//E101Factory
static task* e101FactoryFloor_tp;
task* last1aHighFloor_tp[8];


struct maker_event {
	Sint8 mode;
	Sint32 id;
};

#define EVENTFUNC_SLOTS_NUM 500
maker_event funcs[EVENTFUNC_SLOTS_NUM];

enum {
	FUNC_ROTARY,
	FUNC_ICHIMAIE,
	FUNC_SKYWALK,
	FUNC_SPHBOMB,
	FUNC_CRUSHLIGHT,
	FUNC_CAPTURE,
	FUNC_WPILLAR,
	FUNC_E101FACTORY,
	FUNC_LAST1A,
	FUNC_CHAOS0,
	FUNC_MAX
};


std::string funcStr[] = {
	"RotaryEmerald (Spawn 7 Rotating Chaos Emeralds)",
	"CIchimaie2 (Draw a background texture)",
	"CSkyWalk (Spawn an invisible floor at the player's feet)",
	"SphereBomb (Explosion)",
	"crushLight (Light Beams)",
	"CaptureBeam (Eggman's Steal Thing From Sonic Beam)",
	"WaterPillar (Chaos's Steal Thing From Sonic Beam)",
	"E101Factory (E101's room)",
	"Last1AHigh (Perfect Chaos cutscene platform)",
	"Boss Spawner (Chaos 0)"
};


/// <summary>
/// HUD for Event Functions
/// </summary>
/// 
void displayEventInfo(Sint32 col, Sint32 pno) {
	njPrintC(NJM_LOCATION(1, col++), "Mode: Event Functions");
	col++;
	njPrintC(NJM_LOCATION(2, col++), "Use various event functions..");
	njPrintC(NJM_LOCATION(2, col++), "D-Pad Left/Right = Change Function");
	njPrintC(NJM_LOCATION(2, col++), "Z Button (RB) = Play/Delete");
	njPrintC(NJM_LOCATION(2, col++), "/ = Reset");
	col++;
	col++;

	Sint32 colArrow = col + cursor;
	Sint32 colRow = 1;

	if (funcID == FUNC_CRUSHLIGHT && crush_editID > 3)
		colRow = 14;

	njPrintC(NJM_LOCATION(colRow, colArrow), ">");


	njPrintC(NJM_LOCATION(2, col++), "BACK");
	njPrint(NJM_LOCATION(2, col++), "Effect: %s", funcStr[funcID].c_str());
	switch (funcID) {
	case FUNC_ROTARY:
		njPrint(NJM_LOCATION(2, col++), "Radius: %.2f", rotRad);
		njPrint(NJM_LOCATION(2, col++), "Speed: %02X", rotSpd);
		break;
	case FUNC_ICHIMAIE:
		njPrintC(NJM_LOCATION(2, col++), "Change the image to anything you want.");
		njPrintC(NJM_LOCATION(2, col++), "It's located in Meme Maker 2/textures/ICM_BG");
		njPrint(NJM_LOCATION(2, col++), "Priority: %.2f", IchimaiePri);
		break;
	case FUNC_SKYWALK:
		njPrintC(NJM_LOCATION(2, col++), "Plane will spawn at the player's feet.");
		njPrint(NJM_LOCATION(2, col++), "Loaded: %s", skywalk_tp[pno] ? "TRUE" : "FALSE");
		if (skywalk_tp[pno]) {
			njPrint(NJM_LOCATION(2, col++), "Location: %.2f", skywalk_tp[pno]->twp->pos.y);
		}
		break;
	case FUNC_SPHBOMB:
	{
		njPrintC(NJM_LOCATION(2, col++), "Explosion will spawn at sphere's position.");
		njPrint(NJM_LOCATION(2, col++), "Loaded: %s", sphbomb_tp ? "TRUE" : "FALSE");
		njPrint(NJM_LOCATION(2, col++), "Frozen: %s", sphbomb_follow ? "N" : "Y");
		njPrint(NJM_LOCATION(2, col++), "X Pos: %.2f", sphereBomb.pos.x);
		njPrint(NJM_LOCATION(2, col++), "Y Pos: %.2f", sphereBomb.pos.y);
		njPrint(NJM_LOCATION(2, col++), "Z Pos: %.2f", sphereBomb.pos.z);
		col++;
		njPrint(NJM_LOCATION(2, col++), "Frame: %d", sphereBomb.frame);
		njPrint(NJM_LOCATION(2, col++), "Scale Speed: %.2f", sphereBomb.scl_spd);
		njPrint(NJM_LOCATION(2, col++), "Sphere Radius: %.2f", sphereBomb.sphere_radius);
		njPrint(NJM_LOCATION(2, col++), "Scale Decel: %.2f", sphereBomb.scl_reduce_acc);
		njPrint(NJM_LOCATION(2, col++), "VScroll Speed: %d", (Sint32)sphereBomb.vscroll_spd);
		DispSphere(&sphereBomb.pos, 1.0f);
		break;
	}
	case FUNC_CRUSHLIGHT:
	{
		njPrintC(NJM_LOCATION(2, col++), "Light will spawn at sphere's position.");
		njPrint(NJM_LOCATION(2, col++), "Loaded: %s", p_CrushLightTask ? "TRUE" : "FALSE");
		njPrint(NJM_LOCATION(2, col++), "Frozen: %s", crush_follow ? "N" : "Y");
		njPrint(NJM_LOCATION(2, col++), "X Pos: %.2f", crushlight.pos.x);
		njPrint(NJM_LOCATION(2, col++), "Y Pos: %.2f", crushlight.pos.y);
		njPrint(NJM_LOCATION(2, col++), "Z Pos: %.2f", crushlight.pos.z);

		if (crushlight.num > 40 && crushlight.life > 40) {
			njPrintC(NJM_LOCATION(2, col), "WARNING! High num+life can fill memory and crash the game.");
		}

		col++;
		njPrint(NJM_LOCATION(2, col++), "Num: %d", crushlight.num);
		njPrint(NJM_LOCATION(2, col++), "Life: %d", crushlight.life);
		njPrint(NJM_LOCATION(2, col++), "Start Width: %.2f", crushlight.startWidth);
		njPrint(NJM_LOCATION(2, col++), "End Width: %.2f", crushlight.endWidth);
		njPrint(NJM_LOCATION(2, col++), "Start Colour A: %d", crushlight.startCol.argb.a);
		njPrint(NJM_LOCATION(2, col++), "             R: %d", crushlight.startCol.argb.r);
		njPrint(NJM_LOCATION(2, col++), "             G: %d", crushlight.startCol.argb.g);
		njPrint(NJM_LOCATION(2, col++), "             B: %d", crushlight.startCol.argb.b);
		njPrint(NJM_LOCATION(2, col++), "End Colour   A: %d", crushlight.endCol.argb.a);
		njPrint(NJM_LOCATION(2, col++), "             R: %d", crushlight.endCol.argb.r);
		njPrint(NJM_LOCATION(2, col++), "             G: %d", crushlight.endCol.argb.g);
		njPrint(NJM_LOCATION(2, col++), "             B: %d", crushlight.endCol.argb.b);
		DispSphere(&crushlight.pos, 5.0f);

		break;
	}
	case FUNC_CAPTURE:
	{
		njPrintC(NJM_LOCATION(2, col++), "Beam will spawn at sphere's position.");
		njPrint(NJM_LOCATION(2, col++), "Loaded: %s", capture_tp ? "TRUE" : "FALSE");

		njPrint(NJM_LOCATION(2, col++), "Frozen: %s", capture_follow ? "N" : "Y");
		njPrint(NJM_LOCATION(2, col++), "X Pos: %.2f", capture_data.pos.x);
		njPrint(NJM_LOCATION(2, col++), "Y Pos: %.2f", capture_data.pos.y);
		njPrint(NJM_LOCATION(2, col++), "Z Pos: %.2f", capture_data.pos.z);
		col++;
		njPrint(NJM_LOCATION(2, col++), "X Scl: %.2f", capture_data.scl.x);
		njPrint(NJM_LOCATION(2, col++), "Y Scl: %.2f", capture_data.scl.y);
		njPrint(NJM_LOCATION(2, col++), "Z Scl: %.2f", capture_data.scl.z);
		njPrint(NJM_LOCATION(2, col++), "Scl Large Spd: %.2f", capture_data.scl_large_spd);
		njPrint(NJM_LOCATION(2, col++), "Scl Small Spd: %.2f", capture_data.scl_small_spd);
		njPrint(NJM_LOCATION(2, col++), "Rot Speed: %d", capture_data.rot_spd);
		njPrint(NJM_LOCATION(2, col++), "Use Angle: %s", useAngle ? "TRUE" : "FALSE");
		DispSphere(&capture_data.pos, njScalor(&capture_data.scl));

		break;
	}
	case FUNC_WPILLAR:
		njPrint(NJM_LOCATION(2, col++), "Frozen: %s", wpillar_follow ? "N" : "Y");
		njPrint(NJM_LOCATION(2, col++), "X Pos: %.2f", wpillar_pos.x);
		njPrint(NJM_LOCATION(2, col++), "Y Pos: %.2f", wpillar_pos.y);
		njPrint(NJM_LOCATION(2, col++), "Z Pos: %.2f", wpillar_pos.z);
		col++;
		njPrint(NJM_LOCATION(2, col++), "SPEED: %.2f", wpillar_spd);
		njPrint(NJM_LOCATION(2, col++), "SCALE: %.2f", wpillar_scl);

		DispSphere(&wpillar_pos, wpillar_scl);

		break;
	case FUNC_E101FACTORY:
		njPrintC(NJM_LOCATION(2, col++), "Egg Carrier Main Area Only.");
		njPrint(NJM_LOCATION(2, col++), "Loaded: %s", e101Factory_tp ? "TRUE" : "FALSE");
		break;
	case FUNC_LAST1A:
		njPrintC(NJM_LOCATION(2, col++), "Perfect Chaos Only.");
		njPrint(NJM_LOCATION(2, col++), "Loaded: %s", p_Last1AHighTask ? "TRUE" : "FALSE");
		if (playerpwp[pno] && playertwp[pno]->counter.b[1] == PLNO_SONIC) {
			njPrint(NJM_LOCATION(2, col++), "Super Sonic: %s", playerpwp[pno]->equipment & 0x8000 ? "Y" : "N");
		}
		break;
	case FUNC_CHAOS0:
		njPrintC(NJM_LOCATION(2, col++), "Spawn Chaos 0.");
		njPrint(NJM_LOCATION(2, col++), "Loaded: %s", p_Last1AHighTask ? "TRUE" : "FALSE");
		break;
	}



	col++;
}

NJS_TEXLIST* rotEmeTex[] = {
	&texlist_m_em_red,
	&texlist_m_em_green,
	&texlist_m_em_white,
	&texlist_m_em_yellow,
	&texlist_m_em_purple,
	&texlist_m_em_sky,
	&texlist_m_em_blue
};


/// <summary>
/// Deletes the water pillar and its child tasks.
/// </summary>
void wPillar_clear() {
	if (p_WaterPillarTask) {
		WaterPillarOff();

		if (p_PillarTask) {
			FreeTask(p_PillarTask);
			p_PillarTask = 0;
		}
		if (p_SplashTask) {
			FreeTask(p_SplashTask);
			p_SplashTask = 0;
		}
	}
}

void checkLast1A(Sint32 pno) {
	if (!EV_Check() && p_Last1AHighTask && !last1aHighFloor_tp[pno] && playertp[pno]) {
		last1aHighFloor_tp[pno] = CSkyWalk_create2(playertp[pno], 150.0f);
	}
}

void setFunc(Sint32 id, Sint32 pno) {

	switch (funcID) {
	case FUNC_ROTARY:
		switch (rotEmeMode) {
		case 0:

			//Create the emeralds
			for (Sint32 i = 0; i < 7; ++i) {
				if (rotEme[i]) {
					FreeTask(rotEme[i]);
					rotEme[i] = 0;
				}
				EV_CreateObject(&rotEme[i], 0.0f, 0.0f, 0.0f, 0, 0, 0);
			}

			//Load Emeralds' Textures
			LoadPVM("M_EM_BLUE", &texlist_m_em_blue);
			LoadPVM("M_EM_RED", &texlist_m_em_red);
			LoadPVM("M_EM_YELLOW", &texlist_m_em_yellow);
			LoadPVM("M_EM_SKY", &texlist_m_em_sky);
			LoadPVM("M_EM_WHITE", &texlist_m_em_white);
			LoadPVM("M_EM_PURPLE", &texlist_m_em_purple);
			LoadPVM("M_EM_GREEN", &texlist_m_em_green);


			//If creation loop completed, create the ring of emeralds.
			if (rotEme[6]) {
				CreateRotaryEmerald(playertwp[pno]->pos.x, playertwp[pno]->pos.y + playerpwp[pno]->p.center_height, playertwp[pno]->pos.z, rotRad, rotSpd,
					rotEme[0], rotEme[1], rotEme[2], rotEme[3], rotEme[4], rotEme[5], rotEme[6]);
			}
			else {
				PrintDebug("\nMeme Maker 2: ERROR! Couldn't create RotaryEmerald.");
				rotEmeMode = 0;
				break;
			}

			for (Sint32 i = 0; i < 7; ++i) {
				EV_SetAction(rotEme[i], &action_m_em_blue, rotEmeTex[i], 1.0f, 1, 0);
				EV_SetMode(rotEme[i], 0);
			}

			//Glow effects
			SetEffectRotaryEmerald(1, &ev_effect_list19, 0.20f, 1.0f, 1.0f, 0.0f, 0.0f);
			SetEffectRotaryEmerald(2, &ev_effect_list19, 0.20f, 1.0f, 0.0f, 1.0f, 0.0f);
			SetEffectRotaryEmerald(3, &ev_effect_list19, 0.20f, 1.0f, 1.0f, 1.0f, 1.0f);
			SetEffectRotaryEmerald(4, &ev_effect_list19, 0.20f, 1.0f, 1.0f, 1.0f, 0.0f);
			SetEffectRotaryEmerald(5, &ev_effect_list19, 0.20f, 1.0f, 1.0f, 0.0f, 1.0f);
			SetEffectRotaryEmerald(6, &ev_effect_list19, 0.20f, 1.0f, 0.0f, 1.0f, 1.0f);
			SetEffectRotaryEmerald(7, &ev_effect_list19, 0.20f, 1.0f, 0.0f, 0.0f, 1.0f);

			++rotEmeMode;
			break;
		case 1:
			SplashRotaryEmerald(5.0f); //Scatter them
			++rotEmeMode;
			break;
		default:
			break;
		}
		break;
	case FUNC_ICHIMAIE:
		if (!ichiMaie_tp[pno]) {
			LoadPVM("ICM_BG", &texlist_icm_bg);
			ichiMaie_tp[pno] = CIchimaie2_Create(&texlist_icm_bg, 0);
			CIchimaie2_SetPriority(ichiMaie_tp[pno], IchimaiePri);
			CIchimaie2_SetDstAlpha(ichiMaie_tp[pno], 1.0f, 1);
		}
		else {
			FreeTask(ichiMaie_tp[pno]);
			ichiMaie_tp[pno] = 0;
			late_ReleaseTexture(&texlist_icm_bg);
		}
		break;
	case FUNC_SKYWALK:
		if (!skywalk_tp[pno]) {

			if (playermwp[pno] && playermwp[pno]->spd.y < 0.0f)
				playermwp[pno]->spd.y = 0;
			if (playerpwp[pno] && playerpwp[pno]->spd.y < 0.0f)
				playerpwp[pno]->spd.y = 0;
			skywalk_tp[pno] = CSkyWalk_create2(playertp[pno], playertwp[pno]->pos.y);
		}
		else {
			FreeTask(skywalk_tp[pno]);
			skywalk_tp[pno] = 0;
		}
		break;
	case FUNC_SPHBOMB:
		if (!sphbomb_tp) {
			sphbomb_tp = Create_SphereBomb(sphereBomb.pos.x, sphereBomb.pos.y, sphereBomb.pos.z,
				sphereBomb.frame, sphereBomb.scl_spd, sphereBomb.sphere_radius);
		}
		else {
			ChgSphereBombMode(sphbomb_tp, sphbomb_tp->twp->mode + 1);
		}
		break;
	case FUNC_CRUSHLIGHT:
		if (!p_CrushLightTask) {
			crushLightOn(crushlight.pos.x, crushlight.pos.y, crushlight.pos.z,
				crushlight.num, crushlight.life, crushlight.startWidth, crushlight.endWidth,
				crushlight.startCol.color, crushlight.endCol.color);
		}
		else {
			crushLightOff();
		}
		break;
	case FUNC_CAPTURE:
		if (!capture_tp) {
			LoadPVM("CAPTUREBEAM", &texlist_capturebeam);

			//Using angle causes the beam to tilt on sloped surfaces when following the player.
			if (!useAngle)
				capture_data.ang = { 0 };

			capture_tp = CreateCaptureBeam(capture_data.pos.x, capture_data.pos.y, capture_data.pos.z,
				capture_data.ang.x, capture_data.ang.y, capture_data.ang.z);
		}
		else {
			ChgCaptureMod(capture_tp, capture_tp->twp->mode + 1);
		}
		break;
	case FUNC_WPILLAR:
		if (!p_WaterPillarTask) {
			LoadPVM("L_SIBUKI", &texlist_l_sibuki);
			WaterPillarOn(wpillar_pos.x, wpillar_pos.y, wpillar_pos.z, wpillar_spd, wpillar_scl);
		}
		else {
			wPillar_clear();
		}
		break;
	case FUNC_E101FACTORY:
		if (GetStageNumber() == 0x2001)
			if (!e101Factory_tp) {

				//This area has no collision so use skywalk to create a floor for it.
				if (!e101FactoryFloor_tp)
					e101FactoryFloor_tp = CSkyWalk_create2(playertp[0], 500.0f);

				LoadPVM("PVME101FACTORY", &texlist_e101factory);
				E101Factory_Create();

				BGM_Play(40);
				PClearSpeed(playermwp[0], playerpwp[0]);
				SetPositionP(0, -95.0f, 500.0f, 0.0f);
			}
			else {
				E101Factory_Delete();

				BGM_Play(31);

				PClearSpeed(playermwp[0], playerpwp[0]);
				SetPlayerInitialPosition(playertwp[0]);

				if (e101FactoryFloor_tp) {
					FreeTask(e101FactoryFloor_tp);
					e101FactoryFloor_tp = 0;
				}
			}
		break;
	case FUNC_LAST1A:
		if (GetStageNumber() == 0x1300) {
			if (!p_Last1AHighTask) {

				//This area has no collision so use skywalk to create a floor for it.
				for(Sint32 i = 0; i < 8; ++i){
					if (!last1aHighFloor_tp[i] && playertp[0]) {
						last1aHighFloor_tp[i] = CSkyWalk_create2(playertp[0], 150.0f);
					}
				}

				PauseMusic();

				chaos7_game_clear_flag = 1;
				LoadPVM("LAST1A_HIGHWAY_A", &texlist_last1a_highway_a);
				Last1AHigh_Create();

				PClearSpeed(playermwp[0], playerpwp[0]);
				SetPositionP(0, 806.63f, 150.0f, -251.4f);
				StgChaos7CtrlOn(150.0, 0.0, 50.0, 0, 0, 0, 910);

			}
			else {
				if (playertwp[0] && playertwp[0]->counter.b[1] == PLNO_SONIC && playertwp[0]->mode != 99)
					SetInputP(0, 46);

				StgChaos7CtrlOff(0);
				chaos7_game_clear_flag = 0;
				ResumeMusic();
				for (Sint32 i = 0; i < 8; ++i) {
					if (last1aHighFloor_tp[i]) {
						FreeTask(last1aHighFloor_tp[i]);
						last1aHighFloor_tp[i] = 0;
					}
				}
				SetPlayerInitialPosition(playertwp[0]);
				Last1AHigh_Delete();
			}
		}
		break;
	case FUNC_CHAOS0:
		if (!p_Chaos0Task) {
			if (playertwp[pno]) {
				njLoadTexturePvmFile("CHAOS0", BOSSCHAOS0_TEXLISTS[0]);
				njLoadTexturePvmFile("EV_CHAOS0_MANJU", &texlist_ev_chaos0_manju);
				CreateChaos0(playertwp[pno]->pos.x, playertwp[pno]->pos.y, playertwp[pno]->pos.z,
					playertwp[pno]->ang.x, playertwp[pno]->ang.y, playertwp[pno]->ang.z, 80);
			}
		}
		else {
			DeleteChaos0();
			p_Chaos0Task = 0;
		}
		break;
	}

}

void advanceEventEffects() {
	//Time until it deletes the emeralds if they've been scattered.
	if (g_RotaryEmerald_p && rotEmeMode == 2 && ++rotTimer > 60) {
		rotTimer = 0;
		deleteRotaryEmeraldTask();
		for (Sint32 i = 0; i < 7; ++i) {
			FreeTask(rotEme[i]);
			rotEme[i] = 0;
		}
		rotEmeMode = 0;
	}

	//Advancement for sphbomb
	if (sphbomb_tp) {
		switch (sphbomb_tp->twp->mode) {
		case 0:
			if (++sphbomb_wait > 5) {
				SetSphereBombParameter(sphbomb_tp, sphereBomb.scl_reduce_acc, sphereBomb.vscroll_spd, sphereBomb.sphere_radius);
				ChgSphereBombMode(sphbomb_tp, sphbomb_tp->twp->mode + 1);

			}
			break;
		case 2:
			if (--sphbomb_wait == 0) {
				FreeTask(sphbomb_tp);
				sphbomb_tp = 0;
				sphbomb_wait = 0;
			}

			break;
		}
	}

	//Advancement for capture
	if (capture_tp) {
		switch (capture_tp->twp->mode) {
		case 0:
			if (++capture_wait > 5) {
				SetCaptureParam(capture_tp, capture_data.scl.x, capture_data.scl.y, capture_data.scl.z, capture_data.scl_large_spd, capture_data.scl_small_spd, capture_data.rot_spd);
				ChgCaptureMod(capture_tp, capture_tp->twp->mode + 1);

			}
			break;
		case 2:
			if (!capture_tp->twp->scl.x && !capture_tp->twp->scl.z && --capture_wait == 0) {
				FreeTask(capture_tp);
				capture_tp = 0;
				capture_wait = 0;
			}

			break;
		}
	}

	//Advancement for wpillar
	if (p_WaterPillarTask && p_WaterPillarTask->twp->mode == 2)
		wPillar_clear();
}

void toggleSuperSonic() {
	if (maker_mode == 6 && playerpwp[0] && playertwp[0]->counter.b[1] == PLNO_SONIC) {
		if (playerpwp[0]->equipment & 0x8000) {
			playerpwp[0]->equipment &= ~0x8000;

			if (playertwp[0]->mode != 99) {
				playertwp[0]->mode = 1;
				SetInputP(0, 24); //Prevent smode lingering and causing him to re-transform in certain situations.
				
			}
		}
		else if (playertwp[0]->mode != 99) {
			if (GetStageNumber() != 0x1300) {
				texLoadTexturePvmFile("SUPERSONIC", &SUPERSONIC_TEXLIST);
			}

			SetInputP(0, 46);
		}
	}
}

void eventFunctionsMain(Sint32 pno) {

	//ID controls



			//Selection Arrow
	Uint32 isPressorOn = per[0]->press;
	if (per[0]->on & Buttons_Y) {
		isPressorOn = per[0]->on & ~Buttons_Y;
	}

	if (per[0]->press & Buttons_Y && cursor == 0) {
		cursor = 1;
		maker_mode = 0;
		return;
	}

	switch (isPressorOn) {
	case Buttons_Up:
		cursor = NJM_MAX(0, cursor - 1);

		switch (funcID) {
		case FUNC_ICHIMAIE:
			if (cursor > 1 && cursor < 4)
				cursor = 1;
			break;
		case FUNC_SPHBOMB:
			if (cursor > 1 && cursor < 4)
				cursor = 1;

			if (cursor > 4 && cursor < 9)
				cursor = 4;
			break;
		case FUNC_CRUSHLIGHT:
			if (cursor > 1 && cursor < 4)
				cursor = 1;

			if (cursor > 4 && cursor < 9)
				cursor = 4;
			break;
		case FUNC_CAPTURE:
			if (cursor > 1 && cursor < 4)
				cursor = 1;

			if (cursor > 4 && cursor < 9)
				cursor = 4;
			break;
		case FUNC_WPILLAR:
			if (cursor > 2 && cursor < 7)
				cursor = 2;
			break;
		case FUNC_LAST1A:
			if (cursor > 1 && cursor < 4)
				cursor = 1;
			break;
		}

		break;
	case Buttons_Down:
		++cursor;

		switch (funcID) {
		case FUNC_ROTARY:
			cursor = NJM_MIN(3, cursor);
			break;
		case FUNC_ICHIMAIE:
			cursor = NJM_MIN(4, cursor);
			if (cursor > 1 && cursor < 4)
				cursor = 4;
			break;
		case FUNC_SKYWALK:
			cursor = 1;
			break;
		case FUNC_SPHBOMB:
			cursor = NJM_MIN(13, cursor);
			if (cursor > 1 && cursor < 4)
				cursor = 4;

			if (cursor > 4 && cursor < 9)
				cursor = 9;
			break;
		case FUNC_CRUSHLIGHT:
			cursor = NJM_MIN(20, cursor);
			if (cursor > 1 && cursor < 4)
				cursor = 4;

			if (cursor > 4 && cursor < 9)
				cursor = 9;
			break;
		case FUNC_CAPTURE:
			cursor = NJM_MIN(15, cursor);
			if (cursor > 1 && cursor < 4)
				cursor = 4;

			if (cursor > 4 && cursor < 9)
				cursor = 9;

			break;
		case FUNC_WPILLAR:
			cursor = NJM_MIN(8, cursor);
			if (cursor > 2 && cursor < 7)
				cursor = 7;
			break;
		case FUNC_LAST1A:
			cursor = NJM_MIN(4, cursor);
			if (cursor > 1 && cursor < 4)
				cursor = 4;

			if (cursor > 1 && playertwp[pno]->counter.b[1] != PLNO_SONIC) {
				cursor = 1;
			}
			break;
		default:
			cursor = NJM_MIN(1, cursor);
			break;
		}


		break;
	case Buttons_Right:
		switch (cursor) {
		case 1:
			funcID = NJM_MIN(funcID + 1, FUNC_MAX - 1);
			break;
		case 2:
			switch (funcID) {
			case FUNC_ROTARY:
				rotRad = NJM_MIN(10000.0f, rotRad + 0.1f);
				break;
			case FUNC_WPILLAR:
				wpillar_follow = wpillar_follow != 1;
				break;
			}
			break;
		case 3:
			switch (funcID) {
			case FUNC_ROTARY:
				rotSpd = NJM_MIN(9999, rotSpd + 1);
				break;
			}
			break;
		case 4:
			switch (funcID) {
			case FUNC_ICHIMAIE:
				IchimaiePri = NJM_MIN(-1.0f, IchimaiePri + 1.0f);
				if (ichiMaie_tp[pno])
					CIchimaie2_SetPriority(ichiMaie_tp[pno], IchimaiePri);
				break;
			case FUNC_SPHBOMB:
				sphbomb_follow = sphbomb_follow != 1;
				break;
			case FUNC_CRUSHLIGHT:
				crush_follow = crush_follow != 1;
				break;
			case FUNC_CAPTURE:
				capture_follow = capture_follow != 1;
				break;
			case FUNC_LAST1A:
				toggleSuperSonic();
				break;
			}
		case 7: //only used by wpillar
			wpillar_spd = NJM_MIN(10000.0f, wpillar_spd + 0.01f);
			break;
		case 8: //only used by wpillar
			wpillar_scl = NJM_MIN(10000.0f, wpillar_scl + 0.01f);
			break;
		}
		break;
	case Buttons_Left:
		switch (cursor) {
		case 1:
			funcID = NJM_MAX(0, funcID - 1);
			break;
		case 2:
			switch (funcID) {
			case FUNC_ROTARY:
				rotRad = NJM_MAX(0.0f, rotRad - 0.1f);
				break;
			case FUNC_WPILLAR:
				wpillar_follow = wpillar_follow != 1;
				break;
			}
			break;
		case 3:
			switch (funcID) {
			case FUNC_ROTARY:
				rotSpd = NJM_MAX(0, rotSpd - 1);
				break;
			}
			break;
		case 4:
			switch (funcID) {
			case FUNC_ICHIMAIE:
				IchimaiePri = NJM_MIN(4000.0f, IchimaiePri - 1.0f);

				if (ichiMaie_tp[pno])
					CIchimaie2_SetPriority(ichiMaie_tp[pno], IchimaiePri);
				break;
			case FUNC_SPHBOMB:
				sphbomb_follow = sphbomb_follow != 1;
				break;
			case FUNC_CRUSHLIGHT:
				crush_follow = crush_follow != 1;
				break;
			case FUNC_CAPTURE:
				capture_follow = capture_follow != 1;
				break;
			case FUNC_LAST1A:
				toggleSuperSonic();
				break;
			}
		case 7: //only used by wpillar
			wpillar_spd = NJM_MAX(0.01f, wpillar_spd - 0.01f);
			break;
		case 8: //only used by wpillar
			wpillar_scl = NJM_MAX(0.0f, wpillar_scl - 0.01f);
			break;
		}
		break;
	}

	if (per[0]->press & Buttons_Z) {
		setFunc(funcID, pno);
	}


	

	switch (funcID) {
	case FUNC_ROTARY:
		//Movement Speed
		if (KeyGetOn(KEYS_MINUS)) {
			rotRad = NJM_MAX(0.0f, rotRad - 0.1f);
		}
		else if (KeyGetOn(KEYS_PLUS)) {
			rotRad = NJM_MIN(10000.0f, rotRad + 0.1f);
		}

		//Interval
		if (KeyGetOn(KEYS_LBRACKET)) {
			rotSpd = NJM_MAX(0, rotSpd - 1);
		}
		else if (KeyGetOn(KEYS_RBRACKET)) {
			rotSpd = NJM_MIN(9999, rotSpd + 1);
		}

		///Reset
		if (KeyGetPress(KEYS_FSLASH)) {
			rotRad = 7.0f;
			rotSpd = 0x120;
		}


		break;
	case FUNC_ICHIMAIE:
		//Pri
		if (KeyGetOn(KEYS_MINUS)) {
			IchimaiePri = NJM_MIN(4000.0f, IchimaiePri - 5.0f);

			if (ichiMaie_tp[pno])
				CIchimaie2_SetPriority(ichiMaie_tp[pno], IchimaiePri);
		}
		else if (KeyGetOn(KEYS_PLUS)) {
			IchimaiePri = NJM_MIN(-1.0f, IchimaiePri + 5.0f);
			if (ichiMaie_tp[pno])
				CIchimaie2_SetPriority(ichiMaie_tp[pno], IchimaiePri);
		}

		///Reset
		if (KeyGetPress(KEYS_FSLASH)) {
			IchimaiePri = -70.0f;
		}
		break;
	case FUNC_SKYWALK:
		// Doesn't need anything
		break;

	case FUNC_SPHBOMB:
		//Fix Pos
		if (KeyGetPress(KEYS_LCTRL)) {
			sphbomb_follow = sphbomb_follow != TRUE;
		}

		if (sphbomb_follow && playertwp[pno]) {
			sphereBomb.pos = playertwp[pno]->pos;
		}

		///Reset
		if (KeyGetPress(KEYS_FSLASH)) {
			sphereBomb.frame = 550;
			sphereBomb.scl_spd = 2.0f;
			sphereBomb.sphere_radius = 350.0f;
			sphereBomb.scl_reduce_acc = 0.8f;
			sphereBomb.vscroll_spd = 10.0f;
		}

		if (cursor < 9)
			return;

		sphbomb_editID = cursor - 9;

		switch (isPressorOn) {
		case Buttons_Left:
			switch (sphbomb_editID) {
			case 0:
				sphereBomb.frame = NJM_MAX(1, sphereBomb.frame - 1);
				break;
			case 1:
				sphereBomb.scl_spd = NJM_MAX(0.0f, sphereBomb.scl_spd - 0.01f);
				break;
			case 2:
				sphereBomb.sphere_radius = NJM_MAX(0.0f, sphereBomb.sphere_radius - 1.0f);
				break;
			case 3:
				sphereBomb.scl_reduce_acc = NJM_MAX(0.00f, sphereBomb.scl_reduce_acc - 0.01f);
				break;
			case 4:
				sphereBomb.vscroll_spd = NJM_MAX(0.0f, sphereBomb.vscroll_spd - 1.0f);
				break;
			}
			break;
		case Buttons_Right:
			switch (sphbomb_editID) {
			case 0:
				sphereBomb.frame = NJM_MIN(1000, sphereBomb.frame + 1);
				break;
			case 1:
				sphereBomb.scl_spd = NJM_MIN(1000.0f, sphereBomb.scl_spd + 0.01f);
				break;
			case 2:
				sphereBomb.sphere_radius = NJM_MIN(1000.0f, sphereBomb.sphere_radius + 1.0f);
				break;
			case 3:
				sphereBomb.scl_reduce_acc = NJM_MIN(1000.0f, sphereBomb.scl_reduce_acc + 0.01f);
				break;
			case 4:
				sphereBomb.vscroll_spd = NJM_MIN(1000.0f, sphereBomb.vscroll_spd + 1.0f);
				break;
			}
			break;
		}
		break;
	case FUNC_CRUSHLIGHT:
		//Fix Pos
		if (KeyGetPress(KEYS_LCTRL)) {
			crush_follow = crush_follow != TRUE;
		}

		if (crush_follow && playertwp[pno]) {
			crushlight.pos = playertwp[pno]->pos;
		}

		///Reset
		if (KeyGetPress(KEYS_FSLASH)) {
			crushlight.num = 3;
			crushlight.life = 10;
			crushlight.startWidth = 0.4f;
			crushlight.endWidth = 2.0f;
			crushlight.startCol.color = 0xFFFF64FF;
			crushlight.endCol.color = 0x00FF1EFF;
		}



		if (cursor < 9)
			return;

		crush_editID = cursor - 9;

		switch (isPressorOn) {
		case Buttons_Left:
			switch (crush_editID) {
			case 0:
				crushlight.num = NJM_MAX(1, crushlight.num - 1);
				break;
			case 1:
				crushlight.life = NJM_MAX(1, crushlight.life - 1);
				break;
			case 2:
				crushlight.startWidth = NJM_MAX(0.0f, crushlight.startWidth - 0.01f);
				break;
			case 3:
				crushlight.endWidth = NJM_MAX(0.01f, crushlight.endWidth - 0.01f);
				break;
			case 4:
				crushlight.startCol.argb.a = NJM_MAX(0, crushlight.startCol.argb.a - 1);
				break;
			case 5:
				crushlight.startCol.argb.r = NJM_MAX(0, crushlight.startCol.argb.r - 1);
				break;
			case 6:
				crushlight.startCol.argb.g = NJM_MAX(0, crushlight.startCol.argb.g - 1);
				break;
			case 7:
				crushlight.startCol.argb.b = NJM_MAX(0, crushlight.startCol.argb.b - 1);
				break;
			case 8:
				crushlight.endCol.argb.a = NJM_MAX(0, crushlight.endCol.argb.a - 1);
				break;
			case 9:
				crushlight.endCol.argb.r = NJM_MAX(0, crushlight.endCol.argb.r - 1);
				break;
			case 10:
				crushlight.endCol.argb.g = NJM_MAX(0, crushlight.endCol.argb.g - 1);
				break;
			case 11:
				crushlight.endCol.argb.b = NJM_MAX(0, crushlight.endCol.argb.b - 1);
				break;
			}
			break;
		case Buttons_Right:
			switch (crush_editID) {
			case 0:
				crushlight.num = NJM_MIN(1000, crushlight.num + 1);
				break;
			case 1:
				crushlight.life = NJM_MIN(1000, crushlight.life + 1);
				break;
			case 2:
				crushlight.startWidth = NJM_MIN(1000.0f, crushlight.startWidth + 0.01f);
				break;
			case 3:
				crushlight.endWidth = NJM_MIN(1000.0f, crushlight.endWidth + 0.01f);
				break;
			case 4:
				crushlight.startCol.argb.a = NJM_MIN((Uint8)255, crushlight.startCol.argb.a + 1);
				break;
			case 5:
				crushlight.startCol.argb.r = NJM_MIN((Uint8)255, crushlight.startCol.argb.r + 1);
				break;
			case 6:
				crushlight.startCol.argb.g = NJM_MIN((Uint8)255, crushlight.startCol.argb.g + 1);
				break;
			case 7:
				crushlight.startCol.argb.b = NJM_MIN((Uint8)255, crushlight.startCol.argb.b + 1);
				break;
			case 8:
				crushlight.endCol.argb.a = NJM_MIN((Uint8)255, crushlight.endCol.argb.a + 1);
				break;
			case 9:
				crushlight.endCol.argb.r = NJM_MIN((Uint8)255, crushlight.endCol.argb.r + 1);
				break;
			case 10:
				crushlight.endCol.argb.g = NJM_MIN((Uint8)255, crushlight.endCol.argb.g + 1);
				break;
			case 11:
				crushlight.endCol.argb.b = NJM_MIN((Uint8)255, crushlight.endCol.argb.b + 1);
				break;

			}
			break;

		}
		break;
	case FUNC_CAPTURE:
		

		//Fix Pos
		if (KeyGetPress(KEYS_LCTRL)) {
			capture_follow = capture_follow != TRUE;
		}

		if (capture_follow && playertwp[pno]) {
			capture_data.pos = playertwp[pno]->pos;
			capture_data.ang = playertwp[pno]->ang;
		}


		if (cursor < 9)
			return;

		capture_editID = cursor - 9;

		switch (isPressorOn) {
		case Buttons_Left:
			switch (capture_editID) {
			case 0:
				capture_data.scl.x = NJM_MAX(0.0f, capture_data.scl.x - 0.01f);
				break;
			case 1:
				capture_data.scl.y = NJM_MAX(0.0f, capture_data.scl.y - 0.01f);
				break;
			case 2:
				capture_data.scl.z = NJM_MAX(0.0f, capture_data.scl.z - 0.01f);
				break;
			case 3:
				capture_data.scl_large_spd = NJM_MAX(0.01f, capture_data.scl_large_spd - 0.01f);
				break;
			case 4:
				capture_data.scl_small_spd = NJM_MAX(0.01f, capture_data.scl_small_spd - 0.01f);
				break;
			case 5:
				capture_data.rot_spd = NJM_MAX(0, capture_data.rot_spd - 1);
				break;
			case 6:
				useAngle = useAngle != 1;
				break;
			}
			break;
		case Buttons_Right:
			switch (capture_editID) {
			case 0:
				capture_data.scl.x = NJM_MIN(1000.0f, capture_data.scl.x + 0.01f);
				break;
			case 1:
				capture_data.scl.y = NJM_MIN(1000.0f, capture_data.scl.y + 0.01f);
				break;
			case 2:
				capture_data.scl.z = NJM_MIN(1000.0f, capture_data.scl.z + 0.01f);
				break;
			case 3:
				capture_data.scl_large_spd = NJM_MIN(1000.0f, capture_data.scl_large_spd + 0.01f);
				break;
			case 4:
				capture_data.scl_small_spd = NJM_MIN(1000.0f, capture_data.scl_small_spd + 0.01f);
				break;
			case 5:
				capture_data.rot_spd = NJM_MIN(9999, capture_data.rot_spd + 1);
				break;
			case 6:
				useAngle = useAngle != 1;
				break;
			}
			break;
		}

		//Update beam
		if (capture_tp) {
			SetCaptureParam(capture_tp, capture_data.scl.x, capture_data.scl.y, capture_data.scl.z, capture_data.scl_large_spd, capture_data.scl_small_spd, capture_data.rot_spd);
		}
		break;
	case FUNC_WPILLAR:
		//Movement Speed
		if (KeyGetOn(KEYS_MINUS)) {
			wpillar_spd = NJM_MAX(0.01f, wpillar_spd - 0.01f);
		}
		else if (KeyGetOn(KEYS_PLUS)) {
			wpillar_spd = NJM_MIN(10000.0f, wpillar_spd + 0.01f);
		}

		//Interval
		if (KeyGetOn(KEYS_LBRACKET)) {
			wpillar_scl = NJM_MAX(0.0f, wpillar_scl - 0.01f);
		}
		else if (KeyGetOn(KEYS_RBRACKET)) {
			wpillar_scl = NJM_MIN(10000.0f, wpillar_scl + 0.01f);
		}

		//Fix Pos
		if (KeyGetPress(KEYS_LCTRL)) {
			wpillar_follow = wpillar_follow != TRUE;
		}

		if (wpillar_follow && playertwp[pno])
			wpillar_pos = playertwp[pno]->pos;

		///Reset
		if (KeyGetPress(KEYS_FSLASH)) {
			wpillar_spd = 0.5f;
			wpillar_scl = 0.7f;

		}
		break;
	}
}