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
int64 timerRes;
#ifdef WIN32
static inline
int64 GetTimeTSC() {
   __asm _emit 0x0f __asm _emit 0x31
}
#else
static inline
int64 GetTimeTSC() {
   int64 t;
   __asm__ __volatile__("rdtsc" : "=A" (t));
   return t;
}
#endif

extern int64 GetTime(void);
extern unsigned int GetTimeMillis(void);
extern int64 CyclesToUsecs(int64 cycles);

/*
 * XXX brcc currently has grief with typedefs mixed with Brook code, so we
 * just prototype all these here.  Since brcc runs before cpp, we sidestep
 * the issue.  I apologize deeply.  --Jeremy.
 */
extern int64 start, mid, mid2, stop;
#endif
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
int64 stop;
int64 start;
void CleanupMillisTimer(void) {
  if ((int64)timeEndPeriod((unsigned int)timerRes) == (int64)TIMERR_NOCANDO) {
    std::cerr << "WARNING: bad return value of call to timeEndPeriod.\n";
  }
}



/*
 * CyclesToUsecs --
 *
 *      Simple function to convert clock cycles to usecs.  We rely upon
 *      GetTimeMillis() to be fairly accurate over a medium duration Sleep()
 *      and then compute the MHz rating once based on that.
 */

int64
CyclesToUsecs(int64 cycles)
{
   static double Hz;

   if (Hz == 0) {
      int64 cCount;
      int msCount;

      cCount = GetTimeTSC();
      msCount = GetTimeMillis();
#ifdef WIN32
      Sleep(100);
#else
      usleep(100 * 1000);
#endif
      cCount = GetTimeTSC() - cCount;
      msCount = GetTimeMillis() - msCount;
      Hz = cCount * 1000 / (double) msCount;

#if 0
      std::cerr << "Your CPU is roughly "
                << (int) (Hz / 1000000) << " MHz.\n";
#endif
   }

   return (int64) (1000000 * cycles / Hz);
}
