#include "SADXModLoader.h"
#include "AnimationVars.h"
#include "ObjectImport.h"
#include "Keys.h"

Sint8 ap_character_no;
Sint32 ap_anim_no;
Sint32 ap_anim_no_base;
Sint32 link_num = 8;
NJS_ACTION** gameplay_anim;
static Float animSpeed = 0.5f;
static Float animSpeedRate = 0.1f;
static Float hammerScale;
Bool charlist_Error;
Bool isLoop;
LPVOID* anidata[5];
Bool giveMeTheBird;
Bool tailsTails;


static Sint32 animEditID = 1;

std::string AnimFolderPath;
std::string selectedCustomAnim;
static Sint32 customID;

DataPointer(NJS_OBJECT*, object_sonic_s_r_a5_01_s_r_a5_01_ptr, 0x6D711E); //Ref to Sonic's finger

EVENT_ACTION_LIST* playlist[8];



enum {
	TYPE_EVENT,
	TYPE_GAMEPLAY,
	TYPE_CUSTOM
};


/// <summary>
/// Sets the prop folder location
/// </summary>
/// <param name="folderpath"></param>
void setAnimFolder(std::string folderpath) {
	AnimFolderPath = folderpath;
}

/// <summary>
/// Check if it's one of the gameplay animation categories since they're stored differently.
/// </summary>
Sint32 isGameplayOrCustomAnim() {
	if (ap_anim_no_base < 6) //Gameplay
		return TYPE_GAMEPLAY;
	else if (ap_anim_no_base < 14) //Custom
		return TYPE_CUSTOM;
	else
		return TYPE_EVENT; //Event
}

/// <summary>
/// HUD for Animation Player
/// </summary>
void displayAnimInfo(Sint32 col, Sint32 pno) {
	njPrintC(NJM_LOCATION(1, col++), "Mode: Animation Player");
	col++;
	njPrintC(NJM_LOCATION(2, col++), "Play NJS_ACTION animations.");
	if (playlist[pno]) {
		njPrint(NJM_LOCATION(2, col++), "LSHIFT/Z Button (RB) = %s", playlist[pno] ? "Play queue" : "Play");
	}
	else {
		njPrintC(NJM_LOCATION(2, col++), "LSHIFT/Z Button (RB) = Play");
	}
	njPrintC(NJM_LOCATION(2, col++), "CTRL/Y+Z Button (Y+RB) = Store");
	njPrintC(NJM_LOCATION(2, col++), "Y (Held) = Fast List");
	njPrintC(NJM_LOCATION(2, col++), "/ = Reset");
	col++;
	Sint32 storedNum = 0;
	for (EVENT_ACTION_LIST* list = playlist[pno]; list; list = list->next) {
		++storedNum;
	}
	njPrint(NJM_LOCATION(2, col++), "Animations stored: %d", storedNum);
	col++;
	Sint32 colArrow = col + animEditID;
	njPrintC(NJM_LOCATION(1, colArrow), ">");
	njPrintC(NJM_LOCATION(2, col++), "BACK");

	if (playlist[pno])
		njPrint(NJM_LOCATION(2, col++), "Interpolation frames = %d", link_num);
	else
		njPrint(NJM_LOCATION(2, col++), "Loop: %s", isLoop ? "ON" : "OFF");
	njPrint(NJM_LOCATION(2, col++), "Character: %s", character_name[ap_character_no].c_str());
	if (charlist_Error) { //Need to clarify this wasn't updated due to the below error
		njPrintColor(0xFFFF0000);
	}

	switch (isGameplayOrCustomAnim()) {
	case TYPE_EVENT:
		njPrint(NJM_LOCATION(2, col++), "Animation: 0x%04X (%d)", animList[ap_anim_no].action, ap_anim_no - ap_anim_no_base);
		njPrint(NJM_LOCATION(3, col++), "Name: %s", animList[ap_anim_no].name.c_str());
		break;
	case TYPE_GAMEPLAY:
		njPrint(NJM_LOCATION(2, col++), "Animation: %d", ap_anim_no);
		njPrint(NJM_LOCATION(3, col++), "Name: %s[%d]", animList[ap_anim_no_base].name.c_str(), ap_anim_no);
		break;
	case TYPE_CUSTOM:
		njPrint(NJM_LOCATION(2, col++), "Animation: %d", ap_anim_no);
		njPrint(NJM_LOCATION(3, col++), "Name: %s", selectedCustomAnim.c_str());
		break;
	}

	if (charlist_Error) {
		njPrintC(NJM_LOCATION(40, col), "ERROR: Could not find start of anim list.");
		njPrintColor(0xFFFFFFFF);
	}
	njPrint(NJM_LOCATION(2, col++), "Movement Speed = %.3f", animSpeed);
	njPrint(NJM_LOCATION(2, col++), "Adjust Rate = %.3f", animSpeedRate);

	if (playertwp[pno]) {
		switch (playertwp[pno]->counter.b[1])
		{
		case PLNO_SONIC:
			njPrint(NJM_LOCATION(2, col++), "Sonic's Finger: %s", SONIC_OBJECTS[6]->sibling == SONIC_OBJECTS[5] ? "OFF" : "ON");
			break;
		case PLNO_TAILS:
			njPrint(NJM_LOCATION(2, col++), "Animate tails: %s", tailsTails ? "OFF" : "ON");
			break;
		case PLNO_BIG:
		case PLNO_E102:
			njPrintC(NJM_LOCATION(2, col++), "NOTE: Non-Player anims disabled for this character.");
			njPrintC(NJM_LOCATION(8, col++), "Their secondary objects can cause problems.");
			break;
		;	case PLNO_AMY:
			njPrint(NJM_LOCATION(2, col++), "Hammer Scale: %.2f", hammerScale);
			njPrint(NJM_LOCATION(2, col++), "Disable Bird: %s", giveMeTheBird ? "Y" : "N");
			break;
		}
	}
}

