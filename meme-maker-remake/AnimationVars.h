#pragma once

#include "SADXModLoader.h"

struct anim_entry {
	Uint8 type;
	Sint32 action;
	std::string name;
	const char* texname;
	NJS_TEXLIST* texlist;
};

enum anim_type {
	ANIM_SONICG,
	ANIM_SONIC,
	ANIM_SONICC,
	ANIM_EGGMAN,
	ANIM_EGGMANC,
	ANIM_TAILSG,
	ANIM_TAILS,
	ANIM_TAILSC,
	ANIM_KNUCKLESG,
	ANIM_KNUCKLES,
	ANIM_KNUCKLESC,
	ANIM_TIKAL,
	ANIM_TIKALC,
	ANIM_AMYG,
	ANIM_AMY,
	ANIM_AMYC,
	ANIM_E102G,
	ANIM_E102,
	ANIM_E102C,
	ANIM_BIGG,
	ANIM_BIG,
	ANIM_BIGC,
	ANIM_ZERO,
	ANIM_PACHACAMAC,
	ANIM_TIKALCHAO,
	ANIM_EGGMOBILE,
	ANIM_EMERALD,
	ANIM_SSPOLICE,
	ANIM_PLANE,
	ANIM_CAR,
	ANIM_CUSTOM,
	ANIM_MISC,
	ANIM_MAX
};


extern Sint32 playerCharNos[][2];
extern std::string character_name[];
extern anim_entry animList[];
extern Sint32 anim_gameplay_size[];