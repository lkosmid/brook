/*
 * codegen.cpp --
 *
 *      Codegen takes already parsed brook input and produces the resulting
 *      .[ch] files required to invoke it.  It converts to CG and feeds that
 *      through a CG compiler.
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
#include "project.h"
#include "stemnt.h"
#include "brtkernel.h"
#include "b2ctransform.h"

// TIM: needed?
#include "brtdecl.h"

/*
 * generate_hlsl_code --
 *
 *      This function takes a parsed kernel function as input and produces
 *      the CG code reflected, plus the support code required.
 */
static void generate_shader_subroutines(std::ostream&  out) {
   TransUnit * tu = gProject->units.back();
   Statement *ste, *prev;
   for (ste=tu->head, prev=NULL; ste; prev = ste, ste=ste->next) {
      if (ste->isFuncDef()) {
         FunctionDef * fd = static_cast<FunctionDef *> (ste);
//TIM: I'm unsure why we don't output the reductions
//         if (fd->decl->isKernel()&&!fd->decl->isReduce()) {
         if (fd->decl->isKernel()) {
            BRTPS20KernelCode(*fd).printInnerCode(out);
         }
      }
   }
}

static Symbol* findStructureTag( Type* inType )
{
	BaseType* base = inType->getBase();
	while(true)
	{
		BaseTypeSpec mask = base->typemask;
		if( mask & BT_UserType )
		{
			base = base->typeName->entry->uVarDecl->form->getBase();
		}
		else if( mask & BT_Struct )
			return base->tag;
		else break;
	}
	return NULL;
}

static StructDef* findStructureDef( Type* inType )
{
	Symbol* tag = findStructureTag( inType );
	if( tag == NULL ) return NULL;
	return tag->entry->uStructDef->stDefn;
}

static void printShaderStructureDef( std::ostream& out, StructDef* structure )
{
	out << "struct " << structure->tag->name << " {\n";
	int fieldCount = structure->nComponents;
	for( int i = 0; i < fieldCount; i++ )
	{
		Decl* fieldDecl = structure->components[i];
		if( fieldDecl->isStatic() ) continue;
		if( fieldDecl->isTypedef() ) continue;

		out << "\t";
		fieldDecl->form->printType( out, fieldDecl->name, true, 0 );
		out << ";\n";
	}
	out << "};\n\n";
}

static void generate_shader_type_declaration( std::ostream& out, DeclStemnt* inStmt )
{
	for( DeclVector::iterator i = inStmt->decls.begin(); i != inStmt->decls.end(); ++i )
	{
		Decl* decl = *i;
		Type* form = decl->form;
		/*
		Symbol* structureTag = findStructureTag( form );
		if( structureTag != NULL ) {
			StructDef* structure = structureTag->entry->uStructDef->stDefn;
			printShaderStructureDef( out, structure );
		}*/

		if( decl->isTypedef() )
		{
			out << "typedef ";
			form->printBase(out,0);
			out << " " << decl->name->name;
			out << ";\n\n";
		}
	}
}

static void generate_shader_structure_definitions( std::ostream& out ) {
	TransUnit * tu = gProject->units.back();
	Statement *ste, *prev;
	for (ste=tu->head, prev=NULL; ste; prev = ste, ste=ste->next) {
		if(ste->isDeclaration() || ste->isTypedef())
		{
			DeclStemnt* decl = static_cast<DeclStemnt*>(ste);
			generate_shader_type_declaration( out, decl );
		}
	}
}

static void expandOutputArgumentStructureDecl(
  std::ostream& shader, const std::string& argumentName, StructDef* structure, int& ioOutputReg )
{
	assert( !structure->isUnion() );

	int elementCount = structure->nComponents;
	for( int i = 0; i < elementCount; i++ )
	{
		Decl* elementDecl = structure->components[i];
		if( elementDecl->storage & ST_Static ) continue;
		Type* form = elementDecl->form;

		// TIM: for now
		assert( form->isBaseType() );
		BaseType* base = form->getBase();
		StructDef* structure = findStructureDef( base );
		if( structure )
			expandOutputArgumentStructureDecl( shader, argumentName, structure, ioOutputReg );
		else
		{
		  // it had better be just a floatN
      shader << ",\n\t\t";
      shader << "out float4 __output_" << ioOutputReg;
      shader << " : COLOR" << ioOutputReg++;
		}
	}
}


