#pragma once
#include "SADXModLoader.h"
#include "Keys.h"

extern task* last1aHighFloor_tp[8];

void checkLast1A(Sint32 pno);
void advanceEventEffects();
void eventFunctionsMain(Sint32 pno);
void displayEventInfo(Sint32 col, Sint32 pno);