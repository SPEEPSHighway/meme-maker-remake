#include "SADXModLoader.h"
#include "AnimationVars.h"
#include "ObjectImport.h"
#include "Keys.h"

Sint8 ap_character_no[8];
Sint32 ap_anim_no[8];
Sint32 ap_anim_no_base[8];
Sint32 link_num = 8;
NJS_ACTION** gameplay_anim;
static Float animSpeed[8] = { 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, };
static Float animSpeedRate[8] = { 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f };
static Float hammerScale;
Bool charlist_Error;
Bool isLoop;
LPVOID* anidata[5];
Bool giveMeTheBird;
Bool tailsTails;


static Sint32 animEditID[8] = { 1, 1, 1, 1, 1, 1, 1, 1 };

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
Sint32 isGameplayOrCustomAnim(Sint32 pno) {
	if (ap_anim_no_base[pno] < 6) //Gameplay
		return TYPE_GAMEPLAY;
	else if (ap_anim_no_base[pno] < 14) //Custom
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
	njPrintC(NJM_LOCATION(2, col++), "LSHIFT/Z Button (RB) = Play");
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
	Sint32 colArrow = col + animEditID[pno];
	njPrintC(NJM_LOCATION(1, colArrow), ">");
	njPrintC(NJM_LOCATION(2, col++), "BACK");

	if (playlist[pno])
		njPrint(NJM_LOCATION(2, col++), "Interpolation frames = %d", link_num);
	else
		njPrint(NJM_LOCATION(2, col++), "Loop: %s", isLoop ? "ON" : "OFF");
	njPrint(NJM_LOCATION(2, col++), "Character: %s", character_name[ap_character_no[pno]].c_str());
	if (charlist_Error) { //Need to clarify this wasn't updated due to the below error
		njPrintColor(0xFFFF0000);
	}

	switch (isGameplayOrCustomAnim(pno)) {
	case TYPE_EVENT:
		njPrint(NJM_LOCATION(2, col++), "Animation: 0x%04X (%d)", animList[ap_anim_no[pno]].action, ap_anim_no[pno] - ap_anim_no_base[pno]);
		njPrint(NJM_LOCATION(3, col++), "Name: %s", animList[ap_anim_no[pno]].name.c_str());
		break;
	case TYPE_GAMEPLAY:
		njPrint(NJM_LOCATION(2, col++), "Animation: %d", ap_anim_no[pno]);
		njPrint(NJM_LOCATION(3, col++), "Name: %s[%d]", animList[ap_anim_no_base[pno]].name.c_str(), ap_anim_no[pno]);
		break;
	case TYPE_CUSTOM:
		njPrint(NJM_LOCATION(2, col++), "Animation: %d", ap_anim_no[pno]);
		njPrint(NJM_LOCATION(3, col++), "Name: %s", selectedCustomAnim.c_str());
		break;
	}

	if (charlist_Error) {
		njPrintC(NJM_LOCATION(40, col), "ERROR: Could not find start of anim list.");
		njPrintColor(0xFFFFFFFF);
	}
	njPrint(NJM_LOCATION(2, col++), "Movement Speed = %.3f", animSpeed[pno]);
	njPrint(NJM_LOCATION(2, col++), "Adjust Rate = %.3f", animSpeedRate[pno]);

	if (playertwp[pno]) {
		switch (playertwp[pno]->counter.b[1])
		{
		case PLNO_SONIC:
			njPrint(NJM_LOCATION(2, col++), "Sonic's Finger: %s", SONIC_OBJECTS[6]->sibling == SONIC_OBJECTS[5] ? "OFF" : "ON");
			col++;
			break;
		case PLNO_TAILS:
			njPrint(NJM_LOCATION(2, col++), "Animate tails: %s", tailsTails ? "OFF" : "ON");
			col++;
			break;
		case PLNO_BIG:
		case PLNO_E102:
			njPrintC(NJM_LOCATION(2, col++), "NOTE: Non-Player anims disabled for this character.");
			njPrintC(NJM_LOCATION(8, col++), "Their secondary objects can cause problems.");
			break;
		case PLNO_AMY:
			njPrint(NJM_LOCATION(2, col++), "Hammer Scale: %.2f", hammerScale);
			njPrint(NJM_LOCATION(2, col++), "Disable Bird: %s", giveMeTheBird ? "Y" : "N");
			break;
		default:
			col++;
			col++;
			break;
		}
		njPrintC(NJM_LOCATION(2, col++), "QUEUE OPTIONS (Z Button when selected)");
		njPrintC(NJM_LOCATION(2, col++), "Remove Last Animation");
		njPrintC(NJM_LOCATION(2, col++), "Clear Queue");
		njPrintC(NJM_LOCATION(2, col++), "PLAY");


	}
}

