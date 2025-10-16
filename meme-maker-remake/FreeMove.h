#pragma once

#include "SADXModLoader.h"

extern Float movementSpeed;
extern Float movementSpeedRate;
extern Bool playerFreeze[8];
void doBasicAnimation(Sint32 pno);
void displayFreeMoveInfo(Sint32 col, Sint32 pno);
void doPlayerMovement(Sint32 pno);