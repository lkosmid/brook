
#ifdef _WIN32
#include <windows.h>
#endif



/*
 * main.h
 *
 *      Exports from main.cpp.  Pretty much infrastructure type functions
 *      (timing, logging, etc.) plus command line arguments.
 */

#ifndef __MAIN_H_
#define __MAIN_H_

#ifdef WIN32
typedef __int64 int64;
#else
typedef long long int64;
#endif

extern int64 GetTime(void);
extern unsigned int GetTimeMillis(void);
extern int64 CyclesToUsecs(int64 cycles);
unsigned int GetTimeMillis(void) ;
void SetupMillisTimer(void);
void CleanupMillisTimer(void);

/*
 * XXX brcc currently has grief with typedefs mixed with Brook code, so we
 * just prototype all these here.  Since brcc runs before cpp, we sidestep
 * the issue.  I apologize deeply.  --Jeremy.
 */
extern int64 start, mid, mid2, stop;

#endif
#ifdef TIMING_DEF
int64 timerRes;
int64 stop;
int64 start;


#ifdef _WIN32
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

// Tim is evil...
#pragma comment(lib,"winmm")

unsigned int GetTimeMillis(void) {
  return (unsigned int)timeGetTime();
}

//  By default in 2000/XP, the timeGetTime call is set to some resolution
// between 10-15 ms query for the range of value periods and then set timer
// to the lowest possible.  Note: MUST make call to corresponding
// CleanupMillisTimer
void SetupMillisTimer(void) {

  TIMECAPS timeCaps;
  timeGetDevCaps(&timeCaps, sizeof(TIMECAPS)); 

  if (timeBeginPeriod(timeCaps.wPeriodMin) == TIMERR_NOCANDO) {
    std::cerr << "WARNING: Cannot set timer precision.  Not sure what precision we're getting!\n";
  }
  else {
    timerRes = timeCaps.wPeriodMin;
    //std::cout << "(* Set timer resolution to " << timeCaps.wPeriodMin << " ms. *)\n";
  }

}
void CleanupMillisTimer(void) {
  if ((int64)timeEndPeriod((unsigned int)timerRes) == (int64)TIMERR_NOCANDO) {
    std::cerr << "WARNING: bad return value of call to timeEndPeriod.\n";
  }
}
#else
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
void SetupMillisTimer(void) {}
void CleanupMillisTimer(void) {}
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


#endif
