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
#include <vector>
#include <string>
extern "C" {
#include <stdio.h>
#include <string.h>
#include <assert.h>
}

#include "main.h"
#include "decl.h"
#include "subprocess.h"
#include "cgc.h"

using namespace std;

#define CG_VERSION_HEADER "cgc version "

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

static void replace(char *string, const char *find, const char *replace)
{
	size_t len=strlen(string), flen=strlen(find), rlen=strlen(replace);
	char *s;
	s=strstr(string, find);
	memmove(s+rlen, s+flen, len-(s-string));
	memcpy(s, replace, rlen);
	len+=rlen-flen;
	len-=s-string;
	string=s+rlen;
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
  const char gles[]="glesf";
  const char profileopts[] = "-profileopts";
  const char ps2b_opt[] = "NumTemps=32,ArbitrarySwizzle=1,NoTexInstructionLimit=1,NoDependentReadLimit=0,NumInstructionSlots=512";
  const char ps2a_opt[] = "NumTemps=22,ArbitrarySwizzle=1,NoTexInstructionLimit=1,NumInstructionSlots=512";

  const char arbfp_opt[]  = "MaxTexIndirections=4,NoDepenentReadLimit=0,NumInstructionSlots=512";
  const char arbfp_x800[] = "MaxTexIndirections=4,NoDepenentReadLimit=0,NumInstructionSlots=512";
  const char arbfp_6800[] = "NumInstructionSlots=2048";

  //char glsl_opt[] = "";
  const char userect[] ="-DUSERECT=1";
  const char dxpixelshader[] = "-DDXPIXELSHADER";
  const char define_gles[] = "-DGL_ES";
  const char * cgversion=NULL;

  vector<string> uniform_list_names;
  vector<string> uniform_list_types;
  vector<string> output_list_types;
  vector<string> for_list_control_vars_inits;

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
  case CODEGEN_GLES:
    argv[4] = gles;
    argv[5] = define_gles;
    break;
  default: 
     fprintf(stderr, "Unsupported Cgc target.\n");
     return NULL;
  }

  if(CODEGEN_GLES==target) {
     char * uniform_p=tempCode;
     //detect all input stream names and their types
     while((uniform_p=strstr(uniform_p,"uniform _stype"))!=NULL)
     {
        char tmp[1024];
        uniform_p+=16;
        char * lineend=strstr(uniform_p," ");
        assert(lineend-uniform_p+1 <= 1024);
        snprintf(tmp, lineend-uniform_p+1, "%s", uniform_p);
        uniform_list_names.push_back(tmp);

        char * type_p=strstr(uniform_p,"//GL_ES_in");
        type_p+=11;
        lineend=strstr(uniform_p,"\n");
        char *type_name;
        assert((lineend-type_p) <= 1024);
        snprintf(tmp, lineend-type_p, "%s", type_p);
        replaceAll(tmp, " ", "_");
        uniform_list_types.push_back(tmp);
     }

     char * output_p=tempCode;
     //detect all output types
     while((output_p=strstr(output_p,"//GL_ES_out"))!=NULL)
     {
        char tmp[1024];
        output_p+=12;
        char * lineend=strstr(output_p," ");
        assert(lineend-output_p+1 <= 1024);
        snprintf(tmp, lineend-output_p+1, "%s", output_p);
        replaceAll(tmp, " ", "_");
        output_list_types.push_back(tmp);
     }
  }

  /* Run CGC */
  fpcode = Subprocess_Run(argv, tempCode, true);

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
    case CODEGEN_GLES:
       fprintf(stderr, "GLES target.");
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

    fprintf(stderr, "Faulty code is following:\n");
    fprintf(stderr, "%s\n", tempCode);

    free( tempCode );

    return NULL;
  }

  free( tempCode );

  if (target == CODEGEN_PS20 ||
      target == CODEGEN_PS2B ||
      target == CODEGEN_PS2A)
     return fpcode;

  // cgc has this annoying feature that it outputs warnings
  // and errors to stdout rather than stderr.  So lets
  // figure out where the fragment code really starts...

  startline = strstr (fpcode, "!!");
  if (!startline) startline = strstr(fpcode, "//");
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

  //Detect version
  if (!cgversion)
  {
	  const char * versionstart = strstr(fpcode, CG_VERSION_HEADER);
	  versionstart+=strlen(CG_VERSION_HEADER);
	  const char * endversion = strstr(fpcode, ",");
	  cgversion=strndup(versionstart, endversion-versionstart);
	  fprintf(stderr, "Cg version: %s\n", cgversion);
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
  memmove (fpcode, startline, endline - startline);

  // ned: Many hacks to fix up broken GLSL output from cgc
  char *fpcodenew = (char *) malloc(strlen(fpcode)+16384);
  //Remove the fake definition of gl_FragCoord and remove _ from the name
  replaceAll(fpcode, "vec4 _gl_FragCoord;", "");
  replaceAll(fpcode, "_gl_FragCoord", "gl_FragCoord");
  if(CODEGEN_GLSL==target) {
	  //Versions prior to 3.1.0013 required fixes
	  if(strncmp(cgversion, "3.1.0013", strlen("3.1.0013") > 0))
	  {
		  printf("lower than 3.1\n");
    // ned: Many hacks to fix up broken GLSL output from cgc
    // Prepend required headers
    const char header[]="# version 110\n# extension GL_ARB_texture_rectangle : enable\n";
    memcpy(fpcodenew, header, sizeof(header));
    strcpy(fpcodenew+sizeof(header)-1, fpcode);
	  
    // cgc isn't aware of the GL_ARB_texture_rectangle so munge
    replaceAll(fpcodenew, "samplerRect", "sampler2DRect");
    // cgc is particularly braindead for not knowing what gl_FragData is
    // DESPITE understanding multiple output buffers for glslv
    if(strstr(fpcodenew, "gl_FragColor") && strstr(fpcodenew, "#extension GL_ARB_draw_buffers : enable")) {
      // Replace pend_s4___output_N with gl_FragData[N]
      char *outptr=strchr(strstr(fpcodenew, "gl_FragColor"), 10)+1;
      if(strstr(fpcodenew, "pend_s4___output_3")) {
        memmove(outptr+43, outptr, strlen(outptr)+1);
        memcpy(outptr, "    gl_FragData[3].x = pend_s4___output_3;\n", 43);
      }
      if(strstr(fpcodenew, "pend_s4___output_2")) {
        memmove(outptr+43, outptr, strlen(outptr)+1);
        memcpy(outptr, "    gl_FragData[2].x = pend_s4___output_2;\n", 43);
      }
      if(strstr(fpcodenew, "pend_s4___output_1")) {
        memmove(outptr+43, outptr, strlen(outptr)+1);
        memcpy(outptr, "    gl_FragData[1].x = pend_s4___output_1;\n", 43);
      }
      replaceAll(fpcodenew, "gl_FragColor", "gl_FragData[0]");
    }
    // GLSL may not legally contain __, so why does cgc output that?
    replaceAll(fpcodenew, "___", "_CGCdashdash_");
    replaceAll(fpcodenew, "__", "_CGCdash_");
      }
	  else
	  {
		  strcpy(fpcodenew, fpcode);
	  }
  }
  else if(CODEGEN_GLES==target) {
    strcpy(fpcodenew, fpcode);
    char * uniform_p=fpcodenew;
    //Replace texture reads with their coresponding reconstructing input function
    while((uniform_p=strstr(uniform_p,"texture2D"))!=NULL)
    {
       char line[50];
       char * lineend=strstr(uniform_p,"\n");
       char * linestart=uniform_p;
       while(*(--linestart)!='\n');
       assert(lineend-uniform_p+1 <= 50);
       //get the entire line
       snprintf(line, lineend-linestart, "%s", linestart+1);
       uniform_p=lineend;
       char t0[50], t1[50];
       //read input variable and sampler name
       sscanf(line, "%s = texture2D(%s);", t0, t1);

       //find the type of the input variable based on the sampler name
       for(int i=0; i<uniform_list_names.size(); i++)
       {
           if(strcmp(uniform_list_names[i].c_str(),t0)!=0)
           {
               //GLES can only have inputs up to 32 bits wide
               if( (uniform_list_types[0].find("2")!=std::string::npos) || 
                   (uniform_list_types[0].find("3")!=std::string::npos) || 
                   (uniform_list_types[0].find("4")!=std::string::npos)
                 )
               {
                  printf("Error in brcc OpenGL ES 2.0 backend: In GLES 2.0 each input is restricted in <= 32 bits\n");
                  printf("Please rewrite your kernel to use it with the GLSL ES backend\n");
                  exit(-1);
               }
               //change the reconstruction function
               char replacement_str[50];
               snprintf(replacement_str, 50, "reconstruct_%s(%s.x, ", uniform_list_types[i].c_str(), t0);
               snprintf(line, 50, "%s = texture2D(", t0);
               replaceAll(fpcodenew, line, replacement_str);
           }
       }
    }

    char * output_p=fpcodenew;
    //Use the corresponding encoding function for the output based on its type
    while((output_p=strstr(output_p,"gl_FragColor"))!=NULL)
    {
       char line[255];
       char * delim=strstr(output_p,"=");
       char * lineend=strstr(output_p,"\n");

       assert(lineend-output_p+1 <= 255);
       //get the entire line
       snprintf(line, lineend-output_p+1, "%s", output_p);
       output_p+=(lineend-output_p);
//printf("---%s\n", line);

       assert(lineend-delim <= 255);
       char rvalue[255];
       //get the entire line
       snprintf(rvalue, lineend-delim-1, "%s", delim+1);
//printf("---%s\n", rvalue);

       //get the type of the output variable and add the appropriate encoding function
       //GLES can only have a single 4-component output at most
       if( (output_list_types.size() > 1) || 
           (output_list_types.size() && (
             (output_list_types[0].find("2")!=std::string::npos) || 
             (output_list_types[0].find("3")!=std::string::npos) || 
             (output_list_types[0].find("4")!=std::string::npos)
                                        )
           )
         )
       {
          printf("Error in brcc OpenGL ES 2.0 backend: In GLES 2.0 the output is restricted in <= 32 bits\n");
          printf("Please rewrite your kernel to use it with the GLSL ES backend\n");
          exit(-1);
       }
       //if this fails, then we need to update the backend because it is a special case
       // eg. gathreop, no output specifier, reduce
       //assert(output_list_types.size());
       char replacement_str[255];
       if(output_list_types.size())
         snprintf(replacement_str, 255, "encode_output_%s(%s);", output_list_types[0].c_str(), rvalue);
       else
       {
         printf("Warning brcc: Output type for kernel is missing, defaulting to float. ");
         printf("You may have to check the backend for that case.\n");
         //TODO this should be done properly in the backend so that it outputs the type always
         snprintf(replacement_str, 255, "encode_output_float(%s);", rvalue);
       }
       replaceAll(fpcodenew, line, replacement_str);
    }

    //Brook's parser doesn't allow control variables to be declared in the for control statement
    //Also the generated cgc code misses the control variable initialisation in for statement, 
    //instead it is placed just before it
    //However, some glsl es compilers eg. VideoCore IV treat it as invalid
    //detect all for loops and their control variables
    char * for_position=fpcodenew;
    while((for_position=strstr(for_position,"for (;"))!=NULL)
    {
       char tmp[1024];
       const char * control_var=for_position-1;
       while(*control_var-- != ';');
       const char * nameend=control_var;
       while(*control_var-- != '=');
       while(isspace(*control_var)) control_var--;
       while(!isspace(*control_var)) control_var--;
       assert(nameend-control_var+1 <= 1024);
       snprintf(tmp, nameend-control_var+1, "%s", control_var+1);
       //save the entire control initialisation statement
       for_list_control_vars_inits.push_back(tmp);
       for_position++;
    }
    const char * code_pos=fpcodenew;
    for(unsigned int i=0; i< for_list_control_vars_inits.size(); i++)
    {
       char tmp[1024];
       //get the name of the control variable from the initialisation statement
       const char* var_name = for_list_control_vars_inits[i].substr(0, for_list_control_vars_inits[i].find(" ")).c_str();
       const char* name_start=strstr(code_pos, var_name);
       const char* type_end=name_start-1;
       while(isspace(*type_end)) type_end--;
       const char* type_start=type_end;
       while(!isspace(*type_start) && *type_start!=';') type_start--;
       assert(type_end-type_start+1 <= 1024);
       snprintf(tmp, type_end-type_start+1, "%s", type_start+1);
       //delete the declaration
       replaceAll(fpcodenew, (string(tmp)+string(" ")+string(var_name)+string(";\n")).c_str(), "");
       //delete the initialisation
       replaceAll(fpcodenew, (for_list_control_vars_inits[i]+string(";\n")).c_str(), "");
       char * control_var=fpcodenew;
       //find the for statement we want to fix
       for(unsigned int j=0; j< for_list_control_vars_inits.size(); j++) 
          control_var=strstr(control_var,"for (;"); 
       //put the type and the initialisation
       string replacement_string=string("for ( ")+string(tmp)+string(" ")+for_list_control_vars_inits[i]+string(";");
       replace(control_var, "for (;",  replacement_string.c_str());
    } 
    
    //Finally force high precision to have arithmetic accuracy instead of the default medium that cgc generates
    replaceAll(fpcodenew, "precision mediump float;", "precision highp float;");
  }
  else strcpy(fpcodenew, fpcode);
  // Wasn't escaping backslashes on Windows either
  replaceAll(fpcodenew, "\\", "\\\\");
  free(fpcode);
  fpcode=fpcodenew;
  return fpcode;
}


