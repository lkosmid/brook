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

int getGatherStructureSamplerCount( StructDef* structure )
{
  int result = 0;
  int fieldCount = structure->nComponents;
	for( int i = 0; i < fieldCount; i++ )
	{
		Decl* fieldDecl = structure->components[i];
		if( fieldDecl->isStatic() ) continue;
		if( fieldDecl->isTypedef() ) continue;

    StructDef* subStructure = findStructureDef( fieldDecl->form );
    if( subStructure )
      result += getGatherStructureSamplerCount( subStructure );
    else
      result++;
	}
  return result;
}

int getGatherStructureSamplerCount( Type* form )
{
  StructDef* structure = findStructureDef( form );
  if( !structure ) return 1;
  return getGatherStructureSamplerCount( structure );
}

static bool printGatherStructureFunctionBody( std::ostream& out, const std::string& name, StructDef* structure, int& ioIndex )
{
  int fieldCount = structure->nComponents;
	for( int i = 0; i < fieldCount; i++ )
	{
		Decl* fieldDecl = structure->components[i];
		if( fieldDecl->isStatic() ) continue;
		if( fieldDecl->isTypedef() ) continue;
    
    std::string subName = name + "." + fieldDecl->name->name;

    Type* form = fieldDecl->form;
    StructDef* subStructure = findStructureDef( form );
    if( subStructure )
    {
      if(!printGatherStructureFunctionBody( out, subName, subStructure, ioIndex ))
        return false;
    }
    else
    {
      out << "result" << subName << " = ";

      BaseType* base = form->getBase();
      switch(base->typemask) {
      case BT_Float:
        out << "__fetch_float";
      break;
      case BT_Float2:
        out << "__fetch_float2";
      break;
      case BT_Float3:
        out << "__fetch_float3";
      break;
      case BT_Float4:
        out << "__fetch_float4";
      break;
      default:
        out << "__fetchunknown";
        break;
      }
      out << "( ";
      out << "samplers[" << ioIndex++ << "], index );\n";
    }
	}
  return true;
}

static void printGatherStructureFunction( std::ostream& out, const std::string& name, Type* form )
{
  StructDef* structure = findStructureDef( form );
  if( !structure ) return;

  std::stringstream s;
  int index = 0;
  if(!printGatherStructureFunctionBody( s, "", structure, index ))
    return;
  std::string body = s.str();


  out << name << " __gather_" << name << "( _stype samplers[" << getGatherStructureSamplerCount(form);
  out << "], float index ) {\n";
  out << name << " result;\n";
  out << body;
  out << "\treturn result;\n}\n\n";

  out << name << " __gather_" << name << "( _stype samplers[" << getGatherStructureSamplerCount(form);
  out << "], float2 index ) {\n";
  out << name << " result;\n";
  out << body;
  out << "\treturn result;\n}\n\n";
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

      printGatherStructureFunction( out, decl->name->name, decl->form );
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

static bool expandOutputArgumentStructureDecl(
  std::ostream& shader, const std::string& argumentName, StructDef* structure, int& ioOutputReg, int inFirstOutput, int inOutputCount )
{
	assert( !structure->isUnion() );

  bool used = false;

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
			used = used || expandOutputArgumentStructureDecl( shader, argumentName, structure, ioOutputReg, inFirstOutput, inOutputCount );
		else
		{
      int outr = ioOutputReg++;
      if( outr >= inFirstOutput
        && outr < inFirstOutput+inOutputCount )
      {
        used = true;
		    // it had better be just a floatN
        shader << ",\n\t\t";
        shader << "out float4 __output_" << outr;
        shader << " : COLOR" << (outr - inFirstOutput);
      }
		}
	}
  return used;
}


static bool expandOutputArgumentDecl(
  std::ostream& shader, const std::string& argumentName, Type* form, int& ioOutputReg, int inFirstOutput, int inOutputCount )
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
		return expandOutputArgumentStructureDecl( shader, argumentName, structure, ioOutputReg, inFirstOutput, inOutputCount );
	}
	else
	{
    int outr = ioOutputReg++;
    if( outr < inFirstOutput ) return false;
    if( outr >= inFirstOutput+inOutputCount ) return false;

		// it had better be just a floatN
    shader << ",\n\t\t";
    shader << "out float4 __output_" << outr;
    shader << " : COLOR" << (outr - inFirstOutput);

    return true;
	}
}

