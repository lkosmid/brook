/*
 * statrecord.c --
 *
 *      Simple routines for gathering statistics.
 *
 */

#include <math.h>
#ifndef MAXFLOAT
#define MAXFLOAT        FLT_MAX
#endif

typedef struct StatRecord {
   int64 min, max;
   float total, squareTotal;
   int n;
} StatRecord;


static inline void
StatRecord_Print(StatRecord *r, const char *name)
{
   printf("(* %s: %d samples: %.2f mean, %d min, %d max, %.2f variance *)\n",
         name, r->n, r->total / r->n, (int) r->min, (int) r->max,
         r->squareTotal / r->n - r->total / r->n * r->total / r->n);
}


static inline void
StatRecord_Clear(StatRecord *r) {
   r->min = ((int64) 1)<<62;
   r->max = 0;
   r->total = r->squareTotal = 0.0f;
   r->n = 0;
}


static inline void
StatRecord_Record(StatRecord *r, int64 time)
{
   int usec = (int) (CyclesToUsecs(time));

   if (usec < r->min) r->min = usec;
   if (usec > r->max) r->max = usec;
   r->total += usec;
   r->squareTotal = r->squareTotal + usec * usec;
   r->n++;
}
