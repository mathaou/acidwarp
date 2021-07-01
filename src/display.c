/* ACID WARP (c)Copyright 1992, 1993 by Noah Spurrier
 * All Rights reserved. Private Proprietary Source Code by Noah Spurrier
 * Ported to Linux by Steven Wills
 * Ported to SDL by Boris Gjenero
 */

#include <string.h>
#include <SDL-1.2/include/SDL.h>
#include "acidwarp.h"
#include "display.h"

#if !defined(WIN32) && !SDL_VERSION_ATLEAST(2, 0, 0)
#define HAVE_PALETTE
#endif

static SDL_Surface *screen = NULL, *out_surf = NULL;
#ifdef ENABLE_THREADS
static uint8_t *out_buf = NULL;
static SDL_Surface *draw_surf = NULL;
#else /* !ENABLE_THREADS */
#define draw_surf out_surf
#endif /* !ENABLE_THREADS */
static int disp_DrawingOnSurface;
static uint8_t *draw_buf = NULL;

#ifdef HAVE_PALETTE
static int disp_UsePalette;
#endif

#ifdef HAVE_FULLSCREEN
static int fullscreen = 0;
static int nativewidth = 0, nativeheight;
#endif
static int winwidth = 0;
static int winheight;

static int scaling = 1;
static int width, height;

void disp_setPalette(unsigned char *palette)
{
  static SDL_Color sdlColors[256];

  int i;
  for (i = 0; i < 256; i++)
  {
    sdlColors[i].r = palette[i * 3 + 0] << 2;
    sdlColors[i].g = palette[i * 3 + 1] << 2;
    sdlColors[i].b = palette[i * 3 + 2] << 2;
  }

#ifdef HAVE_PALETTE
  if (disp_UsePalette)
  {
    /* Simply change the palette */
    SDL_SetPalette(screen, SDL_PHYSPAL, sdlColors, 0, 256);
  }
  else
#endif /* HAVE_PALETTE */
  {
    /* Update colours in software surface, blit it to the screen
     * with updated colours, and then show it on the screen.
     */
    SDL_SetColors(out_surf, sdlColors, 0, 256);
    if (out_surf != screen)
    {
      SDL_BlitSurface(out_surf, NULL, screen, NULL);
    }
    SDL_Flip(screen);
  }
}

void disp_beginUpdate(uint8_t **p, unsigned int *pitch,
                      unsigned int *w, unsigned int *h)
{
#ifndef WITH_GL
  /* Locking only needed at this point if drawing routines directly draw
   * on a surface, and that surface needs locking.
   */
  if (disp_DrawingOnSurface)
  {
    if (SDL_MUSTLOCK(draw_surf))
    {
      if (SDL_LockSurface(draw_surf) != 0)
      {
        fatalSDLError("locking surface when starting update");
      }
    }
    *p = draw_surf->pixels;
    *pitch = draw_surf->pitch;
  }
  else
#endif
  {
    *p = draw_buf;
    *pitch = width;
  }
  *w = width;
  *h = height;
}

void disp_finishUpdate(void)
{
#ifndef WITH_GL
  /* Locking only needed at this point if drawing routines directly draw
   * on a surface, and that surface needs locking.
   */
  if (disp_DrawingOnSurface)
  {
    if (SDL_MUSTLOCK(draw_surf))
      SDL_UnlockSurface(draw_surf);
    draw_buf = NULL;
  }
#endif
}

#ifndef WITH_GL
static void disp_toSurface(void)
{
  int row;
  unsigned char *outp, *inp =
#ifdef ENABLE_THREADS
                           out_buf;
#else /* !ENABLE_THREADS */
                           draw_buf;
#endif
  /* This means drawing was on a separate buffer and it needs to be
   * copied to the surface. It also means the surface hasn't been locked.
   */
  if (SDL_MUSTLOCK(out_surf))
  {
    if (SDL_LockSurface(out_surf) != 0)
    {
      fatalSDLError("locking surface when ending update");
    }
  }
  outp = out_surf->pixels;

  if (scaling == 1)
  {
    for (row = 0; row < height; row++)
    {
      memcpy(outp, inp, width);
      outp += out_surf->pitch;
      inp += width;
    }
  }
  else if (scaling == 2)
  {
    unsigned char *outp2 = outp + out_surf->pitch;
    int skip = (out_surf->pitch - width) << 1;
    int col;
    unsigned char c;
    for (row = 0; row < height; row++)
    {
      for (col = 0; col < width; col++)
      {
        c = *(inp++);
        *(outp++) = c;
        *(outp++) = c;
        *(outp2++) = c;
        *(outp2++) = c;
      }
      outp += skip;
      outp2 += skip;
    }
  }
  if (SDL_MUSTLOCK(out_surf))
  {
    SDL_UnlockSurface(out_surf);
  }
}
#endif /* !WITH_GL */

