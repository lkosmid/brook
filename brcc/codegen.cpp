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
#include <fcntl.h>

#ifdef WIN32
#include <process.h>
#include <io.h>
#else
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif
}

#include "main.h"
#include "decl.h"
#include "project.h"

#define READ_HANDLE 0
#define WRITE_HANDLE 1

static const char legaltypes[][16] = { "float", "float2", "float3", "float4" };
static const char fp30_assist[] =
#include "v01/fp30_assist.cg.bin"
;

/*
 * check_semantics --
 *
 *      Takes a kernel function and does semantic checking.
 */

static void
check_semantics (Type *retType, Decl **args, int nArgs) {
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

  bool debug = Project::gDebug;
  int hStdOut;
  int hStdOutPipe[2];
  int hStdIn;
  int hStdInPipe[2];

  int pid;

  int ret;

  char *fpcode;
  int  fpcode_alloc;
  int  fpcode_pos;
  char buf[1024];
  char *endline;

  /* Only support fp30 right now */
  assert (globals.target == TARGET_FP30);

  // Create the pipe
#ifdef WIN32
  if(_pipe(hStdOutPipe, 512, O_TEXT | O_NOINHERIT) == -1) {
    fprintf (stderr, "Unable to create pipe\n");
    exit (1);
  }
  if(_pipe(hStdInPipe, 512, O_TEXT | O_NOINHERIT) == -1) {
    fprintf (stderr, "Unable to create pipe\n");
    exit (1);
  }

  // Save the current stdout/stdin handles
  hStdOut = _dup(_fileno(stdout));
  hStdIn  = _dup(_fileno(stdin));

  // Duplicate the pipes
  if(_dup2(hStdOutPipe[WRITE_HANDLE], _fileno(stdout)) != 0) {
    fprintf (stderr, "Unable to redirect stdout\n");
    exit (1);
  }
  if(_dup2(hStdInPipe[READ_HANDLE], _fileno(stdin)) != 0) {
    fprintf (stderr, "Unable to redirect stdin\n");
    exit (1);
  }

#else
  if(pipe(hStdOutPipe) == -1) {
       fprintf (stderr, "Unable to create pipe\n");
       exit (1);
  }
  if(pipe(hStdInPipe) == -1) {
    fprintf (stderr, "Unable to create pipe\n");
    exit (1);
  }
  if((hStdOut = dup(fileno(stdout)))  == -1 ){
       fprintf (stderr, "dup stdout\n");
       exit (1);
  }
  if((hStdIn  = dup(fileno(stdin))) == -1 ){
       fprintf (stderr, "dup stdin\n");
       exit (1);
  };
  if( dup2(hStdOutPipe[WRITE_HANDLE], fileno(stdout)) == -1 ){
       fprintf (stderr, "Unable to redirect stdout\n");
       exit (1);
  }
  if( dup2(hStdInPipe[READ_HANDLE], fileno(stdin)) == -1) {
       fprintf (stderr, "Unable to redirect stdout\n");
       exit (1);
  }
#endif

  // Close our side of the pipes.
  // Cgc will be writing/reading these pipes
  if(close(hStdOutPipe[WRITE_HANDLE]) !=0){
       fprintf(stderr, "Write close error\n");
  }
  if(close(hStdInPipe[READ_HANDLE]) !=0){
       fprintf(stderr, "Write close error\n");
  }

#if WIN32

  /* Excute the compiler */
  if( ( pid = _spawnlp( P_NOWAIT, "cgc", "cgc", "-profile",
			"fp30", "-quiet", NULL)) == -1) {
    fprintf( stderr, "Unable to start cgc\n" );
    exit(1);
  }

#else

  if ((pid = fork()) == 0) {
       close (hStdInPipe[WRITE_HANDLE]);

       if (execlp( "cgc", "cgc", "-profile",
		   "fp30", "-quiet", NULL) == -1) {
	    fprintf( stderr, "Unable to start cgc\n" );
	    exit(1);
       }
       /* Unreached... */
  } else{
       fprintf(stderr, "Child has pid %d\n", pid);
  }

#endif


#if WIN32
  // Restore the pipes for us.
  if(_dup2(hStdOut, _fileno(stdout)) != 0) {
    fprintf (stderr, "Unable to restore stdout\n");
    exit (1);
  }
  if(_dup2(hStdIn, _fileno(stdin)) != 0) {
    fprintf (stderr, "Unable to restore stdin\n");
    exit (1);
  }
#else
  if(dup2(hStdOut, fileno(stdout)) == -1) {
       fprintf (stderr, "Unable to restore stdout\n");
       exit (1);
  }
  if( dup2(hStdIn, fileno(stdin)) == -1) {
       fprintf (stderr, "Unable to restore stdin\n");
       exit (1);
  }
#endif


  // Close the remaining pipes
  if(close(hStdOut) !=0){
       fprintf(stderr, "Write close error\n");
  }
  if(close(hStdIn) !=0){
       fprintf(stderr, "Write close error\n");
  }

  /* Feed the cg code to the compiler */
  if (debug) fprintf(stderr, "Sending CG code to the compiler\n");
#if WIN32
  _write (hStdInPipe[WRITE_HANDLE], cgcode, strlen(cgcode));
#else
  {
       char eof_holder = EOF;

       /* fprintf(stderr, "Writing\n[35;1m%s[0m\n", cgcode); */
       if (debug) fprintf(stderr, "Writing...\n");
       if(write (hStdInPipe[WRITE_HANDLE], cgcode, strlen(cgcode))
	  != strlen(cgcode)){
	    perror("Write problem: ");
       }
       write(hStdInPipe[WRITE_HANDLE], &eof_holder, 1);
  }
#endif
  if (debug) fprintf(stderr, "Closing pipe to cgc\n");
  if(close(hStdInPipe[WRITE_HANDLE]) !=0){
       fprintf(stderr, "Write close error\n");
  }

  fpcode = (char *) malloc (1024);
  fpcode_alloc = 1024;
  fpcode_pos   = 0;

  while (1) {
    if (debug) fprintf(stderr, "Reading pipe from cgc...\n");
#ifdef WIN32
    ret = _read(hStdOutPipe[READ_HANDLE], buf, 1023);
#else
    ret = read(hStdOutPipe[READ_HANDLE], buf, 1023);
#endif

    if (ret == 0) {
      if (debug) fprintf(stderr, "Got everything from cgc.\n");
      break;
    } else if (ret == -1) {
      fprintf (stderr, "Error reading output compiler pipe.\n");
      exit(1);
    }
    buf[ret]='\0';

    if (debug) fprintf(stderr, "Read %d bytes\n", ret);
    /*fprintf(stderr, "Read %d bytes '%s'\n", ret, buf);*/
    while (fpcode_alloc - fpcode_pos  <= ret) {
      fpcode = (char *) realloc (fpcode, fpcode_alloc*2);
      fpcode_alloc *= 2;
    }
    memcpy (fpcode + fpcode_pos, buf, ret);
    fpcode_pos += ret;
  }
  fpcode[fpcode_pos] = '\0';

#if WIN32
  /* Wait for cgc to be done, get the return value */
  _cwait(&ret, pid, 0);
#else
  waitpid(pid, &ret, 0);
  ret = WIFEXITED(ret) ? 0 : WEXITSTATUS(ret);
#endif
  if (ret != 0) {
    fprintf (stderr, "cgc exited with an error:\n");
    fwrite (fpcode, strlen(fpcode), 1, stderr);
    exit(1);
  }

  close (hStdOutPipe[READ_HANDLE]);

  /* Tolerate CRLF or LF line endings. */
  endline = strstr (fpcode, "\nEND\n");
  if (!endline) {
     endline = strstr (fpcode, "\nEND\r\n");
  }
  if (!endline) {
       fprintf (stderr, "Unable to parse returned CG Code: [35;1m%s[0m\n",
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
 * generate_c_code --
 *
 *      Takes completely processed fp code and emits a C wrapper to invoke
 *      it.
 */

static char *
generate_c_code(char *fpcode, Type *retType, const char *name,
                Decl **args, int nArgs)
{
  std::ostringstream fp;
  int i;

  assert (globals.coutputname);
  assert (retType);
  assert (name);
  assert (args);

  fp << "#include<brook.h>\n\n";

  fp << "static const char " << name << "_fp[] = " << std::endl;

  fp << "\"";
  while ((i = *fpcode++) != '\0') {
    if (i == '\n')
      fp << "\\n\"\n\"";
    else
      fp << (char) i;
  }
  fp << "\";\n\n";

  fp << "static int p = 0;\n\n";

  retType->printType(fp, NULL, true, 0);
  fp << " " << name << " (";

  for (i = 0; i < nArgs; i++) {
     if (i) fp << ",\n\t";

     if (args[i]->isStream()) {
        fp << "stream arg_" << i;
     } else {
        args[i]->form->printBase(fp, 0);
        args[i]->form->printBefore(fp, NULL, 0);
        fp << " arg_" << i;
     }
  }
  fp << ") {\n";

  fp << "  if (!p) p = LoadKernelText(" << name << "_fp);\n";
  fp << "  KernelMap (p, ";
  for (i=0; i < nArgs; i++) {
    if (i) fp << ", ";
    fp << "arg_" << i;
  }
  fp << ");\n}\n\n";

#if 0
  /*
   *  I don't know anything about reduce kernels and these hardcoded
   * parameter positional meanings don't mean anything to me.  So, I'm
   * ignoring them for now.  -Jeremy.
   */

  /* XXX is kernel a legal reduce kernel ? */

  fp << retType << " " << name << "_reduce (";
  fp << "stream arg_src,";
  fp << arglist->args[2]->type << " *arg_out";

  for (i=3; i<arglist->len; i++) {
    struct argtype *arg = arglist->args[i];
    if (i) fp << ",\n\t";
    if (arg->modifier & MODIFIER_OUT)
      fp << arg->type << " *arg_" << i;
    else if (arg->modifier & MODIFIER_STREAM ||
	     arg->modifier & MODIFIER_GATHER)
      fp << "stream arg_" << i;
    else
      fp << arg->type << " arg_" << i;
   }
  fp << ") {\n";

  fp << "  if (!p) p = LoadKernelText(" << name << "_fp);\n";
  fp << "  KernelReduce (p, arg_src, arg_out";
  for (i=3; i<arglist->len; i++) {
    if (i) fp << ",";
    if (!arglist->args[i]->modifier)
      fp << "&";
    fp << "arg_" << i;
  }
  fp << ");\n}\n\n";
#endif

  return strdup(fp.str().c_str());
}


/*
 * generate_header_file --
 *
 *      Takes a kernel functions prototype and emits the C header
 *      information to call it.
 */

static char *
generate_header_file (Type *retType, const char *name, Decl **args, int nArgs)
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
 * dump_to_file --
 *
 *      SImple helper function that dumps the speicifed string to a file of
 *      the specified name.
 */

static void
dump_to_file (char *fname, char *s) {
  FILE *fp;
  fp = fopen (fname, "wt");
  if (!fp) {
    fprintf (stderr, "Unable to open file %s for writing\n",
	     fname);
    exit(1);
  }
  fwrite (s, strlen(s), 1, fp);
  fclose(fp);
}


/*
 * newkernel --
 *
 *      Entry point to codegen.  This function takes the prototype and
 *      kernel body and drives the whole process through to dumping a .c and
 *      .h file to be linked and included to invoke the kernel.
 */

void
CodeGen_Kernel(Type *retType, const char *name,
               Decl **args, int nArgs, const char *body)
{
  char *cgcode, *fpcode, *fpcode_with_brccinfo, *c_code, *h_code;

  check_semantics(retType, args, nArgs);

  cgcode = generate_cg_code(args, nArgs, body);
  std::cerr << "\n***Produced this cgcode:\n" << cgcode << "\n";
  if (globals.cgonly) {
    dump_to_file(globals.cgoutputname, cgcode);
    free(cgcode);
    return;
  }

  fpcode = compile_cg_code(cgcode);
  free(cgcode);
  std::cerr << "***Produced this fpcode:\n" << fpcode << "\n";

  fpcode_with_brccinfo = append_argument_information(fpcode, args, nArgs, body);
  free(fpcode);
  std::cerr << "***Produced this instrumented fpcode:\n"
            << fpcode_with_brccinfo << "\n";

  if (globals.fponly) {
    dump_to_file(globals.fpoutputname, fpcode_with_brccinfo);
    free (fpcode_with_brccinfo);
    return;
  }

  c_code = generate_c_code(fpcode_with_brccinfo, retType, name, args, nArgs);
  free(fpcode_with_brccinfo);
  std::cerr << "***Produced this C code:\n" << c_code;
  //dump_to_file(globals.coutputname, c_code);
  free(c_code);

  h_code = generate_header_file(retType, name, args, nArgs);
  std::cerr << "***Produced this header code:\n" << h_code;
  //dump_to_file(globals.houtputname, h_code);
  free(h_code);

  std::cerr << "***\n";
}

