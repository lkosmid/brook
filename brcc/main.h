#ifndef __MAIN_H__
#define __MAIN_H__

#define TARGET_CPU      (1<<0)
#define TARGET_PS20     (1<<1)
#define TARGET_FP30     (1<<2)  /* XXX Not yet */
#define TARGET_ARB      (1<<3)  /* XXX Not yet */

struct globals_struct {
  bool multiThread;
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