/// <summary>
/// Find where to start the anim list when switching characters
/// </summary>
void findCharAnimsBase() {
	for (Sint32 i = 0; animList[i].type != ANIM_MAX; ++i) {
		if (animList[i].type == ap_character_no) {
			charlist_Error = FALSE;
			ap_anim_no = i;
			ap_anim_no_base = i;
			return;
		}
	}

	//Didn't find it
	charlist_Error = TRUE;
}

///Edit of EV_SetAction to store in queue instead of play
void EV_StoreAction(task* tp, NJS_ACTION* ap, NJS_TEXLIST* lp, Float speed, Sint32 mode, Sint32 linkframe) {
	EVENT_ACTION_LIST* listp2;
	EVENT_ACTION_LIST* action;

	taskwk* twp = tp->twp;
	EVENT_ACTION_LIST** listp = &playlist[twp->counter.b[0]];
	if (playlist[twp->counter.b[0]])
	{
		do
		{
			listp2 = *listp;
			action = listp2->next;
			listp = &listp2->next;
		} while (action);
	}
	EVENT_ACTION_LIST* actionlist = (EVENT_ACTION_LIST*)CAlloc(1, 0x18);
	*listp = actionlist;
	actionlist->mode = mode;
	actionlist->linkframe = linkframe;
	actionlist->action = *ap;
	actionlist->texlist = lp;
	actionlist->speed = animSpeed;
}

///Sonic's pointing finger
void toggleSonicFinger() {
	if (SONIC_OBJECTS[6]->sibling == SONIC_OBJECTS[5]) {
		SONIC_OBJECTS[6]->sibling = object_sonic_s_r_a5_01_s_r_a5_01_ptr;
	}
	else {
		SONIC_OBJECTS[6]->sibling = SONIC_OBJECTS[5];
	}
}

/// <summary>
/// Accounts for Metal, Super Sonic and the snowboard.
/// </summary>
/// <param name="pno"></param>
void makerAnim_OtherSonics(NJS_ACTION* anim, NJS_TEXLIST** texlist, const char** texname) {
	if (gu8flgPlayingMetalSonic) {
		if (anim->object == SONIC_OBJECTS[0])
			anim->object = SONIC_OBJECTS[68];
		else if (anim->object == SONIC_OBJECTS[66])
			anim->object = SONIC_OBJECTS[69];
		else if (anim->object == SONIC_OBJECTS[67])
			anim->object = SONIC_OBJECTS[70];

		*texlist = &texlist_metalsonic;
		*texname = "METALSONIC";
	}

	if (playerpwp[0]->equipment & 0x8000) {
		if (anim->object == SONIC_OBJECTS[0])
			anim->object = SONIC_OBJECTS[22];
	}

	if (anim->object == SONIC_OBJECTS[22]) {
		*texlist = &SUPERSONIC_TEXLIST;
		*texname = "SUPERSONIC";
	}

	if (anim->object == SONIC_OBJECTS[71]) {
		*texlist = &texlist_s_sboard;
	}
}


