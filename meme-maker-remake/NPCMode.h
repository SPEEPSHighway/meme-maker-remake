#pragma once
#include "SADXModLoader.h"
#include "Keys.h"

void npcModeFunction(Sint32 pno);
Sint32 getNPCMotion(Sint32 index, Sint32 anim);
Sint32 getNPCIndex(Sint32 pno);
Bool isNPC(Sint32 pno);
void doNPCModeDisplay(Sint32 pno);
void checkNPCMode(Sint32 pno);
void displayNPCInfo(Sint32 col, Sint32 pno);