/*
 * fxc.cpp
 *
 *  Handles compiling a shader to ps2.0
 */

#ifdef _WIN32
#pragma warning(disable:4786)
#endif
#include <sstream>
#include <iomanip>
#include <fstream>
extern "C" {
#include <stdio.h>
#include <string.h>
#include <assert.h>
}

#include "main.h"
#include "subprocess.h"
#include "project.h"
#include "fxc.h"

#include "ps2arb.h"

/*
 * compile_fxc --
 *
 *      Takes shader and runs it through the FXC compiler (and parses the
 *      results) to produce the corresponding fragment program.
 */

char *
compile_fxc (const char *shader, CodeGenTarget target) {

  char *argv[] = { "fxc", "/Tps_2_0", "/nologo", 0, 0, NULL };
  char *fpcode,  *errcode;

  FILE *fp = fopen (globals.shaderoutputname, "wb+");
  if (fp == NULL) {
    fprintf (stderr, "Unable to open tmp file %s\n", globals.shaderoutputname);
    return NULL;
  }
  fwrite(shader, sizeof(char), strlen(shader), fp);
  fclose(fp);
  
  switch (target) {
  case CODEGEN_PS20:
  case CODEGEN_ARB: 
     break;
  case CODEGEN_FP30:
  default: 
     fprintf(stderr, "Unsupported fxc target.\n");
     return NULL;
  }
  
  argv[3] = (char *) malloc(strlen("/Fc.ps") +
                            strlen(globals.shaderoutputname) + 1);
  sprintf (argv[3], "/Fc%s.ps", globals.shaderoutputname);
  argv[4] = globals.shaderoutputname;

  /* Run FXC */
  errcode = Subprocess_Run(argv, NULL);

  if (!globals.keepFiles) 
    remove(globals.shaderoutputname);

  if (errcode == NULL) {
    fprintf(stderr, "%s resulted in an error,"
            "skipping ps20 / dx9 target ", argv[0]);
     remove(argv[3]+3);
     return NULL;
  }

  if (globals.verbose)
    fprintf(stderr, "FXC returned: [35;1m%s[0m\n",
            errcode);

  fp = fopen(argv[3]+3, "rt");
  if (fp == NULL) {
    fprintf (stderr, "Unable to open compiler output file %s\n", 
             argv[3]+3);
    fprintf(stderr, "FXC returned: [35;1m%s[0m\n",
            errcode);
    return NULL;
  }

  fseek(fp, 0, SEEK_END);
  long flen = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  fpcode = (char *) malloc (flen+1);
  
  // Have to do it this way to fix the \r\n's
  int pos = 0;
  int i;
  bool incomment = false;

  while ((i = fgetc(fp)) != EOF) {
     
    // Remove comment lines
    if (incomment) {
      if (i == (int) '\n') {
        incomment = false;
        while ((i = fgetc(fp)) != EOF &&
               i == (int) '\n');
        if (i == EOF)
           break;
      } else
        continue;
    }  else if (pos > 0 && 
                fpcode[pos-1] == '/' &&
                i == (int) '/') {
      incomment = true;
      fpcode[--pos] = '\0';
      continue;
    }
    
    fpcode[pos++] = (char) i;
  }  
  fpcode[pos] = '\0';
  
  fclose(fp);
  remove(argv[3]+3);
  free(argv[3]);

  if (target == CODEGEN_ARB) {
     std::istringstream ifpcode(fpcode);
     std::ostringstream ofpcode;
     
     convert_ps2arb (ifpcode, ofpcode);
     free(fpcode);
     fpcode = strdup(ofpcode.str().c_str());
  }

  return fpcode;
}