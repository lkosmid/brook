/*
 * runkernel.h
 *
 *      Exports from runkernel.br.  Single entry point for all the kernel
 *      overhead tests.
 */

#ifndef __RUNKERNEL_H_
#define __RUNKERNEL_H_

extern void RunKernel1D_Time(int maxLength);
extern void RunKernel2D_Time(int maxLength);
extern void RunKernel_GPUvsCPU(int maxLength);

#endif
