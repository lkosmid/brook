/*
 * main.c
 *
 *      Minor bit of code to drive the whole program.  Nothing interesting
 *      should happen here.
 */

#include <fstream>

extern "C" {
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "getopt.h"
}

#include "main.h"
#include "ctool.h"
#include "brtscatter.h"
#include "brtvout.h"
struct globals_struct globals;


/*
 * usage --
 *
 *      Dumps the legitimate commandline options and exits.
 */

static void
usage (void) {
  fprintf (stderr, "Brook CG Compiler\n");
  fprintf (stderr, "Version: 0.2  Built: %s, %s\n", __DATE__, __TIME__);
  fprintf (stderr,
        "brcc [-hvnd] [-o prefix] [-w workspace] [-p shader ] foo.br\n\n"
        "\t-h\t\thelp (print this message)\n"
        "\t-v\t\tverbose (print intermediate generated code)\n"
        "\t-n\t\tno codegen (just parse and reemit the input)\n"
        "\t-d\t\tdebug (print cTool internal state)\n"
        "\t-k\t\tkeep generated fragment program (in foo.cg)\n"
        "\t-o prefix\tprefix prepended to all output files\n"
        "\t-w workspace\tworkspace size (16 - 2048, default 1024)\n"
        "\t-p shader\tcpu / ps20 / fp30 (can specify multiple)\n"
        "\n");

  exit(1);
}


/*
 * parse_args --
 *
 *      Parses argv and sets the global options as a result.
 */

static void
parse_args (int argc, char *argv[]) {
  int opt, n;
  char *outputprefix = NULL;

  /*
   * zero initialization from the bss will take care of the rest of the
   * defaults.
   */
  globals.workspace    = 1024;
  globals.compilername = argv[0];
  while ((opt = getopt(argc, argv, "d:hkmnyo:p:vw")) != EOF) {
     switch(opt) {
     case 'h':
        usage();
        break;
     case 'k':
        globals.keepFiles = true;
        break;
     case 'm':
        globals.multiThread = true;
        break;
     case 'n':
        globals.parseOnly = true;
        break;
    // TIM: totally hacked
     case 'y':
        globals.allowDX9MultiOut = true;
        break;
     case 'o':
	if (outputprefix) usage();
	outputprefix = strdup(optarg);
	break;
     case 'p':
	if (strcasecmp (optarg, "cpu") == 0)
	  globals.target |= TARGET_CPU;
	else if (strcasecmp (optarg, "ps20") == 0)
	  globals.target |= TARGET_PS20;
	else if (strcasecmp (optarg, "fp30") == 0)
	  globals.target |= TARGET_FP30;
	else if (strcasecmp (optarg, "arb") == 0)
	  globals.target |= TARGET_ARB;
	else
	  usage();
	break;
     case 'v':
	globals.verbose = 1;
	break;
     case 'd':
        Project::gDebug = true;
        break;
     case 'w':
	globals.workspace = strtol(optarg, NULL, 0);
	if (globals.workspace < 16 ||
	    globals.workspace > 2048)
	  usage();
	break;
     default:
	usage();
     }

     // The default build targets
     if (globals.target == 0)
        globals.target = TARGET_PS20 | TARGET_CPU | TARGET_FP30;
  }

  argv += optind;
  argc -= optind;
  if (argc < 1) usage();
  globals.sourcename = (char *) argv[0];

  n = strlen(globals.sourcename);
  if (n < 4 || strcmp (globals.sourcename + n - 3, ".br"))
    usage();

  if (!outputprefix) {
    outputprefix = strdup(globals.sourcename);
    outputprefix[n-3] = (char)  '\0';
  }

  globals.shaderoutputname = (char *) malloc (strlen(outputprefix) +
                                              strlen(".cg") + 1);
  sprintf (globals.shaderoutputname, "%s.cg",outputprefix);

  globals.coutputname = (char *) malloc (strlen(outputprefix) +
					 strlen(".cpp") + 1);
  sprintf (globals.coutputname, "%s.cpp",outputprefix);
  globals.houtputname = (char *) malloc (strlen(outputprefix) +
					 strlen(".h") + 1);
  sprintf (globals.houtputname, "%s.h",outputprefix);

  free(outputprefix);
}


