/*
 * main.cpp --
 *
 *      Simple infrastructure for the timing app.
 */

#include <iostream>
#include <stdlib.h>
#include <ctype.h>

#include "main.h"
#include "readwrite.h"
#include "runkernel.h"
#include "blas.h"
#include "fft2d.h"
#include "sparseMat.h"
#include "matmult4x4.h"
#include "reduction.h"



typedef void (*timingFn) (int length);

struct timingEntry {
   char *id;
   timingFn f;
};

static const struct timingEntry tests[] = {
   { "rw1d",    ReadWrite1D_Time },
   { "rw2d",    ReadWrite2D_Time },
   { "runk1d",  RunKernel1D_Time },
   { "runk2d",  RunKernel2D_Time },
   { "runkvs",  RunKernel_GPUvsCPU },
   { "blas",    Blas_Time },
   { "sparseMat", SparseMat_Time},
   { "mm4_1w",  Matmult4x4_1way_Time },
//   { "mm4_1w_t",Matmult4x4_1wayPretransposed_Time},
   { "mm4_4w",  Matmult4x4_4way_Time },
//   { "mm4_4w_t",Matmult4x4_4wayPretransposed_Time },
   { "fftslow", doFFT },
   { "fft",     doOptFFT },
   { "fftw",    doFFTW },
   { "fft1d",   fft1d},
   { "red",     Reduction_Time}
};
static const int numTests = sizeof tests / sizeof tests[0];

/*
 * This is lame, but cTool doesn't cope with typedefs or #includes, so we
 * stash the unusual variables here.  I'm a bad person.  --Jeremy.
 */
int64 start, stop, t1, t2;

static unsigned int timerRes;
#ifndef _WIN32
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

#else
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
    std::cout << "(* Set timer resolution to " << timeCaps.wPeriodMin << " ms. *)\n";
  }

}

void CleanupMillisTimer(void) {
  if (timeEndPeriod(timerRes) == TIMERR_NOCANDO) {
    std::cerr << "WARNING: bad return value of call to timeEndPeriod.\n";
  }
}

#endif


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


/*
 * RunTest --
 *
 *      Takes a test id string (specified on the commandline) and runs the
 *      requested test.  The searches are boring and linear since they're so
 *      short.
 */
#ifndef _WIN32
#define _stricmp strcasecmp
#endif
static void
RunTest(char *id, int length)
{
   int i;

   for (i = 0; i < numTests; i++) {
      if (_stricmp(id, tests[i].id) == 0) {
         tests[i].f(length);
         std::cout << std::endl;
         return;
      }
   }

   std::cerr << "Invalid test " << id << " requested.  Valid choices are:\n";
   for (i = 0; i < numTests; i++) {
      std::cerr << tests[i].id << " ";
   }
   std::cerr << std::endl;
}

/*
 * main --
 *
 *      Very simple for now.  Pull the stream length from argv and then run
 *      all the tests.
 */

int
main(int argc, char *argv[])
{
   int length = 1024;

   argc--; argv++;      /* Skip argv[0] */

   /* See if argv[1] was a stream length */
   if (argc > 0 && isdigit(argv[0][0])) {
      if ((length = strtol(argv[0], NULL, 0)) <= 0) {
         std::cerr << "Must specify a positive length!\n";
         exit(1);
      }
      argc--; argv++;
   }

   SetupMillisTimer();

   /* Now process any requested tests */
   if (argc > 0) {
      do {
         RunTest(*argv, length);
         argc--; argv++;
      } while (argc > 0);
   } else {
      RunTest("rw", length);
      RunTest("runk1d", length);
      RunTest("runk2d", length);
   }

   //char c;
   //std::cerr << "Press <ENTER> to exit\n";
   //std::cin >> c;

   CleanupMillisTimer();

   return 0;
}