static void expandOutputArgumentDecl(
  std::ostream& shader, const std::string& argumentName, Type* form, int& ioOutputReg )
{
  StructDef* structure = NULL;

  if( form->isStream() )
  {
	  BrtStreamType* streamType = (BrtStreamType*)(form);

	  // TIM: can't handle arrays with a BaseType
	  BaseType* elementType = streamType->getBase();
	  structure = findStructureDef( elementType );
  }
  else
  {
    assert( (form->getQualifiers() & TQ_Reduce) != 0 );
    structure = findStructureDef( form );
  }

	if( structure )
	{
		expandOutputArgumentStructureDecl( shader, argumentName, structure, ioOutputReg );
	}
	else
	{
		// it had better be just a floatN
    shader << ",\n\t\t";
    shader << "out float4 __output_" << ioOutputReg;
    shader << " : COLOR" << ioOutputReg++;
	}
}

static void expandSimpleOutputArgumentWrite(
  std::ostream& shader, const std::string& argumentName, Type* form, int outputReg )
{
  assert( form );
	BaseType* base = form->getBase();
	assert( base );

  shader << "\t__output_" << outputReg << " = ";

	switch(base->typemask) {
		case BT_Float:
			shader << "float4( " << argumentName << ", 0, 0, 0);\n";
			break;
		case BT_Float2:
			shader << "float4( " << argumentName << ", 0, 0);\n";
			break;
		case BT_Float3:
			shader << "float4( " << argumentName << ", 0);\n";
			break;
		case BT_Float4:
			shader << argumentName << ";\n";
			break;
		default:
			fprintf(stderr, "Strange stream base type:");
			base->printBase(std::cerr, 0);
			abort();
	}
}

static void expandOutputArgumentStructureWrite( std::ostream& shader, const std::string& fieldName, StructDef* structure, int& ioOutputReg )
{
	assert( !structure->isUnion() );

	int elementCount = structure->nComponents;
	for( int i = 0; i < elementCount; i++ )
	{
		Decl* elementDecl = structure->components[i];
		if( elementDecl->storage & ST_Static ) continue;
		Type* form = elementDecl->form;

		std::string subFieldName = fieldName + "." + elementDecl->name->name;

		// TIM: for now
		assert( form->isBaseType() );
		BaseType* base = form->getBase();
		StructDef* structure = findStructureDef( base );
		if( structure )
		{
			expandOutputArgumentStructureWrite( shader, subFieldName, structure, ioOutputReg );
		}
		else
		{
      expandSimpleOutputArgumentWrite( shader, subFieldName, base, ioOutputReg++ );
		}
	}
}

static void expandOutputArgumentWrite(
  std::ostream& shader, const std::string& argumentName, Type* form, int& ioOutputReg )
{
  StructDef* structure = NULL;
  Type* elementType = NULL;

  if( form->isStream() )
  {
	  BrtStreamType* streamType = (BrtStreamType*)(form);

	  // TIM: can't handle arrays with a BaseType
	  elementType = streamType->getBase();
	  structure = findStructureDef( elementType );
  }
  else
  {
    assert( (form->getQualifiers() & TQ_Reduce) != 0 );
    elementType = form;
    structure = findStructureDef( form );
  }

	if( structure )
	{
		expandOutputArgumentStructureWrite( shader, argumentName, structure, ioOutputReg );
	}
	else
	{
    expandSimpleOutputArgumentWrite( shader, argumentName, elementType, ioOutputReg++ );
	}
}

