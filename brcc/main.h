#ifndef __MAIN_H__
#define __MAIN_H__

#define TARGET_CPU      (1<<0)
#define TARGET_PS20     (1<<1)
#define TARGET_FP30     (1<<2)  /* XXX Not yet */
#define TARGET_ARB      (1<<3)  /* XXX Not yet */

struct globals_struct {
  bool verbose;
  bool parseOnly;
  char *sourcename;
  char *compilername;
  char *cgoutputname;
  char *houtputname;
  char *coutputname;
  int fponly;
  int target;
  int workspace;
};

extern struct globals_struct globals;

#endif
