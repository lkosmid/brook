/*
 * codegen.cpp --
 *
 *      Codegen takes already parsed brook input and produces the resulting
 *      .[ch] files required to invoke it.  It converts to CG and feeds that
 *      through a CG compiler.
 */

#include <sstream>
#include <iomanip>

extern "C" {
#include <stdio.h>
#include <string.h>
#include <assert.h>
}

#include "main.h"
#include "decl.h"
#include "subprocess.h"

static const char fp30_assist[] =
#include "v01/fp30_assist.cg.bin"
;

/*
 * CodeGen_CheckSemantics --
 *
 *      Takes a kernel function and does semantic checking.
 */

void
CodeGen_CheckSemantics (Type *retType, Decl **args, int nArgs) {
  Decl *outArg = NULL;

  for (int i = 0; i < nArgs; i++) {
     BaseTypeSpec baseType;

     if (args[i]->form->getQualifiers() & TQ_Out) {
        if (outArg) {
           std::cerr << "Multiple outputs not supported: ";
           outArg->print(std::cerr, true);
           std::cerr << ", ";
           args[i]->print(std::cerr, true);
           std::cerr << ".\n";
           exit(1);

        }
        outArg = args[i];

        if (!args[i]->isStream()) {
           std::cerr << "Output is not a stream: ";
           args[i]->print(std::cerr, true);
           std::cerr << ".\n";
           exit(1);
        }
     }

     baseType = args[i]->form->getBase()->typemask;
     if (baseType < BT_Float || baseType > BT_Float4) {
        std::cerr << "Illegal type in ";
        args[i]->print(std::cerr, true);
        std::cerr << ". (Must be floatN).\n";
        exit(1);
     }
  }

  /* check kernel return type */
  if (!retType->isBaseType() || ((BaseType *) retType)->typemask != BT_Void) {
     std::cerr << "Illegal kernel return type: " << retType
               << ". Must be void.\n";
     exit(1);
  }

  if (outArg == NULL) {
     std::cerr << "Warning, kernel has no output.\n";
  }
}


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
  int texcoord;

  if (globals.target != TARGET_FP30) {
    fprintf (stderr, "Only FP30 target supported\n");
    exit(1);
  }

  cg << "#define _WORKSPACE " << globals.workspace << std::endl;
  cg << "#define _WORKSPACE_INV " << std::setprecision(9.9) <<
        1.0 / globals.workspace << std::endl;

  cg << fp30_assist;

  cg << "fragout_float main (\n\t\t";

  /* Print the argument list */
  texcoord = 0;
  for (int i=0; i < nArgs; i++) {
     /* Don't put the output in the argument list */
     if (args[i]->form->getQualifiers() & TQ_Out) {
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

  /* Tolerate CRLF or LF line endings. */
  endline = strstr (fpcode, "\nEND\n");
  if (!endline) {
     endline = strstr (fpcode, "\nEND\r\n");
  }
  if (!endline) {
     fprintf(stderr, "Unable to parse returned CG Code: [35;1m%s[0m\n",
             fpcode);
     exit(1);
  }

  endline += strlen("\nEND");
  *endline = '\0';
  endline++;
  fprintf(stderr, "***Summary information from cgc:\n");
  fwrite (endline, strlen(endline), 1, stderr);
  return fpcode;
}

/*
 * append_argument_information --
 *
 *      Takes the fp code from the CG compiler and tacks on high level
 *      information from the original kernel function at the bottom.
 */

static char *
append_argument_information (char *fpcode,
                             Decl **args, int nArgs, const char *body)
{
  std::ostringstream fp(fpcode);

  /* Add the brcc flag */
  fp << "\n#!!BRCC\n";

  /* Include the program aguments */
  fp << "#narg:" << nArgs << std::endl;

  /* Add the argument information */
  for (int i=0; i < nArgs; i++) {
     char type;
     int dimension = FloatDimension(args[i]->form->getBase()->typemask);

     if (args[i]->form->getQualifiers() & TQ_Out) {
        type = 'o';
     } else if (args[i]->isStream()) {
        type = 's';
     } else {
        type = 'c';
     }

     fp << "#" << type << ":" << dimension << ":" << *args[i]->name << "\n";
  }

  fp << "#workspace:" << globals.workspace << std::endl;

  return strdup(fp.str().c_str());
}


/*
 * generate_c_fp_code --
 *
 *      Spits out the compiled fp code as a string available to the emitted
 *      C code.
 */

static char *
generate_c_fp_code(char *fpcode, const char *name)
{
  std::ostringstream fp;
  int i;

  assert (name);

  fp << "\nstatic const char " << name << "_fp[] =" << std::endl;

  fp << "\"";
  while ((i = *fpcode++) != '\0') {
    if (i == '\n')
      fp << "\\n\"\n\"";
    else
      fp << (char) i;
  }
  fp << "\";\n\n";

  return strdup(fp.str().c_str());
}


/*
 * CodeGen_GenerateStub --
 *
 *      Spits out a small wrapper function to setup the runtime and then
 *      invoke the kernel.
 */

char *
CodeGen_GenerateStub(Type *retType, const char *name, Decl **args, int nArgs)
{
  std::ostringstream cStub;
  int i;

  cStub << "static int __" << name << "_p = 0;\n\n";

  retType->printType(cStub, NULL, true, 0);
  cStub << " " << name << " (";

  for (i = 0; i < nArgs; i++) {
     if (i) cStub << ",\n\t";

     if (args[i]->isStream()) {
        cStub << "stream arg_" << i;
     } else {
        args[i]->form->printBase(cStub, 0);
        args[i]->form->printBefore(cStub, NULL, 0);
        cStub << " arg_" << i;
     }
  }
  cStub << ") {\n";

  cStub << "  if (!__" << name << "_p) __" << name << "_p = LoadKernelText("
     << name << "_fp);\n";
  cStub << "  KernelMap (p, ";
  for (i=0; i < nArgs; i++) {
    if (i) cStub << ", ";
    cStub << "arg_" << i;
  }
  cStub << ");\n}\n\n";

#if 0
  /*
   *  I don't know anything about reduce kernels and these hardcoded
   * parameter positional meanings don't mean anything to me.  So, I'm
   * ignoring them for now.  -Jeremy.
   */

  /* XXX is kernel a legal reduce kernel ? */

  cStub << retType << " " << name << "_reduce (";
  cStub << "stream arg_src,";
  cStub << arglist->args[2]->type << " *arg_out";

  for (i=3; i<arglist->len; i++) {
    struct argtype *arg = arglist->args[i];
    if (i) cStub << ",\n\t";
    if (arg->modifier & MODIFIER_OUT)
      cStub << arg->type << " *arg_" << i;
    else if (arg->modifier & MODIFIER_STREAM ||
	     arg->modifier & MODIFIER_GATHER)
      cStub << "stream arg_" << i;
    else
      cStub << arg->type << " arg_" << i;
   }
  cStub << ") {\n";

  cStub << "  if (!p) p = LoadKernelText(" << name << "_fp);\n";
  cStub << "  KernelReduce (p, arg_src, arg_out";
  for (i=3; i<arglist->len; i++) {
    if (i) cStub << ",";
    if (!arglist->args[i]->modifier)
      cStub << "&";
    cStub << "arg_" << i;
  }
  cStub << ");\n}\n\n";
#endif

  return strdup(cStub.str().c_str());
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
   * See the comment about reduce kernels in generate_c_code().
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
 * CodeGen_GenerateCode --
 *
 *      Takes a parsed kernel and crunches it down to C code:
 *              . Creates and annotates equivalent CG
 *              . Compiles the CG
 *              . Spits out the fragment program as a C string
 *
 *      Note: The caller is responsible for free()ing the returned string.
 */

char *
CodeGen_GenerateCode(Type *retType, const char *name,
                     Decl **args, int nArgs, const char *body)
{
  char *cgcode, *fpcode, *fpcode_with_brccinfo, *c_code;

  cgcode = generate_cg_code(args, nArgs, body);
  //std::cerr << "\n***Produced this cgcode:\n" << cgcode << "\n";

  fpcode = compile_cg_code(cgcode);
  free(cgcode);
  //std::cerr << "***Produced this fpcode:\n" << fpcode << "\n";

  fpcode_with_brccinfo = append_argument_information(fpcode, args, nArgs, body);
  free(fpcode);
  //std::cerr << "***Produced this instrumented fpcode:\n"
  //          << fpcode_with_brccinfo << "\n";

  c_code = generate_c_fp_code(fpcode_with_brccinfo, name);
  free(fpcode_with_brccinfo);
  //std::cerr << "***Produced this C code:\n" << c_code;

  return c_code;
}