static void expandStreamStructureSamplerDecls( std::ostream& shader, const std::string& argumentName, StructDef* structure, int& ioIndex, int& ioSamplerReg )
{
	assert( !structure->isUnion() );

	int elementCount = structure->nComponents;
	for( int i = 0; i < elementCount; i++ )
	{
		Decl* elementDecl = structure->components[i];
		if( elementDecl->storage & ST_Static ) continue;
		Type* form = elementDecl->form;

		// TIM: for now
		assert( form->isBaseType() );
		BaseType* base = form->getBase();
		StructDef* structure = findStructureDef( base );
		if( structure )
			expandStreamStructureSamplerDecls( shader, argumentName, structure, ioIndex, ioSamplerReg );
		else
		{
      shader <<  ",\n\t\t";
			shader << "uniform _stype __structsampler" << ioIndex++ <<"_" << argumentName;
			shader << " : register (s" << ioSamplerReg++ << ")";
		}
	}
}

static void exandStreamSamplerDecls( std::ostream& shader, const std::string& inArgumentName, Type* inForm, int& samplerreg )
{
  StructDef* structure = NULL;

  if( inForm->isStream() )
  {
	  BrtStreamType* streamType = (BrtStreamType*)(inForm);

	  // TIM: can't handle arrays with a BaseType
	  BaseType* elementType = streamType->getBase();
	  structure = findStructureDef( elementType );
  }
  else
  {
    assert( (inForm->getQualifiers() & TQ_Reduce) != 0 );
    structure = findStructureDef( inForm );
  }

  if( structure )
	{
		int index = 0;
		expandStreamStructureSamplerDecls( shader, inArgumentName, structure, index, samplerreg );
	}
	else
	{
		// it had better be just a floatN
		// Output a sampler, texcoord, and scale_bias for 
		// a stream
    shader <<  ",\n\t\t";
		shader << "uniform _stype _tex_" << inArgumentName;
		shader << " : register (s" << samplerreg++ << ")";
	}
}

static void printSwizzle( std::ostream& shader, Type* inForm )
{
	assert( inForm );
	BaseType* base = inForm->getBase();
	assert( base );

	switch(base->typemask) {
		case BT_Float:
			shader << "x";
			break;
		case BT_Float2:
			shader << "xy";
			break;
		case BT_Float3:
			shader << "xyz";
			break;
		case BT_Float4:
			shader << "xyzw";
			break;
		default:
			fprintf(stderr, "Strange stream base type:");
			base->printBase(std::cerr, 0);
			abort();
	}
}

static void expandStreamStructureFetches( std::ostream& shader, const std::string& argumentName, const std::string& fieldName, StructDef* structure, int& ioIndex )
{
	assert( !structure->isUnion() );

	int elementCount = structure->nComponents;
	for( int i = 0; i < elementCount; i++ )
	{
		Decl* elementDecl = structure->components[i];
		if( elementDecl->storage & ST_Static ) continue;
		Type* form = elementDecl->form;

		std::string subFieldName = fieldName + "." + elementDecl->name->name;

		// TIM: for now
		assert( form->isBaseType() );
		BaseType* base = form->getBase();
		StructDef* structure = findStructureDef( base );
		if( structure )
		{
			expandStreamStructureFetches( shader, argumentName, subFieldName, structure, ioIndex );
		}
		else
		{
			shader << "\t" << subFieldName << " = _sfetch"
				<< "(__structsampler" << ioIndex++ << "_" << argumentName << ","
				<< " _tex_" << argumentName << "_pos).";

			printSwizzle( shader, base );

			shader << ";\n";
		}
	}
}

static void expandStreamFetches( std::ostream& shader, const std::string& argumentName, Type* inForm )
{
  StructDef* structure = NULL;
  Type* elementType = NULL;

  if( inForm->isStream() )
  {
	  BrtStreamType* streamType = (BrtStreamType*)(inForm);

	  // TIM: can't handle arrays with a BaseType
	  elementType = streamType->getBase();
	  structure = findStructureDef( elementType );
  }
  else
  {
    assert( (inForm->getQualifiers() & TQ_Reduce) != 0 );
    elementType = inForm;
    structure = findStructureDef( inForm );
  }

	if( structure )
	{
		int index = 0;
		expandStreamStructureFetches( shader, argumentName, argumentName, structure, index );
	}
	else
	{
		shader << "\t" << argumentName << " = _sfetch"
			<< "(_tex_" << argumentName << ", _tex_" << argumentName
			<< "_pos).";

		printSwizzle( shader, elementType );

		shader << ";\n";
	}
}