void disp_swapBuffers(void)
{
  if (!disp_DrawingOnSurface)
  {
#ifdef ENABLE_THREADS
    {
      uint8_t *temp = draw_buf;
      draw_buf = out_buf;
      out_buf = temp;
    }
#endif /* ENABLE_THREADS */
    disp_toSurface();
  }
#ifdef ENABLE_THREADS
  else
  {
    SDL_Surface *temp = draw_surf;
    draw_surf = out_surf;
    out_surf = temp;
  }
#endif /* ENABLE_THREADS */

  if (out_surf != screen)
  {
    SDL_BlitSurface(out_surf, NULL, screen, NULL);
  }
  SDL_Flip(screen);
}

#ifdef HAVE_FULLSCREEN
static void disp_toggleFullscreen(void)
{
  if (fullscreen)
  {
    /* If going back to windowed mode, restore window size */
    if (winwidth != 0)
    {
      disp_init(winwidth, winheight, 0);
      winwidth = 0;
    }
    else
    {
      disp_init(width, height, 0);
    }
  }
  else
  {
    /* Save window size for return to windowed mode */
    winwidth = width;
    winheight = height;
    /* disp_init() may select a different size than suggested. It will
     * handle resizing if needed.
     */
    disp_init(width, height, DISP_FULLSCREEN);
  }
#endif
  SDL_ShowCursor(!fullscreen);
}

static void disp_processKey(
    SDLKey key, SDLMod keymod)
{
  switch (key)
  {
  case SDLK_UP:
    handleinput(CMD_PAL_FASTER);
    break;
  case SDLK_DOWN:
    handleinput(CMD_PAL_SLOWER);
    break;
  case SDLK_p:
    handleinput(CMD_PAUSE);
    break;
  case SDLK_n:
    handleinput(CMD_SKIP);
    break;
#ifndef EMSCRIPTEN
  case SDLK_c:
  case SDLK_PAUSE:
    if ((keymod & KMOD_CTRL) == 0)
      break;
  case SDLK_q:
    handleinput(CMD_QUIT);
    break;
#endif
  case SDLK_k:
    handleinput(CMD_NEWPAL);
    break;
  case SDLK_l:
    handleinput(CMD_LOCK);
    break;
#ifdef HAVE_FULLSCREEN
  case SDLK_ESCAPE:
    if (fullscreen)
      disp_toggleFullscreen();
    break;
  case SDLK_RETURN:
    if (keymod & KMOD_ALT)
      disp_toggleFullscreen();
    break;
#endif /* HAVE_FULLSCREEN */
  default:
    break;
  }
#undef keymod
}

static void display_redraw(void)
{
  return;
}

