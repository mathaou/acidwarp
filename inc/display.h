#ifndef DISPLAY_H_
#define DISPLAY_H_

/**
 * @file display.h
 * @brief Handlers for how the program interfaces with the display.
 *
 * @attention ACID WARP &copy; (c) Copyright 1992, 1993 by Noah Spurrier
 *
 * All Rights reserved. Private Proprietary Source Code by Noah Spurrier <br/>
 * Ported to Linux by Steven Wills <br/>
 * Ported to SDL by Boris Gjenero <br/>
 * Organized by Matt Farstad
 */

#include <stdint.h>

void disp_setPalette(unsigned char *palette);
void disp_beginUpdate(uint8_t **p, unsigned int *pitch,
                      unsigned int *w, unsigned int *h);
void disp_finishUpdate(void);
void disp_swapBuffers(void);
void disp_processInput(void);
#define DISP_FULLSCREEN 1
#define DISP_DESKTOP_RES_FS 2
void disp_init(int width, int height, int flags);
void disp_quit(void);

/* Callback */
void stopdrawing(void);
#endif