static char *
generate_shader_code (Decl **args, int nArgs, 
                    const char *body, const char* functionName) {
  const char xyzw[] = "xyzw";
  std::ostringstream shader;
  bool isReduction = false;
  int texcoord, constreg, samplerreg, i;

  shader << "#ifdef USERECT\n";
  shader << "#define _stype   samplerRECT\n";
  shader << "#define _sfetch  texRECT\n";
  shader << "#define _gather1(a,b,c) texRECT((a),float2(b,0))\n";
  shader << "#define _gather2(a,b,c) texRECT((a),(b))\n";
  shader << "#define _computeindexof(a,b) float4(a, 0, 0)\n";
  shader << "#else\n";
  shader << "#define _stype   sampler\n";
  shader << "#define _sfetch  tex2D\n";
  shader << "#define _gather1(a,b,c) tex1D((a),(c))\n";
  shader << "#define _gather2(a,b,c) tex2D((a),(c))\n";
  shader << "#define _computeindexof(a,b) (b)\n";
  shader << "#endif\n\n";

  generate_shader_structure_definitions(shader);
  generate_shader_subroutines(shader);

  // Find if it is a reduction
  for (i=0; i < nArgs; i++) {
    TypeQual qual = args[i]->form->getQualifiers();
    
    if ((qual & TQ_Reduce) != 0) {
      isReduction = true;
    }
  }

  shader << "void main (";

  constreg = 0;
  texcoord = 0;
  samplerreg = 0;
  int outputReg = 0;

  // Add the workspace variable
  shader << "   uniform float4 _workspace    : register (c"
       << constreg++ << ")";

  /* Print the argument list */

  for (i=0; i < nArgs; i++) {
    TypeQual qual = args[i]->form->getQualifiers();
    
    /* put the output in the argument list */
    if ((qual & TQ_Out) != 0 || (qual & TQ_Reduce) != 0) {
      expandOutputArgumentDecl( shader, (args[i]->name)->name, args[i]->form, outputReg );      
    }
    
    if (args[i]->isStream() || (qual & TQ_Reduce) != 0) {

      if ((qual & TQ_Iter) != 0) {
        
        // Just output a texcoord for an iterator
        shader <<  ",\n\t\t";
        args[i]->form->getBase()->qualifier &= ~TQ_Iter;
        args[i]->form->printBase(shader, 0);
        args[i]->form->getBase()->qualifier = qual;
        
        shader << *args[i]->name << " : TEXCOORD" << texcoord++; 

      } else if((qual & TQ_Out) != 0) {
        if( FunctionProp[functionName].contains(i) ) {
          shader <<  ",\n\t\t";
          shader << "uniform float4 _const_" << *args[i]->name << "_invscalebias"
                << " : register (c" << constreg++ << ")";
          shader <<  ",\n\t\t";
          shader << "float2 _tex_" << *args[i]->name << "_pos : TEXCOORD"
              << texcoord++;
        }
      } else
      {

    		exandStreamSamplerDecls( shader, (args[i]->name)->name, args[i]->form, samplerreg );

        // Output a texcoord, and optional scale/bias
        if( FunctionProp[functionName].contains(i) ) {
          shader <<  ",\n\t\t";
          shader << "uniform float4 _const_" << *args[i]->name << "_invscalebias"
                << " : register (c" << constreg++ << ")";
        }
        shader <<  ",\n\t\t";
        shader << "float2 _tex_" << *args[i]->name << "_pos : TEXCOORD"
            << texcoord++;
      }
    } else if (args[i]->isArray()) {
      
      shader <<  ",\n\t\t";
      shader << "uniform _stype " << *args[i]->name;
      shader << " : register (s" << samplerreg++ << ")";
      shader <<  ",\n\t\t";
      shader << "uniform float4 _const_" << *args[i]->name << "_scalebias"
                << " : register (c" << constreg++ << ")";
    
    } else {
      shader <<  ",\n\t\t";
      shader << "uniform ";
      args[i]->print(shader, true);
      shader << " : register (c" << constreg++ << ")";
    }
  }
  shader << ") {\n";


  /* Declare the output variable */
//  shader << "float4 _OUT;\n\t\t";

  /* Declare the stream variables */
  for (i=0; i < nArgs; i++) {
     TypeQual qual = args[i]->form->getQualifiers();

     if((qual & TQ_Iter) != 0)
       continue;

     if ((qual & TQ_Out) != 0) {
        args[i]->form->getBase()->qualifier &= ~TQ_Out;
        shader << "\t";
        args[i]->form->printBase(shader, 0);
        shader << " " << *args[i]->name << ";\n";
        args[i]->form->getBase()->qualifier = qual;
     } else if(args[i]->isStream() || (qual & TQ_Reduce) != 0) {
        shader << "\t";
        args[i]->form->printBase(shader, 0);
        shader << " " << *args[i]->name << ";\n";
     }
  }

  /* Perform stream fetches */
  for (i=0; i < nArgs; i++) {
     TypeQual qual = args[i]->form->getQualifiers();

     if ((qual & TQ_Iter) != 0) continue; /* No texture fetch for iterators */

     if (args[i]->isStream() || 
         (qual & TQ_Reduce) != 0) {

        if ((qual & TQ_Out) == 0 ) {
	        expandStreamFetches( shader, args[i]->name->name, args[i]->form );
        }
       if( FunctionProp[functionName].contains(i) )
         {
           shader << "\t" << "float4 __indexof_" << *args[i]->name << " = "
                  << "_computeindexof( "
                  << "_tex_" << *args[i]->name << "_pos, "
                  << "float4( _tex_" << *args[i]->name << "_pos*"
                  << "_const_" << *args[i]->name << "_invscalebias.xy + "
                  << "_const_" << *args[i]->name << "_invscalebias.zw,0,0));\n";
         }
     }
  }

  /* Include the body of the kernel */
//  shader << body << std::endl;
  // TIM: just call the body as a subroutine
  shader << std::endl;

  shader << "\t" << functionName << "(\n";
  shader << "\t\t";

  for (i=0; i < nArgs; i++) {
    if( i != 0 )
      shader << ",\n\t\t";
    std::string name = args[i]->name->name;
    Type* form = args[i]->form;
    if( args[i]->isArray() ) {// hacked way to detect a gather
      shader << name << ", " << "_const_" << name << "_scalebias";
    }
    else {
      shader << name;
    }
  }
  std::set<unsigned int>::iterator indexofIterator=
    FunctionProp[ functionName ].begin();
  std::set<unsigned int>::iterator indexofEnd =
    FunctionProp[ functionName ].end();
  for(; indexofIterator != indexofEnd; ++indexofIterator ) {
    shader << ",\n\t\t__indexof_" << args[*indexofIterator]->name->name;
  }

  shader << " );\n\n";

  /* do any output unpacking */
  outputReg = 0;
  for (i=0; i < nArgs; i++) {
    TypeQual qual = args[i]->form->getQualifiers();
    if((qual & TQ_Out) == 0 && (qual & TQ_Reduce) == 0) continue;
    
    expandOutputArgumentWrite( shader, (args[i]->name)->name, args[i]->form, outputReg );
  }

  shader << "}\n";

  return strdup(shader.str().c_str());
}


