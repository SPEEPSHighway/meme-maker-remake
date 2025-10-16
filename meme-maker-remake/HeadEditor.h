#pragma once
#include "SADXModLoader.h"
#include "Keys.h"

extern Sint8 faceNo[8];
extern task* playerheads[8];

void initFaceTableBackup();
void hookHead();
void getDirectAheadTask(task* tp);
void displayHeadInfo(Sint32 col, Sint32 pno);
void doHeadFunctions(Sint32 pno);