//Check the playlist exists for the global bind
Bool checkPlaylistExists(Sint32 pno) {
	return playlist[pno] != 0;
}

/// <summary>
/// Find where to start the anim list when switching characters
/// </summary>
void findCharAnimsBase(Sint32 pno) {
	for (Sint32 i = 0; animList[i].type != ANIM_MAX; ++i) {
		if (animList[i].type == ap_character_no[pno]) {
			charlist_Error = FALSE;
			ap_anim_no[pno] = i;
			ap_anim_no_base[pno] = i;
			return;
		}
	}

	//Didn't find it
	charlist_Error = TRUE;
}


//Copy animation playlist
EVENT_ACTION_LIST* EV_CopyPlaylist(EVENT_ACTION_LIST* src_p) {

	if (!src_p)
		return NULL;

	EVENT_ACTION_LIST* actionlist = (EVENT_ACTION_LIST*)CAlloc(1, sizeof(EVENT_ACTION_LIST));
	actionlist->mode = src_p->mode;
	actionlist->linkframe = src_p->linkframe;
	actionlist->action = src_p->action;
	actionlist->texlist = src_p->texlist;
	actionlist->speed = src_p->speed;
	actionlist->next = EV_CopyPlaylist(src_p->next);
	return actionlist;
}

//Delete animation playlist
void EV_ClearPlaylist(EVENT_ACTION_LIST* src_p) {

	if (!src_p)
		return;

	EVENT_ACTION_LIST* next;
	EVENT_ACTION_LIST* list = src_p;
	if (list)
	{
		do
		{
			next = list->next;
			Free(list);
			list = next;
		} while (next);
	}
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
	EVENT_ACTION_LIST* actionlist = (EVENT_ACTION_LIST*)CAlloc(1, sizeof(EVENT_ACTION_LIST));
	*listp = actionlist;
	actionlist->mode = mode;
	actionlist->linkframe = linkframe;
	actionlist->action = *ap;
	actionlist->texlist = lp;
	actionlist->speed = speed;
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
	NJS_TEXLIST* texlist = animList[ap_anim_no_base[pno]].texlist;
	const char* texname = animList[ap_anim_no_base[pno]].texname;

	if (isGameplayOrCustomAnim(pno) == TYPE_EVENT) {
		texlist = animList[ap_anim_no[pno]].texlist;
		texname = animList[ap_anim_no[pno]].texname;
	}


	//Store the final anim here.
	NJS_ACTION animation{};

	switch (isGameplayOrCustomAnim(pno)) {
	case TYPE_GAMEPLAY:
	{
		NJS_ACTION** anim = (NJS_ACTION**)animList[ap_anim_no_base[pno]].action;

		if (anim[ap_anim_no[pno]]) {
			animation = *anim[ap_anim_no[pno]];
		}
		break;
	}
	case TYPE_EVENT:
	{
		NJS_ACTION* anim = (NJS_ACTION*)animList[ap_anim_no[pno]].action;
		
		//Planes need to be read from a reference to be compatible with DC characters.
		if (animList[ap_anim_no[pno]].type == ANIM_PLANE) {
			anim = *(NJS_ACTION**)animList[ap_anim_no[pno]].action;
		}


		if (anim) {
			animation = *anim;
		}
		break;
	}
	case TYPE_CUSTOM:
	{
		NJS_ACTION** anim = (NJS_ACTION**)animList[ap_anim_no_base[pno]].action;
		std::string animstr = animList[ap_anim_no_base[pno]].name + selectedCustomAnim;
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
			EV_StoreAction(playertp[pno], &animation, texlist, animSpeed[pno], 0, link_num);
		else
			EV_SetAction(playertp[pno], &animation, texlist, animSpeed[pno], isLoop, link_num);
	}


}


void checkLinkframe(Sint32 pno) {
	/// <summary>
	/// Game can crash trying to linkframe between different models, so remove linkframe if somehow plays from a different model
	/// </summary>
	if (playertwp[pno] && playertwp[pno]->ewp && playertwp[pno]->ewp->action.list) {
		for (EVENT_ACTION_LIST* list = playertwp[pno]->ewp->action.list; list->next; list = list->next) {
			if (list->next->action.object != list->action.object) {
				list->linkframe = 0;
				list->next->linkframe = 0;
			}
		}
	}
}

void playAnimQueue(Sint32 pno) {

	if (!playlist[pno])
		return;

	//Back up the queue, since EV_SetAction deletes the data it uses afterward.
	EVENT_ACTION_LIST* playlist_copy = EV_CopyPlaylist(playlist[pno]);

	//Reset action
	EV_ClrAction(playertp[pno]);

	//Play the anim and restore the backup.
	playertwp[pno]->ewp->action.list = playlist[pno];
	playlist[pno] = playlist_copy;

	checkLinkframe(pno);
}


///Play animation
void playAnimSingle(Sint32 pno) {
	//Need to use this or it won't clean the action list, leading to the interpolation value causing problems.
	EV_ClrAction(playertp[pno]);

	//Play the anim
	processAnim(pno, FALSE);

	checkLinkframe(pno);
	
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
		animSpeed[pno] = NJM_MAX(0.0f, animSpeed[pno] - animSpeedRate[pno]);
	}
	else if (KeyGetOn(KEYS_PLUS)) {
		animSpeed[pno] = NJM_MIN(10000.0f, animSpeed[pno] + animSpeedRate[pno]);
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
			animSpeedRate[pno] = NJM_MAX(0.0f, animSpeedRate[pno] - 0.001f);
		}
		else if (KeyGetOn(KEYS_RBRACKET)) {
			animSpeedRate[pno] = NJM_MIN(1.0f, animSpeedRate[pno] + 0.001f);
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
		playAnimSingle(pno);
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

/// <summary>
/// Removes the last animation in the queue
/// </summary>
/// <param name="list"></param>
void removeLastAnimation(EVENT_ACTION_LIST* src_p) {

	if (!src_p)
		return;

	EVENT_ACTION_LIST* list_last{};
	EVENT_ACTION_LIST* list_2ndlast{};


	EVENT_ACTION_LIST* list = src_p;
	while (list->next) {
		list_2ndlast = list;
		list = list->next;
	}

	Free(list_last);

	if (list_2ndlast) {
		list_2ndlast->next = NULL;
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
		if (ap_character_no[pno] < ANIM_ZERO && playerCharNos[ap_character_no[pno]][0] != playertwp[pno]->counter.b[1]) {
			for (Sint32 i = 0; i < ANIM_ZERO; ++i) {
				if (playerCharNos[i][0] == playertwp[pno]->counter.b[1]) {
					ap_character_no[pno] = i;
					findCharAnimsBase(pno);
					if (isGameplayOrCustomAnim(pno)) //Reset anim number if gameplay anim
						ap_anim_no[pno] = 0;
					break;
				}
			}
		}
	}


	if (isGameplayOrCustomAnim(pno) == TYPE_CUSTOM) {
		std::string folder = AnimFolderPath + animList[ap_anim_no_base[pno]].name;

		//Update current prop selection
		getNameOfFile(&selectedCustomAnim, folder, ap_anim_no[pno], FALSE);

		/* Set the upper limit for propID.
		   If selectedPropFolder is empty (You deleted a folder) then adjust propID accordingly.*/
		if (selectedCustomAnim == "" && ap_anim_no[pno] > 0) {
			getNameOfFile(&selectedCustomAnim, folder, --ap_anim_no[pno], FALSE);
		}
	}

	Sint32 list_firstmax = 6;

	switch (playertwp[pno]->counter.b[1]) {
	case PLNO_SONIC:
	case PLNO_TAILS:
		list_firstmax = 7;
		break;
	case PLNO_AMY:
		list_firstmax = 8;
		break;
	}


	if (per[0]->press & Buttons_Z) {
		if (animEditID[pno] == 10) {

			Sint32 storedNum = 0;
			for (EVENT_ACTION_LIST* list = playlist[pno]; list; list = list->next) {
				++storedNum;
			}

			if (storedNum > 1) {
				removeLastAnimation(playlist[pno]);
			}
			else {
				EV_ClearPlaylist(playlist[pno]);
				playlist[pno] = NULL;
			}
		}
		else if (animEditID[pno] == 11) {
			EV_ClrAction(playertp[pno]);
			EV_ClearPlaylist(playlist[pno]);
			playlist[pno] = NULL;
		}
	}


	switch (isPressorOn) {
	case Buttons_Right:
		switch (animEditID[pno]) {
		case 1:
			if (playlist[pno])
				link_num = NJM_MIN(100, link_num + 1);
			else
				isLoop = isLoop != 1;
			break;
		case 2:
			if (character_name[ap_character_no[pno] + 1].size()) {
				if (ap_character_no[pno]++ < ANIM_ZERO && playerCharNos[ap_character_no[pno]][0] != playertwp[pno]->counter.b[1]) {
					//Big and Gamma have secondary objects that cause problems, so don't let them be non-players.
					if (playertwp[pno]->counter.b[1] != PLNO_E102 && playertwp[pno]->counter.b[1] != PLNO_BIG) {
						ap_character_no[pno] = ANIM_ZERO;
					}
					else {
						--ap_character_no[pno];
					}
				}

				findCharAnimsBase(pno);
			}
			break;
		case 3:
		{
			Sint32 newAnim = KeyGetOn(KEYS_RSHIFT) ? ap_anim_no[pno] + 10 : ap_anim_no[pno] + 1;


			switch (isGameplayOrCustomAnim(pno)) {
			case TYPE_EVENT:
				if (animList[newAnim].action && animList[newAnim].type == ap_character_no[pno]) {
					ap_anim_no[pno] = newAnim;
				}
				break;
			case TYPE_GAMEPLAY:
			{
				NJS_ACTION** anim = (NJS_ACTION**)animList[ap_anim_no_base[pno]].action;
				if (newAnim <= anim_gameplay_size[ap_anim_no_base[pno]]) {
					++ap_anim_no[pno];
				}
				break;
			}
			case TYPE_CUSTOM:
				++ap_anim_no[pno];
				break;
			}


			break;
		}
		case 5:
			animSpeed[pno] = NJM_MIN(10000.0f, animSpeed[pno] + animSpeedRate[pno]);
			break;
		case 6:
			animSpeedRate[pno] = NJM_MIN(1.0f, animSpeedRate[pno] + 0.001f);
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
		switch (animEditID[pno]) {
		case 1:
			if (playlist[pno])
				link_num = NJM_MAX(0, link_num - 1);
			else
				isLoop = isLoop != 1;
			break;
		case 2:
			ap_character_no[pno] = NJM_MAX(0, ap_character_no[pno] - 1);
			if (ap_character_no[pno] < ANIM_ZERO && playerCharNos[ap_character_no[pno]][0] != playertwp[pno]->counter.b[1]) {
				for (Sint32 i = ap_character_no[pno] + 1; i; --i) {
					if (playerCharNos[i][0] == playertwp[pno]->counter.b[1]) {
						ap_character_no[pno] = i;
						break;
					}
				}
			}

			findCharAnimsBase(pno);

			if (isGameplayOrCustomAnim(pno)) //Reset anim number if gameplay anim
				ap_anim_no[pno] = 0;
			break;
		case 3:
		{
			Sint32 newAnim = KeyGetOn(KEYS_RSHIFT) ? ap_anim_no[pno] - 10 : ap_anim_no[pno] - 1;
			switch (isGameplayOrCustomAnim(pno)) {
			case TYPE_EVENT:
				if (animList[newAnim].type == ap_character_no[pno]) {
					ap_anim_no[pno] = NJM_MAX(ap_anim_no_base[pno], newAnim);
				}
				break;
			case TYPE_GAMEPLAY:
			case TYPE_CUSTOM:
				ap_anim_no[pno] = NJM_MAX(0, newAnim);
				break;
			}
			break;
		}
		case 5:
			animSpeed[pno] = NJM_MAX(0.0f, animSpeed[pno] - animSpeedRate[pno]);
			break;
		case 6:
			animSpeedRate[pno] = NJM_MAX(0.0f, animSpeedRate[pno] - 0.001f);
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
		animEditID[pno] = NJM_MAX(0, animEditID[pno] - 1);

		if (animEditID[pno] > list_firstmax && animEditID[pno] < 10) {
			animEditID[pno] = list_firstmax;
		}

		if (animEditID[pno] == 4) //Skip over the name part
			--animEditID[pno];
		break;
	case Buttons_Down:
		animEditID[pno] = NJM_MIN(12, animEditID[pno] + 1);

		if (animEditID[pno] > list_firstmax && animEditID[pno] < 10) {
			animEditID[pno] = 10;
		}

		
		if (animEditID[pno] == 4) //Skip over the name part
			++animEditID[pno];
		break;
	}

	if (per[0]->press & Buttons_Y && animEditID[pno] == 0) {
		animEditID[pno] = 1;
		maker_mode = 0;
		return;
	}

	//Store/Play
	if (per[0]->press & Buttons_Z) {

		if (animEditID[pno] < 10) {
			if (per[0]->on & Buttons_Y)
				storeAnim(pno);
			else
				playAnimSingle(pno);
		}
		else if (animEditID[pno] == 12) {
			playAnimQueue(pno);
		}
	}

	if (playertwp[pno]->counter.b[1] == PLNO_AMY) {
		if (hammerScale)
			AmyForEventHammerScaleIm(pno, hammerScale);
	}

	///Reset
	if (KeyGetPress(KEYS_FSLASH)) {
		isLoop = FALSE;
		EV_ClrAction(playertp[pno]);
		EV_ClearPlaylist(playlist[pno]);
		playlist[pno] = NULL;

		animSpeed[pno] = 0.5f;
		animSpeedRate[pno] = 0.01f;
		link_num = 8;
		hammerScale = 0.0f;
		SONIC_OBJECTS[6]->sibling = SONIC_OBJECTS[5];
	}

	doAnimKeyboardControls(pno);
};