/*
 * compile_cg_code --
 *
 *      Takes CG code and runs it through the CG compiler (and parses the
 *      results) to produce the corresponding fragment program.
 */

static char *
compile_cg_code (char *cgcode) {

  char *argv[16] = { "cgc", "-profile", "fp30", 
                     "-DUSERECT", "-quiet", NULL };
  char *fpcode, *endline;

  fpcode = Subprocess_Run(argv, cgcode);
  if (fpcode == NULL) {
     fprintf(stderr, "Unable to run %s, skipping fp30 / nv30gl target.\n",
             argv[0]);
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

  FILE *fp = fopen (globals.shaderoutputname, "wb+");
  if (fp == NULL) {
    fprintf (stderr, "Unable to open tmp file %s\n", globals.shaderoutputname);
    return NULL;
  }
  fwrite(hlslcode, sizeof(char), strlen(hlslcode), fp);
  fclose(fp);
  
  argv[3] = (char *) malloc(strlen("/Fc.ps") +
                            strlen(globals.shaderoutputname) + 1);
  sprintf (argv[3], "/Fc%s.ps", globals.shaderoutputname);
  argv[4] = globals.shaderoutputname;
  errcode = Subprocess_Run(argv, NULL);
  if (!globals.keepFiles) remove(globals.shaderoutputname);
  if (errcode == NULL) {
     fprintf(stderr, "Unable to run %s, skipping ps20 / dx9 target.\n",
             argv[0]);
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

  fclose(fp);
  remove(argv[3]+3);
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
                             Decl **args, int nArgs, const char *body,
                             const char* functionName)
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

     fp << commentstring << type;
     if( FunctionProp[functionName].contains(i) )
       fp << "i";
     fp << ":" << dimension << ":" << *args[i]->name << "\n";
  }

  fp << commentstring << "workspace:" << globals.workspace << std::endl;

  return strdup(fp.str().c_str());
}


