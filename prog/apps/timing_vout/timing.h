
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
