/*
 * main.cpp --
 *
 *      Simple infrastructure for the timing app.
 */

#include <iostream>
#include <stdlib.h>

#include "main.h"
#include "volfire.h"

/*
 * This is lame, but cTool doesn't cope with typedefs or #includes, so we
 * stash the unusual variables here.  I'm a bad person.  --Jeremy.
 */
int64 start, mid, stop;

#ifdef WIN32
#include <windows.h>

/*
 * GetTime --
 *
 *      Returns the curent time (from some uninteresting origin) in usecs
 *      based on the performance counters.
 */

int64
GetTime(void)
{
   static double cycles_per_usec;
   LARGE_INTEGER counter;

   if (cycles_per_usec == 0) {
      static LARGE_INTEGER lFreq;
      if (!QueryPerformanceFrequency(&lFreq)) {
         std::cerr << "Unable to read the performance counter frquency!\n";
         return 0;
      }

      cycles_per_usec = 1000000 / ((double) lFreq.QuadPart);
   }

   if (!QueryPerformanceCounter(&counter)) {
      std::cerr << "Unable to read the performance counter!\n";
      return 0;
   }

   return ((int64) (((double) counter.QuadPart) * cycles_per_usec));
}
#else
#include <unistd.h>
#include <sys/time.h>
void SetupMillisTimer(void) {}
void CleanupMillisTimer(void) {}
static unsigned int timerRes;
int64 GetTime (void) {
  struct timeval tv;
  timerRes = 1000;
  gettimeofday(&tv,NULL);
  int64 temp = tv.tv_usec;
  temp+=tv.tv_sec*1000000;
  return temp;
}
unsigned int GetTimeMillis () {
  return (unsigned int)(GetTime ()/1000);
}
#endif

int
main(int argc, char *argv[])
{
   int retval = volfire_main(argc, argv);  
  
   return retval;
}
