/*
 * cgc.cpp
 *
 *  Handles compiling a shader with cgc.exe
 */

#ifdef _WIN32
#pragma warning(disable:4786)
#include <ios>
#else
#include <iostream>
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
#include "decl.h"
#include "subprocess.h"
#include "cgc.h"

/*
 * compile_cgc --
 *
 *      Takes CG code and runs it through the CG compiler (and parses the
 *      results) to produce the corresponding fragment program.
 */


char *
compile_cgc (const char *cgcode, CodeGenTarget target) {

  char *argv[16] = { "cgc", "-quiet", "-profile", NULL,
                     NULL, NULL };
  char *fpcode, *endline, *startline;
  char* tempCode = strdup( cgcode );
  char arbfp[]="arbfp1";
  char fp30[]="fp30";
  char ps20[]="ps_2_0";
  char userect[]="-DUSERECT=1";

  switch (target) {
  case CODEGEN_PS20:
     argv[3] = ps20;
     break;
  case CODEGEN_FP30:
     argv[3] = fp30;
     argv[4] = userect;
     break;
  case CODEGEN_ARB: 
     argv[3] = arbfp;
     argv[4] = userect;
     break;
  default: 
     fprintf(stderr, "Unsupported Cgc target.\n");
     return NULL;
  }

  /* Run CGC */
  fpcode = Subprocess_Run(argv, tempCode);

  free( tempCode );

  if (fpcode == NULL) {
     fprintf(stderr, "%s resulted in an error, skipping fp30 / nv30gl target ",
             argv[0]);
     return NULL;
  }

  // cgc has this annoying feature that it outputs warnings
  // and errors to stdout rather than stderr.  So lets
  // figure out where the fragment code really starts...
  startline = strstr (fpcode, "!!");
  if (startline) {
     char *p, *q;
     // Find where the fragment code really ends
     /* Tolerate CRLF or LF line endings. */
     /* strip out CR line endings */
     for (p = q = fpcode; *p; p++) {
        if (*p != '\r') *q++ = *p;
     }
     *q = *p;
     
     endline = strstr (fpcode, "\nEND\n");
  }

  if (!startline || !endline ) {
      fprintf(stderr, "Unable to parse returned CG Code: %s\n",
	      fpcode);
      return NULL;
  }

  // Trim off the execess cgc commentary
  endline += strlen("\nEND");
  *endline = '\0';
  endline++;

  // Print any warning messages
  if (startline != fpcode) {
    fprintf (stderr, "cgc warnings:\n");
    fwrite (fpcode, startline-fpcode, 1, stderr);
  }

  // Print the commentary
  if (globals.verbose) {
     fprintf(stderr, "***Summary information from cgc:\n");
     fwrite (endline, strlen(endline), 1, stderr);
  }

  // Trim off the warning messages
  memcpy (fpcode, startline, endline - startline);
  return fpcode;
}


