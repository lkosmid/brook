/*
 * brook2cpp.h --
 *
 *      Interface to brook2cpp-- transforms kernels into C++ code that can
 *      execute them on a CPU.
 */
#ifndef __BROOK2CPP_H__
#define __BROOK2CPP_H__
#ifdef _WIN32
#pragma warning(disable:4786)
//the above warning disables visual studio's annoying habit of warning when using the standard set lib
#endif
#include <map>
#include <string>
enum FunctionProperties {
   FP_NONE=0x0,
   FP_INDEXOF=0x1,
   FP_LINEARINDEXOF=0x2
};
class functionProperties{public:
   unsigned int p;
   functionProperties() {p=FP_NONE;}
};
extern std::map<std::string,functionProperties> FunctionProp;

class FunctionDef;
extern bool reduceNeeded(const FunctionDef * fDef);

extern void
Brook2Cpp_IdentifyIndexOf(class TransUnit * tu);

extern void
Brook2Cpp_ConvertKernel(FunctionDef *kDef);
extern void 
BrookReduce_ConvertKernel (FunctionDef * kdef);
extern void
BrookCombine_ConvertKernel (FunctionDef * kdef);
extern FunctionDef * changeFunctionCallForIndexOf(FunctionDef * fd);
#endif