/// Sets up an action for EV_SetAction and either plays it or stores it.
void processAnim(Sint32 pno, Bool isStore) {

	//Set up texlist
	NJS_TEXLIST* texlist = animList[ap_anim_no_base].texlist;
	const char* texname = animList[ap_anim_no_base].texname;

	if (isGameplayOrCustomAnim() == TYPE_EVENT) {
		texlist = animList[ap_anim_no].texlist;
		texname = animList[ap_anim_no].texname;
	}


	//Store the final anim here.
	NJS_ACTION animation{};

	switch (isGameplayOrCustomAnim()) {
	case TYPE_GAMEPLAY:
	{
		NJS_ACTION** anim = (NJS_ACTION**)animList[ap_anim_no_base].action;

		if (anim[ap_anim_no]) {
			animation = *anim[ap_anim_no];
		}
		break;
	}
	case TYPE_EVENT:
	{
		NJS_ACTION* anim = (NJS_ACTION*)animList[ap_anim_no].action;
		
		//Planes need to be read from a reference to be compatible with DC characters.
		if (animList[ap_anim_no].type == ANIM_PLANE) {
			anim = *(NJS_ACTION**)animList[ap_anim_no].action;
		}


		if (anim) {
			animation = *anim;
		}
		break;
	}
	case TYPE_CUSTOM:
	{
		NJS_ACTION** anim = (NJS_ACTION**)animList[ap_anim_no_base].action;
		std::string animstr = animList[ap_anim_no_base].name + selectedCustomAnim;
		NJS_MOTION* mtn = importSAANIM(&anidata[0], animstr.c_str());

		if (anim[0]->object && mtn) {
			animation.object = anim[0]->object;
			animation.motion = mtn;
		}
		break;
	}
	}

	if (animation.object && animation.motion) {

		//Account for Metal Sonic, Super Sonic, and the snowboard.
		makerAnim_OtherSonics(&animation, &texlist, &texname);
		
		//Snowboard is a pvr so it uses a different function
		if (texlist == &texlist_s_sboard) {
			NeonuLoadTexture(texlist);
		}
		else {
			texLoadTexturePvmFile(texname, texlist);
		}

		//Store or play
		if (isStore)
			EV_StoreAction(playertp[pno], &animation, texlist, animSpeed, 0, link_num);
		else
			EV_SetAction(playertp[pno], &animation, texlist, animSpeed, isLoop, link_num);
	}


}


///Play animation
void playMakerAnim(Sint32 pno) {
	if (playlist[pno]) {
		//Play stored queue and reset it.
		EV_ClrAction(playertp[pno]);
		playertwp[pno]->ewp->action.list = playlist[pno];
		playlist[pno] = 0;
	}
	else {
		//Need to use this or it won't clean the action list, leading to the interpolation value causing problems.
		EV_ClrAction(playertp[pno]);

		processAnim(pno, FALSE);
	}

	/// <summary>
	/// Game can crash trying to linkframe between different models, so remove linkframe if somehow plays from a different model
	/// </summary>
	/// 
	if (playertwp[pno] && playertwp[pno]->ewp && playertwp[pno]->ewp->action.list) {
		for (EVENT_ACTION_LIST* list = playertwp[pno]->ewp->action.list; list->next; list = list->next) {
			if (list->next->action.object != list->action.object) {
				list->linkframe = 0;
				list->next->linkframe = 0;
			}
		}
	}
}


/// <summary>
/// Store Animation
/// </summary>
/// <param name="pno"></param>
void storeAnim(Sint32 pno) {
	processAnim(pno, TRUE);
}

//Keyboard controls I had before making it use the controller list. No point in removing them.
static void doAnimKeyboardControls(Sint32 pno) {
	//KEYBOARD CONTROLS
	//Sonic's Finger
	if (KeyGetPress(KEYS_F) && playertwp[pno]->counter.b[1] == PLNO_SONIC) {
		toggleSonicFinger();
	}

	//Movement Speed
	if (KeyGetOn(KEYS_MINUS)) {
		animSpeed = NJM_MAX(0.0f, animSpeed - animSpeedRate);
	}
	else if (KeyGetOn(KEYS_PLUS)) {
		animSpeed = NJM_MIN(10000.0f, animSpeed + animSpeedRate);
	}

	if (KeyGetOn(KEYS_RSHIFT)) {
		//Interpolation (linkframe)
		if (KeyGetOn(KEYS_LBRACKET)) {
			link_num = NJM_MAX(0, link_num - 1);
		}
		else if (KeyGetOn(KEYS_RBRACKET)) {
			link_num = NJM_MIN(100, link_num + 1);
		}
	}
	else {
		//Adjust Rate
		if (KeyGetOn(KEYS_LBRACKET)) {
			animSpeedRate = NJM_MAX(0.0f, animSpeedRate - 0.001f);
		}
		else if (KeyGetOn(KEYS_RBRACKET)) {
			animSpeedRate = NJM_MIN(1.0f, animSpeedRate + 0.001f);
		}
	}

	if (KeyGetPress(KEYS_C)) {
		isLoop = isLoop ? FALSE : TRUE;
	}

	//Store Animation
	if (KeyGetPress(KEYS_LCTRL)) {
		storeAnim(pno);
	}

	//Play Animation/Animation Queue
	if (KeyGetPress(KEYS_LSHIFT)) {
		playMakerAnim(pno);
	}

}

