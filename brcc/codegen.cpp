/*
 * codegen.cpp --
 *
 *      Codegen takes already parsed brook input and produces the resulting
 *      .[ch] files required to invoke it.  It converts to CG and feeds that
 *      through a CG compiler.
 */

#include <sstream>
#include <iomanip>
#include <ios>

extern "C" {
#include <stdio.h>
#include <string.h>
#include <assert.h>
}

#include "main.h"
#include "decl.h"
#include "subprocess.h"

static const char fp30_assist[] =
#include "fp30_assist.cg.bin"
;


/*
 * generate_cg_code --
 *
 *      This function takes a parsed kernel function as input and produces
 *      the CG code reflected, plus the support code required.
 */

static char *
generate_cg_code (Decl **args, int nArgs, const char *body) {
  const char xyzw[] = "xyzw";
  std::ostringstream cg;
  Decl *outArg = NULL;
  int texcoord, i;

  cg << "#define _WORKSPACE " << globals.workspace << std::endl;
  cg << "#define _WORKSPACE_INV " 
     << std::setprecision((std::streamsize) 9.9) 
     << 1.0 / globals.workspace << std::endl;

  cg << fp30_assist;

  cg << "fragout_float main (\n\t\t";

  /* Print the argument list */
  texcoord = 0;
  for (i=0; i < nArgs; i++) {
     /* Don't put the output in the argument list */
     if ((args[i]->form->getQualifiers() & TQ_Out)!=0) {
        outArg = args[i];
        continue;
     }

     if (i) cg <<  ",\n\t\t";

     if (args[i]->isStream()) {
       cg << "uniform texobjRECT _tex_" << *args[i]->name << ",\n\t\t";
       cg << "float2 _tex_" << *args[i]->name << "_pos : TEX"
          << (texcoord < 1) ? texcoord++ : texcoord;
     } else {
       cg << "uniform ";
       args[i]->print(cg, true);
     }
  }
  cg << ") {\n";

  /* Declare the stream variables */
  for (i=0; i < nArgs; i++) {
     if (args[i] == outArg) {
        TypeQual qualifier = outArg->form->getBase()->qualifier;

        outArg->form->getBase()->qualifier &= ~TQ_Out;
        cg << "\t";
        args[i]->form->printBase(cg, 0);
        cg << " " << *args[i]->name << ";\n";
        outArg->form->getBase()->qualifier = qualifier;
     } else if (args[i]->isStream()) {
        cg << "\t";
        args[i]->form->printBase(cg, 0);
        cg << " " << *args[i]->name << ";\n";
     }
  }

  /* Declare the output variable */
  cg << "\tfragout_float  _OUT;\n";

  /* Perform stream fetches */
  for (i=0; i < nArgs; i++) {
     if (args[i] == outArg) continue;

     if (args[i]->isStream()) {
        int dimension = FloatDimension(args[i]->form->getBase()->typemask);
        assert(dimension > 0);

        cg << "\t" << *args[i]->name << " = f" << (char) (dimension + '0')
           << "texRECT (_tex_" << *args[i]->name << ", _tex_" << *args[i]->name
           << "_pos);\n";
     }
  }

  /* Include the body of the kernel */
  cg << body << std::endl;

  /* Return the result */
  if (outArg) {
     int dimension = FloatDimension(outArg->form->getBase()->typemask);

     for (i=0; i < dimension; i++) {
        cg << "\t_OUT.col." << xyzw[i] << "= " << *outArg->name << "."
           << xyzw[i] << ";\n";
     }

     cg << "\treturn _OUT;\n}\n";
  }

  return strdup(cg.str().c_str());
}

/*
 * generate_hlsl_code --
 *
 *      This function takes a parsed kernel function as input and produces
 *      the CG code reflected, plus the support code required.
 */

