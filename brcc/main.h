#ifndef __MAIN_H__
#define __MAIN_H__

#define TARGET_CPU      (1<<0)
#define TARGET_PS20     (1<<1)
#define TARGET_FP30     (1<<2)
#define TARGET_ARB      (1<<3)
#define TARGET_MULTITHREADED_CPU (1<<4)
#define TARGET_FP40     (1<<5)
#define TARGET_PS30     (1<<6)

#define COMPILER_DEFAULT 0
#define COMPILER_CGC     1
#define COMPILER_FXC     2

struct globals_struct {
  globals_struct() {
      verbose=false,parseOnly=false;
      keepFiles=false,fponly=0,target=0,workspace=0;
      printLineDirectives=false,allowDX9MultiOut=false;
      enableGPUAddressTranslation=false;
      allowKernelToKernel=true,noTypeChecks=false;
  }             
  bool verbose;
  bool parseOnly;
  bool keepFiles;
  char *sourcename;
  char *compilername;
  char *shaderoutputname;
  char *coutputname;
  int fponly;
  int target;
  int workspace;
  bool printLineDirectives;
  bool allowDX9MultiOut;
  bool enableGPUAddressTranslation;
  bool allowKernelToKernel;
  bool noTypeChecks;
  int favorcompiler;
  // TIM: hacked flag for now
  bool enableKernelSplitting;
};

extern struct globals_struct globals;

#endif
