/**
 * @file handy.c
 */

/** @brief Used by both ordinary and Emscripten worker drawing code.
 *  @param *list a list of integers
 *  @param listSize the size of the list
 *
 *  @see RANDOM
 *   */

#include "handy.h"

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
