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

static void replaceAll(char *string, const char *find, const char *replace)
{
	size_t len=strlen(string), flen=strlen(find), rlen=strlen(replace);
	char *s;
	while((s=strstr(string, find)))
	{
		memmove(s+rlen, s+flen, len-(s-string));
		memcpy(s, replace, rlen);
		len+=rlen-flen;
		len-=s-string;
		string=s+rlen;
	}
}

/*
 * compile_cgc --
 *
 *      Takes CG code and runs it through the CG compiler (and parses the
 *      results) to produce the corresponding fragment program.
 */


char *
compile_cgc (const char * /*name*/,
             const char *shader, 
             CodeGenTarget target, 
             ShaderResourceUsage* /*outUsage*/, 
             bool /*inValidate*/) {


  const char *argv[16] = { "cgc", "-quiet", "-DCGC=1", "-profile", NULL,
                     NULL, NULL, NULL, NULL};
  char *fpcode, *endline, *startline;
  char* tempCode = strdup(shader);
  const char arbfp[]="arbfp1";
  const char fp30[]="fp30";
  const char fp40[]="fp40";
  const char ps20[]="ps_2_0";
  const char ps2b[]="ps_2_x";
  const char ps2a[]="ps_2_x";
  const char glsl[]="glslf";
  const char profileopts[] = "-profileopts";
  const char ps2b_opt[] = "NumTemps=32,ArbitrarySwizzle=1,NoTexInstructionLimit=1,NoDependentReadLimit=0,NumInstructionSlots=512";
  const char ps2a_opt[] = "NumTemps=22,ArbitrarySwizzle=1,NoTexInstructionLimit=1,NumInstructionSlots=512";

  const char arbfp_opt[]  = "MaxTexIndirections=4,NoDepenentReadLimit=0,NumInstructionSlots=96";
  const char arbfp_x800[] = "MaxTexIndirections=4,NoDepenentReadLimit=0,NumInstructionSlots=512";
  const char arbfp_6800[] = "NumInstructionSlots=2048";

  //char glsl_opt[] = "";
  const char userect[] ="-DUSERECT=1";
  const char dxpixelshader[] = "-DDXPIXELSHADER";

  switch (target) {
  case CODEGEN_PS20:
     argv[4] = ps20;
     argv[5] = dxpixelshader;
     break;
  case CODEGEN_PS2B:
     argv[4] = ps2b;
     argv[5] = profileopts;
     argv[6] = ps2b_opt;
     argv[7] = dxpixelshader;
     break;
  case CODEGEN_PS2A:
     argv[4] = ps2a;
     argv[5] = profileopts;
     argv[6] = ps2a_opt;
     argv[7] = dxpixelshader;
     break;
  case CODEGEN_FP30:
     argv[4] = fp30;
     argv[5] = userect;
     break;
  case CODEGEN_FP40:
     argv[4] = fp40;
     argv[5] = userect;
     break;
  case CODEGEN_ARB:
    argv[4] = arbfp;
    argv[5] = userect;
    argv[6] = profileopts;

    switch (globals.arch) {
    case GPU_ARCH_X800:
      argv[7] = arbfp_x800;
      break;
    case GPU_ARCH_6800:
      argv[7] = arbfp_6800;
      break;
    default:
      argv[7] = arbfp_opt;
      break;
    }
    break;
  case CODEGEN_GLSL:
    argv[4] = glsl;
    argv[5] = userect;
    //argv[6] = glsl_opt;
    break;
  default: 
     fprintf(stderr, "Unsupported Cgc target.\n");
     return NULL;
  }

  /* Run CGC */
  fpcode = Subprocess_Run(argv, tempCode, true);

  free( tempCode );

  if (fpcode == NULL) {
     fprintf(stderr, "%s resulted in an error, skipping ",
             argv[0]);

    switch (target) {
    case CODEGEN_PS2B:
       fprintf(stderr, "PS2B target.");
       break;
    case CODEGEN_PS2A:
       fprintf(stderr, "PS2A target.");
       break;
    case CODEGEN_PS20:
       fprintf(stderr, "PS20 target.");
       break;
    case CODEGEN_ARB:
       fprintf(stderr, "ARB target.");
       break;
    case CODEGEN_GLSL:
       fprintf(stderr, "GLSL target.");
       break;
    case CODEGEN_PS30:
       fprintf(stderr, "PS30 target.");
       break;
    case CODEGEN_FP30:
       fprintf(stderr, "FP30 target.");
       break;
    case CODEGEN_FP40:
       fprintf(stderr, "FP40 target.");
       break;
    default:
       break;
    }   
    fprintf(stderr, "\n");


    return NULL;
  }

  if (target == CODEGEN_PS20 ||
      target == CODEGEN_PS2B ||
      target == CODEGEN_PS2A)
     return fpcode;

  // cgc has this annoying feature that it outputs warnings
  // and errors to stdout rather than stderr.  So lets
  // figure out where the fragment code really starts...

  startline = strstr (fpcode, "!!");
  if (!startline) startline = strstr(fpcode, "// transl output by Cg compiler");
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
     if (!endline) endline = strstr(fpcode, " // main end\n");
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

  // ned: Hack to fix up broken GLSL output from cgc
  char *fpcodenew = (char *) malloc(strlen(fpcode)+16384);
  if(CODEGEN_GLSL==target) {
    // Prepend required headers
    const char header[]="# version 110\n# extension GL_ARB_texture_rectangle : enable\n";
    memcpy(fpcodenew, header, sizeof(header));
    strcpy(fpcodenew+sizeof(header)-1, fpcode);
    // cgc isn't aware of the GL_ARB_texture_rectangle so munge
    replaceAll(fpcodenew, "samplerRect", "sampler2DRect");
    // GLSL may not legally contain __, so why does cgc output that?
    replaceAll(fpcodenew, "___", "_CGCdashdash_");
    replaceAll(fpcodenew, "__", "_CGCdash_");
  }
  else strcpy(fpcodenew, fpcode);
  // Wasn't escaping backslashes on Windows either
  replaceAll(fpcodenew, "\\", "\\\\");
  free(fpcode);
  fpcode=fpcodenew;
  return fpcode;
}


