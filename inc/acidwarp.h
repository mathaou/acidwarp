#ifndef ACIDWARP_H_
#define ACIDWARP_H_

/**
 * @file acidwarp.h
 * @brief Main entry point for the program.
 *
 * @attention ACID WARP &copy; (c) Copyright 1992, 1993 by Noah Spurrier
 *
 * All Rights reserved. Private Proprietary Source Code by Noah Spurrier <br/>
 * Ported to Linux by Steven Wills <br/>
 * Ported to SDL by Boris Gjenero <br/>
 * Organized by Matt Farstad
 */

#include <string.h>
#include <SDL-1.2/include/SDL.h>
#include <SDL-1.2/include/SDL_main.h>

#include "handy.h"
#include "rolnfade.h"
#include "display.h"

#define VERSION "Acid Warp Version 4.10 (C)Copyright 1992, 1993 by Noah Spurrier and Mark Bilk\nSDL port by Boris Gjenero based on Linux port by Steven Wills"

#define NUM_IMAGE_FUNCTIONS             40
#define NOAHS_FACE                      0
#define DIRECTN_CHANGE_PERIOD_IN_TICKS  256
#define TIMER_INTERVAL                  (ROTATION_DELAY / 1000)

#ifdef EMSCRIPTEN
#define ENABLE_WORKER
#include <emscripten.h>
#else /* !EMSCRIPTEN */
/* Full screen doesn't work with Emscripten. Use F11 instead. */
void timer_quit(void);
#define HAVE_FULLSCREEN
#define ENABLE_THREADS
#endif

/* Palette types  */
#define RGBW_PAL          	    0
#define W_PAL                   1
#define W_HALF_PAL              2
#define PASTEL_PAL              3
#define RGBW_LIGHTNING_PAL      4
#define W_LIGHTNING_PAL         5
#define W_HALF_LIGHTNING_PAL    6
#define PASTEL_LIGHTNING_PAL    7
#define RED    0
#define GREEN  1
#define BLUE   2
#define NUM_PALETTE_TYPES       8

#define NUM_IMAGE_FUNCTIONS 40
#define NOAHS_FACE          0

/**
 * @brief Enumerated type for handling command line input
 */
enum acidwarp_command {
  CMD_PAUSE = 1,
  CMD_SKIP,
  CMD_QUIT,
  CMD_NEWPAL,
  CMD_LOCK,
  CMD_PAL_FASTER,
  CMD_PAL_SLOWER,
  CMD_RESIZE
};

/**
 * @brief structure for holding timer data - defined in @c acidwarp.c
 *
 */
struct timer_data {
  SDL_cond *cond /**asdfasdf*/;
  SDL_mutex *mutex;
  SDL_TimerID timer_id;
  SDL_bool flag;
};

/**
 * @brief asdf
 */
enum display_state {
  STATE_INITIAL,
  STATE_NEXT,
#if defined(EMSCRIPTEN) && defined(ENABLE_WORKER)
  STATE_WAIT,
#endif /* EMSCRIPTEN && ENABLE_WORKER */
  STATE_DISPLAY,
  STATE_FADEOUT
};

/* Prototypes for forward referenced functions */
void printStrArray(char *strArray[]);
void commandline(int argc, char *argv[]);
void mainLoop(void);
void handleinput(enum acidwarp_command cmd);
void generate_image_float(int imageFuncNum, uint8_t *buf_graf,
                          int xcenter, int ycenter,
                          int width, int height,
                          int colors, int pitch, int normalize);
void generate_image(int imageFuncNum, uint8_t *buf_graf,
                    int xcenter, int ycenter,
                    int width, int height,
                    int colors, int pitch);
void fatalSDLError(const char *msg);
void quit(int retcode);

void timer_lock(void);
void timer_unlock(void);
void timer_wait(void);
void timer_init(void);
uint32_t timer_proc(uint32_t interval, void *param);


void makeShuffledList(int *list, int listSize);
#ifdef ENABLE_WORKER
void startloop(void);
#endif

#define DRAW_LOGO   1
#define DRAW_FLOAT  2
#define DRAW_SCALED 4
void draw_init(int flags);
void draw_same(void);
void draw_next(void);
void draw_abort(void);
void draw_quit(void);
extern int abort_draw;

#endif
