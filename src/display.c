/* ACID WARP (c)Copyright 1992, 1993 by Noah Spurrier
 * All Rights reserved. Private Proprietary Source Code by Noah Spurrier
 * Ported to Linux by Steven Wills
 * Ported to SDL by Boris Gjenero
 */

#include <string.h>
#include "SDL/include/SDL.h"
#include "inc/acidwarp.h"
#include "inc/display.h"

static SDL_Window* window;
static SDL_Renderer* renderer;
static SDL_Texture* screen = NULL;
static SDL_Surface* draw_surface = NULL, * out_surface = NULL;

#ifdef ENABLE_THREADS
static uint8_t* out_buf = NULL;
#else /* !ENABLE_THREADS */
#define draw_surf out_surf
#endif /* !ENABLE_THREADS */
static int disp_DrawingOnSurface;
static uint8_t* draw_buf = NULL;

#ifdef HAVE_FULLSCREEN
static int fullscreen = 0;
static int nativewidth = 0, nativeheight;
#endif
static int winwidth = 0;
static int winheight;

static int scaling = 1;
static int width, height;

SDL_Palette* sdlColors;

void disp_setPalette(unsigned char* palette)
{
	static SDL_Color colors[256];
	for( auto i = 0; i < 256; i++ )
	{
		colors[i].r = palette[i * 3 + 0] << 2;
		colors[i].g = palette[i * 3 + 1] << 2;
		colors[i].b = palette[i * 3 + 2] << 2;
	}

	SDL_SetPaletteColors(out_surface->format->palette, colors, 0, 256);

	if( screen )
		SDL_DestroyTexture(screen);

	screen = SDL_CreateTextureFromSurface(renderer, out_surface);

	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, screen, NULL, NULL);
	SDL_RenderPresent(renderer);
}

