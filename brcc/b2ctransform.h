/*
 * brook2cpp.h --
 *
 *      Interface to brook2cpp-- transforms kernels into C++ code that can
 *      execute them on a CPU.
 */
#ifndef __BROOK2CPP_H__
#define __BROOK2CPP_H__

class FunctionDef;
extern bool reduceNeeded(const FunctionDef * fDef);

extern void
Brook2Cpp_ConvertKernel(FunctionDef *kDef);
extern void 
BrookReduce_ConvertKernel (FunctionDef * kdef);
extern void
BrookCombine_ConvertKernel (FunctionDef * kdef);
#endif

