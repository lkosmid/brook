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
compile_fxc (const char *shader, CodeGenTarget target, ShaderResourceUsage* outUsage, bool inValidate) {

  static const int kInputFileArgument = 5;
  static const int kOutputFileArgument = 4;
  char *argv[] = { "fxc", inValidate ? "/Tps_2_0" : "/Tps_2_sw", inValidate ? "" : "/Vd", "/nologo", 0, 0, NULL };
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
  
  argv[kOutputFileArgument] = (char *) malloc(strlen("/Fc.ps") +
                            strlen(globals.shaderoutputname) + 1);
  sprintf (argv[kOutputFileArgument], "/Fc%s.ps", globals.shaderoutputname);
  argv[kInputFileArgument] = globals.shaderoutputname;

  /* Run FXC */
  errcode = Subprocess_Run(argv, NULL);

  if (!globals.keepFiles) 
    remove(globals.shaderoutputname);

  if (errcode == NULL) {
    fprintf(stderr, "%s resulted in an error,"
            "skipping ps20 / dx9 target ", argv[0]);
    
     remove(argv[kOutputFileArgument]+3);
     return NULL;
  }

  if (globals.verbose)
    fprintf(stderr, "FXC returned: [35;1m%s[0m\n",
            errcode);

  fp = fopen(argv[kOutputFileArgument]+3, "rt");
  if (fp == NULL) {
    fprintf (stderr, "Unable to open compiler output file %s\n", 
             argv[kOutputFileArgument]+3);
    fprintf(stderr, "FXC returned: [35;1m%s[0m\n",
            errcode);
    return NULL;
  }

  fseek(fp, 0, SEEK_END);
  long flen = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  fpcode = (char *) malloc (flen+1);
  char* comments = (char *) malloc(flen+1);
  
  // Have to do it this way to fix the \r\n's
  int pos = 0;
  int cpos = 0;
  int i;
  bool incomment = false;

  while ((i = fgetc(fp)) != EOF) {
     
    // Remove comment lines
    if (incomment) {
      if (i == (int) '\n') {
        comments[cpos++] = '\n';
        incomment = false;
        while ((i = fgetc(fp)) != EOF &&
               i == (int) '\n');
        if (i == EOF)
           break;
      } else {
        comments[cpos++] = (char)i;
        continue;
      }
    }  else if (pos > 0 && 
                fpcode[pos-1] == '/' &&
                i == (int) '/') {
      incomment = true;
      comments[cpos++] = '/';
      comments[cpos++] = '/';
      fpcode[--pos] = '\0';
      continue;
    }
    
    fpcode[pos++] = (char) i;
  }  
  fpcode[pos] = '\0';
  comments[cpos] = '\0';

  // TIM: get instruction count information before we axe the damn thing... :)
  if( outUsage )
  {
    const char* instructionLine = strstr( comments, "// approximately" );
    assert( instructionLine );

	const char* nextLine = strstr( instructionLine, "\n" );
    const char* textureCount = strstr( instructionLine, "(" );
    if( (textureCount && !nextLine) || (textureCount && textureCount < nextLine) )
    {
      textureCount += strlen("(");

      const char* arithmeticCount = strstr( textureCount, ", " );
      //assert( arithmeticCount );
      arithmeticCount += strlen(", ");

      outUsage->arithmeticInstructionCount = atoi( arithmeticCount );
      outUsage->textureInstructionCount = atoi( textureCount );
    }
    else
    {
      outUsage->arithmeticInstructionCount = 0;
      outUsage->textureInstructionCount = 0;
    }

    // now look for register usage..
    // we know the pattern for what temps/constants/etc will look like, so:
    int samplerCount = 0;
    int interpolantCount = 0;
    int constantCount = 0;
    int registerCount = 0;
    int outputCount = 0;

    char registerName[128];

    for( int i = 0; i < 16; i++ )
    {
      sprintf( registerName, " s%d", i );
      if( strstr( fpcode, registerName ) )
        samplerCount = i+1;
      sprintf( registerName, " t%d", i );
      if( strstr( fpcode, registerName ) )
        interpolantCount = i+1;
      sprintf( registerName, " c%d", i );
      if( strstr( fpcode, registerName ) )
        constantCount = i+1;
      sprintf( registerName, " r%d", i );
      if( strstr( fpcode, registerName ) )
        registerCount = i+1;
      sprintf( registerName, " oC%d", i );
      if( strstr( fpcode, registerName ) )
        outputCount = i+1;
    }
    outUsage->samplerRegisterCount = samplerCount;
    outUsage->interpolantRegisterCount = interpolantCount;
    outUsage->constantRegisterCount = constantCount;
    outUsage->temporaryRegisterCount = registerCount;
    outUsage->outputRegisterCount = outputCount;
  }
  free(comments);
  
  fclose(fp);
  remove(argv[kOutputFileArgument]+3);
  free(argv[kOutputFileArgument]);

  if (target == CODEGEN_ARB) {
     std::istringstream ifpcode(fpcode);
     std::ostringstream ofpcode;
     
     convert_ps2arb (ifpcode, ofpcode);
     free(fpcode);
     fpcode = strdup(ofpcode.str().c_str());
  }

  return fpcode;
}
