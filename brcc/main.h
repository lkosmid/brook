#ifndef __MAIN_H__
#define __MAIN_H__

#define TARGET_CPU      (1<<0)
#define TARGET_PS20     (1<<1)
#define TARGET_FP30     (1<<2)  
#define TARGET_ARB      (1<<3)  /* XXX Not yet */
#define TARGET_MULTITHREADED_CPU (1<<4)
struct globals_struct {
  globals_struct() {
      verbose=false,parseOnly=false;
      keepFiles=false,fponly=0,target=0,workspace=0;
      allowDX9MultiOut=false,enableGPUAddressTranslation=false;
      allowKernelToKernel=true,noTypeChecks=false;
  }             
  bool verbose;
  bool parseOnly;
  bool keepFiles;
  char *sourcename;
  char *compilername;
  char *shaderoutputname;
  char *houtputname;
  char *coutputname;
  int fponly;
  int target;
  int workspace;
  bool allowDX9MultiOut;
  bool enableGPUAddressTranslation;
  bool allowKernelToKernel;
  bool noTypeChecks;
};

extern struct globals_struct globals;

#endif