static void disp_toSurface()
{
	int row;
	unsigned char* outp, * inp = out_buf;

	if( SDL_MUSTLOCK(out_surface) )
	{
		if( SDL_LockSurface(out_surface) != 0 )
		{
			fatalSDLError("locking surface when ending update");
		}
	}
	outp = out_surface->pixels;

	if( scaling == 1 )
	{
		for( row = 0; row < height; row++ )
		{
			memcpy(outp, inp, width);
			outp += out_surface->pitch;
			inp += width;
		}
	}
	else if( scaling == 2 )
	{
		unsigned char* outp2 = outp + out_surface->pitch;
		int skip = (out_surface->pitch - width) << 1;
		int col;
		unsigned char c;
		for( row = 0; row < height; row++ )
		{
			for( col = 0; col < width; col++ )
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
	if( SDL_MUSTLOCK(out_surface) )
	{
		SDL_UnlockSurface(out_surface);
	}
}

static void disp_toggleFullscreen(void)
{
	if( fullscreen )
	{
		/* If going back to windowed mode, restore window size */
		if( winwidth != 0 )
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

	SDL_ShowCursor(!fullscreen);
}

static void disp_processKey(
	int key, SDL_Keymod keymod)
{
	switch( key )
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
		case SDLK_c:
		case SDLK_PAUSE:
			if( (keymod & KMOD_CTRL) == 0 )
				break;
		case SDLK_q:
			handleinput(CMD_QUIT);
			break;
		case SDLK_k:
			handleinput(CMD_NEWPAL);
			break;
		case SDLK_l:
			handleinput(CMD_LOCK);
			break;
		case SDLK_ESCAPE:
			if( fullscreen )
				disp_toggleFullscreen();
			break;
		case SDLK_RETURN:
			if( keymod & KMOD_ALT )
				disp_toggleFullscreen();
			break;
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

	while( SDL_PollEvent(&event) > 0 )
	{
		switch( event.type )
		{
			/* SDL full screen switching has no useful effect with Emscripten */
			case SDL_MOUSEBUTTONDOWN:
				if( event.button.button == SDL_BUTTON_LEFT )
				{
					// Earlier SDL versions don't report double clicks
					static uint32_t dblclicktm = 0;
					uint32_t clicktime = SDL_GetTicks();
					// Like !SDL_TICKS_PASSED(), which may not be available
					if( (int32_t) (dblclicktm - clicktime) > 0 )
					{
						disp_toggleFullscreen();
					}
					dblclicktm = clicktime + 200;
				}
				break;
			case SDL_KEYDOWN:
				disp_processKey(event.key.keysym.sym, SDL_GetModState());
				break;
			case SDL_WINDOWEVENT:
				if( event.window.event == SDL_WINDOWEVENT_RESIZED )
				{
					if( width != (event.window.data1 / scaling) ||
					   height != (event.window.data2 / scaling) )
					{
						disp_init(event.window.data1 / scaling, event.window.data2 / scaling,
								  fullscreen
						);
					}
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

#ifdef ADDICON
extern unsigned char acidwarp_rgb[];
/* For SDL 1 call before SDL_SetWindowMode().
 * For SDL 2 call after window is created.
 */
static void disp_setIcon(void)
{
	SDL_Surface* iconsurface =
		SDL_CreateRGBSurfaceFrom(acidwarp_rgb, 64, 64, 24, 64 * 3,
								 0x0000ff, 0x00ff00, 0xff0000, 0
								 /* Big endian may need:  0xff0000, 0x00ff00, 0x0000ff, 0 */
		);
	if( iconsurface == NULL )
		fatalSDLError("creating icon surface");

	SDL_SetWindowIcon(window, iconsurface);
	SDL_FreeSurface(iconsurface);
}
#endif /* ADDICON */

static void disp_allocateOffscreen()
{
	draw_abort();

	if( out_surface )
		SDL_FreeSurface(out_surface);

	if( draw_surface )
		SDL_FreeSurface(draw_surface);

	out_surface = SDL_CreateRGBSurface(
		SDL_SWSURFACE, width, height, 8, 0, 0, 0, 0
	);

	if( !disp_DrawingOnSurface )
	{

		if( out_buf )
			free(out_buf);

		if( draw_buf )
			free(draw_buf);

		disp_DrawingOnSurface = 1;
		draw_surface = SDL_CreateRGBSurface(
			SDL_SWSURFACE, width, height, 8, 0, 0, 0, 0
		);
	}

	sdlColors = SDL_AllocPalette(256);

}

void disp_beginUpdate(uint8_t** p, unsigned int* pitch,
					  unsigned int* w, unsigned int* h)
{
	if( disp_DrawingOnSurface )
	{
		if( SDL_MUSTLOCK(draw_surface) )
		{
			if( SDL_LockSurface(draw_surface) != 0 )
			{

			}
		}

		*p = draw_surface->pixels;
		*pitch = draw_surface->pitch;
	}
	else
	{
		*p = draw_buf;
		*pitch = width;
	}

	*w = width;
	*h = height;
}

void disp_finishUpdate()
{
	if( disp_DrawingOnSurface )
	{
		if( SDL_MUSTLOCK(draw_surface) )
		{
			SDL_LockSurface(draw_surface);
		}

		draw_buf = NULL;
	}
}

void disp_swapBuffers()
{
	if( !disp_DrawingOnSurface )
	{
		uint8_t* temp = draw_buf;
		draw_buf = out_buf;
		out_buf = temp;

		disp_toSurface();
	}
	else
	{
		SDL_Surface* temp = draw_surface;
		draw_surface = out_surface;
		out_surface = temp;
	}

	if( screen )
		SDL_DestroyTexture(screen);

	screen = SDL_CreateTextureFromSurface(renderer, out_surface);

	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, screen, NULL, NULL);
	SDL_RenderPresent(renderer);
}

void disp_init(int newwidth, int newheight, int flags)
{
	Uint32 videoflags;
	static int inited = 0;
	static int desktopaspect = 0;

	videoflags = SDL_WINDOW_BORDERLESS | SDL_WINDOW_SHOWN;
	if( flags != 0 )
		videoflags |= SDL_WINDOW_FULLSCREEN;

	if( !inited )
	{
		SDL_DisplayMode dm;
		SDL_GetCurrentDisplayMode(0, &dm);

		nativewidth = dm.w;
		nativeheight = dm.h;

		if( flags == 0 )
		{
			nativeheight /= 2;
			nativewidth /= 2;
		}

		SDL_Init(SDL_INIT_VIDEO);

#ifdef ADDICON
		/* Must be called before SDL_SetVideoMode() */
		disp_setIcon();
#endif

		SDL_ShowCursor(!fullscreen);

		inited = 1;
	} /* !inited */

	width = nativewidth;
	height = nativeheight;

	scaling = 1;

	SDL_CreateWindowAndRenderer(width * scaling, height * scaling, videoflags, &window, &renderer);
	SDL_SetWindowTitle(window, "acidwarp | 'topher remix");
	SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

	SDL_RenderSetLogicalSize(renderer, width, height);

	disp_allocateOffscreen();

	/* This may be unnecessary if switching between windowed
	 * and full screen mode with the same dimensions. */
	handleinput(CMD_RESIZE);
}

void disp_quit(void)
{
	SDL_DestroyTexture(screen);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}