void disp_processInput(void)
{
  SDL_Event event;

  while (SDL_PollEvent(&event) > 0)
  {
    switch (event.type)
    {
#if !SDL_VERSION_ATLEAST(2, 0, 0)
    case SDL_VIDEOEXPOSE:
      display_redraw();
      break;
#endif /* !SDL_VERSION_ATLEAST(2,0,0) */
#if defined(HAVE_FULLSCREEN) && !defined(EMSCRIPTEN)
    /* SDL full screen switching has no useful effect with Emscripten */
    case SDL_MOUSEBUTTONDOWN:
      if (event.button.button == SDL_BUTTON_LEFT)
      {
        // Earlier SDL versions don't report double clicks
        static uint32_t dblclicktm = 0;
        uint32_t clicktime = SDL_GetTicks();
        // Like !SDL_TICKS_PASSED(), which may not be available
        if ((int32_t)(dblclicktm - clicktime) > 0)
        {
          disp_toggleFullscreen();
        }
        dblclicktm = clicktime + 200;
#endif // !SDL_VERSION_ATLEAST(2,0,2)
      }
      break;
    case SDL_KEYDOWN:
      disp_processKey(event.key.keysym.sym
#if !SDL_VERSION_ATLEAST(2, 0, 0)
                      ,
                      event.key.keysym.mod
#endif
      );
      break;
    case SDL_VIDEORESIZE:
      /* Why are there events when there is no resize? */
      if (width != (event.resize.w / scaling) ||
          height != (event.resize.h / scaling))
      {
        disp_init(event.resize.w / scaling, event.resize.h / scaling,
#ifdef HAVE_FULLSCREEN
                  fullscreen
#else /* !HAVE_FULLSCREEN */
                0
#endif
        );
      }
      break;
    case SDL_QUIT:
      handleinput(CMD_QUIT);
      break;

    default:
      break;
    }
  }
}

#if defined(HAVE_FULLSCREEN) && !SDL_VERSION_ATLEAST(2, 0, 0)
/* Function for finding the best SDL full screen mode for filling the screen.
 *
 * Inputs:
 * modes: array of pointers to SDL_Rect structures describing modes.
 * width, height: dimensions of desired mode
 * desiredaspect: desired aspect ratio
 *
 * Outputs:
 * width, height: updated with dimensions of found mode
 * scaling: updated with scaling to be used along with that mode
 */
static void disp_findBestMode(SDL_Rect **modes,
                              int *width, int *height,
                              int *scaling, int desiredaspect)
{
  int bestdiff = -1;
  int curpix = *width * *height;
  int i, j;
  for (i = 0; modes[i]; i++)
  {
    /* For every mode, try every possible scaling */
    for (j = 1; j <= 2; j++)
    {
      int asperr, pixerr, curdiff;

      /* Difference in number of pixels */
      pixerr = modes[i]->w * modes[i]->h / (j * j) - curpix;
      if (pixerr < 0)
        pixerr = -pixerr;

      /* Difference in aspect ratio compared to desktop */
      if (desiredaspect > 0)
      {
        int aspect = modes[i]->w * 1024 / modes[i]->h;
        asperr = aspect - desiredaspect;
        if (asperr < 0)
          asperr = -asperr;
        /* Aspect ratio is important because we want to fill screen */
        asperr *= 1024;
      }
      else
      {
        asperr = 0;
      }

      /* Use sum of pixel and aspect ratio error */
      curdiff = pixerr + asperr;

      /* Check if this mode is better */
      if (bestdiff == -1 || curdiff < bestdiff ||
          (curdiff == bestdiff && j < *scaling))
      {
        *scaling = j;
        *width = modes[i]->w / j;
        *height = modes[i]->h / j;
        bestdiff = curdiff;
      }
    }
  }
}
#endif /* HAVE_FULLSCREEN */

#ifdef ADDICON
extern unsigned char acidwarp_rgb[];
/* For SDL 1 call before SDL_SetWindowMode().
 * For SDL 2 call after window is created.
 */
static void disp_setIcon(void)
{
  SDL_Surface *iconsurface =
      SDL_CreateRGBSurfaceFrom(acidwarp_rgb, 64, 64, 24, 64 * 3,
                               0x0000ff, 0x00ff00, 0xff0000, 0
                               /* Big endian may need:  0xff0000, 0x00ff00, 0x0000ff, 0 */
      );
  if (iconsurface == NULL)
    fatalSDLError("creating icon surface");

#if SDL_VERSION_ATLEAST(2, 0, 0)
  SDL_SetWindowIcon(window, iconsurface);
#else
  /* Must be called before SDL_SetVideoMode() */
  SDL_WM_SetIcon(iconsurface, NULL);
#endif
  SDL_FreeSurface(iconsurface);
}
#endif /* ADDICON */

static void disp_freeBuffer(uint8_t **buf)
{
  if (*buf != NULL)
  {
    free(*buf);
    *buf = NULL;
  }
}

static void disp_reallocBuffer(uint8_t **buf)
{
  disp_freeBuffer(buf);
  *buf = calloc(width * height, 1);
  if (*buf == NULL)
  {
    printf("Couldn't allocate graphics buffer.\n");
    quit(-1);
  }
}

