#ifndef __MAIN_H__
#define __MAIN_H__

#define TARGET_FP30 1
#define TARGET_ARB  2

struct globals_struct {
  bool verbose;
  char *sourcename;
  char *compilername;
  char *cgoutputname;
  char *fpoutputname;
  char *houtputname;
  char *coutputname;
  int cgonly;
  int fponly;
  int target;
  int workspace;
};

extern struct globals_struct globals;

#endif