static void expandSimpleOutputArgumentWrite(
  std::ostream& shader, const std::string& argumentName, Type* form, int& outputReg, int inFirstOutput, int inOutputCount )
{
  int outr = outputReg++;
  if( outr < inFirstOutput || outr >= inFirstOutput + inOutputCount ) return;

  assert( form );
	BaseType* base = form->getBase();
	assert( base );

  shader << "\t__output_" << outr << " = ";

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

static void expandOutputArgumentStructureWrite( std::ostream& shader, const std::string& fieldName, StructDef* structure, int& ioOutputReg, int inFirstOutput, int inOutputCount )
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
			expandOutputArgumentStructureWrite( shader, subFieldName, structure, ioOutputReg, inFirstOutput, inOutputCount );
		}
		else
		{
      expandSimpleOutputArgumentWrite( shader, subFieldName, base, ioOutputReg, inFirstOutput, inOutputCount );
		}
	}
}

static void expandOutputArgumentWrite(
  std::ostream& shader, const std::string& argumentName, Type* form, int& ioOutputReg, int inFirstOutput, int inOutputCount )
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
		expandOutputArgumentStructureWrite( shader, argumentName, structure, ioOutputReg, inFirstOutput, inOutputCount );
	}
	else
	{
    expandSimpleOutputArgumentWrite( shader, argumentName, elementType, ioOutputReg, inFirstOutput, inOutputCount );
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
      shader << subFieldName << " = ";

      switch(base->typemask) {
      case BT_Float:
        shader << "__fetch_float";
        break;
      case BT_Float2:
        shader << "__fetch_float2";
       break;
      case BT_Float3:
        shader << "__fetch_float3";
       break;
      case BT_Float4:
        shader << "__fetch_float4";
       break;
      default:
        shader << "__gatherunknown";
        break;
      }

      shader << "( __structsampler" << ioIndex++ << "_" << argumentName
        << ", _tex_" << argumentName << "_pos );\n";
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
    shader << argumentName << " = ";

    BaseType* base = inForm->getBase();
    switch(base->typemask) {
    case BT_Float:
      shader << "__fetch_float";
      break;
    case BT_Float2:
      shader << "__fetch_float2";
      break;
    case BT_Float3:
      shader << "__fetch_float3";
      break;
    case BT_Float4:
      shader << "__fetch_float4";
      break;
    default:
      shader << "__fetchunknown";
      break;
    }

    shader << "(_tex_" << argumentName
      << ", _tex_" << argumentName << "_pos );\n";
	}
}

