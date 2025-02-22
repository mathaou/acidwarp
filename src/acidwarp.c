/**
 * @file acidwarp.c
 */

#include "inc/acidwarp.h"
#include "warp_text.c" /**<Very gross practice generally, but its a really big constant that would clutter anything else so its fine?*/

 /*
  * GLOBAL VARIABLES
  */
int ROTATION_DELAY = 30000;
int show_logo = 1;
int image_time = 20;
int disp_flags = 0;
int draw_flags = DRAW_FLOAT | DRAW_SCALED;
int GO = TRUE;
int SKIP = FALSE;
int NP = FALSE;   /**<flag indicates new palette */
int LOCK = FALSE; /**<flag indicates don't change to next image */
int RESIZE = FALSE;

static struct timer_data timer_data = { NULL, NULL, 0, SDL_FALSE };
static enum display_state state = STATE_INITIAL;

unsigned int buf_graf_stride = 0;
uint8_t* buf_graf = NULL;

/* Intent is to set size to the browser window. This must not accidentally
 * match the current window size, or else resizing would break.
 */
int width = 320, height = 200;

void timer_lock(void)
{
	if( SDL_LockMutex(timer_data.mutex) != 0 )
	{
		fatalSDLError("locking timer mutex");
	}
}

void timer_unlock(void)
{
	if( SDL_UnlockMutex(timer_data.mutex) != 0 )
	{
		fatalSDLError("unlocking timer mutex");
	}
}

uint32_t timer_proc(uint32_t interval, void* param)
{
	unsigned int tmint = TIMER_INTERVAL;
	timer_lock();
	timer_data.flag = SDL_TRUE;
	SDL_CondSignal(timer_data.cond);
	timer_unlock();
	return tmint ? tmint : 1;
}

void timer_init(void)
{
	timer_data.mutex = SDL_CreateMutex();
	if( timer_data.mutex == NULL )
	{
		fatalSDLError("creating timer mutex");
	}
	timer_data.cond = SDL_CreateCond();
	if( timer_data.cond == NULL )
	{
		fatalSDLError("creating timer condition variable");
	}
	timer_data.timer_id = SDL_AddTimer(TIMER_INTERVAL, timer_proc,
									   timer_data.cond);
	if( timer_data.timer_id == 0 )
	{
		fatalSDLError("adding timer");
	}
}

void timer_quit(void)
{
	if( timer_data.timer_id != 0 )
	{
		SDL_RemoveTimer(timer_data.timer_id);
		timer_data.timer_id = 0;
	}
	if( timer_data.cond != NULL )
	{
		SDL_DestroyCond(timer_data.cond);
		timer_data.cond = 0;
	}
	if( timer_data.mutex != NULL )
	{
		SDL_DestroyMutex(timer_data.mutex);
		timer_data.mutex = NULL;
	}
}

void timer_wait(void)
{
	timer_lock();
	while( !timer_data.flag )
	{
		if( SDL_CondWait(timer_data.cond, timer_data.mutex) != 0 )
		{
			fatalSDLError("waiting on condition");
		}
	}
	timer_data.flag = SDL_FALSE;
	timer_unlock();
}

void handleinput(enum acidwarp_command cmd)
{
	switch( cmd )
	{
		case CMD_PAUSE:
			if( GO )
				GO = FALSE;
			else
				GO = TRUE;
			break;
		case CMD_SKIP:
			SKIP = TRUE;
			break;
		case CMD_QUIT:
			quit(0);
			break;
		case CMD_NEWPAL:
			NP = TRUE;
			break;
		case CMD_LOCK:
			if( LOCK )
				LOCK = FALSE;
			else
				LOCK = TRUE;
			break;
		case CMD_PAL_FASTER:
			ROTATION_DELAY = ROTATION_DELAY - 5000;
			if( ROTATION_DELAY < 0 )
				ROTATION_DELAY = 0;
			break;
		case CMD_PAL_SLOWER:
			ROTATION_DELAY = ROTATION_DELAY + 5000;
			break;
		case CMD_RESIZE:
			RESIZE = TRUE;
			break;
	}
}