static char *
generate_hlsl_code (Decl **args, int nArgs, const char *body) {
  const char xyzw[] = "xyzw";
  std::ostringstream hlsl;
  Decl *outArg = NULL;
  int texcoord, constreg, i;


  // Add the workspace variable

  //  hlsl << "#define _WORKSPACE " << globals.workspace << std::endl;
  //  hlsl << "#define _WORKSPACE_INV " << std::setprecision(9.9)
  //       <<  1.0 / globals.workspace << std::endl;

  constreg = 0;
  hlsl << "float2 _workspace    : register (c" 
       << constreg++ << ");\n";
  hlsl << "float2 _workspaceinv : register (c"
       << constreg++ << ");\n";
  
  /* Print out the texture stream */
  texcoord = 0;
  for (i=0; i < nArgs; i++) {
    /* Don't put the output in the argument list */
    if ((args[i]->form->getQualifiers() & TQ_Out)!=0) {
      outArg = args[i];
      continue;
    }
    
    if (args[i]->isStream()) {
       hlsl << "sampler _tex_" << *args[i]->name;
       hlsl << " : register (s" << texcoord++ << ");\n";
    } else if (args[i]->isArray()) {
       hlsl << "sampler " << *args[i]->name;
       hlsl << " : register (s" << texcoord++ << ");\n";
       hlsl << "float2 " << *args[i]->name << "_scale" 
            << " : register (c" << constreg++ << ");\n";
    } else {
       args[i]->print(hlsl, true);
       hlsl << " : register (c" << constreg++ << ");\n";
    }
  }
  hlsl << "\n";

  hlsl << "float4 main (\n\t\t";

  /* Print the argument list */
  texcoord = 0;
  for (i=0; i < nArgs; i++) {
     /* Don't put the output in the argument list */
     if ((args[i]->form->getQualifiers() & TQ_Out)!=0) {
        outArg = args[i];
        continue;
     }

     if (args[i]->isStream()) {
       if (i) hlsl <<  ",\n\t\t";
       hlsl << "float2 _tex_" << *args[i]->name << "_pos : TEXCOORD"
            << texcoord++;
     }
  }
  hlsl << ") : COLOR0 {\n";

  /* Declare the stream variables */
  for (i=0; i < nArgs; i++) {
     if (args[i] == outArg) {
        TypeQual qualifier = outArg->form->getBase()->qualifier;
        outArg->form->getBase()->qualifier &= ~TQ_Out;
        hlsl << "\t";
        args[i]->form->printBase(hlsl, 0);
        hlsl << " " << *args[i]->name << ";\n";
        outArg->form->getBase()->qualifier = qualifier;
     } else if (args[i]->isStream()) {
        hlsl << "\t";
        args[i]->form->printBase(hlsl, 0);
        hlsl << " " << *args[i]->name << ";\n";
     }
  }

  /* Declare the output variable */
  hlsl << "\tfloat4  _OUT;\n";

  /* Perform stream fetches */
  for (i=0; i < nArgs; i++) {
     if (args[i] == outArg) continue;

     if (args[i]->isStream()) {
        int dimension = FloatDimension(args[i]->form->getBase()->typemask);
        assert(dimension > 0);

        hlsl << "\t" << *args[i]->name << " = tex2D"
             << "(_tex_" << *args[i]->name << ", _tex_" << *args[i]->name
             << "_pos);\n";
     }
  }

  /* Include the body of the kernel */
  hlsl << body << std::endl;

  /* Return the result */
  if (outArg) {
     int dimension = FloatDimension(outArg->form->getBase()->typemask);

     for (i=0; i < dimension; i++) {
        hlsl << "\t_OUT." << xyzw[i] << "= " << *outArg->name << "."
           << xyzw[i] << ";\n";
     }
     for (;i < 4; i++) {
        hlsl << "\t_OUT." << xyzw[i] << "= " << *outArg->name << "."
           << xyzw[dimension-1] << ";\n";
     }

     hlsl << "\treturn _OUT;\n}\n";
  }

  return strdup(hlsl.str().c_str());
}


/*
 * compile_cg_code --
 *
 *      Takes CG code and runs it through the CG compiler (and parses the
 *      results) to produce the corresponding fragment program.
 */

static char *
compile_cg_code (char *cgcode) {

  char *argv[16] = { "cgc", "-profile", "fp30", "-quiet", NULL };
  char *fpcode, *endline;

  /* Only support fp30 right now */
  assert (globals.target == TARGET_FP30);

  fpcode = Subprocess_Run(argv, cgcode);
  if (fpcode == NULL) {
     return NULL;
  }

  /* Tolerate CRLF or LF line endings. */
  endline = strstr (fpcode, "\nEND\n");
  if (!endline) {
     endline = strstr (fpcode, "\nEND\r\n");
  }
  if (!endline) {
     fprintf(stderr, "Unable to parse returned CG Code: [35;1m%s[0m\n",
             fpcode);
     return NULL;
  }

  endline += strlen("\nEND");
  *endline = '\0';
  endline++;
  fprintf(stderr, "***Summary information from cgc:\n");
  fwrite (endline, strlen(endline), 1, stderr);
  return fpcode;
}

/*
 * compile_hlsl_code --
 *
 *      Takes HLSL code and runs it through the FXC compiler (and parses the
 *      results) to produce the corresponding fragment program.
 */

