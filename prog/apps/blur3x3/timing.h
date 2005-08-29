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
#define printf64(a) printf("%8I64d", a)
#else
typedef long long int64;
#define printf64(a) printf("%8Ld", a)
#endif

#ifdef WIN32
/*
static inline
int64 GetTimeTSC() {
   __asm _emit 0x0f __asm _emit 0x31
}
*/
#else
#if defined(__ppc__)
static inline
long long GetTimeTSC() {
   int64 t;
   int tl, th;
   __asm__ __volatile__("mftbu %0" : "=b" (th));
   __asm__ __volatile__("mftb %0" : "=b" (tl));
   t = th;
   t <<= 32;
   t += tl;
   return t;
}
#else
static inline
int64 GetTimeTSC() {
   int64 t;
   __asm__ __volatile__("rdtsc" : "=A" (t));
   return t;
}
#endif
#endif

extern int64 GetTime(void);
extern unsigned int GetTimeMillis(void);
extern int64 CyclesToUsecs(int64 cycles);

/*
 * XXX brcc currently has grief with typedefs mixed with Brook code, so we
 * just prototype all these here.  Since brcc runs before cpp, we sidestep
 * the issue.  I apologize deeply.  --Jeremy.
 */
extern int64 start, stop, t1, t2;
#endif