void commandline(int argc, char* argv[])
{
	int argNum;

	/* Parse the command line */
	if( argc >= 2 )
	{
		for( argNum = 1; argNum < argc; ++argNum )
		{
			if( !strcmp("-w", argv[argNum]) )
			{
				printStrArray(The_warper_string);
				exit(0);
			}
			else if( !strcmp("-h", argv[argNum]) )
			{
				printStrArray(Help_string);
				printf("\n%s\n", VERSION);
				exit(0);
			}
			else if( !strcmp("-n", argv[argNum]) )
			{
				show_logo = 0;
			}
			else if( !strcmp("-f", argv[argNum]) )
			{
				disp_flags |= DISP_FULLSCREEN;
			}
			else if( !strcmp("-k", argv[argNum]) )
			{
				disp_flags |= DISP_DESKTOP_RES_FS;
			}
			else if( !strcmp("-o", argv[argNum]) )
			{
				draw_flags &= ~DRAW_FLOAT;
			}
			else if( !strcmp("-u", argv[argNum]) )
			{
				draw_flags &= ~DRAW_SCALED;
			}
			else if( !strcmp("-c", argv[argNum]) )
			{
				draw_flags ^= 1UL << 4;
			}
			else if( !strcmp("-d", argv[argNum]) )
			{
				if( (argc - 1) > argNum )
				{
					argNum++;
					image_time = atoi(argv[argNum]);
				}
			}
			else if( !strcmp("-s", argv[argNum]) )
			{
				if( (argc - 1) > argNum )
				{
					argNum++;
					ROTATION_DELAY = atoi(argv[argNum]);
				}
			}
			else
			{
				fprintf(stderr, "Unknown option \"%s\"\n", argv[argNum]);
				exit(-1);
			}
		}
	}
}

void printStrArray(char* strArray[])
{
	/* Prints an array of strings.  The array is terminated with a null string.     */
	char** strPtr;

	for( strPtr = strArray; **strPtr; ++strPtr )
		printf("%s", *strPtr);
}

void quit(int retcode)
{
	timer_quit();
	draw_quit();
	disp_quit();
	SDL_Quit();
	exit(retcode);
}

void fatalSDLError(const char* msg)
{
	fprintf(stderr, "SDL error while %s: %s", msg, SDL_GetError());
	quit(-1);
}

int main(int argc, char* argv[])
{
	/* Initialize SDL */
	if( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0 )
	{
		fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
		return -1;
	}

	RANDOMIZE();

	/* Default options */

	commandline(argc, argv);

	printf("\nPlease wait...\n"
		   "\n\n*** Press Control-C to exit the program at any time. ***\n");
	printf("\n\n%s\n", VERSION);

	disp_init(width, height, disp_flags);

	timer_init();
	while( 1 )
	{
		mainLoop();
		timer_wait();
	}
}

void mainLoop(void)
{
	static time_t ltime, mtime;

	disp_processInput();

	if( RESIZE )
	{
		RESIZE = FALSE;
		if( state != STATE_INITIAL )
		{
			draw_same();
			applyPalette();
		}
	}

	if( SKIP )
	{
		if( state != STATE_INITIAL )
			state = STATE_NEXT;
		show_logo = 0;
	}

	if( NP )
	{
		if( !show_logo )
			newPalette();
		NP = FALSE;
	}

	switch( state )
	{
		case STATE_INITIAL:
			draw_init(draw_flags | (show_logo ? DRAW_LOGO : 0));
			initRolNFade(show_logo);

			/* Fall through */
		case STATE_NEXT:
			/* install a new image */
			draw_next();

			if( !show_logo && !SKIP )
			{	
				newPalette();
			}
			SKIP = FALSE;

			ltime = time(NULL);
			mtime = ltime + image_time;
			state = STATE_DISPLAY;
			/* Fall through */
		case STATE_DISPLAY:
			/* rotate the palette for a while */
			// if (GO)
			// {
			fadeInAndRotate();
			// }

			ltime = time(NULL);
			if( ltime > mtime && !LOCK )
			{
				/* Transition from logo only fades to black,
				 * like the first transition in Acidwarp 4.10.
				 */
				beginFadeOut(show_logo);
				state = STATE_FADEOUT;
			}
			break;

		case STATE_FADEOUT:
			/* fade out */
			if( GO )
			{
				if( fadeOut() )
				{
					show_logo = 0;
					state = STATE_NEXT;
				}
			}
			break;
	}
}