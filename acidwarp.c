/* ACID WARP (c)Copyright 1992, 1993 by Noah Spurrier
 * All Rights reserved. Private Proprietary Source Code by Noah Spurrier
 * Ported to Linux by Steven Wills
 * Ported to X by Boris Gjenero
 */

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef SVGALIB
#include <vga.h>
#include <vgagl.h>
#include <vgakeyboard.h>
#endif
#include <unistd.h>

#ifdef SDL
#include "SDL.h"
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef BOOL
#define BOOL IGNOREBOOL
#undef FALSE
#undef TRUE
#endif
#endif
 
#include "warp_text.c"
#include "handy.h"
#include "acidwarp.h"
#include "lut.h"
#include "bit_map.h"
#include "palinit.h"
#include "rolnfade.h"

#ifdef LIBXPCE
#include <X11/keysym.h>
#include "libxpce/libxpce.h"
#endif

#ifdef SDL
 
static SDL_Surface *surface;
static SDL_Color sdlPalette[256];
static int fullscreen;
static int scaling = 1;
// Save window size when in full screen
static int winwidth = 0;
static int winheight;

#endif
 
#define NUM_IMAGE_FUNCTIONS 40
#define NOAHS_FACE   0

/* there are WAY too many global things here... */
extern int RedRollDirection, GrnRollDirection, BluRollDirection;
extern UINT FadeCompleteFlag;
int VGAMODE;
int VIRTUAL;
int RES = 0;
int ROTATION_DELAY = 30000;
/* GraphicsContext *physicalscreen; */
int logo_time = 30, image_time = 20;
int XMax = 319, YMax = 199;
UCHAR *buf_graf = NULL;
int GO = TRUE;
int SKIP = FALSE;
int NP = FALSE; /* flag indicates new palette */
int LOCK = FALSE; /* flag indicates don't change to next image */
UCHAR MainPalArray [256 * 3];
UCHAR TargetPalArray [256 * 3];

#ifdef SDL

void setSDLPalette(unsigned char *palette) {
  int i;
  for(i=0;i<256;i++) {
    sdlPalette[i].r = palette[i*3+0] << 2;
    sdlPalette[i].g = palette[i*3+1] << 2;
    sdlPalette[i].b = palette[i*3+2] << 2;
  }
  
  SDL_SetPalette(surface, SDL_PHYSPAL, sdlPalette, 0, 256);
  /* SDL_SetColors(surface, sdlPalette, 0, 256); */
}