/*
 * generate_c_code --
 *
 *      Spits out the compiled pixel shader
 *      code as a string available to the emitted
 *      C code.
 */

static char *
generate_c_code(char *fpcode, const char *name, const char *id)
{
  std::ostringstream fp;
  int i;

  assert (name);

  if (fpcode == NULL) {
    fp << "\nstatic const char *__" << name 
       << "_" << id << " = NULL;\n";
     return strdup(fp.str().c_str());
  }

  fp << "\nstatic const char __" << name 
     << "_" << id << "[] = {" << std::endl;

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
 * CodeGen_GenerateCode --
 *
 *      Takes a parsed kernel and crunches it down to C code:
 *              . Creates and annotates equivalent HLSL
 *              . Compiles the HLSL to ps20/fp30 assembly
 *              . Spits out the fragment program as a C string
 *
 *      Note: The caller is responsible for free()ing the returned string.
 */

char *
CodeGen_GenerateCode(Type *retType, const char *name,
                     Decl **args, int nArgs, const char *body, 
                     bool ps20_not_fp30)
{
  char *shadercode, *fpcode, *fpcode_with_brccinfo, *c_code;

  shadercode = generate_shader_code(args, nArgs, body, name);
  if (shadercode) {
     if (globals.verbose)
       std::cerr << "\n***Produced this shader:\n" << shadercode << "\n";

     if (globals.keepFiles) {
        std::ofstream out;

        out.open(globals.shaderoutputname);
        if (out.fail()) {
           std::cerr << "***Unable to open " <<globals.shaderoutputname<< "\n";
        } else {
           out << shadercode;
           out.close();
        }
     }
     fprintf (stderr, "Generating code for %s\n",name);
     fpcode = (ps20_not_fp30 ? compile_hlsl_code : compile_cg_code)(shadercode);
     free(shadercode);
  } else {
     fpcode = NULL;
  }

  if (fpcode) {
    if (globals.verbose)
      std::cerr << "***Produced this assembly:\n" << fpcode << "\n";

     fpcode_with_brccinfo =
       append_argument_information(ps20_not_fp30?"//":"##",
                                   fpcode, args, nArgs, body, name);
     free(fpcode);

     if (globals.verbose)
       std::cerr << "***Produced this instrumented assembly:\n"
                 << fpcode_with_brccinfo << "\n";
  } else {
     fpcode_with_brccinfo = NULL;
  }

  c_code = generate_c_code(fpcode_with_brccinfo, name,
                            ps20_not_fp30?"ps20":"fp30");
  free(fpcode_with_brccinfo);

  if (globals.verbose)
    std::cerr << "***Produced this C code:\n" << c_code;

  return c_code;
}

