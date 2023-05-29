#include "inc/acidwarp.h"
#include "inc/palinit.h"

static void add_sparkles_to_palette(uint8_t* palArray, int sparkle_amount);
static void init_rgbw_palArray(uint8_t* palArray);
static void init_w_palArray(uint8_t* palArray);
static void init_w_half_palArray(uint8_t* palArray);
static void init_pastel_palArray(uint8_t* palArray);
static void init_test_palArray(uint8_t* palArray);

// 256 is maximum for palRegNum
// palArray are RGB values right next to each other, so to update a single pixel to be red, you would need to
// palArray[palRegNum * 3] = 255;     // <-- red channel
// palArray[palRegNum * 3 + 1] = 0;   // <-- blue channel
// palArray[palRegNum * 3 + 2] = 0;   // <- green channel
static void init_chris_template(uint8_t* palArray)
{
	for( int palRegNum = 0; palRegNum < 256; ++palRegNum )
	{
		palArray[palRegNum * 3] = (uint8_t) palRegNum / 2;
		palArray[palRegNum * 3 + 1] = (uint8_t) palRegNum / 2;
		palArray[palRegNum * 3 + 2] = (uint8_t) palRegNum / 2;
	}
}

/* Initialzes a palette array to one of the palette types      */

/*
* ATTENTION CHRIS ATTENTION CHRIS ATTENTION CHRIS ATTENTION CHRIS ATTENTION CHRIS ATTENTION CHRIS ATTENTION CHRIS ATTENTION CHRIS ATTENTION CHRIS
*
* If you want to make a new pallete then follow steps in acidwarp.h (near top between 'ATTENTION CHRIS' brackets)
* and then follow steps below
*
* NOTE: to hardcode your palete you can just comment out (type // at start of line) everything but the default case
*/
void initPalArray(uint8_t* palArray, int pal_type)
{
	init_test_palArray(palArray);
	return;
	switch( pal_type )
	{
		case RGBW_PAL:
			init_rgbw_palArray(palArray);
			break;

		case W_PAL:
			init_w_palArray(palArray);
			break;

		case W_HALF_PAL:
			init_w_half_palArray(palArray);
			break;

		case PASTEL_PAL:
			init_pastel_palArray(palArray);
			break;

		case RGBW_LIGHTNING_PAL:
			init_rgbw_palArray(palArray);
			add_sparkles_to_palette(palArray, 9);
			break;

		case W_LIGHTNING_PAL:
			init_w_palArray(palArray);
			add_sparkles_to_palette(palArray, 9);
			break;

		case W_HALF_LIGHTNING_PAL:
			init_w_half_palArray(palArray);
			add_sparkles_to_palette(palArray, 9);
			break;

		case PASTEL_LIGHTNING_PAL:
			init_pastel_palArray(palArray);
			add_sparkles_to_palette(palArray, 9);
			break;
			/*
			case CHRIS_NEW_PAL:
				init_chris_template(palArray);
				// add_sparkles_to_palette(palArray, 9); // <-- optional
				break;
			*/
		default:
			//init_chris_template(palArray); 
			init_w_palArray(palArray);
			add_sparkles_to_palette(palArray, 9);
			break;
	}
}
/*
* ATTENTION CHRIS ATTENTION CHRIS ATTENTION CHRIS ATTENTION CHRIS ATTENTION CHRIS ATTENTION CHRIS ATTENTION CHRIS ATTENTION CHRIS ATTENTION CHRIS
*/

static void init_test_palArray(uint8_t* palArray)
{
	for( int palRegNum = 0; palRegNum < 128; ++palRegNum )
	{
		palArray[palRegNum * 3] = (uint8_t) palRegNum / 2;
		palArray[palRegNum * 3 + 1] = (uint8_t) palRegNum / 2;
		palArray[palRegNum * 3 + 2] = (uint8_t) palRegNum / 2;
	}

	for( int palRegNum = 128; palRegNum < 256; ++palRegNum )
	{
		palArray[palRegNum * 3] = (uint8_t) (255 - palRegNum) / 2;
		palArray[palRegNum * 3 + 1] = (uint8_t) (255 - palRegNum) / 2;
		palArray[palRegNum * 3 + 2] = (uint8_t) (255 - palRegNum) / 2;
	}
}

static void add_sparkles_to_palette(uint8_t* palArray, int sparkle_amount)
{
	int palRegNum;

	for( palRegNum = 1; palRegNum < 256; palRegNum += 4 )
	{
		palArray[palRegNum * 3] = (uint8_t) MIN(63, palArray[palRegNum * 3] + sparkle_amount);
		palArray[palRegNum * 3 + 1] = (uint8_t) MIN(63, palArray[palRegNum * 3 + 1] + sparkle_amount);
		palArray[palRegNum * 3 + 2] = (uint8_t) MIN(63, palArray[palRegNum * 3 + 2] + sparkle_amount);
	}
}

