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

#endif
