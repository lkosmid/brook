// timing.hpp

#ifndef __TIMING_HPP__
#define __TIMING_HPP__

#ifdef WIN32
typedef __int64 int64;
#else
typedef long long int64;
#endif

extern int64 GetTime();
extern unsigned int GetTimeMillis();

extern void CleanupMillisTimer();
extern void SetupMillisTimer();


#endif