static void init_rgbw_palArray(uint8_t* palArray)
{
	int palRegNum;

	for( palRegNum = 0; palRegNum < 32; ++palRegNum )
	{
		palArray[palRegNum * 3] = (uint8_t) palRegNum * 2;
		palArray[(palRegNum + 64) * 3] = (uint8_t) 0;
		palArray[(palRegNum + 128) * 3] = (uint8_t) 0;
		palArray[(palRegNum + 192) * 3] = (uint8_t) palRegNum * 2;

		palArray[palRegNum * 3 + 1] = (uint8_t) 0;
		palArray[(palRegNum + 64) * 3 + 1] = (uint8_t) palRegNum * 2;
		palArray[(palRegNum + 128) * 3 + 1] = (uint8_t) 0;
		palArray[(palRegNum + 192) * 3 + 1] = (uint8_t) palRegNum * 2;

		palArray[palRegNum * 3 + 2] = (uint8_t) 0;
		palArray[(palRegNum + 64) * 3 + 2] = (uint8_t) 0;
		palArray[(palRegNum + 128) * 3 + 2] = (uint8_t) palRegNum * 2;
		palArray[(palRegNum + 192) * 3 + 2] = (uint8_t) palRegNum * 2;
	}

	for( palRegNum = 32; palRegNum < 64; ++palRegNum )
	{
		palArray[palRegNum * 3] = (uint8_t) (63 - palRegNum) * 2;
		palArray[(palRegNum + 64) * 3] = (uint8_t) 0;
		palArray[(palRegNum + 128) * 3] = (uint8_t) 0;
		palArray[(palRegNum + 192) * 3] = (uint8_t) (63 - palRegNum) * 2;

		palArray[palRegNum * 3 + 1] = (uint8_t) 0;
		palArray[(palRegNum + 64) * 3 + 1] = (uint8_t) (63 - palRegNum) * 2;
		palArray[(palRegNum + 128) * 3 + 1] = (uint8_t) 0;
		palArray[(palRegNum + 192) * 3 + 1] = (uint8_t) (63 - palRegNum) * 2;

		palArray[palRegNum * 3 + 2] = (uint8_t) 0;
		palArray[(palRegNum + 64) * 3 + 2] = (uint8_t) 0;
		palArray[(palRegNum + 128) * 3 + 2] = (uint8_t) (63 - palRegNum) * 2;
		palArray[(palRegNum + 192) * 3 + 2] = (uint8_t) (63 - palRegNum) * 2;
	}
}

static void init_w_palArray(uint8_t* palArray)
{
	int palRegNum;

	for( palRegNum = 0; palRegNum < 128; ++palRegNum )
	{
		palArray[palRegNum * 3] = (uint8_t) palRegNum / 2;
		palArray[palRegNum * 3 + 1] = (uint8_t) palRegNum / 2;
		palArray[palRegNum * 3 + 2] = (uint8_t) palRegNum / 2;
	}

	for( palRegNum = 128; palRegNum < 256; ++palRegNum )
	{
		palArray[palRegNum * 3] = (uint8_t) (255 - palRegNum) / 2;
		palArray[palRegNum * 3 + 1] = (uint8_t) (255 - palRegNum) / 2;
		palArray[palRegNum * 3 + 2] = (uint8_t) (255 - palRegNum) / 2;
	}
}

static void init_w_half_palArray(uint8_t* palArray)
{
	int palRegNum;

	for( palRegNum = 0; palRegNum < 64; ++palRegNum )
	{
		palArray[palRegNum * 3] = (uint8_t) palRegNum;
		palArray[palRegNum * 3 + 1] = (uint8_t) palRegNum;
		palArray[palRegNum * 3 + 2] = (uint8_t) palRegNum;

		palArray[(palRegNum + 64) * 3] = (uint8_t) (63 - palRegNum);
		palArray[(palRegNum + 64) * 3 + 1] = (uint8_t) (63 - palRegNum);
		palArray[(palRegNum + 64) * 3 + 2] = (uint8_t) (63 - palRegNum);
	}

	for( palRegNum = 128; palRegNum < 256; ++palRegNum )
	{
		palArray[palRegNum * 3] = 0;
		palArray[palRegNum * 3 + 1] = 0;
		palArray[palRegNum * 3 + 2] = 0;
	}
}

void init_pastel_palArray(uint8_t* palArray)
{
	int palRegNum;

	for( palRegNum = 0; palRegNum < 128; ++palRegNum )
	{
		palArray[palRegNum * 3] = (uint8_t) 31 + palRegNum / 4;
		palArray[palRegNum * 3 + 1] = (uint8_t) 31 + palRegNum / 4;
		palArray[palRegNum * 3 + 2] = (uint8_t) 31 + palRegNum / 4;

		palArray[(palRegNum + 128) * 3] = (uint8_t) 31 + (127 - palRegNum) / 4;
		palArray[(palRegNum + 128) * 3 + 1] = (uint8_t) 31 + (127 - palRegNum) / 4;
		palArray[(palRegNum + 128) * 3 + 2] = (uint8_t) 31 + (127 - palRegNum) / 4;
	}
}
