/*
 * brook2cpp.h --
 *
 *      Interface to brook2cpp-- transforms kernels into C++ code that can
 *      execute them on a CPU.
 */
#ifndef __BROOK2CPP_H__
#define __BROOK2CPP_H__

class BRTKernelDef;

extern void
Brook2Cpp_ConvertKernel(BRTKernelDef *kDef);

#endif