///Toggles Amy's bird
static void birdToggle() {
	giveMeTheBird = giveMeTheBird != 1;

	if (!giveMeTheBird) {
		AmyBirdStart();
	}
}

///Toggles animation for Tails' tails.
static void tailsToggle(Sint32 pno) {
	tailsTails = tailsTails != 1;

	if (tailsTails) {
		playertwp[pno]->timer.b[3] |= 0x10;
	}
	else {
		playertwp[pno]->timer.b[3] &= ~0x4;
		playertwp[pno]->timer.b[3] &= ~0x10;
	}

}

///Animation Player Main Function
void doAnimPlayer(Sint32 pno) {
	//Selection Arrow
	Uint32 isPressorOn = per[0]->press;
	if (per[0]->on & Buttons_Y) {
		isPressorOn = per[0]->on & ~Buttons_Y;
	}

	//Search for the current character's designated areas if this is a player anim
	if (playertwp[pno]) {
		if (ap_character_no < ANIM_ZERO && playerCharNos[ap_character_no][0] != playertwp[pno]->counter.b[1]) {
			for (Sint32 i = 0; i < ANIM_ZERO; ++i) {
				if (playerCharNos[i][0] == playertwp[pno]->counter.b[1]) {
					ap_character_no = i;
					findCharAnimsBase();
					if (isGameplayOrCustomAnim()) //Reset anim number if gameplay anim
						ap_anim_no = 0;
					break;
				}
			}
		}
	}


	if (isGameplayOrCustomAnim() == TYPE_CUSTOM) {
		std::string folder = AnimFolderPath + animList[ap_anim_no_base].name;

		//Update current prop selection
		getNameOfFile(&selectedCustomAnim, folder, ap_anim_no, FALSE);

		/* Set the upper limit for propID.
		   If selectedPropFolder is empty (You deleted a folder) then adjust propID accordingly.*/
		if (selectedCustomAnim == "" && ap_anim_no > 0) {
			getNameOfFile(&selectedCustomAnim, folder, --ap_anim_no, FALSE);
		}
	}


	switch (isPressorOn) {
	case Buttons_Right:
		switch (animEditID) {
		case 1:
			if (playlist[pno])
				link_num = NJM_MIN(100, link_num + 1);
			else
				isLoop = isLoop != 1;
			break;
		case 2:
			if (character_name[ap_character_no + 1].size()) {
				if (ap_character_no++ < ANIM_ZERO && playerCharNos[ap_character_no][0] != playertwp[pno]->counter.b[1]) {
					//Big and Gamma have secondary objects that cause problems, so don't let them be non-players.
					if (playertwp[pno]->counter.b[1] != PLNO_E102 && playertwp[pno]->counter.b[1] != PLNO_BIG) {
						ap_character_no = ANIM_ZERO;
					}
					else {
						--ap_character_no;
					}
				}

				findCharAnimsBase();
			}
			break;
		case 3:
		{
			Sint32 newAnim = KeyGetOn(KEYS_RSHIFT) ? ap_anim_no + 10 : ap_anim_no + 1;


			switch (isGameplayOrCustomAnim()) {
			case TYPE_EVENT:
				if (animList[newAnim].action && animList[newAnim].type == ap_character_no) {
					ap_anim_no = newAnim;
				}
				break;
			case TYPE_GAMEPLAY:
			{
				NJS_ACTION** anim = (NJS_ACTION**)animList[ap_anim_no_base].action;
				if (newAnim <= anim_gameplay_size[ap_anim_no_base]) {
					++ap_anim_no;
				}
				break;
			}
			case TYPE_CUSTOM:
				++ap_anim_no;
				break;
			}


			break;
		}
		case 5:
			animSpeed = NJM_MIN(10000.0f, animSpeed + animSpeedRate);
			break;
		case 6:
			animSpeedRate = NJM_MIN(1.0f, animSpeedRate + 0.001f);
			break;
		case 7:
			if (playertwp[pno]) {
				switch (playertwp[pno]->counter.b[1]) {
				case PLNO_SONIC:
					toggleSonicFinger();
					break;
				case PLNO_TAILS:
					tailsToggle(pno);
					break;
				case PLNO_AMY:
					hammerScale = NJM_MIN(10000.0f, hammerScale + 0.01f);
					break;
				}
			}
			break;
		case 8:
			birdToggle();
			break;
		}
		break;
	case Buttons_Left:
		switch (animEditID) {
		case 1:
			if (playlist[pno])
				link_num = NJM_MAX(0, link_num - 1);
			else
				isLoop = isLoop != 1;
			break;
		case 2:
			ap_character_no = NJM_MAX(0, ap_character_no - 1);
			if (ap_character_no < ANIM_ZERO && playerCharNos[ap_character_no][0] != playertwp[pno]->counter.b[1]) {
				for (Sint32 i = ap_character_no + 1; i; --i) {
					if (playerCharNos[i][0] == playertwp[pno]->counter.b[1]) {
						ap_character_no = i;
						break;
					}
				}
			}

			findCharAnimsBase();

			if (isGameplayOrCustomAnim()) //Reset anim number if gameplay anim
				ap_anim_no = 0;
			break;
		case 3:
		{
			Sint32 newAnim = KeyGetOn(KEYS_RSHIFT) ? ap_anim_no - 10 : ap_anim_no - 1;
			switch (isGameplayOrCustomAnim()) {
			case TYPE_EVENT:
				if (animList[newAnim].type == ap_character_no) {
					ap_anim_no = NJM_MAX(ap_anim_no_base, newAnim);
				}
				break;
			case TYPE_GAMEPLAY:
			case TYPE_CUSTOM:
				ap_anim_no = NJM_MAX(0, newAnim);
				break;
			}
			break;
		}
		case 5:
			animSpeed = NJM_MAX(0.0f, animSpeed - animSpeedRate);
			break;
		case 6:
			animSpeedRate = NJM_MAX(0.0f, animSpeedRate - 0.001f);
			break;
		case 7:
			if (playertwp[pno]) {
				switch (playertwp[pno]->counter.b[1]) {
				case PLNO_SONIC:
					toggleSonicFinger();
					break;
				case PLNO_TAILS:
					tailsToggle(pno);
					break;
				case PLNO_AMY:
					hammerScale = NJM_MAX(0.0f, hammerScale - 0.01f);
					if (hammerScale == 0.0f) //Reset the hammer using this function's reset
						AmyForEventHammerScaleIm(pno, hammerScale);
					break;
				}
			}
			break;
		case 8:
			birdToggle();
			break;
		}
		break;
	case Buttons_Up:
		animEditID = NJM_MAX(0, animEditID - 1);

		if (animEditID == 4) //Skip over the name part
			--animEditID;
		break;
	case Buttons_Down:
		//Only Sonic and Amy have the last list option
		if (animEditID != 6 ||
			((playertwp[pno]->counter.b[1] == PLNO_SONIC && animEditID != 7) ||
				playertwp[pno]->counter.b[1] == PLNO_TAILS && animEditID != 7 ||
				playertwp[pno]->counter.b[1] == PLNO_AMY))
			animEditID = NJM_MIN(8, animEditID + 1);
		if (animEditID == 4) //Skip over the name part
			++animEditID;
		break;
	}

	if (per[0]->press & Buttons_Y && animEditID == 0) {
		animEditID = 1;
		maker_mode = 0;
		return;
	}

	//Store/Play
	if (per[0]->press & Buttons_Z) {
		if (per[0]->on & Buttons_Y)
			storeAnim(pno);
		else
			playMakerAnim(pno);
	}

	if (playertwp[pno]->counter.b[1] == PLNO_AMY) {
		if (hammerScale)
			AmyForEventHammerScaleIm(pno, hammerScale);
	}

	///Reset
	if (KeyGetPress(KEYS_FSLASH)) {
		isLoop = FALSE;
		EV_ClrAction(playertp[pno]);
		animSpeed = 0.5f;
		animSpeedRate = 0.01f;
		link_num = 8;
		hammerScale = 0.0f;
		SONIC_OBJECTS[6]->sibling = SONIC_OBJECTS[5];
	}

	doAnimKeyboardControls(pno);
};