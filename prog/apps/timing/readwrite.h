/*
 * readwrite.h
 *
 *      Exports from readwrite.br.  Just the single entry point for timing
 *      stream reads and writes.
 */

#ifndef __READWRITE_H_
#define __READWRITE_H_

extern void TimeReadWrite(int streamLength);

extern int64 start, mid, stop;
#endif