static char *
compile_hlsl_code (char *hlslcode) {

  char *argv[] = { "fxc", "/Tps_2_0", "/nologo", 0, 0, NULL };
  char *fpcode,  *errcode;

  char *fname = tmpnam(NULL);
  if (fname == NULL) {
    fprintf (stderr, "Unable to get tmp file name\n");
    return NULL;
  }
  FILE *fp = fopen (fname, "wb+");
  if (fp == NULL) {
    fprintf (stderr, "Unable to open tmp file %s\n", fname);
    return NULL;
  }
  fwrite(hlslcode, sizeof(char), strlen(hlslcode), fp);
  fclose(fp);
  
  argv[3] = (char *) malloc (strlen("/Fc.ps") + strlen(fname) + 1);
  sprintf (argv[3], "/Fc%s.ps", fname);
  argv[4] = fname;
  errcode = Subprocess_Run(argv, NULL);
  remove(fname);
  if (errcode == NULL) {
     return NULL;
  }

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
     
#if 1
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
#endif

    fpcode[pos++] = (char) i;
  }  
  fpcode[pos] = '\0';

  free(argv[3]);
#if 0
  //daniel: ==20000==   //looks like this is the result of mktemp which stores its result in some static place
  //at 0x4002CD17: free (vg_replace_malloc.c:231)
  // by 0x807BBD9: compile_hlsl_code(char*) (codegen.cpp:338)
  //by 0x807C742: CodeGen_HLSLGenerateCode(Type*, char const*, Decl**, int, char const*) (codegen.cpp:577)
  //by 0x807DBEF: BRTGPUKernelDef::printCode(std::ostream&) const (brtstemnt.cpp:186)
  //Address 0x404CAC98 is not stack'd, malloc'd or free'd
  free(argv[4]);
#endif
  return fpcode;
}


/*
 * append_argument_information --
 *
 *      Takes the fp code from the CG compiler and tacks on high level
 *      information from the original kernel function at the bottom.
 */

static char *
append_argument_information (const char *commentstring, char *fpcode,
                             Decl **args, int nArgs, const char *body)
{
  std::ostringstream fp;

  fp << fpcode;

  /* Add the brcc flag */
  fp << " \n" << commentstring << "!!BRCC\n";

  /* Include the program aguments */
  fp << commentstring << "narg:" << nArgs << std::endl;

  /* Add the argument information */
  for (int i=0; i < nArgs; i++) {
     char type;
     int dimension = FloatDimension(args[i]->form->getBase()->typemask);

     if ((args[i]->form->getQualifiers() & TQ_Out)!=0) {
        type = 'o';
     } else if (args[i]->isStream()) {
        type = 's';
     } else {
        type = 'c';
     }

     fp << commentstring << type << ":" << dimension 
        << ":" << *args[i]->name << "\n";
  }

  fp << commentstring << "workspace:" << globals.workspace << std::endl;

  return strdup(fp.str().c_str());
}


/*
 * generate_c_fp30_code --
 *
 *      Spits out the compiled fp code as a string available to the emitted
 *      C code.
 */

static char *
generate_c_fp30_code(char *fpcode, const char *name)
{
  std::ostringstream fp;
  int i;

  assert (name);

  if (fpcode == NULL) {
     fp << "\nstatic const char *__" << name << "_fp30 = NULL;\n";
     return strdup(fp.str().c_str());
  }

  fp << "\nstatic const char __" << name << "_fp30[] = {" << std::endl;

  fp << "\"";
  while ((i = *fpcode++) != '\0') {
    if (i == '\n')
      fp << "\\n\"\n\"";
    else
      fp << (char) i;
  }
  fp << "\"};\n";

  return strdup(fp.str().c_str());
}

/*
 * generate_c_ps20_code --
 *
 *      Spits out the compiled pixel shader
 *      code as a string available to the emitted
 *      C code.
 */

static char *
generate_c_ps20_code(char *fpcode, const char *name)
{
  std::ostringstream fp;
  int i;

  assert (name);

  if (fpcode == NULL) {
     fp << "\nstatic const char *__" << name << "_ps20 = NULL;\n";
     return strdup(fp.str().c_str());
  }

  fp << "\nstatic const char __" << name << "_ps20[] = {" << std::endl;

  fp << "\"";
  while ((i = *fpcode++) != '\0') {
    if (i == '\n')
      fp << "\\n\"\n\"";
    else
      fp << (char) i;
  }
  fp << "\"};\n";

  return strdup(fp.str().c_str());
}


/*
 * CodeGen_GenerateHeader --
 *
 *      Takes a kernel function's prototype and emits the C header
 *      information to call it.
 */

