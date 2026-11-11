#pragma once
#include "SADXModLoader.h"
#include "Keys.h"

extern Sint8 faceNo[8];
extern task* playerheads[8];
extern Bool isLookingAtObject[8];

void initFaceTableBackup();
void hookHead();
void setLookingObject(Sint32 pno);
void getDirectAheadTask(task* tp);
void displayHeadInfo(Sint32 col, Sint32 pno);
void doHeadFunctions(Sint32 pno);