#pragma once
#include "SADXModLoader.h"

enum {
	M_MODE_NONE = 0,
	M_MODE_FREEMOVE = 1,
	M_MODE_CAMERA = 2,
	M_MODE_HEAD = 3,
	M_MODE_ANIM = 4,
	M_MODE_PARTICLE = 5,
	M_MODE_EVENT = 6,
	M_MODE_PROP = 7,
	M_MODE_NPC = 8,
	M_MODE_FLAGS = 9,
	M_MODE_MAX,
};

extern Sint32 maker_mode;

enum keys {
	KEYS_B = 5,
	KEYS_C = 6,
	KEYS_F = 9,
	KEYS_G = 10,
	KEYS_L = 15,
	KEYS_M = 16, //Y2
	KEYS_N = 17,
	KEYS_P = 19,
	KEYS_S = 22, //Y Button
	KEYS_V = 25, 
	KEYS_1 = 30, 
	KEYS_2 = 31, 
	KEYS_3 = 32, 
	KEYS_4 = 33, 
	KEYS_5 = 34, 
	KEYS_6 = 35, 
	KEYS_7 = 36, 
	KEYS_8 = 37, 
	KEYS_9 = 38, 
	KEYS_0 = 39, 
	KEYS_TAB = 43,
	KEYS_MINUS = 45,
	KEYS_PLUS = 46,
	KEYS_LARROW = 54,
	KEYS_RARROW = 55,
	KEYS_FSLASH = 56,
	KEYS_LBRACKET = 100,
	KEYS_RBRACKET = 101,
	KEYS_CAPSLOCK = 103,
	KEYS_LSHIFT = 104,
	KEYS_RSHIFT = 105,
	KEYS_LCTRL = 106
};

enum PLNO
{
	PLNO_SONIC = 0x0,
	PLNO_EGGMAN = 0x1,
	PLNO_TAILS = 0x2,
	PLNO_KNUCKLES = 0x3,
	PLNO_TIKAL = 0x4,
	PLNO_AMY = 0x5,
	PLNO_E102 = 0x6,
	PLNO_BIG = 0x7,
	PLNO_METAL_SONIC = 0x8,
	NB_PLNO = 0x9,
};

DataArray(FACETBL, faceEntries, 0x91CEC8, 160);

struct sp_info
{
	NJS_TEXLIST* texlist;
	NJS_TEXANIM* texanim;
	int animnum;
	int srcblend;
	int dstblend;
};

DataPointer(NJS_TEXLIST, texlist_capturebeam, 0x30B0104);
DataPointer(NJS_TEXLIST, texlist_l_sibuki, 0x2F3E234);
DataPointer(NJS_TEXLIST, texlist_e101factory, 0x306A394);
DataPointer(NJS_TEXLIST, texlist_expoly_0, 0x2F180BC);
DataPointer(NJS_TEXLIST, texlist_eggrob, 0x9924B0);


struct NJS_MESHSET_OLD
{
	Uint16 type_matId;
	Uint16 nbMesh;
	Sint16* meshes;
	Uint32* attrs;
	NJS_POINT3* normals;
	NJS_COLOR* vertcolor;
	NJS_COLOR* vertuv;
};

struct KeyboardKey {
	Uint8 on;
	Uint8 old;
	Uint8 press;
};

static const char* playerNames[] = {
	"SONIC",
	"EGGMAN",
	"TAILS",
	"KNUCKLES",
	"TIKAL",
	"AMY",
	"E102",
	"BIG"
};

static const char* metalSonicName = "METAL SONIC";

FunctionPointer(void, Free, (void* data), 0x40B310);
FunctionPointer(void, Chaos7SetModeChange, (Sint32 mode), 0x559210);
VoidFunc(LimitPosPlayer, 0x55FE00);
FunctionPointer(Sint32, SeqGetTime2, (), 0x412C10); // Get time of day

DataArray(KeyboardKey, keycheckdata, 0x3B0E3E0, 256);
DataArray(PL_ACTION, eggrob_action, 0x09BE388, 21);
DataPointer(task*, g_RotaryEmerald_p, 0x3C84814);
DataPointer(Uint8, chaos7_2nd_flag, 0x3C69E28);
DataPointer(Uint8, chaos7_game_clear_flag, 0x3C69E1C);
DataPointer(task*, p_Last1AHighTask, 0x3C84B68);
DataPointer(NJS_TEXLIST, texlist_s_sboard, 0x91CBD4);
DataPointer(NJS_TEXLIST, texlist_last1a_highway_a, 0x2E387F8);
DataPointer(NJS_TEXLIST, texlist_sspatcar_body, 0x2DBD918);
DataPointer(NJS_TEXLIST, texlist_sscar, 0x2AEE920);
DataPointer(NJS_TEXLIST, texlist_ev_chaos0_manju, 0x2D64FD0);
DataPointer(int, perspective_value, 0x3B2CBB4);
DataPointer(task*, p_Chaos0Task, 0x3C84800);
DataArray(NJS_ACTION*, anim_car_act, 0x2BBE9C8, 4);
DataArray(PDS_PERIPHERAL, perRaw, 0x3B0E9C8, 4);

FunctionPointer(void, KillHimByFallingDownP2, (int pno), 0x446AF0);
FunctionPointer(void, ObjectSwingableRegularExecute, (task* tp), 0x49DD40);
FunctionPointer(void, AmyBirdExe, (task* tp), 0x4C63F0);
FunctionPointer(void, AmyBirdInit, (task* tp), 0x4C6790);
FunctionPointer(void, AmyBirdStart, (), 0x4C6820);