#ifndef WITH_GL
static void disp_freeSurface(SDL_Surface **surf)
{
  if (*surf != NULL)
  {
    SDL_FreeSurface(*surf);
    *surf = NULL;
  }
}

static void disp_allocSurface(SDL_Surface **surf)
{
  *surf = SDL_CreateRGBSurface(SDL_SWSURFACE,
                               width * scaling, height * scaling,
                               8, 0, 0, 0, 0);
  if (!(*surf))
    fatalSDLError("creating secondary surface");
}

#endif /* !WITH_GL */

static void disp_allocateOffscreen(void)
{
#ifdef ENABLE_THREADS
  /* Drawing must not be happening in the background
   * while the memory being drawn to gets reallocated!
   */
  draw_abort();
#endif /* ENABLE_THREADS */
  /* Free secondary surface */
  if (out_surf != screen)
    disp_freeSurface(&out_surf);
#ifdef ENABLE_THREADS
  disp_freeSurface(&draw_surf);
#endif /* ENABLE_THREADS */

#ifdef HAVE_PALETTE
  if (disp_UsePalette)
  {
    /* When using a real palette, draw_buf is used instead. */
    out_surf = screen;
  }
  else
#endif /* HAVE_PALETTE */
  {
    /* Create 8 bit surface to draw into. This is needed if pixel
     * formats differ or to respond to SDL_VIDEOEXPOSE events.
     */
    disp_allocSurface(&out_surf);
  }

  if (scaling == 1
  /* Normally need to have offscreen data for expose events,
       * but no need for that with Emscripten.
       */
#if defined(HAVE_PALETTE) && !defined(EMSCRIPTEN)
      && !disp_UsePalette
#endif
  )
  {
    if (!disp_DrawingOnSurface)
    {
      disp_freeBuffer(&draw_buf);
#ifdef ENABLE_THREADS
      disp_freeBuffer(&out_buf);
#endif /* ENABLE_THREADS */
    }
    disp_DrawingOnSurface = 1;
#ifdef ENABLE_THREADS
    disp_allocSurface(&draw_surf);
#endif /* ENABLE_THREADS */
  }
  else
  {
    disp_DrawingOnSurface = 0;
    disp_reallocBuffer(&draw_buf);
#ifdef ENABLE_THREADS
    disp_reallocBuffer(&out_buf);
#endif /* ENABLE_THREADS */
  }
}

