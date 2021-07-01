#ifndef ROLNFADE_H_
#define ROLNFADE_H_

/**
 * @file roInfade.h
 * @brief No clue.
 *
 * @attention ACID WARP &copy; (c) Copyright 1992, 1993 by Noah Spurrier
 *
 * All Rights reserved. Private Proprietary Source Code by Noah Spurrier <br/>
 * Ported to Linux by Steven Wills <br/>
 * Ported to SDL by Boris Gjenero <br/>
 * Organized by Matt Farstad
 */

void newPalette(void);
void fadeInAndRotate(void);
void beginFadeOut(int toblack);
int fadeOut(void);
void initRolNFade(int logo);
void applyPalette(void);

void rolNFadMainPalAry2RndTargNLdDAC(uint8_t *MainPalArray,
                                     uint8_t *TargetPalArray);
void rolNFadeMainPalAryToTargNLodDAC(uint8_t *MainPalArray, uint8_t *TargetPalArray);
void rolNFadeBlkMainPalArrayNLoadDAC(uint8_t *MainPalArray);
void rolNFadeWhtMainPalArrayNLoadDAC(uint8_t *MainPalArray);
void rollMainPalArrayAndLoadDACRegs(uint8_t *MainPalArray);
void rotateforward(int color, uint8_t *Pal);
void rotatebackward(int color, uint8_t *Pal);

#endif