char *
CodeGen_GenerateHeader(Type *retType, const char *name, Decl **args, int nArgs)
{
  std::ostringstream fp;

  assert (globals.houtputname);
  assert (retType);
  assert (name);
  assert (args);

  fp << "#ifndef _HEADER_" << name << std::endl;
  fp << "#define _HEADER_" << name << std::endl << std::endl;

  retType->printType(fp, NULL, true, 0);
  fp << " " << name << " (";

  for (int i = 0; i < nArgs; i++) {
     if (i) fp << ", ";

     if (args[i]->isStream()) {
        fp << "stream ";
     } else {
        args[i]->form->printBase(fp, 0);
        args[i]->form->printBefore(fp, NULL, 0);
     }
  }
  fp << ");\n\n";

#if 0
  /*
   *  I don't know anything about reduce kernels and these hardcoded
   * parameter positional meanings don't mean anything to me.  So, I'm
   * ignoring them for now.  -Jeremy.
   */

  /*
   * We really should check to make sure that
   * the kernel is legal reduction.
   */

  fp << retType << " " << name << "_reduce (";
  fp << "stream arg_src,";
  fp << arglist->args[2]->type << " *arg_out";

  for (i=3; i<arglist->len; i++) {
    struct argtype *arg = arglist->args[i];
    if (i) fp << ",";

    if (arg->modifier & MODIFIER_STREAM ||
	arg->modifier & MODIFIER_GATHER)
      fp << "stream";
    else
      fp << arg->type;
   }
  fp << ");\n\n";
  fp << "#endif _HEADER_" << name << std::endl;
#endif

  return strdup(fp.str().c_str());
}


/*
 * CodeGen_FP30GenerateCode --
 *
 *      Takes a parsed kernel and crunches it down to C code:
 *              . Creates and annotates equivalent CG
 *              . Compiles the CG to fp30 assembly
 *              . Spits out the fragment program as a C string
 *
 *      Note: The caller is responsible for free()ing the returned string.
 */

char *
CodeGen_FP30GenerateCode(Type *retType, const char *name,
                         Decl **args, int nArgs, const char *body)
{
  char *cgcode, *fpcode, *fpcode_with_brccinfo, *c_code;

  cgcode = generate_cg_code(args, nArgs, body);
  if (globals.verbose)
    std::cerr << "\n***Produced this cgcode:\n" << cgcode << "\n";

  fpcode = compile_cg_code(cgcode);
  free(cgcode);

  if (fpcode) {
    if (globals.verbose)
      std::cerr << "***Produced this fpcode:\n" << fpcode << "\n";
     fpcode_with_brccinfo =
       append_argument_information("##", fpcode, args, nArgs, body);
     free(fpcode);
     if (globals.verbose)
       std::cerr << "***Produced this instrumented fpcode:\n"
                 << fpcode_with_brccinfo << "\n";
  } else {
     fpcode_with_brccinfo = NULL;
  }

  c_code = generate_c_fp30_code(fpcode_with_brccinfo, name);
  free(fpcode_with_brccinfo);
  if (globals.verbose)
    std::cerr << "***Produced this C code:\n" << c_code;

  return c_code;
}


/*
 * CodeGen_PS20GenerateCode --
 *
 *      Takes a parsed kernel and crunches it down to C code:
 *              . Creates and annotates equivalent HLSL
 *              . Compiles the HLSL to ps20 assembly
 *              . Spits out the fragment program as a C string
 *
 *      Note: The caller is responsible for free()ing the returned string.
 */

char *
CodeGen_PS20GenerateCode(Type *retType, const char *name,
                         Decl **args, int nArgs, const char *body)
{
  char *hlslcode, *fpcode, *fpcode_with_brccinfo, *c_code;

  hlslcode = generate_hlsl_code(args, nArgs, body);

  if (globals.verbose)
    std::cerr << "\n***Produced this hlslcode:\n" << hlslcode << "\n";

  fpcode = compile_hlsl_code(hlslcode);
  free(hlslcode);

  if (fpcode) {
   
    if (globals.verbose)
      std::cerr << "***Produced this fpcode:\n" << fpcode << "\n";

     fpcode_with_brccinfo =
       append_argument_information("//", fpcode, args, nArgs, body);
     free(fpcode);

     if (globals.verbose)
       std::cerr << "***Produced this instrumented fpcode:\n"
                 << fpcode_with_brccinfo << "\n";
  } else {
     fpcode_with_brccinfo = NULL;
  }

  c_code = generate_c_ps20_code(fpcode_with_brccinfo, name);
  free(fpcode_with_brccinfo);

  if (globals.verbose)
    std::cerr << "***Produced this C code:\n" << c_code;

  return c_code;
}