void disp_init(int newwidth, int newheight, int flags)
{
  Uint32 videoflags;
  static int inited = 0;
#ifdef HAVE_PALETTE
  static int nativedepth = 8;
  int usedepth;
#endif
#ifdef HAVE_FULLSCREEN
  static int desktopaspect = 0;
#endif

  width = newwidth;
  height = newheight;
#ifdef HAVE_FULLSCREEN
  fullscreen = (flags & DISP_FULLSCREEN) ? 1 : 0;
#endif
  videoflags = SDL_HWSURFACE | SDL_DOUBLEBUF |
#ifndef HAVE_PALETTE
               SDL_ANYFORMAT |
#endif
#ifdef HAVE_FULLSCREEN
               /* It would make sense to remove SDL_RESIZABLE for full screen,
                * but that causes window to not be resizable anymore in Linux
                * after it returns to windowed mode. */
               (fullscreen ? SDL_FULLSCREEN : 0) |
#endif
               SDL_RESIZABLE;

  if (!inited)
  {
#ifdef HAVE_FULLSCREEN
    const SDL_VideoInfo *vi;

    /* Save information about desktop video mode */
    vi = SDL_GetVideoInfo();
    if (vi != NULL)
    {
#ifdef HAVE_PALETTE
      nativedepth = vi->vfmt->BitsPerPixel;
#endif
      if (vi->current_w > 0 && vi->current_h > 0)
      {
        if (flags & DISP_DESKTOP_RES_FS)
        {
          nativewidth = vi->current_w;
          nativeheight = vi->current_h;
          if (flags & DISP_FULLSCREEN)
          {
            /* Save size, which is for windowed mode */
            winwidth = newwidth;
            winheight = newheight;
          }
        }
        else
        {
          desktopaspect = vi->current_w * 1024 / vi->current_h;
        }
      }
    }
#endif /* HAVE_FULLSCREEN */

    SDL_WM_SetCaption("Acidwarp", "acidwarp");

#ifdef ADDICON
    /* Must be called before SDL_SetVideoMode() */
    disp_setIcon();
#endif

#ifdef HAVE_FULLSCREEN
    /* This causes an error when using Emscripten and Firefox */
    SDL_ShowCursor(!fullscreen);
#endif

    inited = 1;
  } /* !inited */

#ifdef HAVE_PALETTE
  usedepth = nativedepth;
#endif
#ifdef HAVE_FULLSCREEN
  if (fullscreen && nativewidth == 0)
  {
    SDL_Rect **modes;

#ifdef HAVE_PALETTE
    /* Attempt to list 256 colour modes */
    struct SDL_PixelFormat wantpf;
    memset(&wantpf, 0, sizeof(wantpf));
    wantpf.BitsPerPixel = 8;
    wantpf.BytesPerPixel = 1;
    modes = SDL_ListModes(&wantpf, videoflags | SDL_HWPALETTE);
    if (modes != NULL)
    {
      /* Found 256 colour mode. Use it. */
      disp_UsePalette = 1;
      usedepth = 8;
      videoflags |= SDL_HWPALETTE;
    }
    else
    {
      /* Couldn't find a 256 colour mode. Try to find any mode. */
      disp_UsePalette = 0;
      videoflags |= SDL_ANYFORMAT;
      modes = SDL_ListModes(NULL, videoflags);
    }
#else  /* !HAVE_PALETTE */
    /* Get available fullscreen modes */
    modes = SDL_ListModes(NULL, videoflags);
#endif /* !HAVE_PALETTE */
    if (modes == NULL)
    {
      fatalSDLError("listing full screen modes");
    }
    else if (modes == (SDL_Rect **)-1)
    {
      /* All resolutions ok */
      scaling = 1;
    }
    else
    {
      disp_findBestMode(modes, &width, &height, &scaling, desktopaspect);
    }
  }
  else
#endif /* HAVE_FULLSCREEN */
  {
#ifdef HAVE_FULLSCREEN
    if (fullscreen)
    {
      /* This happens when using desktop
       * resolution for full screen.
       */
      width = nativewidth;
      height = nativeheight;
#ifdef HAVE_PALETTE
      usedepth = 8;
#endif
    }
#endif

    scaling = 1;
#ifdef HAVE_PALETTE
    if (usedepth == 8)
    {
      disp_UsePalette = 1;
      /* This was slower with Emscripten SDL 1 before,
       * but now it should be faster.
       */
      videoflags |= SDL_HWPALETTE;
    }
    else
    {
      disp_UsePalette = 0;
      videoflags |= SDL_ANYFORMAT;
    }
#endif
  }

  /* The screen is a destination for SDL_BlitSurface() copies.
   * Nothing is ever directly drawn here, except with Emscripten.
   */
  screen = SDL_SetVideoMode(width * scaling, height * scaling,
#ifdef HAVE_PALETTE
                            usedepth,
#else
                            0,
#endif
                            videoflags);
  if (!screen)
    fatalSDLError("setting video mode");
  /* No need to ever free the screen surface from SDL_SetVideoMode() */

  disp_allocateOffscreen();

  /* This may be unnecessary if switching between windowed
   * and full screen mode with the same dimensions. */
  handleinput(CMD_RESIZE);
}

void disp_quit(void)
{
  if (disp_DrawingOnSurface)
  {
    if (out_surf == screen)
    {
      out_surf = NULL;
    }
    else
    {
      disp_freeSurface(&out_surf);
    }
#ifdef ENABLE_THREADS
    disp_freeSurface(&draw_surf);
#endif /* ENABLE_THREADS */
  }
  else
  {
    disp_freeBuffer(&draw_buf);
#ifdef ENABLE_THREADS
    disp_freeBuffer(&out_buf);
#endif /* ENABLE_THREADS */
  }
  /* Do not free result of SDL_GetWindowSurface() or SDL_SetVideoMode() */
  screen = NULL;
}