void updateSDLSurface(void) { 
  int row;
  unsigned char *outp;
  unsigned char *inp = buf_graf;
  if (SDL_LockSurface(surface) != 0) {
    fprintf(stderr, "Couldn't lock surface\n");
	exit(-1);
  }
  outp = surface->pixels;

  if (scaling == 1) {	  
    for (row = 0; row <= YMax; row++) {	 
      memcpy(outp, inp, XMax+1);
	  outp += surface->pitch;
	  inp += XMax + 1;
    }
  } else if (scaling == 2) {
    unsigned char *outp2 = outp + surface->pitch;
	int skip = (surface->pitch - XMax - 1) * 2;
	int col;
	unsigned char c;
    for (row = 0; row <= YMax; row++) {	 
	  for (col = 0; col <= XMax; col++) { 
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

#if 0
  // To debug palette
  for (row = 0; row < 3; row++) {
    int col;
    outp = surface->pixels + surface->pitch * row;
	for (col = 0; col < 256; col++) {
	  *(outp++) = col;
	}
  }
#endif
  
  SDL_UnlockSurface(surface);

  SDL_Flip(surface);
}

#undef main

#endif

int main (int argc, char *argv[])
{
  int imageFuncList[NUM_IMAGE_FUNCTIONS], userOptionImageFuncNum;
  int paletteTypeNum = 0, userPaletteTypeNumOptionFlag = FALSE;
  int imageFuncListIndex=0, fade_dir = TRUE;
  time_t ltime, mtime;

  RANDOMIZE();
  
  /* Default options */
  userPaletteTypeNumOptionFlag = 0;       /* User Palette option is OFF */
  userOptionImageFuncNum = -1; /* No particular functions goes first. */
  
  commandline(argc, argv);
  
  printf ("\nPlease wait...\n"
	  "\n\n*** Press Control-C to exit the program at any time. ***\n");
  printf ("\n\n%s\n", VERSION);
  
  graphicsinit();

#ifdef SVGALIB
  physicalscreen = gl_allocatecontext();
  gl_getcontext(physicalscreen);
#endif

  initPalArray(MainPalArray, RGBW_LIGHTNING_PAL);
  initPalArray(TargetPalArray, RGBW_LIGHTNING_PAL);
#if defined(SVGALIB)
      gl_setpalettecolors(0, 256, MainPalArray);
#elif defined(LIBXPCE)
      xpce_SetPalette(MainPalArray);
#elif defined(SDL)
      setSDLPalette(MainPalArray);
#endif
  
  if (logo_time != 0) {
    /* show the logo for a while */
    writeBitmapImageToArray(buf_graf, NOAHS_FACE, XMax, YMax);
#if defined(SVGALIB)
    gl_putbox(1,1,XMax,YMax,buf_graf);
#elif defined(LIBXPCE)
    xpce_DrawImage();
#elif defined(SDL)
	updateSDLSurface();
#endif
    ltime=time(NULL);
    mtime=ltime + logo_time;
    for(;;) {
      processinput();
      if(GO)
	rollMainPalArrayAndLoadDACRegs(MainPalArray);
      if(SKIP)
	break;
      if(ltime>mtime) 
	break; 
      ltime=time(NULL);
      usleep(ROTATION_DELAY);
    }
    while(!FadeCompleteFlag) {
      processinput();
      if(GO)
        rolNFadeBlkMainPalArrayNLoadDAC(MainPalArray);
      if(SKIP)
	break;
      usleep(ROTATION_DELAY);
    }
    FadeCompleteFlag=!FadeCompleteFlag;
  } 
  
  SKIP = FALSE;
  makeShuffledList(imageFuncList, NUM_IMAGE_FUNCTIONS);
  
  for(;;) {
    /* move to the next image */
    if (++imageFuncListIndex >= NUM_IMAGE_FUNCTIONS)
      {
	imageFuncListIndex = 0;
	makeShuffledList(imageFuncList, NUM_IMAGE_FUNCTIONS);
      }
    
    /* install a new image */
    generate_image(
		   (userOptionImageFuncNum < 0) ? 
		   imageFuncList[imageFuncListIndex] : 
		   userOptionImageFuncNum, 
		   buf_graf, XMax/2, YMax/2, XMax, YMax, 
#ifdef LIBXPCE
		   xpce_GetNumColors()-1
#else
		   255
#endif
		   );

#if defined(SVGALIB)
    gl_putbox(1,1,XMax,YMax,buf_graf);
#elif defined(LIBXPCE)
    xpce_DrawImage();
#elif defined(SDL)
	updateSDLSurface();
#endif

    /* create new palette */
    paletteTypeNum = RANDOM(NUM_PALETTE_TYPES +1);
    initPalArray(TargetPalArray, paletteTypeNum);
    
    /* this is the fade in */
    while(!FadeCompleteFlag) {
      processinput();
      if(GO)
	rolNFadeMainPalAryToTargNLodDAC(MainPalArray,TargetPalArray);
      if(SKIP)
	break;
      usleep(ROTATION_DELAY);
#if defined(SVGALIB)
      gl_setpalettecolors(0, 256, MainPalArray);
#elif defined(LIBXPCE)
      xpce_SetPalette(MainPalArray);
#elif defined(SDL)
      setSDLPalette(MainPalArray);
#endif
    }
    
    FadeCompleteFlag=!FadeCompleteFlag;
    ltime = time(NULL);
    mtime = ltime + image_time;
    
    /* rotate the palette for a while */
    for(;;) {
      processinput();
      if(GO)
	rollMainPalArrayAndLoadDACRegs(MainPalArray);
      if(SKIP)
	break;
      if(NP) {
	newpal();
	NP = FALSE;
      }
      ltime=time(NULL);
      if((ltime>mtime) && !LOCK)
	break;
      usleep(ROTATION_DELAY);
    }
    
    /* fade out */
    while(!FadeCompleteFlag) {
      if(SKIP) 
	break;
      processinput();
      if(GO) {
	if (fade_dir)
	  rolNFadeBlkMainPalArrayNLoadDAC(MainPalArray);
	else
	  rolNFadeWhtMainPalArrayNLoadDAC(MainPalArray);
      }
      usleep(ROTATION_DELAY);
    }
    FadeCompleteFlag=!FadeCompleteFlag;
    SKIP = FALSE;
  }
  /* exit */
  printStrArray(Command_summary_string);
  printf("%s\n", VERSION);

  return 0;
}

/* ------------------------END MAIN----------------------------------------- */

void newpal()
{
  int paletteTypeNum;
  
  paletteTypeNum = RANDOM(NUM_PALETTE_TYPES +1);
  initPalArray(MainPalArray, paletteTypeNum);

#if defined(SVGALIB)
  gl_setpalettecolors(0, 256, MainPalArray);  /* FIXME really target?? */
#elif defined(LIBXPCE)
  xpce_SetPalette(MainPalArray);
#elif defined(SDL)
  setSDLPalette(MainPalArray);
#endif
}

#if 0
int checkinput()
{
  keyboard_update();
  if(keyboard_keypressed(SCANCODE_P)) {
    while(keyboard_keypressed(SCANCODE_P))
      keyboard_update();
    return 1;
  }
  if(keyboard_keypressed(SCANCODE_N)) {
    while(keyboard_keypressed(SCANCODE_N))
      keyboard_update();
    return 2;
  }
  if(keyboard_keypressed(SCANCODE_Q)) {
    while(keyboard_keypressed(SCANCODE_Q))
      keyboard_update();
    return 3;
  }
  if(keyboard_keypressed(SCANCODE_K)) {
    while(keyboard_keypressed(SCANCODE_K))
      keyboard_update();
    return 4;
  }
  if(keyboard_keypressed(SCANCODE_L)) {
    while(keyboard_keypressed(SCANCODE_L))
      keyboard_update();
    return 5;
  }
  if(keyboard_keypressed(SCANCODE_CURSORBLOCKUP)) {
    while(keyboard_keypressed(SCANCODE_CURSORBLOCKUP))
      keyboard_update();
    return 6;
  }
  if(keyboard_keypressed(SCANCODE_CURSORBLOCKDOWN)) {
    while(keyboard_keypressed(SCANCODE_CURSORBLOCKDOWN))
      keyboard_update();
    return 7;
  }
  /* default case */
  return 0;
}
#endif

#ifdef LIBXPCE
void processinput() {
  xpce_HandleEvents();
}  
#endif

void handleinput(int key)
{
  switch(key)
    {
    case 1:
      if(GO)
	GO = FALSE;
      else
	GO = TRUE;
      break;
    case 2:
      SKIP = TRUE;
      break;
    case 3:
#if defined(LIBXPCE)
      xpce_QuitNotify();
      xpce_CloseGraph();
#elif defined(SDL)
	  SDL_Quit();
#endif
      exit(0);
      break;
    case 4:
      NP = TRUE;
      break;
    case 5:
      if(LOCK)
	LOCK = FALSE;
      else
	LOCK = TRUE;
      break;
    case 6:
      ROTATION_DELAY = ROTATION_DELAY - 5000;
      if (ROTATION_DELAY < 0)
	ROTATION_DELAY = 0;
      break;
    case 7:
      ROTATION_DELAY = ROTATION_DELAY + 5000;
      break;
    }
}

#if defined(LIBXPCE) || defined(SDL)
void xpceC_HandleInputChar(int c) {
  int r;
  
  switch (c) {
  case 'p':
  case 'P':        r = 1;     break;
  case 'n':
  case 'N':        r = 2;     break;
  case 'q':
  case 'Q':        r = 3;     break;
      
  case 'k':
  case 'K':        r = 4;     break;
  case 'l':
  case 'L':        r = 5;     break;
  default:
    return;
  }
  handleinput(r);
}
#endif

#if 0
void xpceC_HandleInputKeySym(int k) {
  int r;

  switch (k) {
  case XK_Up:       r = 6;     break;
  case XK_Down:     r = 7;     break;
  default:
    return;
  }
  handleinput(r);
}
#endif

void processinput() {
  SDL_Event event;
 
  char keyHit = 0;
  
  while ( SDL_PollEvent(&event) > 0 ) {
    switch (event.type) {
      case SDL_MOUSEBUTTONDOWN:
		fullscreen = !fullscreen;
        graphicsinit();
		break;
      case SDL_KEYDOWN:
        ///* Ignore key releases */
        //if ( event.key.state == SDL_RELEASED ) {
        //  break;
        //}
        /* Ignore ALT-TAB for windows */
        if ( (event.key.keysym.sym == SDLK_LALT) ||
             (event.key.keysym.sym == SDLK_TAB) ) {
          break;
        } else if (event.key.keysym.sym == SDLK_UP) {
          handleinput(6);
		} else if (event.key.keysym.sym == SDLK_DOWN) {
		  handleinput(7);
		} else if (event.key.keysym.unicode <= 255) {
          keyHit = event.key.keysym.unicode;
		}
        break;
      case SDL_VIDEORESIZE:
		XMax = event.resize.w - 1;
		YMax = event.resize.h - 1;
		if (buf_graf != 0) {
		  free(buf_graf);
		  buf_graf = 0;
	    }
		SKIP = TRUE;
		graphicsinit();
		break;
      case SDL_QUIT:
		  //abort();
		handleinput(3);
		break;
      default:
        break;
    }
  }
  
  if (keyHit != 0) xpceC_HandleInputChar(keyHit);
}

#ifdef LIBXPCE
void xpceC_QuitRequest() {
  handleinput(3);
}

void xpceC_HandleResize(unsigned int xsize, unsigned int ysize) {
  XMax = xsize - 1;
  YMax = ysize - 1;
  SKIP = TRUE;
}
#endif


void commandline(int argc, char *argv[])
{
  int argNum;

  /* Parse the command line */
  if (argc >= 2) {
    for (argNum = 1; argNum < argc; ++argNum) {
      if (!strcmp("-w",argv[argNum])) {
        printStrArray(The_warper_string);
        exit (0);
      } 
      else
      if (!strcmp("-h",argv[argNum])) {
        printStrArray(Help_string);
#ifdef LIBXPCE	
	xpce_CLHelp();
#endif
        printf("\n%s\n", VERSION);
        exit (0);
      }
      else
      if(!strcmp("-n",argv[argNum])) {
        logo_time = 0;
      }
#if 0
      else
      if(!strcmp("-r",argv[argNum])) {
        XMax = -1;
	YMax = -1;
      }
#endif
      else
      if(!strcmp("-d",argv[argNum])) {
        if((argc-1) > argNum) {
          argNum++;
          image_time = atoi(argv[argNum]);
        }
      }
      else
      if(!strcmp("-s", argv[argNum])) {
        if((argc-1) > argNum) {
          argNum++;
          ROTATION_DELAY = atoi(argv[argNum]);
        }
      }
      else
#ifdef LIBXPCE
	if (!xpce_ProcessOption(argc, argv, &argNum))
#endif
	  {
	    fprintf(stderr, "Unknown option \"%s\"\n", argv[argNum]);
	    exit(-1);
	  }
    }
  }
}  

void graphicsinit()
{
  /* setup the screen */
  /*
  switch (RES)
    {
    case 0:
      XMax = 319;
      YMax = 199;
      buf_graf = alloca((XMax+1)*(YMax+1));
      memset(buf_graf, 0x00, (size_t)(320*200));
      break;
    case 1:
      XMax = 639;
      YMax = 479;
      buf_graf = alloca((XMax+1)*(YMax+1));
      memset(buf_graf, 0x00, (size_t)(640*480));
      break;
    case 2:
      XMax = 799;
      YMax = 599;
      buf_graf = alloca((XMax+1)*(YMax+1));
      memset(buf_graf,0x00, (size_t)(800*600));
      break;
    case 3:
      XMax = 1023;
      YMax = 767;
      buf_graf = alloca((XMax+1)*(YMax+1));
      memset(buf_graf,0x00, (size_t)(1024*768));
      break;
    }
  */
#ifdef SDL
  Uint32 videoflags = SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_HWPALETTE |
                      SDL_RESIZABLE | (fullscreen?SDL_FULLSCREEN:0);

  static int inited = 0;
#ifdef WIN32  
  static int palettehack = 0;
  OSVERSIONINFO winver;
  const char *svd = "SDL_VIDEODRIVER";

  if (inited && palettehack ) {
    SDL_Quit();
    inited = 0;
  }
#endif
  
  if (!inited) {
#ifdef WIN32
    if (!palettehack) {
      winver.dwOSVersionInfoSize = sizeof(winver);
      if (GetVersionEx(&winver) && 
          winver.dwPlatformId == 2 && winver.dwMajorVersion == 5 &&
          (winver.dwMinorVersion == 0 || winver.dwMinorVersion == 1)) {
        // Windows XP or 2000 can give us all 256 colours
        palettehack = 0;
      } else if (GetEnvironmentVariable(svd, NULL, 0) > 0) {
        // SDL_VIDEODRIVER already set
        palettehack = 0;
      } else {
        palettehack = 1;
      }
    }

    if (palettehack) {
      if (fullscreen) {
        SetEnvironmentVariable(svd, "directx");
      } else {
        SetEnvironmentVariable(svd, "windib");
      }
    }
#endif

    /* Initialize SDL */
    if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
      char str[1000];
      printf(str, "Could not initialize SDL library: %s\n",SDL_GetError());
      fprintf(stderr,str);
      exit(-1);
    }

    SDL_WM_SetCaption("Acidwarp","acidwarp");

    SDL_EnableUNICODE(1);

    /* XMax = 319;
    YMax = 199;	 */
  }
  
  SDL_ShowCursor(!fullscreen);

  // If going back to windowed mode, restore window size
  if (!fullscreen) {
    scaling = 1;
    if (winwidth != 0) {
	  XMax = winwidth;
	  YMax = winheight;
	  SKIP = TRUE;
	  if (buf_graf != NULL) {
	    free(buf_graf);
        buf_graf = NULL;
	  }
      winwidth = 0;	  
    }
  } else {
    /* Get available fullscreen modes */
    SDL_Rect **modes = SDL_ListModes(0, videoflags);

    if(modes == (SDL_Rect **)-1){
      /* All resolutions ok */
      scaling = 1;
    } else {
      // Full screen should really fill the whole screen
      // Find video mode with closest number of pixels
      int newwidth = 0;
	  int newheight = 0;
	  int curdiff;
	  int bestdiff = -1;
	  int curpix = (XMax+1) * (YMax+1);
	  int i, j;
      for(i=0;modes[i];i++) {
	    for (j=1;j<=2; j++) { // try out scaling
	      curdiff = modes[i]->w * modes[i]->h / (j * j) - curpix;
	      if ((curdiff) < 0)
	        curdiff = -curdiff;
          if (bestdiff == -1 || curdiff < bestdiff || 
	          (curdiff == bestdiff && j < scaling)) {
            scaling = j;
   	        newwidth = modes[i]->w / j;
	        newheight = modes[i]->h / j;
	        bestdiff = curdiff;
		  }	  
	    }
	  }
	  
	  if (newwidth != 0 && (newwidth != XMax+1 || newheight != YMax+1)) {
	    winwidth = XMax;
	    winheight = YMax;
		XMax = newwidth - 1;
		YMax = newheight - 1;
 	    SKIP = TRUE;
	    if (buf_graf != NULL) {
	      free(buf_graf);
          buf_graf = NULL;
	    }
      }
	}
  }

  surface = SDL_SetVideoMode((XMax+1)*scaling, (YMax+1)*scaling, 8, videoflags);
  if (!surface)	{
    fprintf(stderr, "setting video mode");
    exit(-1);
  }
#ifdef WIN32
  if (palettehack)
    SetEnvironmentVariable(svd, NULL);
#endif  

  // SDL_SetColors(surface, sdlPalette, 0, 256);
  if (buf_graf == NULL) {
    buf_graf = malloc ((XMax + 1) * (YMax + 1));
    // Clearing is only needed for the initial logo
    if (!inited)
      memset(buf_graf, 0, (XMax + 1) * (YMax + 1));
  }

  inited = 1;  
  
#endif

#ifdef LIBXPCE
   fprintf(stderr, "DISPLAY=\"%s\"\n", getenv("DISPLAY"));
   xpce_InitGraph();

   XMax = xpce_GetWidth()-1;
   YMax = xpce_GetHeight()-1;
#endif

#ifdef SVGALIB
   /* Is this if here ok? */
   if (XMax == 0 || YMax == 0) {
     XMax = 319;
     YMax = 199;
   }
   vga_init();
#endif

#if 0
  switch (RES)
    {
    case 0:
      VGAMODE=G320x200x256;
      break;
    case 1:
      VGAMODE=G640x480x256;
      break;
    case 2:
      VGAMODE=G800x600x256;
      break;
    case 3:
      VGAMODE=G1024x768x256;
      break;
    }
#endif
  VIRTUAL=0;

#ifdef SVGALIB
  vga_setmode(VGAMODE);
  
  if (keyboard_init()) {
    printf("Could not initialize keyboard.\n");
    exit(1);
  }
  gl_setcontextvga(VGAMODE);  /* Physical screen context. */
#endif

}

void printStrArray(char *strArray[])
{
  /* Prints an array of strings.  The array is terminated with a null string.     */
  char **strPtr;
  
  for (strPtr = strArray; **strPtr; ++strPtr)
    printf ("%s", *strPtr);
}

#if 0
void setNewVideoMode (void)
{
  vga_setmode(G320x200x256);
}
#endif

void restoreOldVideoMode (void)
{
}

#if 0
void writePixel(int x, int y, int color)
{
  int temp;
  
  temp = vga_getcolors();
  vga_setcolor(color);
  vga_drawpixel(x,y);
  vga_setcolor(temp);
}
#endif

void makeShuffledList(int *list, int listSize)
{
  int entryNum, r;
  
  for (entryNum = 0; entryNum < listSize; ++entryNum)
    list[entryNum] = -1;
  
  for (entryNum = 0; entryNum < listSize; ++entryNum)
    {
      do
	r = RANDOM(listSize);
      while (list[r] != -1);
      
      list[r] = entryNum;
    }
}

int generate_image(int imageFuncNum, UCHAR *buf_graf, int xcenter, int ycenter, int xmax, int ymax, int colormax)
{
  
  /* WARNING!!! Major change from long to int.*/
  /* ### Changed back to long. Gives lots of warnings. Will fix soon. */
  
  long /* int */ x, y, dx, dy, dist, angle, xsize = xmax + 1;
  long color;
  
  /* Some general purpose random angles and offsets. Not all functions use them. */
  long a1, a2, a3, a4, x1,x2,x3,x4,y1,y2,y3,y4;
  
  x1 = RANDOM(40)-20;  x2 = RANDOM(40)-20;  x3 = RANDOM(40)-20;  x4 = RANDOM(40)-20;
  y1 = RANDOM(40)-20;  y2 = RANDOM(40)-20;  y3 = RANDOM(40)-20;  y4 = RANDOM(40)-20;
  
  a1 = RANDOM(ANGLE_UNIT);  a2 = RANDOM(ANGLE_UNIT);  a3 = RANDOM(ANGLE_UNIT);  a4 = RANDOM(ANGLE_UNIT);
  for (y = 0; y <= ymax; ++y)
    {
      
      for (x = 0; x <= xmax; ++x)
	{
	  dx = x - xcenter;
	  dy = y - ycenter;
	  
	  dist  = lut_dist (dx, dy);
	  angle = lut_angle (dx, dy);
	  
	  /* select function. Could be made a separate function, but since 
             this function is evaluated over a large iteration of values I am 
             afraid that it might slow things down even more to have a 
             separate function.	*/
	  
	  switch (imageFuncNum)
	    {
	      /* case -1:	Eight Arm Star -- produces weird discontinuity
                color = dist+ lut_sin(angle * (200 - dist)) / 32;
						break;
						*/
	    case 0: /* Rays plus 2D Waves */
	      color = angle + lut_sin (dist * 10) / 64 +
		lut_cos (x * ANGLE_UNIT / xmax * 2) / 32 +
		lut_cos (y * ANGLE_UNIT / ymax * 2) / 32;
	      break;
	      
	    case 1:	/* Rays plus 2D Waves */
	      color = angle + lut_sin (dist * 10) / 16 +
		lut_cos (x * ANGLE_UNIT / xmax * 2) / 8 +
		lut_cos (y * ANGLE_UNIT / ymax * 2) / 8;
	      break;
	      
	    case 2:
	      color = lut_sin (lut_dist(dx + x1, dy + y1) *  4) / 32 +
		lut_sin (lut_dist(dx + x2, dy + y2) *  8) / 32 +
		lut_sin (lut_dist(dx + x3, dy + y3) * 16) / 32 +
		lut_sin (lut_dist(dx + x4, dy + y4) * 32) / 32;
	      break;
	      
	    case 3:	/* Peacock */
	      color = angle + lut_sin (lut_dist(dx + 20, dy) * 10) / 32 +
		angle + lut_sin (lut_dist(dx - 20, dy) * 10) / 32;
	      break;
	      
	    case 4:
	      color = lut_sin (dist) / 16;
	      break;
	      
	    case 5:	/* 2D Wave + Spiral */
	      color = lut_cos (x * ANGLE_UNIT / xmax) / 8 +
		lut_cos (y * ANGLE_UNIT / ymax) / 8 +
		angle + lut_sin(dist) / 32;
	      break;
	      
	    case 6:	/* Peacock, three centers */
	      color = lut_sin (lut_dist(dx,      dy - 20) * 4) / 32+
		lut_sin (lut_dist(dx + 20, dy + 20) * 4) / 32+
		lut_sin (lut_dist(dx - 20, dy + 20) * 4) / 32;
	      break;
	      
	    case 7:	/* Peacock, three centers */
	      color = angle +
		lut_sin (lut_dist(dx,      dy - 20) * 8) / 32+
		lut_sin (lut_dist(dx + 20, dy + 20) * 8) / 32+
		lut_sin (lut_dist(dx - 20, dy + 20) * 8) / 32;
	      break;
	      
	    case 8:	/* Peacock, three centers */
	      color = lut_sin (lut_dist(dx,      dy - 20) * 12) / 32+
		lut_sin (lut_dist(dx + 20, dy + 20) * 12) / 32+
		lut_sin (lut_dist(dx - 20, dy + 20) * 12) / 32;
	      break;
	      
	    case 9:	/* Five Arm Star */
	      color = dist + lut_sin (5 * angle) / 64;
	      break;
	      
	    case 10:	/* 2D Wave */
	      color = lut_cos (x * ANGLE_UNIT / xmax * 2) / 4 +
		lut_cos (y * ANGLE_UNIT / ymax * 2) / 4;
	      break;
	      
	    case 11:	/* 2D Wave */
	      color = lut_cos (x * ANGLE_UNIT / xmax) / 8 +
		lut_cos (y * ANGLE_UNIT / ymax) / 8;
	      break;
	      
	    case 12:	/* Simple Concentric Rings */
	      color = dist;
	      break;
	      
	    case 13:	/* Simple Rays */
	      color = angle;
	      break;
	      
	    case 14:	/* Toothed Spiral Sharp */
	      color = angle + lut_sin(dist * 8)/32;
	      break;
	      
	    case 15:	/* Rings with sine */
	      color = lut_sin(dist * 4)/32;
	      break;
	      
	    case 16:	/* Rings with sine with sliding inner Rings */
	      color = dist+ lut_sin(dist * 4) / 32;
	      break;
	      
	    case 17:
	      color = lut_sin(lut_cos(2 * x * ANGLE_UNIT / xmax)) / (20 + dist)
                + lut_sin(lut_cos(2 * y * ANGLE_UNIT / ymax)) / (20 + dist);
	      break;
	      
	    case 18:	/* 2D Wave */
	      color = lut_cos(7 * x * ANGLE_UNIT / xmax)/(20 + dist) +
		lut_cos(7 * y * ANGLE_UNIT / ymax)/(20 + dist);
	      break;
	      
	    case 19:	/* 2D Wave */
	      color = lut_cos(17 * x * ANGLE_UNIT/xmax)/(20 + dist) +
		lut_cos(17 * y * ANGLE_UNIT/ymax)/(20 + dist);
	      break;
	      
	    case 20:	/* 2D Wave Interference */
	      color = lut_cos(17 * x * ANGLE_UNIT / xmax) / 32 +
		lut_cos(17 * y * ANGLE_UNIT / ymax) / 32 + dist + angle;
	      break;
	      
	    case 21:	/* 2D Wave Interference */
	      color = lut_cos(7 * x * ANGLE_UNIT / xmax) / 32 +
		lut_cos(7 * y * ANGLE_UNIT / ymax) / 32 + dist;
	      break;
	      
	    case 22:	/* 2D Wave Interference */
	      color = lut_cos( 7 * x * ANGLE_UNIT / xmax) / 32 +
		lut_cos( 7 * y * ANGLE_UNIT / ymax) / 32 +
		lut_cos(11 * x * ANGLE_UNIT / xmax) / 32 +
		lut_cos(11 * y * ANGLE_UNIT / ymax) / 32;
	      break;
	      
	    case 23:
	      color = lut_sin (angle * 7) / 32;
	      break;
	      
	    case 24:
	      color = lut_sin (lut_dist(dx + x1, dy + y1) * 2) / 12 +
		lut_sin (lut_dist(dx + x2, dy + y2) * 4) / 12 +
		lut_sin (lut_dist(dx + x3, dy + y3) * 6) / 12 +
		lut_sin (lut_dist(dx + x4, dy + y4) * 8) / 12;
	      break;
	      
	    case 25:
	      color = angle + lut_sin (lut_dist(dx + x1, dy + y1) * 2) / 16 +
		angle + lut_sin (lut_dist(dx + x2, dy + y2) * 4) / 16 +
		lut_sin (lut_dist(dx + x3, dy + y3) * 6) /  8 +
		lut_sin (lut_dist(dx + x4, dy + y4) * 8) /  8;
	      break;
	      
	    case 26:
	      color = angle + lut_sin (lut_dist(dx + x1, dy + y1) * 2) / 12 +
		angle + lut_sin (lut_dist(dx + x2, dy + y2) * 4) / 12 +
		angle + lut_sin (lut_dist(dx + x3, dy + y3) * 6) / 12 +
		angle + lut_sin (lut_dist(dx + x4, dy + y4) * 8) / 12;
	      break;
	      
	    case 27:
	      color = lut_sin (lut_dist(dx + x1, dy + y1) * 2) / 32 +
		lut_sin (lut_dist(dx + x2, dy + y2) * 4) / 32 +
		lut_sin (lut_dist(dx + x3, dy + y3) * 6) / 32 +
		lut_sin (lut_dist(dx + x4, dy + y4) * 8) / 32;
	      break;

	    case 28:	/* Random Curtain of Rain (in strong wind) */
	      if (y == 0 || x == 0)
		color = RANDOM (16);
	      else
		color = (  *(buf_graf + (xsize *  y   ) + (x-1))
			   + *(buf_graf + (xsize * (y-1)) +    x)) / 2
                  + RANDOM (16) - 8;
	      break;
	      
	    case 29:
	      if (y == 0 || x == 0)
		color = RANDOM (1024);
	      else
		color = dist/6 + (*(buf_graf + (xsize * y    ) + (x-1))
				  +  *(buf_graf + (xsize * (y-1)) +    x)) / 2
		+ RANDOM (16) - 8;
	      break;
	      
	    case 30:
	      color = lut_sin (lut_dist(dx,     dy - 20) * 4) / 32 ^
		lut_sin (lut_dist(dx + 20,dy + 20) * 4) / 32 ^
		lut_sin (lut_dist(dx - 20,dy + 20) * 4) / 32;
	      break;
	      
	    case 31:
	      color = (angle % (ANGLE_UNIT/4)) ^ dist;
	      break;
	      
	    case 32:
	      color = dy ^ dx;
	      break;
	      
	    case 33:	/* Variation on Rain */
	      if (y == 0 || x == 0)
		color = RANDOM (16);
	      else
		color = (  *(buf_graf + (xsize *  y   ) + (x-1))
			   + *(buf_graf + (xsize * (y-1)) +  x   )  ) / 2;
	      
	      color += RANDOM (2) - 1;
	      
	      if (color < 64)
		color += RANDOM (16) - 8;
	      else
						color = color;
	      break; 
	      
	    case 34:	/* Variation on Rain */
	      if (y == 0 || x == 0)
		color = RANDOM (16);
	      else
		color = (  *(buf_graf + (xsize *  y   ) + (x-1))
			   + *(buf_graf + (xsize * (y-1)) +  x   )  ) / 2;
	      
	      if (color < 100)
		color += RANDOM (16) - 8;
	      break;
	      
	    case 35:
	      color = angle + lut_sin(dist * 8)/32;
	      dx = x - xcenter;
	      dy = (y - ycenter)*2;
	      dist  = lut_dist (dx, dy);
          angle = lut_angle (dx, dy);
          color = (color + angle + lut_sin(dist * 8)/32) / 2;
	  break;
	  
	    case 36:
	      color = angle + lut_sin (dist * 10) / 16 +
		lut_cos (x * ANGLE_UNIT / xmax * 2) / 8 +
		lut_cos (y * ANGLE_UNIT / ymax * 2) / 8;
	      dx = x - xcenter;
	      dy = (y - ycenter)*2;
	      dist  = lut_dist (dx, dy);
	      angle = lut_angle (dx, dy);
	      color = (color + angle + lut_sin(dist * 8)/32) / 2;
	      break;
	      
	    case 37:
	      color = angle + lut_sin (dist * 10) / 16 +
		lut_cos (x * ANGLE_UNIT / xmax * 2) / 8 +
		lut_cos (y * ANGLE_UNIT / ymax * 2) / 8;
	      dx = x - xcenter;
	      dy = (y - ycenter)*2;
	      dist  = lut_dist (dx, dy);
          angle = lut_angle (dx, dy);
          color = (color + angle + lut_sin (dist * 10) / 16 +
		   lut_cos (x * ANGLE_UNIT / xmax * 2) / 8 +
		   lut_cos (y * ANGLE_UNIT / ymax * 2) / 8)  /  2;
	  break;
	  
	    case 38:
	      if (dy%2)
		{
		  dy *= 2;
		  dist  = lut_dist (dx, dy);
		  angle = lut_angle (dx, dy);
		}
	      color = angle + lut_sin(dist * 8)/32;
	      break;
	      
	    case 39:
	      color = (angle % (ANGLE_UNIT/4)) ^ dist;
	      dx = x - xcenter;
	      dy = (y - ycenter)*2;
	      dist = lut_dist (dx, dy);
	      angle = lut_angle (dx, dy);
	      color = (color +  ((angle % (ANGLE_UNIT/4)) ^ dist)) / 2;
	      break;
	      
	    case 40:
	      color = dy ^ dx;
	      dx = x - xcenter;
	      dy = (y - ycenter)*2;
	      color = (color +  (dy ^ dx)) / 2;
	      break;

	    default:
	      color = RANDOM (colormax - 1) + 1;
	      break;
	    }
	  
	  /* Fit color value into the palette range using modulo.  It seems 
             that the Turbo-C MOD function does not behave the way I expect.
             It gives negative values for the MOD of a negative number.
             I expect MOD to function as it does on my HP-28S.
	   */

	  color = color % (colormax-1);
	  
	  if (color < 0)
	    color += (colormax - 1);
	  
	  ++color;
          /* color 0 is never used, so all colors are from 1 through 255 */
	  
	  *(buf_graf + (xsize * y) + x) = (UCHAR)color;
          /* Store the color in the buffer */
	}
      /* end for (y = 0; y < ymax; ++y)	*/
    }
  /* end for (x = 0; x < xmax; ++x)	*/
  
#if 0	/* For diagnosis, put palette display line at top of new image	*/
  for (x = 0; x < xmax; ++x)
    {
      color = (x <= 255) ? x : 0;
      
      for (y = 0; y < 3; ++y)
	*(buf_graf + (xsize * y) + x) = (UCHAR)color;
    }
#endif
  
  return (0);
}