/*
 * ConvertToBrtStreams --
 *
 *      Converts stream declaration statement objects into BrtStreams.
 */

static void
ConvertToBrtStreams(Statement *s)
{
   DeclStemnt *declStemnt;

   if (!s->isDeclaration()) { return; }
   declStemnt = (DeclStemnt *) s;

   for (unsigned int i=0; i<declStemnt->decls.size(); i++) {
      Decl *decl = declStemnt->decls[i];
      ArrayType *stream;
      Type *brtType;

      if (!decl->form || !decl->isStream()) continue;
      stream = (ArrayType *) decl->form;

      if (decl->initializer == NULL) {
         assert((stream->getQualifiers() & TQ_Iter) == 0);
         brtType = new BrtStreamType (stream);
         // TIM: we don't want any initializer for stream types...
         // they are initialized by their constructor...
         //
         // decl->initializer = new BrtStreamInitializer(brtStream,
         //                                              declStemnt->location);
      } else {
         assert(decl->initializer->etype == ET_FunctionCall);
         assert((stream->getQualifiers() & TQ_Iter) != 0);
         brtType = new BrtIterType(stream, (FunctionCall *) decl->initializer);
         delete decl->initializer;
         decl->initializer = NULL;
      }
      assert (decl->initializer == NULL);
      decl->form = brtType;
   }
}


static void
ConvertToBrtScatters(Statement *s)
{
   s->findExpr(ConvertToBrtScatterCalls);
}


/*
 * ConvertToBrtFunctions --
 *
 *      This is the callback for the portion of the transformation phase
 *      when we iterate over all the function definitions and convert Brook
 *      specific ones to our types.
 */

static FunctionDef *
ConvertToBrtFunctions(FunctionDef *fDef)
{
   /*
    * The 'isReduce()' check _must_ come before the 'isKernel()' check
    * because, for better or for worse, reduction kernels are also
    * considered kernels.
    */
   if (fDef->decl->isReduce()) {
      return new BRTReduceKernelDef(*fDef);
   } else if (fDef->decl->isKernel()) {
      return new BRTMapKernelDef(*fDef);
   } else return NULL;

}


/*
 * main --
 *
 *      Drive everything.  Parse the arguments, then compile the requested
 *      file.
 */

int
main(int argc, char *argv[])
{
   Project *proj;
   TransUnit *tu;

   parse_args(argc, argv);
   if (globals.verbose) {
      std::cerr << "***Compiling " << globals.sourcename << "\n";
   }

   proj = new Project();
   tu = proj->parse(globals.sourcename, false, NULL, false, NULL, NULL, NULL);
   if (tu) {
      std::ofstream out;

      if (!globals.parseOnly) {
         /*
          * If I didn't mind violating some abstractions, I'd roll my own loop
          * here instead of using the Translation Unit methods.
          */
         Brook2Cpp_IdentifyIndexOf(tu);
         transform_vout(tu);
         tu->findStemnt(ConvertToBrtStreams);
         tu->findStemnt(ConvertToBrtScatters);
         tu->findFunctionDef(ConvertToBrtFunctions);
      }

     out.open(globals.coutputname);
     if (out.fail()) {
        std::cerr << "***Unable to open " << globals.coutputname << "\n";
        exit(1);
     }

     out << *tu << std::endl;
     out.close();
   } else {
      std::cerr << "***Unable to parse " << globals.sourcename << std::endl;
      exit(1);
   }

   if (globals.verbose) {
      std::cerr << "***Successfully compiled " << globals.sourcename << "\n";
   }
   delete proj;
   exit(0);
   return 0;    /* Appease CL */
}
