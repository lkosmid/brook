/*
 * main.c
 *
 *      Minor bit of code to drive the whole program.  Nothing interesting
 *      should happen here.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "getopt.h"

#include "main.h"
#include "ctool.h"

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
  fprintf (stderr, "brcc [-v] [-c | -f] [-o outputfileprefix] [-w workspace] [-p fp30|arb] foo.br\n");

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
  globals.target       = TARGET_FP30;
  globals.workspace    = 1024;
  globals.compilername = argv[0];

  while ((opt = getopt(argc, argv, "cfo:p:vw:")) != EOF) {
     switch(opt) {
     case 'c':
	if (globals.fponly) usage();
	globals.cgonly = 1;
	break;
     case 'f':
	if (globals.cgonly) usage();
	globals.fponly = 1;
	break;
     case 'o':
	if (outputprefix) usage();
	outputprefix = strdup(optarg);
	break;
     case 'p':
	if (strcmp (optarg, "fp30") == 0)
	  globals.target = TARGET_FP30;
	else if (strcmp (optarg, "arb") == 0)
	  globals.target = TARGET_ARB;
	else
	  usage();
	break;
     case 'v':
	globals.verbose = 1;
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
  }

  argv += optind;
  argc -= optind;
  /* if (argc < 1) usage(); */
  globals.sourcename = argc < 1 ? "toy.br" : argv[0];

  n = strlen(globals.sourcename);
  if (n < 4 || strcmp (globals.sourcename + n - 3, ".br"))
    usage();

  if (!outputprefix) {
    outputprefix = strdup(globals.sourcename);
    outputprefix[n-3] = (char)  '\0';
  }

  globals.cgoutputname = (char *) malloc (strlen(outputprefix) +
					  strlen(".cg") + 1);
  sprintf (globals.cgoutputname, "%s.cg",outputprefix);

  if (globals.target == TARGET_FP30) {
    globals.fpoutputname = (char *) malloc (strlen(outputprefix) +
					    strlen(".fp") + 1);
    sprintf (globals.fpoutputname, "%s.fp",outputprefix);
  } else {
    assert (globals.target == TARGET_ARB);
    globals.fpoutputname = (char *) malloc (strlen(outputprefix) +
					    strlen(".arb") + 1);
    sprintf (globals.fpoutputname, "%s.arb",outputprefix);
  }

  globals.coutputname = (char *) malloc (strlen(outputprefix) +
					 strlen(".c") + 1);
  sprintf (globals.coutputname, "%s.c",outputprefix);
  globals.houtputname = (char *) malloc (strlen(outputprefix) +
					 strlen(".h") + 1);
  sprintf (globals.houtputname, "%s.h",outputprefix);

  free(outputprefix);
}


/*
 * ConvertToBrtStream --
 *
 *      Converts stream declaration statement objects into BrtStreams.
 */

void ConvertToBrtStream (Statement *stemnt) {
  if (!stemnt->isDeclaration())
    return;
  DeclStemnt *declstemnt = (DeclStemnt *)stemnt;

  for (unsigned int i=0; i<declstemnt->decls.size(); i++) {
    Decl *decl = declstemnt->decls[i];
    if (!decl->form) continue;
    Type *form = decl->form;
    if (form->type != TT_Stream) continue;
    ArrayType *tarray = (ArrayType *) form;

    BrtStreamType *brtStream = new BrtStreamType (tarray);
    decl->form = brtStream;
    assert (decl->initializer == NULL);

    decl->initializer = new BrtStreamInitializer(brtStream,
						 stemnt->location);
  }
}


/*
 * main --
 *
 *      Drive everything.  Parse the arguments, the compile the requested
 *      file.
 */

int
main(int argc, char *argv[])
{
   Project *proj;
   TransUnit *tu;

   parse_args(argc, argv);
   std::cerr << "***Compiling " << globals.sourcename << "\n";

   proj = new Project();
   tu = proj->parse(globals.sourcename, false, NULL, false, NULL, NULL, NULL);
   if (tu) {

     // Convert Stream Declarations into brt decls
     tu->findStemnt (ConvertToBrtStream);

     std::cout << *tu << std::endl;
   } else {
      std::cout << "Unable to parse " << globals.sourcename << std::endl;
      exit(1);
   }

   delete proj;
   exit(0);
   return 0;    /* Appease CL */
}