static char *
generate_shader_code (Decl **args, int nArgs, const char* functionName, int inFirstOutput, int inOutputCount ) {
  const char xyzw[] = "xyzw";
  std::ostringstream shader;
  bool isReduction = false;
  int texcoord, constreg, samplerreg, i;

  shader << "#ifdef USERECT\n";
  shader << "#define _stype   samplerRECT\n";
  shader << "#define _sfetch  texRECT\n";
  shader << "#define __sample1(s,i) texRECT((s),float2(i,0))\n";
  shader << "#define __sample2(s,i) texRECT((s),(i))\n";
  shader << "#define _computeindexof(a,b) float4(a, 0, 0)\n";
  if( !globals.enableGPUAddressTranslation ) {
    shader << "float __gatherindex1( float index, float4 scalebias ) { ";
    shader << "return index; }\n";
    shader << "float2 __gatherindex2( float2 index, float4 scalebias ) { ";
    shader << "return index; }\n";
  }
  shader << "#else\n";
  shader << "#define _stype   sampler\n";
  shader << "#define _sfetch  tex2D\n";
  shader << "#define __sample1(s,i) tex1D((s),(i))\n";
  shader << "#define __sample2(s,i) tex2D((s),(i))\n";
  shader << "#define _computeindexof(a,b) (b)\n";
  if( !globals.enableGPUAddressTranslation ) {
    shader << "float __gatherindex1( float index, float4 scalebias ) { ";
    shader << "return index*scalebias.x+scalebias.z; }\n";
    shader << "float2 __gatherindex2( float2 index, float4 scalebias ) { ";
    shader << "return index*scalebias.xy+scalebias.zw; }\n";
  }
  shader << "#endif\n\n";

  // TIM: simple subroutines
  shader << "float __fetch_float( _stype s, float i ) { return __sample1(s,i).x; }\n";
  shader << "float __fetch_float( _stype s, float2 i ) { return __sample2(s,i).x; }\n";
  shader << "float2 __fetch_float2( _stype s, float i ) { return __sample1(s,i).xy; }\n";
  shader << "float2 __fetch_float2( _stype s, float2 i ) { return __sample2(s,i).xy; }\n";
  shader << "float3 __fetch_float3( _stype s, float i ) { return __sample1(s,i).xyz; }\n";
  shader << "float3 __fetch_float3( _stype s, float2 i ) { return __sample2(s,i).xyz; }\n";
  shader << "float4 __fetch_float4( _stype s, float i ) { return __sample1(s,i).xyzw; }\n";
  shader << "float4 __fetch_float4( _stype s, float2 i ) { return __sample2(s,i).xyzw; }\n";

  shader << "\n\n";

  shader << "float __gather_float( _stype s[1], float i ) { return __sample1(s[0],i).x; }\n";
  shader << "float __gather_float( _stype s[1], float2 i ) { return __sample2(s[0],i).x; }\n";
  shader << "float2 __gather_float2( _stype s[1], float i ) { return __sample1(s[0],i).xy; }\n";
  shader << "float2 __gather_float2( _stype s[1], float2 i ) { return __sample2(s[0],i).xy; }\n";
  shader << "float3 __gather_float3( _stype s[1], float i ) { return __sample1(s[0],i).xyz; }\n";
  shader << "float3 __gather_float3( _stype s[1], float2 i ) { return __sample2(s[0],i).xyz; }\n";
  shader << "float4 __gather_float4( _stype s[1], float i ) { return __sample1(s[0],i).xyzw; }\n";
  shader << "float4 __gather_float4( _stype s[1], float2 i ) { return __sample2(s[0],i).xyzw; }\n";

  if( globals.enableGPUAddressTranslation )
  {
    shader << "\n\n";

    shader << "float4 __calculateindexof( float4 outputpos_01, float4 shape ) {\n";
    shader << "\treturn floor( outputpos_01*shape + 0.001 ); }\n";

    shader << "float2 __calculatetexpos( float4 index, float4 linearizeConst, float2 reshapeConst, float hackConst ) {\n";
    shader << "\tfloat linearIndex = dot( index, linearizeConst );\n";
    shader << "#ifndef USERECT\n";
    shader << "//HLSL codegen bug workaround\n";
    shader << "\tlinearIndex *= hackConst;\n";
    shader << "#endif\n";
    shader << "\tfloat2 result;\n";
    shader << "\tresult.x = frac( linearIndex );\n";
    shader << "\tresult.y = linearIndex - result.x;\n";
    shader << "\tresult *= reshapeConst;\n";
    shader << "return result;\n}\n\n";

    shader << "void __calculateoutputpos( float2 interpolant, float4 shape, float4 invshape,\n";
    shader << "\tout float4 index, out float4 index01 ) {\n";
    shader << "\tfloat linearIndex = interpolant.y + interpolant.x;\n";
    shader << "\tindex01.x = frac( linearIndex );\n";
    shader << "\tindex.y = linearIndex - index01.x;\n";
    shader << "\tindex.x = floor( index01.x * shape.x + 0.001 );\n";
    shader << "\tindex01.y = index.y * invshape.y;\n";
    shader << "\tindex01.z = 0;\n";
    shader << "\tindex01.w = 0;\n";
    shader << "\tindex.z = 0;\n";
    shader << "\tindex.w = 0;\n";
    shader << "}\n\n";

    shader << "float2 __gatherindex1( float1 index, float4 linearizeConst, float2 reshapeConst, float hackConst ) {\n";
    shader << "\treturn __calculatetexpos( float4(index,0,0,0), linearizeConst, reshapeConst, hackConst ); }\n";
    shader << "float2 __gatherindex2( float2 index, float4 linearizeConst, float2 reshapeConst, float hackConst ) {\n";
    shader << "\treturn __calculatetexpos( float4(index,0,0), linearizeConst, reshapeConst, hackConst ); }\n";
    shader << "float2 __gatherindex3( float3 index, float4 linearizeConst, float2 reshapeConst, float hackConst ) {\n";
    shader << "\treturn __calculatetexpos( float4(index,0), linearizeConst, reshapeConst, hackConst ); }\n";
    shader << "float2 __gatherindex4( float4 index, float4 linearizeConst, float2 reshapeConst, float hackConst ) {\n";
    shader << "\treturn __calculatetexpos( index, linearizeConst, reshapeConst, hackConst ); }\n";
  }

  shader << "\n\n";

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


  if( !globals.enableGPUAddressTranslation )
  {
    // Add the workspace variable
    shader << "\n\t\tuniform float4 __workspace    : register (c"
        << constreg++ << ")";
  }
  else
  {
    shader << "\n\t\t";
    shader << "float2 __outputtexcoord : TEXCOORD"
      << texcoord++;
    shader << ",\n\t\t";
    shader << "float2 __outputaddrinterpolant : TEXCOORD"
      << texcoord++;
    shader << ",\n\t\t";
    shader << "uniform float4 __outputshape : register(c"
      << constreg++ << ")";
    shader << ",\n\t\t";
    shader << "uniform float4 __outputinvshape : register(c"
      << constreg++ << ")";
    shader << ",\n\t\t";
    shader << "uniform float __hackconst : register(c"
      << constreg++ << ")";

  }

  /* Print the argument list */

  bool hasDoneIndexofOutput = false;

  for (i=0; i < nArgs; i++) {
    std::string argName = args[i]->name->name;
    TypeQual qual = args[i]->form->getQualifiers();
    
    /* put the output in the argument list */
    if ((qual & TQ_Out) != 0 || (qual & TQ_Reduce) != 0) {
      expandOutputArgumentDecl( shader, (args[i]->name)->name, args[i]->form, outputReg, inFirstOutput, inOutputCount );      
    }
    
    if (args[i]->isStream() || (qual & TQ_Reduce) != 0) {

      if ((qual & TQ_Iter) != 0) {

        if( globals.enableGPUAddressTranslation )
        {
          shader << ",\n\t\t";
          shader << "float2 __itershape_" << argName;
          shader << ",\n\t\t";
          shader << "float4 __itermin_" << argName;
          shader << ",\n\t\t";
          shader << "float4 __iterstep_" << argName;
        }
        else
        {
          // Just output a texcoord for an iterator
          shader <<  ",\n\t\t";
          args[i]->form->getBase()->qualifier &= ~TQ_Iter;
          args[i]->form->printBase(shader, 0);
          args[i]->form->getBase()->qualifier = qual;
          
          shader << *args[i]->name << " : TEXCOORD" << texcoord++; 
        }
      } else if((qual & TQ_Out) != 0) {

        if( globals.enableGPUAddressTranslation )
        {
          // index of output should already be available...
        }
        else if( !hasDoneIndexofOutput && FunctionProp[functionName].contains(i) ) {
          hasDoneIndexofOutput = true;
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

        if( globals.enableGPUAddressTranslation )
        {
          shader << ",\n\t\t";
          shader << "uniform float4 __streamshape_" << argName;
          shader << " : register(c" << constreg++ << ")";
          shader << ",\n\t\t";
          shader << "uniform float4 __streamlinearize_" << argName;
          shader << " : register(c" << constreg++ << ")";
          shader << ",\n\t\t";
          shader << "uniform float2 __streamreshape_" << argName;
          shader << " : register(c" << constreg++ << ")";
        }
        else
        {
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
      }
    } else if (args[i]->isArray()) {

      int samplerCount = getGatherStructureSamplerCount( args[i]->form );
      
      if( globals.enableGPUAddressTranslation )
      {
        shader <<  ",\n\t\t";
        shader << "uniform _stype " << *args[i]->name;
        shader << "[" << samplerCount << "] : register (s" << samplerreg << ")";
        samplerreg += samplerCount;

        shader << ",\n\t\t";
        shader << "uniform float4 __gatherlinearize_" << argName;
        shader << " : register(c" << constreg++ << ")";
        shader << ",\n\t\t";
        shader << "uniform float2 __gatherreshape_" << argName;
        shader << " : register(c" << constreg++ << ")";
      }
      else
      {
        // TIM: TODO: handle multi-sampler array for gathers...
        shader <<  ",\n\t\t";
        shader << "uniform _stype " << *args[i]->name;
        shader << "[" << samplerCount << "] : register (s" << samplerreg << ")";
        samplerreg += samplerCount;
        shader <<  ",\n\t\t";
        shader << "uniform float4 __gatherconst_" << *args[i]->name
                  << " : register (c" << constreg++ << ")";
      }
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

  if( globals.enableGPUAddressTranslation )
  {
    // set up output position values
    shader << "\tfloat4 __indexofoutput_01;\n";
    shader << "\tfloat4 __indexofoutput;\n";
    shader << "\t__calculateoutputpos( __outputaddrinterpolant,\n";
    shader << "\t\t__outputshape, __outputinvshape,\n";
    shader << "\t\t__indexofoutput, __indexofoutput_01 );\n";
  }

  /* Perform stream fetches */
  hasDoneIndexofOutput = false;
  for (i=0; i < nArgs; i++) {
     TypeQual qual = args[i]->form->getQualifiers();
     std::string argName = args[i]->name->name;

     if ((qual & TQ_Iter) != 0) continue; /* No texture fetch for iterators */

     if (args[i]->isStream() || 
         (qual & TQ_Reduce) != 0) {

        if ((qual & TQ_Out) != 0 ) {
          if( globals.enableGPUAddressTranslation )
          {
            // should be calculated elsewhere
          }
          else if( !hasDoneIndexofOutput && FunctionProp[functionName].contains(i) )
            {
              hasDoneIndexofOutput= true;
              shader << "\t" << "float4 __indexofoutput = "
                      << "_computeindexof( "
                      << "_tex_" << *args[i]->name << "_pos, "
                      << "float4( _tex_" << *args[i]->name << "_pos*"
                      << "_const_" << *args[i]->name << "_invscalebias.xy + "
                      << "_const_" << *args[i]->name << "_invscalebias.zw,0,0));\n";
            }
        }
        else {
          if( globals.enableGPUAddressTranslation )
          {
            shader << "\tfloat4 __indexof_" << argName << " = ";
            shader << "__calculateindexof( __indexofoutput_01, __streamshape_" << argName;
            shader << " );\n";
            shader << "\tfloat2 _tex_" << argName << "_pos = ";
            shader << "__calculatetexpos( __indexof_" << argName << ", ";
            shader << "__streamlinearize_" << argName << ", ";
            shader << "__streamreshape_" << argName << ", __hackconst );\n";
          }
	        expandStreamFetches( shader, args[i]->name->name, args[i]->form );
          if( !globals.enableGPUAddressTranslation && FunctionProp[functionName].contains(i) )
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
    if( args[i]->isArray() ) {
      if( globals.enableGPUAddressTranslation )
      {
        shader << name;
        shader << ", __gatherlinearize_" << name;
        shader << ", __gatherreshape_" << name;
        shader << ", __hackconst";
      }
      else
      {
        shader << name << ", __gatherconst_" << name;
      }
    } else {
      shader << name;
    }
  }
  std::set<unsigned int>::iterator indexofIterator=
    FunctionProp[ functionName ].begin();
  std::set<unsigned int>::iterator indexofEnd =
    FunctionProp[ functionName ].end();
  for(; indexofIterator != indexofEnd; ++indexofIterator ) {
    if( (args[*indexofIterator]->form->getQualifiers() & TQ_Out) == 0 )
      shader << ",\n\t\t__indexof_" << args[*indexofIterator]->name->name;
    else
      shader << ",\n\n\t__indexofoutput";
  }

  shader << " );\n\n";

  /* do any output unpacking */
  outputReg = 0;
  for (i=0; i < nArgs; i++) {
    TypeQual qual = args[i]->form->getQualifiers();
    if((qual & TQ_Out) == 0 && (qual & TQ_Reduce) == 0) continue;
    
    expandOutputArgumentWrite( shader, (args[i]->name)->name, args[i]->form, outputReg, inFirstOutput, inOutputCount );
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
  if (globals.verbose) {
     fprintf(stderr, "***Summary information from cgc:\n");
     fwrite (endline, strlen(endline), 1, stderr);
  }
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
                             Decl **args, int nArgs,
                             const char* functionName, int firstOutput, int outputCount )
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

  fp << commentstring << "!!multipleOutputInfo:" << firstOutput << ":" << outputCount << ":" << std::endl;

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
generate_c_code( const std::vector<std::string>& strings, const char *name, const char *id)
{
  std::ostringstream fp;
  int i;

  assert (name);

  if (strings.size() == 0) {
    fp << "\nstatic const char *__" << name 
       << "_" << id << " = NULL;\n";
     return strdup(fp.str().c_str());
  }

  fp << "\nstatic const char* __" << name 
     << "_" << id << "[] = {" << std::endl;

  int count = strings.size();
  for( int s = 0; s < count; s++ )
  {
    const char* fpcode = strings[s].c_str();
    fp << "\"";
    while ((i = *fpcode++) != '\0') {
      if (i == '\n')
        fp << "\\n\"\n\"";
      else
        fp << (char) i;
    }
    fp << "\",\n";
  }
  fp << "NULL};\n";

  return strdup(fp.str().c_str());
}

int getShaderOutputCount( int argumentCount, Decl** arguments, bool& outIsReduction )
{
  int result = 0;
  bool isReduction = false;
  for( int i = 0; i < argumentCount; i++ )
  {
    Type* form = arguments[i]->form;

    if((form->getQualifiers() & TQ_Out) != 0 )
      result += getGatherStructureSamplerCount( form );
    else if((form->getQualifiers() & TQ_Reduce) != 0 )
    {
      isReduction = true;
      result += getGatherStructureSamplerCount( form );
    }
  }
  outIsReduction = isReduction;
  return result;
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

static char* generateShaderPass( Decl** args, int nArgs, const char* name, int firstOutput, int outputCount, bool ps20_not_fp30 )
{
  
    char* fpcode;
    char* fpcode_with_brccinfo;
    char* shadercode = generate_shader_code( args, nArgs, name, firstOutput, outputCount );
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
      if (globals.verbose) {
        fprintf(stderr, "Generating %s code for %s outputs [%d, %d).\n",
                ps20_not_fp30 ? "ps20" : "fp30", name, firstOutput, firstOutput+outputCount);
      }
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
                                    fpcode, args, nArgs, name, firstOutput, outputCount );
      free(fpcode);

      if (globals.verbose)
        std::cerr << "***Produced this instrumented assembly:\n"
                  << fpcode_with_brccinfo << "\n";
    } else {
      fpcode_with_brccinfo = NULL;
    }
    
    return fpcode_with_brccinfo;
}

char *
CodeGen_GenerateCode(Type *retType, const char *name,
                     Decl **args, int nArgs, const char *body, 
                     bool ps20_not_fp30)
{
  bool isReduction = false;
  int outputCount = getShaderOutputCount( nArgs, args, isReduction );
  int maxOutputsPerPass = 1;
  if( ps20_not_fp30 && !isReduction && globals.allowDX9MultiOut )
    maxOutputsPerPass = 4;

  std::vector<std::string> shaderStrings;

  int outputsLeft = outputCount;
  int firstOutput = 0;
  while( outputsLeft > 0 )
  {
    int outputsToWrite = maxOutputsPerPass;
    if( outputsLeft < maxOutputsPerPass )
      outputsToWrite = outputsLeft;

    char* shaderString = NULL;
    while( outputsToWrite > 0 )
    {
      shaderString = generateShaderPass( args, nArgs, name, firstOutput, outputsToWrite, ps20_not_fp30 );
      if( shaderString ) break;

      // try again with fewer outputs
      outputsToWrite--;
      
      // we have failed if we can't even do one output
      if( outputsToWrite == 0 )
        return generate_c_code( std::vector<std::string>(), name, ps20_not_fp30?"ps20":"fp30" );
    }

    shaderStrings.push_back( shaderString );
    free( shaderString );

    firstOutput += outputsToWrite;
    outputsLeft -= outputsToWrite;
  }

  char* c_code = generate_c_code( shaderStrings, name,
                            ps20_not_fp30?"ps20":"fp30");

  if (globals.verbose)
    std::cerr << "***Produced this C code:\n" << c_code;

  return c_code;
}

