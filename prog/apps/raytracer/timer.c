/* *************************************************************************
 * Copyright (C) 2004 Jeremy Sugerman
 * All Rights Reserved
 * *************************************************************************/

/*
 * timer.c --
 *
 *      High resolution timing.  I lifted this from gpubench, which likely
 *      stole it from the Brook timing app.
 */

#include <windows.h>
#include <assert.h>

#include "timer.h"

static LARGE_INTEGER start;
static float         freq;


/*
 * Timer_Reset --
 *
 *      Resets the logical timer to zero.  Also takes care of any
 *      one time intialization the first time it's invoked.
 *
 * Returns:
 *      void.
 */

void
Timer_Reset(void)
{
   static int knowFreq;

   if (!knowFreq) {
      LARGE_INTEGER freqInt;

      if (!QueryPerformanceFrequency(&freqInt)) {
         assert(0);
      }
      freq = (float) freqInt.QuadPart;
      knowFreq = 1;
   }

   if (!QueryPerformanceCounter(&start)) {
      assert(0);
   }
}


/*
 * Timer_GetMS --
 *
 *      Returns the elapsed number of msecs since the most recent call to
 *      Timer_Reset().
 *
 * Returns:
 *      See above.
 */

float
Timer_GetMS(void)
{
   LARGE_INTEGER now;

   if (!QueryPerformanceCounter(&now)) {
      assert(0);
   }
   return ((float)(now.QuadPart - start.QuadPart)) / freq * 1000;
}
