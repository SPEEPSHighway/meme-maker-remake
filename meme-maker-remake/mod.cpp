#include "SADXModLoader.h"
#include "FreeCamera.h"
#include "FreeMove.h"
#include "HeadEditor.h"
#include "AnimationPlayer.h"
#include "ParticleGenerator.h"
#include "Keys.h"
#include "FunctionHook.h"
#include "EventFunctions.h"
#include "NPCMode.h"
#include "PropSpawner.h"
#include "SeqvarsEditor.h"

#include <iostream>
#include <fstream>
#include <string>

#include <IniFile.hpp>

extern "C"
{
	Sint32 quotes_rnd;

	const char* quotes[] = {
		"8kwindiirubleseverymonth", //Windii
		"I LOVE MARIA ROBOTNIK", //Windii
		"Join Sonic Adventure Central!",
		"Join X-Hax!",
		"Oh no",
		"inhale", //FNaFb
		"I'm so disappointed, you useless machine!",
		"Use Eggman in the Anim Player for large characters.",
		"yes",
		"thank you for getting me back to good toad",
		"https://speeps-highway.tumblr.com/",
		"https://www.youtube.com/@SPEEPSHighway",
		"Check out Windii's blog: https://browniehideout.wordpress.com/"
	};

	#define QUOTES_SIZE sizeof(quotes) / sizeof(quotes[0])


	Bool makerActive;
	Bool showDisplay = TRUE;
	Bool Testing;
	Sint32 maker_mode;
	Sint32 pno;

	//Config
	Bool allowDupeChars;
	Bool allowDeath;

	PDS_PERIPHERAL* oldper[8];

	static Sint32 cursor;

	static Sint32 characterCursor;
	static void(*characterList[])(task*) = {
		SonicTheHedgehog,
		Eggman,
		MilesTalesPrower,
		KnucklesTheEchidna,
		Tikal,
		AmyRose,
		E102,
		BigTheCat,
		EggrobForEvent0,
	};

	static Sint32 charCursorMax = 7;

	std::string subUpper = "";
	std::string subLower = "";
	std::string subVoice = "";
	std::string subtitleFilePath;
	char customSubtitle[128];
	const char* customMessage[] = { customSubtitle, NULL };

	std::string faceFilePath;
	std::string customFace;

	HMODULE DEBUGMODEMOD;

	static FunctionHook<void, task*> SonicDirectAhead_hook(SonicDirectAhead);
	static FunctionHook<void> ProcessStatusTable_hook(ProcessStatusTable);
	static FunctionHook<void, task*> AmyBirdExe_hook(AmyBirdExe);
	static FunctionHook<void> LimitPosPlayer_hook(LimitPosPlayer);
	static FunctionHook<Sint32> SeqGetTime_hook(SeqGetTime2);
	static FunctionHook<void, Sint32> KillHimByFallingDownP2_hook(KillHimByFallingDownP2);
	static FunctionHook<void, Sint32> KillHimByFallingDownP_hook(KillHimByFallingDownP);
	static FunctionHook<void, Sint32> KillHimP_hook(KillHimP);
	static FunctionHook<void, char> DamegeRingScatter_hook(DamegeRingScatter);


	//Hooked functions

	///Prevent damage
	void keepMyRingsPlease(char pno) {
		if (!makerActive || allowDeath) {
			DamegeRingScatter_hook.Original(pno);
		}
	}

	//Prevent death by falling
	void dontKillHimByFallingDownP(Sint32 pid) {
		if (!makerActive || allowDeath) {
			KillHimByFallingDownP_hook.Original(pid);
		}
	}

	//Prevent death by falling
	void dontKillHimByFallingDownP2(Sint32 pid) {
		if (!makerActive || allowDeath) {
			KillHimByFallingDownP2_hook.Original(pid);
		}
	}

	///Prevent death
	void dontKillHimP(Sint32 pno) {
		if (!makerActive || allowDeath) {
			KillHimP_hook.Original(pno);
		}
	}

	///Remove invisible walls surrounding Perfect Chaos when close to him.
	void chaos7NoBarrier() {
		if(!last1aHighFloor_tp[0])
			LimitPosPlayer_hook.Original();
	}


	///Remove Amy's bird when told to.
	void kill_AmyBird(task* tp) {
		if (giveMeTheBird) {
			FreeTask(tp);
		}
		else {
			AmyBirdExe_hook.Original(tp);
		}
	}

	//Force Time of Day
	Sint32 forceTimeOfDay() {
		if (timeOfDay_forced)
			return timeOfDay_forced - 1;
		else
			return SeqGetTime_hook.Original();
	}

	///Stops the back/first person camera activating when it would conflict with modes that use right analog.
	void firstPersonCamCheck(Sint16 ssCameraMode, Uint8 ucAdjustType) {
		if (maker_mode >= M_MODE_FREEMOVE && maker_mode <= M_MODE_HEAD) {
			return; //Stop the camera from activating.
		}
		else {
			CameraSetCollisionCamera(ssCameraMode, ucAdjustType);
		}

	}

	/// <summary>
	/// Checks for known character mods and updates player strings.
	/// </summary>
	void checkCharacterMods() {
		if (GetModuleHandle(L"ShadowSA1") || GetModuleHandle(L"SHDX") || GetModuleHandle(L"srshadow"))
			playerNames[PLNO_SONIC] = "SHADOW";
		if (GetModuleHandle(L"ScourgeDC") || GetModuleHandle(L"Scourge"))
			playerNames[PLNO_SONIC] = "SCOURGE";
		if (GetModuleHandle(L"Blaze the Cat (Sa1-Style)") || GetModuleHandle(L"AltDimTeam"))
			playerNames[PLNO_SONIC] = "BLAZE";
		if (GetModuleHandle(L"CreamtheRabbit(SA1-Style)"))
			playerNames[PLNO_TAILS] = "CREAM";
		if (GetModuleHandle(L"MTails"))
			playerNames[PLNO_TAILS] = "METAL TAILS";
		if (GetModuleHandle(L"Rouge-the-Bat-(SA1-Style)") || GetModuleHandle(L"RougeTheBat"))
			playerNames[PLNO_KNUCKLES] = "ROUGE";
		if (GetModuleHandle(L"JadeCat"))
			playerNames[PLNO_SONIC] = "JADE";
		if (GetModuleHandle(L"klonoa"))
			playerNames[PLNO_SONIC] = "KLONOA";
		if (GetModuleHandle(L"Chaos"))
			playerNames[PLNO_SONIC] = "CHAOS";
		if (GetModuleHandle(L"silver"))
			playerNames[PLNO_SONIC] = "SILVER";
		if (GetModuleHandle(L"Shadic"))
			playerNames[PLNO_SONIC] = "SHADIC";
		if (GetModuleHandle(L"ZarosguthRe")) {
			playerNames[PLNO_SONIC] = "ZAROSGUTH";
			playerNames[PLNO_TAILS] = "BLOOD/LEO";
			playerNames[PLNO_KNUCKLES] = "SLASH";
			playerNames[PLNO_AMY] = "IRIS";
			playerNames[PLNO_E102] = "Z117/E10X";
			playerNames[PLNO_BIG] = "FAT/SLY";
			metalSonicName = "METAL ZG";
		}
		if (GetModuleHandle(L"NES")) {
			playerNames[PLNO_SONIC] = "SOMARI";
			playerNames[PLNO_TAILS] = "TUIGI";
			playerNames[PLNO_KNUCKLES] = "WARKLES";
			playerNames[PLNO_AMY] = "PEACHY ROSE";
		}
		if (GetModuleHandle(L"Doom"))
			playerNames[PLNO_SONIC] = "SEELKADOOM";
		if (GetModuleHandle(L"MetalSh") || GetModuleHandle(L"MTLSHDW"))
			metalSonicName = "METAL SHADOW";
		if (GetModuleHandle(L"neo"))
			metalSonicName = "NEO METAL SONIC";
		if (GetModuleHandle(L"McCloud weld"))
			playerNames[PLNO_SONIC] = "VERGIL";
		if (GetModuleHandle(L"Infinite"))
			playerNames[PLNO_SONIC] = "INFINITE";
		if (GetModuleHandle(L"JesseWeNeedToCook"))
			playerNames[PLNO_SONIC] = "VIBRI";
		if (GetModuleHandle(L"SA1_Cecil"))
			playerNames[PLNO_SONIC] = "CECIL";
		if (GetModuleHandle(L"dcknux"))
			playerNames[PLNO_SONIC] = "KNUCKLES (OVER SONIC)";
		if (GetModuleHandle(L"HDRosy"))
			playerNames[PLNO_AMY] = "ROSY";
		if (GetModuleHandle(L"TOA"))
			playerNames[PLNO_AMY] = "TIKAL (OVER AMY)";
		if (GetModuleHandle(L"pacweld"))
			playerNames[PLNO_SONIC] = "PAC-MAN";
		if (GetModuleHandle(L"sally"))
			playerNames[PLNO_SONIC] = "SALLY";
		if (GetModuleHandle(L"HoneytheCat"))
			playerNames[PLNO_AMY] = "HONEY";
		if (GetModuleHandle(L"DCKnuxOverAmy"))
			playerNames[PLNO_AMY] = "KNUCKLES (OVER AMY)";
		if (GetModuleHandle(L"MetalAmy"))
			playerNames[PLNO_AMY] = "METAL AMY";
		if (GetModuleHandle(L"SONICOK"))
			playerNames[PLNO_KNUCKLES] = "SONIC (OVER KNUCKLES)";
		if (GetModuleHandle(L"ESeries"))
			playerNames[PLNO_E102] = "E-SERIES";
		if (GetModuleHandle(L"AmyOverSonic")) {
			playerNames[PLNO_SONIC] = "SONIC?";
			playerNames[PLNO_AMY] = "AMY?";
		}
	}

	__declspec(dllexport) ModInfo SADXModInfo = { ModLoaderVer };
	__declspec(dllexport) void __cdecl Init(const char* path, const HelperFunctions& helperFunctions)
	{
		subtitleFilePath = std::string(path) + "\\text.txt";
		faceFilePath = std::string(path) + "\\face.txt";

		setPropFolder(".\\" + std::string(path) + "\\system\\props\\");
		setAnimFolder(".\\" + std::string(path) + "\\system\\");

		//Game doesn't store pointers to player heads so the mod needs to collect them itself
		hookHead();

		//Setup for removing bird
		AmyBirdExe_hook.Hook(kill_AmyBird);

		//Setup for removing pos limiting surrounding perfect chaos
		LimitPosPlayer_hook.Hook(chaos7NoBarrier);

		//Prevent killing the player
		KillHimByFallingDownP_hook.Hook(dontKillHimByFallingDownP);
		KillHimP_hook.Hook(dontKillHimP);
		KillHimByFallingDownP2_hook.Hook(dontKillHimByFallingDownP2);
		DamegeRingScatter_hook.Hook(keepMyRingsPlease);

		//Ini Configuration
		const IniFile* config = new IniFile(std::string(path) + "\\config.ini");
		charCursorMax += config->getBool("General", "EnableZERO", false);
		allowDupeChars = config->getBool("General", "DupeChars", false);
		allowDeath = config->getBool("General", "AllowDeath", false);

		//Set up forcing time of day
		SeqGetTime_hook.Hook(forceTimeOfDay);

		//Back up face animation frame data
		initFaceTableBackup();

		// Always initialize Super Sonic welds. (Stolen from the super sonic mod lol)
		WriteData<2>(reinterpret_cast<Uint8*>(0x0049AC6A), 0x90i8);

		///Add check for if back/first person cams can activate.
		WriteCall((void*)0x43759B, firstPersonCamCheck);
		WriteCall((void*)0x437699, firstPersonCamCheck);


	}

	__declspec(dllexport) void __cdecl OnInitEnd()
	{
		// Executed after every mod has been initialized, mainly used to check if a specific mod is also enabled.

		//Patch SuperSonicStatusManagerDestructor resetting wrong physics.
		WriteData<1>((Uint8*)0x4942F4, 0xF0);
		WriteData<1>((Uint8*)0x4942FD, 0xEC);

		//Extend controllers to 4.
		Sint8* inputNum = (Sint8*)0x40F182;
		if (*inputNum == 2)
			*inputNum = 8;

		//Check for PkR's Debug Mode mod, need to disable number key shortcuts if it's on.
		DEBUGMODEMOD = GetModuleHandle(L"sadx-debug-mode");
		
		//Check for known character mods (Whatever I saw on GameBanana that had a unique DLL lol)
		checkCharacterMods();

	}

	/// <summary>
	/// Roll the random quote generator.
	/// </summary>
	void rollText() {
		quotes_rnd = (Sint32)(ulGlobalTimer % (Sint32)(QUOTES_SIZE));
	}

	/// <summary>
	/// Resets player's mode. Super Sonic's is a different value.
	/// </summary>
	void releaseMode_accountforSuperSonic() {
		if (playertwp[pno]->mode == 99) {
			if (playerpwp[pno]->equipment & 0x8000)
				playertwp[pno]->mode = 75;
			else
				playertwp[pno]->mode = 1;
		}
	}

	/// <summary>
	/// Enable the meme maker ingame
	/// </summary>
	void startMemeMaker() {
		rollText();

		//Account for the question mark mod, which only affects Super Sonic's story.
		if (GetModuleHandle(L"QuestionMark")) {
			if(flgPlayingSuperSonic)
				playerNames[PLNO_SONIC] = "?";
			else
				playerNames[PLNO_SONIC] = "SONIC";
		}

		for (Sint32 i = 0; i < 8; ++i)
			oldper[i] = per[i];

		WriteData<1>((int*)(0x7B43B4), 0x75); //Stop Tikal's Debug movement interfering.
		WriteData<1>((int*)(0x7B5288), 0x75); //Stop Eggman's Debug movement interfering.
		WriteData<1>((int*)(0x7B529C), 0x75); //^

		WriteData<1>((int*)0x413DA4, 0xFF); //Disable the HUD so the timer doesn't overlap the display.

		//Freeze player if told to in basic move
		if (playertwp[pno] && playerFreeze[pno])
			playertwp[pno]->mode = 99;

		if (playertp[0]) {
			throughplayer_on(playertp[0]);
		}

		makerActive = TRUE;
	};

	//Set the meme maker's mode, accounting for going back and the player being frozen by free move.
	void maker_setMode(Sint32 mode) {

		//Exit from basic move's frozen state if not told to keep it
		if (playertwp[pno] && !playerFreeze[pno]) {

			releaseMode_accountforSuperSonic();
		}

		if (maker_mode != mode) {
			maker_mode = mode;
		}
		else {
			maker_mode = M_MODE_NONE;
		}
	}

	/// <summary>
	/// Shut down
	/// </summary>
	void endMemeMaker() {
		//maker_mode = M_MODE_NONE;
		//TODO: Kill code for all modes. Maybe make wiping the mode optional

		for (Sint32 i = 0; i < 8; ++i)
			per[i] = oldper[i];

		//Turn timer back on
		WakeTimer();

		//Reset the camera
		resetFreeCamera();

		//Reset debug text colour
		njPrintColor(0xFFBFBFBF);

		//Unfreeze player if frozen
		if (playertwp[pno] && !playerFreeze[pno]) {
			releaseMode_accountforSuperSonic();
		}			

		//Reset player's face
		faceNo[pno] = -1;
		if (playertwp[pno]) {
			playertwp[pno]->ewp->face.nbFrame = 0;
			playertwp[pno]->ewp->look.ang.y = 0;
			playertwp[pno]->ewp->look.ang.z = 0;
		}

		playertwp[0]->flag &= ~0x4000;

		throughplayer_off(playertp[0]);


		WriteData<1>((int*)0x437760, 0x74); //Enable normal cameras

		//Enable Debug for Eggman and Tikal
		WriteData<1>((int*)(0x7B43B4), 0x74);
		WriteData<1>((int*)(0x7B5288), 0x74);
		WriteData<1>((int*)(0x7B529C), 0x74);

		WriteData<1>((int*)0x413DA4, 0x00); //Enable the HUD

		//End invincibility
		if (playertwp[pno]) {
			playertwp[pno]->wtimer = 0;
		}

		if (CheckEditMode()) {
			DisablePause();
		}

		makerActive = FALSE;
	}

	/// <summary>
	/// HUD master function
	/// </summary>
	void displayBasicInfo() {
		njPrintColor(0xFFFFFFFF);
		Sint32 col = 1;
		njPrintSize((Uint16)(((Float)HorizontalResolution / 640.0f) * 8.0f));

		//Draw BG, probably better when finalized
		//ds_DrawBoxFill2D(ScreenRaitoX * 1.0f, ScreenRaitoY * 1.0f, ScreenRaitoX * 200.0f, ScreenRaitoY * 400.0f, -11.0f, 0x60000080);
		njPrintC(NJM_LOCATION(1, col++), "MEME MAKER 2 (by Speeps)");
		njPrint(NJM_LOCATION(1, col++), "~~ %s ~~", quotes[quotes_rnd]); // Random Text
		col++;

		//Get current character's name
		const char* playerName;
		if (playertwp[pno])
			if (gu8flgPlayingMetalSonic)
				playerName = metalSonicName;
			else
				playerName = playerNames[playertwp[pno]->counter.b[1]];
		else
			playerName = "NONE";
		njPrint(NJM_LOCATION(1, col++), "Player: %d (%s)", pno + 1, playerName);
		col++;

		//Mode-dependent display
		switch (maker_mode) {
		case M_MODE_NONE:
		{
			njPrintC(NJM_LOCATION(1, col++), "Mode: Main Menu");
			col++;

			if (playertp[pno]) {
				Sint32 arrow = col + cursor;
				njPrintC(NJM_LOCATION(1, arrow), ">");
				njPrintC(NJM_LOCATION(2, col++), "1 = Free Move / Action Viewer");
				njPrintC(NJM_LOCATION(2, col++), "2 = Camera Settings");
				njPrintC(NJM_LOCATION(2, col++), "3 = Head/Face Settings");
				njPrintC(NJM_LOCATION(2, col++), "4 = Animation Player");
				njPrintC(NJM_LOCATION(2, col++), "5 = Particle Generator");
				njPrintC(NJM_LOCATION(2, col++), "6 = Event Functions");
				njPrintC(NJM_LOCATION(2, col++), "7 = Prop Spawner");
				njPrintC(NJM_LOCATION(2, col++), "8 = NPC Mode");
				njPrintC(NJM_LOCATION(2, col++), "9 = Field Flags");
				col++;
				njPrintC(NJM_LOCATION(2, col++), "Y Button = Select Menu");
				njPrintC(NJM_LOCATION(2, col++), "D Button (SEL)/Caps Lock = Toggle Display");
				njPrintC(NJM_LOCATION(2, col++), "C Button (LB) = Play Text");
				if(pno)
					njPrintC(NJM_LOCATION(2, col++), "Z Button = Delete Player");
				if (DEBUGMODEMOD) {
					njPrintC(NJM_LOCATION(2, col++), "NOTE: Number key shortcuts disabled due");
					njPrintC(NJM_LOCATION(2, col++), "to conflicting with the Debug Mode mod.");
				}
			}
			else {


				njPrintC(NJM_LOCATION(2, col++), "No character spawned!");
				njPrintC(NJM_LOCATION(2, col++), "Choose who to spawn in this slot:");

				Sint32 arrow = col + characterCursor;
				njPrintC(NJM_LOCATION(1, arrow), ">");

				for (Sint32 i = 0; i < 8; ++i) {
					njPrintC(NJM_LOCATION(2, col++), playerNames[i]);
				}

				if(charCursorMax == 8)
					njPrintC(NJM_LOCATION(2, col++), "ZERO");

				//There's a bunch of character mods that have dlls with common names.
				if (GetModuleHandle(L"SADXWeightedCharacters") || GetModuleHandle(L"SADXSonic") || GetModuleHandle(L"SADXDisableUpgradeModels")) {
					col++;
					njPrintC(NJM_LOCATION(2, col++), "Character names may not be accurate.");
					njPrintC(NJM_LOCATION(2, col++), "The modloader uses the name of the dll to identify mods.");
					njPrintC(NJM_LOCATION(2, col++), "But all I see is:");
					if(GetModuleHandle(L"SADXWeightedCharacters"))
						njPrintC(NJM_LOCATION(2, col++), "SADXWeightedCharacters.dll");
					if(GetModuleHandle(L"SADXSonic"))
						njPrintC(NJM_LOCATION(2, col++), "SADXSonic.dll");
					if(GetModuleHandle(L"SADXDisableUpgradeModels"))
						njPrintC(NJM_LOCATION(2, col++), "SADXDisableUpgradeModels.dll");
				}
			}
			break;
		}
		case M_MODE_FREEMOVE:
			displayFreeMoveInfo(col, pno);
			break;
		case M_MODE_CAMERA:
			displayCameraInfo(col, pno);
			break;
		case M_MODE_HEAD:
			displayHeadInfo(col, pno);
			break;
		case M_MODE_ANIM:
			displayAnimInfo(col, pno);
			break;
		case M_MODE_PARTICLE:
			displayParticleInfo(col, pno);
			break;
		case M_MODE_EVENT:
			displayEventInfo(col, pno);
			break;
		case M_MODE_PROP:
			displayPropInfo(col, pno);
			break;
		case M_MODE_NPC:
			displayNPCInfo(col, pno);
			break;
		case M_MODE_FLAGS:
			displaySeqVarsInfo(col, pno);
			break;
		default:
			break;
		}
	};


	void processInput() {
		//Free Move
		if ((!DEBUGMODEMOD && KeyGetPress(KEYS_1)) || maker_mode == M_MODE_FREEMOVE && per[0]->press & Buttons_A) {

			if (maker_mode == M_MODE_NONE && !cursor && per[0]->press & Buttons_Y) {
				playerFreeze[pno] = TRUE;
			}
			maker_setMode(M_MODE_FREEMOVE);
			return;
		}

		if (!DEBUGMODEMOD) {
			//Camera Options
			if (KeyGetPress(KEYS_2)) {
				maker_setMode(M_MODE_CAMERA);
			}

			//Head Options
			if (KeyGetPress(KEYS_3)) {
				maker_setMode(M_MODE_HEAD);
			}

			//Animation Player
			if (KeyGetPress(KEYS_4)) {
				maker_setMode(M_MODE_ANIM);
			}

			//Particle Generator
			if (KeyGetPress(KEYS_5)) {
				maker_setMode(M_MODE_PARTICLE);
			}

			//Event Functions
			if (KeyGetPress(KEYS_6)) {
				maker_setMode(M_MODE_EVENT);
			}

			//Prop Spawner
			if (KeyGetPress(KEYS_7)) {
				maker_setMode(M_MODE_PROP);
			}

			//NPC Player
			if (KeyGetPress(KEYS_8)) {
				maker_setMode(M_MODE_NPC);
			}

			//Flags
			if (KeyGetPress(KEYS_9)) {
				maker_setMode(M_MODE_FLAGS);
			}
		}


		//Classic B button controls
		if (per[0]->press & Buttons_B) {
			if (playerFreeze[pno]) {
				if (maker_mode && maker_mode + 1 < M_MODE_MAX)
					++maker_mode;
				else
					maker_mode = 1;
			}
		}
	};


	void resetControllers() {
		//Keep the other controller pointers intact
		for (Sint32 i = 0; i < 8; i++) {
			if (i != pno) {
				per[i] = oldper[i];
			}
		}
	}

	__declspec(dllexport) void __cdecl OnFrame()
	{
		//If not in-game don't do anything. TODO: Turn off editor
		if (ssGameMode != MD_GAME_MAIN) {
			return;
		}

		//Copies the ID of the pressed key to the debug console
		if (Testing) {
			for (int i = 1; i < 256; i++) {
				if (KeyGetPress(i))
					PrintDebug("\n%d\n", i);
			}
		}

		//Check for turning on the maker
		if (!makerActive && !CheckEditMode()) {
			if (KeyGetPress(KEYS_TAB) || per[0]->on & Buttons_Z && per[0]->press & Buttons_D) {
				startMemeMaker();
			}
			return;
		}
		//--- When the maker is on ---

		//Show the title display
		if (showDisplay)
			displayBasicInfo();

		//Advance event functions
		advanceEventEffects();

		//Keep Super Sonic from dying
		if (playerpwp[0]->equipment & 0x8000) {
			Rings = 50;
		}

		//Invincibility using damage frames. Carts do stuff differently.
		if (!allowDeath && playertwp[0] && playertwp[0]->counter.b[1] != PLNO_EGGMAN) { // Makes him invisible. He's invincible anyway though.
			playertwp[0]->flag &= ~4;

			if (taskOfPlayerOn) {
				playertwp[0]->wtimer = 0;
				cart_data->invincible_timer = 0;
				CartOtherParam.dead_wait_time = 2;
			}
			else {
				playertwp[0]->wtimer = 2;
			}

		}

		//Freeze timer and reset E102's to prevent timeover.
		SleepTimer();
		if (GetPlayerNumber() == PLNO_E102) SetTime(2, 0);

		
		if (playerpwp[0])
			playerpwp[0]->breathtimer = 0; 


		//Show or Hide Display
		if (KeyGetPress(KEYS_CAPSLOCK) || (per[0]->press & Buttons_D && !KeyGetPress(KEYS_V)) && !(per[0]->on & Buttons_Z)) {
			showDisplay = showDisplay != TRUE; //VS hates our Bool type and kept trying to recommend bitwise with !showDisplay lol

			//Reroll random text
			if (!showDisplay)
				rollText();
		}

		//Exit
		if (KeyGetPress(KEYS_TAB) || CheckEditMode() || per[0]->on & Buttons_Z && per[0]->press & Buttons_D) {
			endMemeMaker();
		}

		//Change player's face and check NPC mode
		for (Sint32 i = 0; i < 8; ++i) {
			checkNPCMode(pno);

			if (playertwp[i] && playertwp[i]->ewp) {
				if (faceNo[i] >= 0) {
					if (faceNo[i]) {
						playertwp[i]->ewp->face.nbFrame = 80;
						playertwp[i]->ewp->face.old = faceNo[i];
						playertwp[i]->ewp->face.__new = faceNo[i];
						playertwp[i]->ewp->face.frame = 0;
					}
				}
			}
		}

		//Play stuff spawned by the particle generator.
		playParticles();

		//Create Custom Subtitle. Aside from names, this is the same as the original.
		if ((per[0]->press & Buttons_C && !KeyGetPress(KEYS_C)) || KeyGetPress(KEYS_L)) {
			std::ifstream subFile(subtitleFilePath);
			if (subFile.is_open())
			{
				std::getline(subFile, subUpper);
				std::getline(subFile, subLower);
				std::getline(subFile, subVoice);
				subFile.close();
			}

			if (subVoice.length() != 0) Serif_Play(atoi(subVoice.c_str()));
			if (!(subUpper.length() == 0 && subLower.length() == 0)) {
				snprintf(customSubtitle, LengthOfArray(customSubtitle), "\a%s \n%s", subUpper.c_str(), subLower.c_str());
				HintMainMessagesTime(customMessage, 180);
			}
		}

		//Player controls. Don't do anything when the display is off though.
		if (maker_mode == M_MODE_NONE && showDisplay) {
			if (per[0]->press & Buttons_Right) {
				pno = NJM_MIN(7, pno + 1);
				resetControllers();
			}
			else if (per[0]->press & Buttons_Left) {
				pno = NJM_MAX(0, pno - 1);
				resetControllers();

				if (pno == 0 && playertwp[0] && playertwp[0]->flag & 0x4000) {
					playertwp[0]->flag &= ~0x4000;
				}
			}

			//If (Not Player 1) character exists, press Z to remove them.
			if (pno && playertp[pno] && per[0]->press & Buttons_Z) {

				if (last1aHighFloor_tp[pno]) {
					FreeTask(last1aHighFloor_tp[pno]);
					last1aHighFloor_tp[pno] = 0;
				}
				EV_RemovePlayer(pno);
				return;
			}
		}

		//If the player doesn't exist, run the character spawn menu code.
		if (!playertp[pno]) {
			if (maker_mode == M_MODE_NONE) {
				switch (per[0]->press) {
				case Buttons_Up:
					characterCursor = NJM_MAX(0, characterCursor - 1);

					break;
				case Buttons_Down:
					characterCursor = NJM_MIN(charCursorMax, characterCursor + 1);
					break;
				case Buttons_Y:

					if (!allowDupeChars) {
						for (Sint32 i = 0; i < 8; ++i) {
							if (playertwp[i] && playertwp[i]->counter.b[1] == characterCursor) {
								Serif_Play(1483);
								return;
							}
						}
					}

					EV_CreatePlayer(pno, characterList[characterCursor], playertwp[0]->pos.x, playertwp[0]->pos.y, playertwp[0]->pos.z, 0, 0, 0);
					break;
				}
			}
			else {
				//Reset to the main menu if the player is deleted.
				//This can happen to Non-1P after the end of a cutscene.
				maker_mode = M_MODE_NONE;
			}
			return;
		}
		else if (pno) {
			//Repoint input to 1P's controller.
			per[pno] = per[0];

			//Don't want 1P moving.
			playerpwp[0]->nocontimer = 10;
			playertwp[0]->flag |= 0x4000;
		}

		//Turn on throughplayer, which disables collision between player characters.
		for (Sint32 i = 0; i < 8; ++i) {
			if (playertp[i]) {
				throughplayer_on(playertp[i]);
			}
		}

		

		//Check the skywalk for the last1a platform is there when it's active (for newly spawned characters)
		checkLast1A(pno);

		ControlEnabled = TRUE;

		//Check for key presses
		processInput();

		//Quick unfreeze
		if (maker_mode == M_MODE_NONE && per[0]->press & Buttons_A && playerFreeze[pno]) {
			playerFreeze[pno] = FALSE;

			releaseMode_accountforSuperSonic();
		}

		
		//Process mode.
		//These are always ran when the mode is active.
		switch (maker_mode) {
		case M_MODE_NONE:
			switch (per[0]->press) {
			case Buttons_Up:
				cursor = NJM_MAX(0, cursor - 1);
				break;
			case Buttons_Down:
				cursor = NJM_MIN(M_MODE_MAX - 2, cursor + 1);
				break;
			case Buttons_Y:
				if(showDisplay) //It's a bit awkward with it off
					maker_setMode(cursor + 1);
				return;
			}


			break;
		case M_MODE_FREEMOVE:
			playertwp[pno]->mode = 99;
			if (KeyGetPress(KEYS_F))
				playerFreeze[pno] = playerFreeze[pno] ? FALSE : TRUE;

			doBasicAnimation(pno);
			doPlayerMovement(pno);

			break;
		case M_MODE_CAMERA:
			doCameraSettngs(pno);
			break;
		case M_MODE_HEAD:
		{
			//Read from file to get face animation
			if (KeyGetPress(KEYS_RSHIFT) || per[0]->press & Buttons_Z) {
				faceNo[pno] = 0;
				std::ifstream subFile(faceFilePath);
				if (subFile.is_open())
				{
					std::getline(subFile, customFace);
					subFile.close();
				}
				EV_ClrFace(EV_GetPlayer(pno));
				if (customFace.length() > 0) EV_SetFace(EV_GetPlayer(pno), (char*)customFace.c_str());
			}
			doHeadFunctions(pno);
			break;
		}
		case M_MODE_ANIM:
			doAnimPlayer(pno);
			break;
		case M_MODE_PARTICLE:
			doParticleGenerator(pno);
			break;
		case M_MODE_EVENT:
			eventFunctionsMain(pno);
			break;
		case M_MODE_PROP:
			propSpawnerMain(pno);
			break;
		case M_MODE_NPC:
			doNPCModeDisplay(pno);
			break;
		case M_MODE_FLAGS:
			doSeqVarsEditor(pno);
			break;
		default:
			break;
		}
	}
}