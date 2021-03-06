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
#include <memory>
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
#include "splitting/splitting.h"
#include "codegen.h"
#include "brtdecl.h"
#include "fxc.h"
#include "cgc.h"


// structures to store information about the resources
// used in each pass/technique
struct shader_input_info
{
  shader_input_info( int arg, int comp )
    : argumentIndex(arg)
  {
    std::ostringstream out;
    out << comp;
    componentName = out.str();
  }

  shader_input_info( int arg, const std::string& comp )
    : argumentIndex(arg), componentName(comp)
  {}

  int argumentIndex;
  std::string componentName;
};

struct pass_info
{
  void addConstant( int arg, int comp ) {
    constants.push_back( shader_input_info( arg, comp ) );
  }

  void addConstant( int arg, const std::string& comp ) {
    constants.push_back( shader_input_info( arg, comp ) );
  }

  void addSampler( int arg, int comp ) {
    samplers.push_back( shader_input_info( arg, comp ) );
  }

  void addInterpolant( int arg, int comp ) {
    interpolants.push_back( shader_input_info( arg, comp ) );
  }

  void addInterpolant( int arg, const std::string& comp ) {
    interpolants.push_back( shader_input_info( arg, comp ) );
  }

  void addOutput( int arg, int comp ) {
    outputs.push_back( shader_input_info( arg, comp ) );
  }

  std::string shader;
  std::vector<shader_input_info> constants;
  std::vector<shader_input_info> samplers;
  std::vector<shader_input_info> interpolants;
  std::vector<shader_input_info> outputs;
};

struct technique_info
{
  technique_info()
    : reductionFactor(-1),
    outputAddressTranslation(false),
    inputAddressTranslation(false)
  {}

  std::vector<pass_info> passes;
  int reductionFactor;
  bool outputAddressTranslation;
  bool inputAddressTranslation;
};


static char * (*shadercompile[CODEGEN_NUM_TARGETS]) (const char *name,
                                   const char *shader, 
                                   CodeGenTarget target, 
                                   ShaderResourceUsage* outUsage, 
                                   bool inValidate);

void
CodeGen_Init(void) {
   switch (globals.favorcompiler) {
   case COMPILER_DEFAULT:
#ifdef WIN32
      shadercompile[CODEGEN_PS20] = compile_fxc;
      shadercompile[CODEGEN_PS2B] = compile_fxc;
      shadercompile[CODEGEN_PS2A] = compile_fxc;
      shadercompile[CODEGEN_PS30] = compile_fxc;
      shadercompile[CODEGEN_CTM]  = compile_fxc;
      shadercompile[CODEGEN_FP30] = compile_cgc;
      shadercompile[CODEGEN_FP40] = compile_cgc;
      shadercompile[CODEGEN_ARB]  = compile_fxc;
      shadercompile[CODEGEN_GLSL] = compile_cgc;
      shadercompile[CODEGEN_GLES] = compile_cgc;
      break;
#endif
   case COMPILER_CGC:
#ifdef WIN32
      shadercompile[CODEGEN_PS20] = compile_cgc;
      shadercompile[CODEGEN_PS2B] = compile_cgc;
      shadercompile[CODEGEN_PS2A] = compile_cgc;
      shadercompile[CODEGEN_PS30] = compile_fxc;
      shadercompile[CODEGEN_CTM]  = compile_fxc;
#else
      shadercompile[CODEGEN_PS20] = NULL;
      shadercompile[CODEGEN_PS2B] = NULL;
      shadercompile[CODEGEN_PS2A] = NULL;
      shadercompile[CODEGEN_PS30] = NULL;
      shadercompile[CODEGEN_CTM]  = NULL;
#endif
      shadercompile[CODEGEN_FP30] = compile_cgc;
      shadercompile[CODEGEN_FP40] = compile_cgc;
      shadercompile[CODEGEN_ARB]  = compile_cgc;
      shadercompile[CODEGEN_GLSL] = compile_cgc;
      shadercompile[CODEGEN_GLES] = compile_cgc;
      break;
   case COMPILER_FXC:
      shadercompile[CODEGEN_PS20] = compile_fxc;
      shadercompile[CODEGEN_PS2B] = compile_fxc;
      shadercompile[CODEGEN_PS2A] = compile_fxc;
      shadercompile[CODEGEN_PS30] = compile_fxc;
      shadercompile[CODEGEN_CTM]  = compile_fxc;
      shadercompile[CODEGEN_FP30] = compile_cgc;
      shadercompile[CODEGEN_FP40] = compile_cgc;
      shadercompile[CODEGEN_ARB]  = compile_fxc;
      shadercompile[CODEGEN_GLSL] = compile_cgc;
      shadercompile[CODEGEN_GLES] = compile_cgc;
      break;
   default:
      fprintf (stderr, 
               "Error Unknown compiler specified: %d\n", 
               globals.favorcompiler);
      exit(1);
   }
}

/*
 * generate_hlsl_code --
 *
 *      This function takes a parsed kernel function as input and produces
 *      the CG code reflected, plus the support code required.
 */
static void generate_shader_subroutines(std::ostream&  out, const char * nam) {
   TransUnit * tu = gProject->units.back();
   Statement *ste;
   for (ste=tu->head; ste; ste=ste->next) {
      if (ste->isFuncDef()) {
         FunctionDef * fd = static_cast<FunctionDef *> (ste);
//TIM: I'm unsure why we don't output the reductions
//         if (fd->decl->isKernel()&&!fd->decl->isReduce()) {
         if (FunctionProp[nam].calls(fd->decl->name->name) 
             || fd->decl->name->name==std::string(nam) 
             || (fd->decl->isKernel()&&globals.keepFiles)) {
            //fd->decl->isKernel()) {
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

#if 0
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
#endif

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

int getGatherRank( Type* form )
{
   Type* f = form;
   int rank = 0;
   while( f->isArray() )
   {
      rank++;
      f = ((ArrayType*) f)->subType;
   }
   return rank;
}

static bool printGatherStructureFunctionBody( std::ostream& out, const std::string& name, StructDef* structure, int& ioIndex )
{
   int fieldCount = structure->nComponents;
   for( int i = 0; i < fieldCount; i++ ) {
      Decl* fieldDecl = structure->components[i];
      if( fieldDecl->isStatic() ) continue;
      if( fieldDecl->isTypedef() ) continue;
      
      std::string subName = name + "." + fieldDecl->name->name;
      
      Type* form = fieldDecl->form;
      StructDef* subStructure = findStructureDef( form );
      if( subStructure ) {
         if(!printGatherStructureFunctionBody( out, subName, 
                                               subStructure, ioIndex ))
            return false;
      } else {
         out << "result" << subName << " = ";
         
         BaseType* base = form->getBase();
         switch(base->typemask) {
         case BT_Double:
           out << "__fetch_double";
           break;
         case BT_Double2:
           out << "__fetch_double2";
           break;
         case BT_Float:
         case BT_Fixed:
         case BT_ShortFixed:
            out << "__fetch_float";
            break;
         case BT_Float2:
         case BT_Fixed2:
         case BT_ShortFixed2:
            out << "__fetch_float2";
            break;
         case BT_Float3:
         case BT_Fixed3:
         case BT_ShortFixed3:
            out << "__fetch_float3";
            break;
         case BT_Float4:
         case BT_Fixed4:
         case BT_ShortFixed4:
            out << "__fetch_float4";
            break;
         default:
            return false;
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


  out << name << " __gather_" << name << "( _stype1 samplers[" << getGatherStructureSamplerCount(form);
  out << "], float index ) {\n";
  out << name << " result;\n";
  out << body;
  out << "\treturn result;\n}\n\n";

  out << name << " __gather_" << name << "( _stype2 samplers[" << getGatherStructureSamplerCount(form);
  out << "], float2 index ) {\n";
  out << name << " result;\n";
  out << body;
  out << "\treturn result;\n}\n\n";

  out << name << " __gather_" << name << "( _stype3 samplers[" << getGatherStructureSamplerCount(form);
     out << "], float3 index ) {\n";
  out << name << " result;\n";
  out << body;
  out << "\treturn result;\n}\n\n";
}

static void generate_shader_type_declaration( std::ostream& out,
                                              DeclStemnt* inStmt )
{
   for( DeclVector::iterator i = inStmt->decls.begin(); 
        i != inStmt->decls.end(); ++i ) {
      Decl* decl = *i;
      Type* form = decl->form;
      /*
        Symbol* structureTag = findStructureTag( form );
        if( structureTag != NULL ) {
        StructDef* structure = structureTag->entry->uStructDef->stDefn;
        printShaderStructureDef( out, structure );
        }*/
      
      if( decl->isTypedef() ) {
         out << "typedef ";
         form->printBase(out,0);
         out << " " << decl->name->name;
         out << ";";
         out << "\n";
         
         printGatherStructureFunction( out, decl->name->name, decl->form );
      }
   }
}

static void generate_shader_structure_definitions( std::ostream& out ) {
	TransUnit * tu = gProject->units.back();
	Statement *ste;
	for (ste=tu->head; ste; ste=ste->next) {
		if(ste->isDeclaration() || ste->isTypedef())
		{
			DeclStemnt* decl = static_cast<DeclStemnt*>(ste);
			generate_shader_type_declaration( out, decl );
		}
	}
}

static bool expandOutputArgumentStructureDecl(std::ostream& shader, 
                                              const std::string& argumentName, 
                                              int inArgumentIndex, 
                                              int inComponentIndex, 
                                              StructDef* structure, 
                                              int& ioOutputReg, 
                                              int inFirstOutput, 
                                              int inOutputCount, 
                                              pass_info& outPass)
{
  assert( !structure->isUnion() );
  
  bool used = false;
  
  int elementCount = structure->nComponents;
  int componentIndex = inComponentIndex;
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
        used = expandOutputArgumentStructureDecl( shader, argumentName, 
                                                  inArgumentIndex, 
                                                  componentIndex, 
                                                  structure, ioOutputReg, 
                                                  inFirstOutput, inOutputCount,
                                                  outPass ) || used;
      else
        {
          int outr = ioOutputReg++;
          if( outr >= inFirstOutput
              && outr < inFirstOutput+inOutputCount )
            {
assert(0);
              used = true;
              // it had better be just a floatN
              shader << "#ifdef DXPIXELSHADER\n\t\t";
              shader << "out float4 __output_" << outr;
              shader << " : COLOR" << (outr - inFirstOutput);
              shader << ",\n\t\t";
              shader << "#else\n\t\t";

              shader << "out float";

              switch(base->typemask) {
              case BT_Float:
              case BT_Fixed:
              case BT_ShortFixed:
                break;
              case BT_Float2:
              case BT_Double:
              case BT_Fixed2:
              case BT_ShortFixed2:
                shader << "2";
                break;
              case BT_Float3:
              case BT_Fixed3:
              case BT_ShortFixed3:
                shader << "3";
                break;
              case BT_Float4:
              case BT_Double2:
              case BT_Fixed4:
              case BT_ShortFixed4:
                shader << "4";
                break;
              default:
                fprintf (stderr, "Unknown output type\n");
                exit(1);
              }
	      shader << " __output_" << outr;
              shader << " : COLOR" << (outr - inFirstOutput);
              shader << ",\n\t\t";
              shader << "#endif\n\t\t";

              outPass.addOutput( inArgumentIndex, componentIndex );
            }
          componentIndex++;
        }
    }
  return used;
}


static bool expandOutputArgumentDecl(std::ostream& shader, 
                                     const std::string& argumentName, 
                                     int inArgumentIndex, 
                                     int inComponentIndex, 
                                     Type* form, 
                                     int& ioOutputReg, 
                                     int inFirstOutput, int inOutputCount, pass_info& outPass)
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
      return expandOutputArgumentStructureDecl( shader, argumentName, inArgumentIndex, 
                                                inComponentIndex, structure, ioOutputReg, 
                                                inFirstOutput, inOutputCount, outPass );
    }
  else
    {
      BaseTypeSpec mask = form->getBase()->typemask;
      int outr = ioOutputReg++;
      if( outr < inFirstOutput ) return false;
      if( outr >= inFirstOutput+inOutputCount ) return false;
 
      shader << "#ifdef DXPIXELSHADER\n";

      shader << "\t\tout float4 __output_" << outr;
      shader << " : COLOR" << (outr - inFirstOutput);
      shader << ",\n\t\t";

      shader << "#else\n";

      bool GLES_compliance=true;

      //print the stream type in a comment to be used later from the GLES or other backend
      std::stringstream s;
      std::stringstream typeonly;
      s << ",//GL_ES_";
      form->printBase(s,0);
      form->printBase(typeonly,0);

      if( (outr>0) ||
           (typeonly.str().find("fixed") == std::string::npos) && 
           (typeonly.str().find("char") == std::string::npos) && 
            (
             (typeonly.str().find("2")!=std::string::npos) || 
             (typeonly.str().find("3")!=std::string::npos) || 
             (typeonly.str().find("4")!=std::string::npos)
            )
        )
      {
         GLES_compliance=false;
         printf("Error: This kernel is cannot be used with the GLES backend, because it outputs more than a 32-bit value. GLES code generation is skipped. If you want to be used with GLES, please rewrite the kernel.\n");
         exit(-1);
      }

      if(GLES_compliance)
      {
         shader << "\t\t\t#ifdef GL_ES\n";
         //In GLES 2.0 we only have one output and must be declared float, independently of our custom encoding
         shader << "\t\t\t" ;

         if(typeonly.str().find("fixed")!=std::string::npos)
           shader << typeonly.str() ;
         else if(typeonly.str().find("2")!=std::string::npos)
           shader << "out float2";
         else if(typeonly.str().find("3")!=std::string::npos)
           shader << "out float3";
         else if(typeonly.str().find("4")!=std::string::npos)
           shader << "out float4";
         else
           shader << "out float";

         shader << " __output_" << outr;
         shader << " : COLOR" << (outr - inFirstOutput);

         shader << s.str();
         shader << "\n\t\t\t";
         shader << "#else\n\t";
      }

      if( mask & BT_UserType )
      {
         mask = form->getBase()->typeName->entry->uVarDecl->form->getBase()->typemask;
      }
      switch(mask) {
      case BT_Char:
      case BT_UChar:
      case BT_UnSigned|BT_Char:
        shader << "\t\tout float";
        break;
      case BT_Float:
      case BT_Fixed:
      case BT_ShortFixed:
        shader << "\t\tout float";
        break;
      case BT_Float2:
      case BT_Fixed2:
      case BT_ShortFixed2:
      case BT_Double:
      case BT_Char2:
      case BT_UChar2:
        shader << "\t\tout float";
        shader << "2";
        break;
      case BT_Fixed3:
      case BT_ShortFixed3:
      case BT_Float3:
      case BT_Char3:
      case BT_UChar3:
        shader << "\t\tout float";
        shader << "3";
        break;
      case BT_Fixed4:
      case BT_ShortFixed4:
      case BT_Float4:
      case BT_Double2:
      case BT_Char4:
      case BT_UChar4:
        shader << "\t\tout float";
        shader << "4";
        break;
      case BT_UnSigned:
      case BT_UnSigned|BT_Int:
        shader << "\t\tout unsigned int";
        break;
      case BT_Int:
        shader << "\t\tout int";
        break;
      default:
        fprintf(stderr, "Strange stream base type: ");
        form->getBase()->printBase(std::cerr, 0);
        abort();      
      }
      shader << " __output_" << outr;
      shader << " : COLOR" << (outr - inFirstOutput);
      shader << ",\n\t\t";
      
      if(GLES_compliance)
         shader << "\t#endif\n\t\t";

      shader << "#endif\n";

      outPass.addOutput( inArgumentIndex, inComponentIndex );
      
      return true;
    }
}

static void expandSimpleOutputArgumentWrite(
                                            std::ostream& shader, 
                                            const std::string& argumentName, 
                                            Type* form, int& outputReg,
                                            int inFirstOutput,
                                            int inOutputCount )
{
  int outr = outputReg++;
  if( outr < inFirstOutput || outr >= inFirstOutput + inOutputCount ) return;

  assert( form );
  BaseType* base = form->getBase();
  assert( base );
  
  shader << "\t#ifdef DXPIXELSHADER\n";

  shader << "\t__output_" << outr << " = ";
  if( base->typemask & BT_UserType )
  {
     base = base->typeName->entry->uVarDecl->form->getBase();
  }
  switch(base->typemask) {
  case BT_Char:
    shader << "char4( " << argumentName << ", 0, 0, 0);\n";
    break;
  case BT_Char2:
    shader << "char4( " << argumentName << ", 0, 0);\n";
    break;
  case BT_Char3:
    shader << "char4( " << argumentName << ", 0);\n";
    break;
  case BT_Char4:
    shader << "char4( " << argumentName << " );\n";
    break;
  case BT_UnSigned|BT_Char:
  case BT_UChar:
    shader << "unsigned_char4( " << argumentName << ", 0, 0, 0);\n";
    break;
  case BT_UChar2:
    shader << "unsigned_char4( " << argumentName << ", 0, 0);\n";
    break;
  case BT_UChar3:
    shader << "unsigned_char4( " << argumentName << ", 0);\n";
    break;
  case BT_UChar4:
    shader << "unsigned_char4( " << argumentName << " );\n";
    break;
  case BT_Float:
  case BT_Fixed:
  case BT_ShortFixed:
    shader << "float4( " << argumentName << ", 0, 0, 0);\n";
    break;
  case BT_Float2:
  case BT_Fixed2:
  case BT_ShortFixed2:
    shader << "float4( " << argumentName << ", 0, 0);\n";
    break;
  case BT_Float3:
  case BT_Fixed3:
  case BT_ShortFixed3:
    shader << "float4( " << argumentName << ", 0);\n";
    break;
  case BT_Float4:
  case BT_Fixed4:
  case BT_ShortFixed4:
    shader << argumentName << ";\n";
    break;
  case BT_Double:
    shader << "float4( "<<argumentName<<".x, 0, 0)\n";
    break;
  case BT_Double2:
    shader << "float4( "<<argumentName<<".xy)\n";
    break;
  case BT_UnSigned:
  case BT_UnSigned|BT_Int:
    shader << "uint4( " << argumentName << ", 0, 0, 0);\n";
    break;
  case BT_Int:
    shader << "int( " << argumentName << ", 0, 0, 0);\n";
    break;
  default:
    fprintf(stderr, "Strange stream base type: ");
    base->printBase(std::cerr, 0);
    abort();
  }

  shader << "\t#else\n";
  shader << "\t__output_" << outr << " = ";
  switch (base->typemask) {
  case BT_Double:
    shader << argumentName <<".x;\n";
    break;
  case BT_Double2:
    shader << "float4 ("<< argumentName << ".xy);\n";
    break;
  default:
    shader << argumentName << ";\n";
  }
  shader << "\t#endif\n";
    
}

static void 
expandOutputArgumentStructureWrite( std::ostream& shader,
                                    const std::string& fieldName, 
                                    StructDef* structure, int& ioOutputReg, 
                                    int inFirstOutput, int inOutputCount )
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
          expandOutputArgumentStructureWrite( shader, subFieldName, 
                                              structure, ioOutputReg, 
                                              inFirstOutput, inOutputCount );
        }
      else
        {
          expandSimpleOutputArgumentWrite( shader, subFieldName, 
                                           base, ioOutputReg, 
                                           inFirstOutput, inOutputCount );
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


static void
expandStreamStructureSamplerDecls(std::ostream& shader,
                                  const std::string& argumentName,
                                  int inArgumentIndex, int inComponentIndex,
                                  StructDef* structure,
                                  int& ioIndex, int& ioSamplerReg, pass_info& outPass, int rank)
{
   assert(!structure->isUnion());

   int elementCount = structure->nComponents;
   for (int i = 0; i < elementCount; i++ ) {
      Decl* elementDecl = structure->components[i];
      if (elementDecl->storage & ST_Static ) continue;
      Type* form = elementDecl->form;

      // TIM: for now
      assert( form->isBaseType() );
      BaseType* base = form->getBase();
      StructDef* structure = findStructureDef( base );
      if (structure)
         expandStreamStructureSamplerDecls(shader, argumentName, inArgumentIndex, inComponentIndex,
                                           structure, ioIndex, ioSamplerReg, outPass, rank );
      else {
         shader << "uniform _stype" << rank << " __structsampler" << ioIndex++
                << "_" << argumentName;
         shader << " : register (s" << ioSamplerReg++ << ")";

         std::stringstream s;
         s << ",//GL_ES_in ";
         base->printBase(s,0);
         shader << s.str();

         shader <<  "\n\t\t";

         outPass.addSampler( inArgumentIndex, inComponentIndex+i );
      }
   }
}


static void
expandStreamSamplerDecls(std::ostream& shader,
                         const std::string& inArgumentName,
                         int inArgumentIndex, int inComponentIndex,
                         Type* inForm, int& samplerreg, pass_info& outPass)
{
  StructDef* structure = NULL;
  int rank = 2; // TIM: TODO: find out how to get correct rank, for now assume 2

  if (inForm->isStream()) {
	  BrtStreamType* streamType = (BrtStreamType*)(inForm);

	  // TIM: can't handle arrays with a BaseType
	  BaseType* elementType = streamType->getBase();
	  structure = findStructureDef( elementType );
  } else {
    assert( (inForm->getQualifiers() & TQ_Reduce) != 0 );
    structure = findStructureDef( inForm );
  }

  if (structure) {
     int index = 0;

     expandStreamStructureSamplerDecls(shader, inArgumentName,
                                       inArgumentIndex, inComponentIndex,
                                       structure, index, samplerreg, outPass, rank );
  } else {
     // it had better be just a floatN
     // Output a sampler, texcoord, and scale_bias for
     // a stream
     shader << "uniform _stype" << rank << " _tex_" << inArgumentName;
     shader << " : register (s" << samplerreg++ << ")";

     //print the stream type in a comment to be used later from the GLES or other backend
     std::stringstream s;
     s << ",//GL_ES_in ";
     inForm->printBase(s,0);
     shader << s.str();

     shader <<  "\n\t\t";

     outPass.addSampler( inArgumentIndex, inComponentIndex );
  }
}

static void
expandStreamStructureFetches(std::ostream& shader,
                             const std::string& argumentName,
                             const std::string& fieldName,
                             StructDef* structure, int& ioIndex,
                             const std::string& positionName)
{
   assert(!structure->isUnion());

   int elementCount = structure->nComponents;
   for (int i = 0; i < elementCount; i++) {
      Decl* elementDecl = structure->components[i];
      if (elementDecl->storage & ST_Static) continue;
      Type* form = elementDecl->form;

      std::string subFieldName = fieldName + "." + elementDecl->name->name;

      // TIM: for now
      assert(form->isBaseType());
      BaseType* base = form->getBase();
      StructDef* structure = findStructureDef(base);

      if (structure) {
         expandStreamStructureFetches(shader, argumentName, subFieldName,
                                      structure, ioIndex, positionName);
      } else {
         shader << subFieldName << " = ";

         switch(base->typemask) {
         case BT_Float:
         case BT_Fixed:
         case BT_ShortFixed:
            shader << "__fetch_float";
            break;
         case BT_Float2:
         case BT_Fixed2:
         case BT_ShortFixed2:
            shader << "__fetch_float2";
            break;
         case BT_Float3:
         case BT_ShortFixed3:
         case BT_Fixed3:
            shader << "__fetch_float3";
            break;
         case BT_Float4:
         case BT_Fixed4:
         case BT_ShortFixed4:
            shader << "__fetch_float4";
            break;
         case BT_Double:
           shader << "__fetch_double";
           break;
         case BT_Double2:
           shader << "__fetch_double2";
           break;
         default:
            shader << "__gatherunknown";
            break;
         }

         shader << "( __structsampler" << ioIndex++
                << "_" << argumentName << ", _tex_"
                << positionName << "_pos );\n";
      }
   }
}


static void
expandStreamFetches(std::ostream& shader, const std::string& argumentName,
                    Type* inForm, const char* inPositionName = NULL, const char* lvalueName=NULL)
{
  StructDef* structure = NULL;
  Type* elementType = NULL;

  std::string positionName =
     (inPositionName != NULL) ? inPositionName : argumentName;

  if (inForm->isStream()) {
     BrtStreamType* streamType = (BrtStreamType*)(inForm);

     // TIM: can't handle arrays with a BaseType
     elementType = streamType->getBase();
     structure = findStructureDef(elementType);
  } else {
     assert((inForm->getQualifiers() & TQ_Reduce) != 0);
     elementType = inForm;
     structure = findStructureDef(inForm);
  }

  if (structure) {
     int index = 0;
     expandStreamStructureFetches(shader, positionName, argumentName,
                                  structure, index, positionName);
  } else {
     if(lvalueName)
        shader << lvalueName << " = ";
     else
        shader << positionName << " = ";
#define TIMISnotCRAZY
#ifdef TIMISCRAZY
     shader << "float4(_tex_" << positionName << "_pos,0,0);\n";
#else
     BaseType* base = inForm->getBase();
     if( base->typemask & BT_UserType )
     {
        base = base->typeName->entry->uVarDecl->form->getBase();
     }
     switch(base->typemask) {
     case BT_Double:
       shader <<"__fetch_double";
       break;
     case BT_Double2:
       shader <<"__fetch_double2";
       break;
     case BT_Float:
     case BT_ShortFixed:
     case BT_Fixed:
        shader << "__fetch_float";
        break;
     case BT_Float2:
     case BT_ShortFixed2:
     case BT_Fixed2:
        shader << "__fetch_float2";
        break;
     case BT_Float3:
     case BT_ShortFixed3:
     case BT_Fixed3:
        shader << "__fetch_float3";
        break;
     case BT_Float4:
     case BT_ShortFixed4:
     case BT_Fixed4:
        shader << "__fetch_float4";
        break;
      case BT_UnSigned:
      case BT_UnSigned|BT_Int:
        shader << "__fetch_unsigned_int";
        break;
      case BT_Int:
        shader << "__fetch_int";
        break;
      case BT_Char:
        shader << "__fetch_char";
        break;
      case BT_Char2:
        shader << "__fetch_char2";
        break;
      case BT_Char3:
        shader << "__fetch_char3";
        break;
      case BT_Char4:
        shader << "__fetch_char4";
        break;
      case BT_UChar:
        shader << "__fetch_unsigned_char";
        break;
      case BT_UChar2:
        shader << "__fetch_unsigned_char2";
        break;
      case BT_UChar3:
        shader << "__fetch_unsigned_char3";
        break;
      case BT_UChar4:
        shader << "__fetch_unsigned_char4";
        break;
      case BT_UnSigned|BT_Char:
        shader << "__fetch_unsigned_char";
        break;
     default:
        fprintf(stderr, "Can't fetch from unknown stream type ");
        base->printBase(std::cerr, 0);
        fprintf(stderr, "\n");
        shader << "__fetchunknown";
        break;
     }

     shader << "(_tex_" << argumentName ;
     //when we provide lvalueName it means that we use a custom positionName so don't mangle it
     if(lvalueName)
            shader << ", " << positionName << " );\n";
     else
            shader << ", _tex_" << positionName << "_pos );\n";
#endif
  }
}


static void
generate_shader_support(std::ostream& shader)
{

  shader << "#if defined(DXPIXELSHADER)\n";
  shader << "#define fixed float\n";
  shader << "#define fixed2 float2\n";
  shader << "#define fixed3 float3\n";
  shader << "#define fixed4 float4\n";
  shader << "#endif\n";
  shader << "#define shortfixed float\n";
  shader << "#define shortfixed2 float2\n";
  shader << "#define shortfixed3 float3\n";
  shader << "#define shortfixed4 float4\n";
  shader << "#define double real\n";
  shader << "#define double2 real2\n";
  shader << "typedef struct double_struct {float2 x;} real;\n";
  shader << "typedef struct double2_struct {float4 xy;} real2;\n";
  shader << "#if defined(DXPIXELSHADER) || !defined(USERECT)\n";
  shader << "#define _stype1   sampler2D\n";
  shader << "#define _stype2   sampler2D\n";
  shader << "#define _stype3   sampler3D\n";
  shader << "#if defined(SHADERMODEL3)\n";
  shader << "#define __sample1(s,i) tex2Dlod((s),float4(i,0,0,0))\n";
  shader << "#define __sample2(s,i) tex2Dlod((s),float4(i,0,0))\n";
  shader << "#define __sample3(s,i) tex3Dlod((s),float4(i,0))\n";
  shader << "#else\n";
  shader << "#define __sample1(s,i) tex2D((s),float2(i,0))\n";
  shader << "#define __sample2(s,i) tex2D((s),(i))\n";
  shader << "#define __sample3(s,i) tex3D((s),(i))\n";
  shader << "#endif\n";
  shader << "#else\n";
  shader << "#define _stype1  samplerRECT\n";
  shader << "#define _stype2  samplerRECT\n";
  shader << "#define _stype3  sampler3D\n";
  shader << "#define __sample1(s,i) texRECT((s),float2(i,0))\n";
  shader << "#define __sample2(s,i) texRECT((s),(i))\n";
  shader << "#define __sample3(s,i) tex3D((s),(i))\n";
  shader << "#endif\n\n";

  shader << "#define __FRAGMENTKILL discard\n";

  shader << "#ifdef USERECT\n";
  shader << "#define SKIPSCALEBIAS\n";
  shader << "#endif\n\n";

  if( !globals.enableGPUAddressTranslation ) {
    shader << "#ifdef SKIPSCALEBIAS\n";
    shader << "float __gatherindex1( float index, float4 scalebias ) { ";
    shader << "return (index+scalebias.z); }\n";
    shader << "float2 __gatherindex2( float2 index, float4 scalebias ) { ";
    shader << "return (index+scalebias.zw); }\n";
    shader << "float3 __gatherindex3( float3 index, float4 scalebias ) { ";
    shader << "return index; }\n";
    shader << "#define _computeindexof(a,b) float4(a, 0, 0)\n";
    shader << "#else\n";
    shader << "float __gatherindex1( float index, float4 scalebias ) { ";
    shader << "return index*scalebias.x+scalebias.z; }\n";
    shader << "float2 __gatherindex2( float2 index, float4 scalebias ) { ";
    shader << "return index*scalebias.xy+scalebias.zw; }\n";
    shader << "float3 __gatherindex3( float3 index, float4 scalebias ) { ";
    shader << "return index; }\n";
    shader << "#define _computeindexof(a,b) (b)\n";
    shader << "#endif\n\n";
  }

  // TIM: simple subroutines
  shader << "double __fetch_double( _stype1 s, float i ) { double r; r.x= __sample1(s,i).xy; return r;}\n";
  shader << "double __fetch_double( _stype2 s, float2 i ) { double r; r.x = __sample2(s,i).xy; return r;}\n";
  shader << "double __fetch_double( _stype3 s, float3 i ) { double r; r.x = __sample3(s,i).xy; return r;}\n";

  shader << "double2 __fetch_double2( _stype1 s, float i ) { double2 r; r.xy= __sample1(s,i).xyzw; return r;}\n";
  shader << "double2 __fetch_double2( _stype2 s, float2 i ) { double2 r; r.xy = __sample2(s,i).xyzw; return r;}\n";
  shader << "double2 __fetch_double2( _stype3 s, float3 i ) { double2 r; r.xy = __sample3(s,i).xyzw; return r;}\n";

  shader << "float __fetch_float( _stype1 s, float i ) { return __sample1(s,i).x; }\n";
  shader << "float __fetch_float( _stype2 s, float2 i ) { return __sample2(s,i).x; }\n";
  shader << "float __fetch_float( _stype3 s, float3 i ) { return __sample3(s,i).x; }\n";
  shader << "float2 __fetch_float2( _stype1 s, float i ) { return __sample1(s,i).xy; }\n";
  shader << "float2 __fetch_float2( _stype2 s, float2 i ) { return __sample2(s,i).xy; }\n";
  shader << "float2 __fetch_float2( _stype3 s, float3 i ) { return __sample3(s,i).xy; }\n";
  shader << "float3 __fetch_float3( _stype1 s, float i ) { return __sample1(s,i).xyz; }\n";
  shader << "float3 __fetch_float3( _stype2 s, float2 i ) { return __sample2(s,i).xyz; }\n";
  shader << "float3 __fetch_float3( _stype3 s, float3 i ) { return __sample3(s,i).xyz; }\n";
  shader << "float4 __fetch_float4( _stype1 s, float i ) { return __sample1(s,i).xyzw; }\n";
  shader << "float4 __fetch_float4( _stype2 s, float2 i ) { return __sample2(s,i).xyzw; }\n";
  shader << "float4 __fetch_float4( _stype3 s, float3 i ) { return __sample3(s,i).xyzw; }\n";
  shader << "float __fetch_unsigned_int( _stype1 s, float i ) { return __sample1(s,i).x; }\n";
  shader << "float __fetch_unsigned_int( _stype2 s, float2 i ) { return __sample2(s,i).x; }\n";
  shader << "float __fetch_unsigned_int( _stype3 s, float3 i ) { return __sample3(s,i).x; }\n";
  shader << "float __fetch_int( _stype1 s, float i ) { return __sample1(s,i).x; }\n";
  shader << "float __fetch_int( _stype2 s, float2 i ) { return __sample2(s,i).x; }\n";
  shader << "float __fetch_int( _stype3 s, float3 i ) { return __sample3(s,i).x; }\n";
  shader << "float __fetch_char( _stype1 s, float i ) { return __sample1(s,i).x; }\n";
  shader << "float __fetch_char( _stype2 s, float2 i ) { return __sample2(s,i).x; }\n";
  shader << "float __fetch_char( _stype3 s, float3 i ) { return __sample3(s,i).x; }\n";
  shader << "float2 __fetch_char2( _stype1 s, float i ) { return __sample1(s,i).xy; }\n";
  shader << "float2 __fetch_char2( _stype2 s, float2 i ) { return __sample2(s,i).xy; }\n";
  shader << "float2 __fetch_char2( _stype3 s, float3 i ) { return __sample3(s,i).xy; }\n";
  shader << "float3 __fetch_char3( _stype1 s, float i ) { return __sample1(s,i).xyz; }\n";
  shader << "float3 __fetch_char3( _stype2 s, float2 i ) { return __sample2(s,i).xyz; }\n";
  shader << "float3 __fetch_char3( _stype3 s, float3 i ) { return __sample3(s,i).xyz; }\n";
  shader << "float4 __fetch_char4( _stype1 s, float i ) { return __sample1(s,i).xyzw; }\n";
  shader << "float4 __fetch_char4( _stype2 s, float2 i ) { return __sample2(s,i).xyzw; }\n";
  shader << "float4 __fetch_char4( _stype3 s, float3 i ) { return __sample3(s,i).xyzw; }\n";
  shader << "float __fetch_unsigned_char( _stype1 s, float i ) { return __sample1(s,i).x; }\n";
  shader << "float __fetch_unsigned_char( _stype2 s, float2 i ) { return __sample2(s,i).x; }\n";
  shader << "float __fetch_unsigned_char( _stype3 s, float3 i ) { return __sample3(s,i).x; }\n";
  shader << "float2 __fetch_unsigned_char2( _stype1 s, float i ) { return __sample1(s,i).xy; }\n";
  shader << "float2 __fetch_unsigned_char2( _stype2 s, float2 i ) { return __sample2(s,i).xy; }\n";
  shader << "float2 __fetch_unsigned_char2( _stype3 s, float3 i ) { return __sample3(s,i).xy; }\n";
  shader << "float3 __fetch_unsigned_char3( _stype1 s, float i ) { return __sample1(s,i).xyz; }\n";
  shader << "float3 __fetch_unsigned_char3( _stype2 s, float2 i ) { return __sample2(s,i).xyz; }\n";
  shader << "float3 __fetch_unsigned_char3( _stype3 s, float3 i ) { return __sample3(s,i).xyz; }\n";
  shader << "float4 __fetch_unsigned_char4( _stype1 s, float i ) { return __sample1(s,i).xyzw; }\n";
  shader << "float4 __fetch_unsigned_char4( _stype2 s, float2 i ) { return __sample2(s,i).xyzw; }\n";
  shader << "float4 __fetch_unsigned_char4( _stype3 s, float3 i ) { return __sample3(s,i).xyzw; }\n";

  shader << "\n\n";

  shader << "float __gather_float( _stype1 s[1], float i ) { return __sample1(s[0],i).x; }\n";
  shader << "float __gather_float( _stype2 s[1], float2 i ) { return __sample2(s[0],i).x; }\n";
  shader << "float __gather_float( _stype3 s[1], float3 i ) { return __sample3(s[0],i).x; }\n";
  shader << "float2 __gather_float2( _stype1 s[1], float i ) { return __sample1(s[0],i).xy; }\n";
  shader << "float2 __gather_float2( _stype2 s[1], float2 i ) { return __sample2(s[0],i).xy; }\n";
  shader << "float2 __gather_float2( _stype3 s[1], float3 i ) { return __sample3(s[0],i).xy; }\n";
  shader << "float3 __gather_float3( _stype1 s[1], float i ) { return __sample1(s[0],i).xyz; }\n";
  shader << "float3 __gather_float3( _stype2 s[1], float2 i ) { return __sample2(s[0],i).xyz; }\n";
  shader << "float3 __gather_float3( _stype3 s[1], float3 i ) { return __sample3(s[0],i).xyz; }\n";
  shader << "float4 __gather_float4( _stype1 s[1], float i ) { return __sample1(s[0],i).xyzw; }\n";
  shader << "float4 __gather_float4( _stype2 s[1], float2 i ) { return __sample2(s[0],i).xyzw; }\n";
  shader << "float4 __gather_float4( _stype3 s[1], float3 i ) { return __sample3(s[0],i).xyzw; }\n";

  shader << "float __gather_shortfixed( _stype1 s[1], float i ) { return __sample1(s[0],i).x; }\n";
  shader << "float __gather_shortfixed( _stype2 s[1], float2 i ) { return __sample2(s[0],i).x; }\n";
  shader << "float __gather_shortfixed( _stype3 s[1], float3 i ) { return __sample3(s[0],i).x; }\n";
  shader << "float2 __gather_shortfixed2( _stype1 s[1], float i ) { return __sample1(s[0],i).xy; }\n";
  shader << "float2 __gather_shortfixed2( _stype2 s[1], float2 i ) { return __sample2(s[0],i).xy; }\n";
  shader << "float2 __gather_shortfixed2( _stype3 s[1], float3 i ) { return __sample3(s[0],i).xy; }\n";
  shader << "float3 __gather_shortfixed3( _stype1 s[1], float i ) { return __sample1(s[0],i).xyz; }\n";
  shader << "float3 __gather_shortfixed3( _stype2 s[1], float2 i ) { return __sample2(s[0],i).xyz; }\n";
  shader << "float3 __gather_shortfixed3( _stype3 s[1], float3 i ) { return __sample3(s[0],i).xyz; }\n";
  shader << "float4 __gather_shortfixed4( _stype1 s[1], float i ) { return __sample1(s[0],i).xyzw; }\n";
  shader << "float4 __gather_shortfixed4( _stype2 s[1], float2 i ) { return __sample2(s[0],i).xyzw; }\n";
  shader << "float4 __gather_shortfixed4( _stype3 s[1], float3 i ) { return __sample3(s[0],i).xyzw; }\n";


  shader << "float __gather_fixed( _stype1 s[1], float i ) { return __sample1(s[0],i).x; }\n";
  shader << "float __gather_fixed( _stype2 s[1], float2 i ) { return __sample2(s[0],i).x; }\n";
  shader << "float __gather_fixed( _stype3 s[1], float3 i ) { return __sample3(s[0],i).x; }\n";
  shader << "float2 __gather_fixed2( _stype1 s[1], float i ) { return __sample1(s[0],i).xy; }\n";
  shader << "float2 __gather_fixed2( _stype2 s[1], float2 i ) { return __sample2(s[0],i).xy; }\n";
  shader << "float2 __gather_fixed2( _stype3 s[1], float3 i ) { return __sample3(s[0],i).xy; }\n";
  shader << "float3 __gather_fixed3( _stype1 s[1], float i ) { return __sample1(s[0],i).xyz; }\n";
  shader << "float3 __gather_fixed3( _stype2 s[1], float2 i ) { return __sample2(s[0],i).xyz; }\n";
  shader << "float3 __gather_fixed3( _stype3 s[1], float3 i ) { return __sample3(s[0],i).xyz; }\n";
  shader << "float4 __gather_fixed4( _stype1 s[1], float i ) { return __sample1(s[0],i).xyzw; }\n";
  shader << "float4 __gather_fixed4( _stype2 s[1], float2 i ) { return __sample2(s[0],i).xyzw; }\n";
  shader << "float4 __gather_fixed4( _stype3 s[1], float3 i ) { return __sample3(s[0],i).xyzw; }\n";

  shader << "float __gather_char( _stype1 s[1], float i ) { return __sample1(s[0],i).x; }\n";
  shader << "float __gather_char( _stype2 s[1], float2 i ) { return __sample2(s[0],i).x; }\n";
  shader << "float __gather_char( _stype3 s[1], float3 i ) { return __sample3(s[0],i).x; }\n";
  shader << "float2 __gather_char2( _stype1 s[1], float i ) { return __sample1(s[0],i).xy; }\n";
  shader << "float2 __gather_char2( _stype2 s[1], float2 i ) { return __sample2(s[0],i).xy; }\n";
  shader << "float2 __gather_char2( _stype3 s[1], float3 i ) { return __sample3(s[0],i).xy; }\n";
  shader << "float3 __gather_char3( _stype1 s[1], float i ) { return __sample1(s[0],i).xyz; }\n";
  shader << "float3 __gather_char3( _stype2 s[1], float2 i ) { return __sample2(s[0],i).xyz; }\n";
  shader << "float3 __gather_char3( _stype3 s[1], float3 i ) { return __sample3(s[0],i).xyz; }\n";
  shader << "float4 __gather_char4( _stype1 s[1], float i ) { return __sample1(s[0],i).xyzw; }\n";
  shader << "float4 __gather_char4( _stype2 s[1], float2 i ) { return __sample2(s[0],i).xyzw; }\n";
  shader << "float4 __gather_char4( _stype3 s[1], float3 i ) { return __sample3(s[0],i).xyzw; }\n";

  shader << "float __gather_uchar( _stype1 s[1], float i ) { return __sample1(s[0],i).x; }\n";
  shader << "float __gather_uchar( _stype2 s[1], float2 i ) { return __sample2(s[0],i).x; }\n";
  shader << "float __gather_uchar( _stype3 s[1], float3 i ) { return __sample3(s[0],i).x; }\n";
  shader << "float2 __gather_uchar2( _stype1 s[1], float i ) { return __sample1(s[0],i).xy; }\n";
  shader << "float2 __gather_uchar2( _stype2 s[1], float2 i ) { return __sample2(s[0],i).xy; }\n";
  shader << "float2 __gather_uchar2( _stype3 s[1], float3 i ) { return __sample3(s[0],i).xy; }\n";
  shader << "float3 __gather_uchar3( _stype1 s[1], float i ) { return __sample1(s[0],i).xyz; }\n";
  shader << "float3 __gather_uchar3( _stype2 s[1], float2 i ) { return __sample2(s[0],i).xyz; }\n";
  shader << "float3 __gather_uchar3( _stype3 s[1], float3 i ) { return __sample3(s[0],i).xyz; }\n";
  shader << "float4 __gather_uchar4( _stype1 s[1], float i ) { return __sample1(s[0],i).xyzw; }\n";
  shader << "float4 __gather_uchar4( _stype2 s[1], float2 i ) { return __sample2(s[0],i).xyzw; }\n";
  shader << "float4 __gather_uchar4( _stype3 s[1], float3 i ) { return __sample3(s[0],i).xyzw; }\n";

  shader << "#ifdef GL_ES\n";
  shader << "#define unsigned \n";
  shader << "#define int float\n";
  shader << "#define char float\n";
  shader << "#endif\n";

  shader << "typedef packed unsigned char uchar[1];\n";
  shader << "typedef packed unsigned char uchar2[2];\n";
  shader << "typedef packed unsigned char uchar3[3];\n";
  shader << "typedef packed unsigned char uchar4[4];\n";

  if (globals.enableGPUAddressTranslation) {
    shader << "\n\n";

    shader << "float4 __calculateindexof( float4 indexofoutput, float4 streamIndexofNumer, float4 streamIndexofInvDenom ) {\n";
    shader << "\treturn floor( (indexofoutput*streamIndexofNumer + 0.5)*streamIndexofInvDenom ); }\n";

    shader << "float2 __calculatetexpos( float4 streamIndex, float4 streamDomainMin,\n";
    shader << "float4 linearizeConst, float4 textureShapeConst ) {\n";
    shader << "float linearIndex = dot( streamIndex + streamDomainMin, linearizeConst ) + 0.5;\n";
    shader << "float2 texIndex;\n";
    shader << "texIndex.y = floor( linearIndex * textureShapeConst.x );\n";
    shader << "texIndex.x = floor( linearIndex - texIndex.y * textureShapeConst.z );\n";
    shader << "float2 texCoord = texIndex + 0.5;\n";
    shader << "#ifndef USERECT\n";
    shader << "// convert to 0-to-1 texture space\n";
    shader << "texCoord *= textureShapeConst.xy;\n";
    shader << "#endif\n";
#ifndef TIMISCRAZY
    shader << "return texCoord;\n}\n\n";
#else
    shader << "return streamIndex.y;\n}\n\n";
#endif

    shader << "void __calculateoutputpos( float2 interpolant, float2 linearize,\n";
    shader << "\tfloat4 stride, float4 invStride, float4 invExtent, float4 domainMin, float4 domainExtent, out float4 index ) {\n";
    shader << "\tfloat2 cleanInterpolant = floor( interpolant );\n";
    shader << "\tfloat linearIndex = dot( cleanInterpolant, linearize );\n";
    shader << "\tfloat4 temp0 = floor( (linearIndex + 0.5) * invStride );\n";
    shader << "\tfloat4 temp1 = linearIndex - temp0*stride;\n";
    shader << "\tindex = floor( (temp1 + 0.5) * invExtent - domainMin );\n";
    shader << "\tif( any( index < 0 ) ) __FRAGMENTKILL;\n";
    shader << "\tif( any( index >= domainExtent ) ) __FRAGMENTKILL;\n";
    shader << "}\n\n";

    shader << "float4 __calculateiteratorvalue( float4 index,\n"
        << "float4 valueBase, float4 valueOffset1, float4 valueOffset4 ) {\n"
        << "return valueBase + index.x*valueOffset1 + index*valueOffset4;\n}\n\n";

#if 0
    shader << "void __calculateoutputpos( float2 interpolant, float4 outputConst, out float4 index ) {\n";
    shader << "\tfloat2 cleanInterpolant = floor(interpolant);\n";
    shader << "\tfloat linearIndex = cleanInterpolant.y*outputConst.x + cleanInterpolant.x;\n";
    shader << "\tfloat scaledIndex = linearIndex * outputConst.y;\n";
    shader << "\tfloat fraction = frac( scaledIndex );\n";
    shader << "\tindex.y = scaledIndex - fraction;\n";
    shader << "\tindex.x = floor( index.y * outputConst.z + linearIndex + outputConst.w );\n";
//    shader << "\tindex.x = floor( fraction * outputConst.z + outputConst.w );\n";
    shader << "\tindex.z = 0;\n";
    shader << "\tindex.w = 0;\n";
    shader << "}\n\n";
#endif

    shader << "float2 __gatherindex1( float1 index, float4 domainConst, float4 linearizeConst, float4 reshapeConst ) {\n";
    shader << "\treturn __calculatetexpos( float4(floor(index+domainConst.x),0,0,0), 0, linearizeConst, reshapeConst ); }\n";
    shader << "float2 __gatherindex2( float2 index, float4 domainConst, float4 linearizeConst, float4 reshapeConst ) {\n";
    shader << "\treturn __calculatetexpos( float4(floor(index+domainConst.xy),0,0), 0, linearizeConst, reshapeConst ); }\n";
    shader << "float2 __gatherindex3( float3 index, float4 domainConst, float4 linearizeConst, float4 reshapeConst ) {\n";
    shader << "\treturn __calculatetexpos( float4(floor(index+domainConst.xyz),0), 0, linearizeConst, reshapeConst ); }\n";
    shader << "float2 __gatherindex4( float4 index, float4 domainConst, float4 linearizeConst, float4 reshapeConst ) {\n";
    shader << "\treturn __calculatetexpos( floor(index+domainConst), 0, linearizeConst, reshapeConst ); }\n";
  }

  shader << "\n\n";
}

static void
generate_shader_iter_arg(std::ostream& shader, Decl *arg, int i, int& texcoord, int& constant, pass_info& outPass, bool& hasDoneStreamDim)
{
   std::string argName = arg->name->name;
   TypeQual qual = arg->form->getQualifiers();

   if (globals.enableGPUAddressTranslation) {
       shader << "uniform float4 __iterindexofnumer_" << argName << " : register(c" << constant++ << ")";
      shader << ",\n\t\t";
      shader << "uniform float4 __iterindexofdenom_" << argName << " : register(c" << constant++ << ")";
      shader << ",\n\t\t";
      shader << "uniform float4 __itervaluebase_" << argName << " : register(c" << constant++ << ")";
      shader << ",\n\t\t";
      shader << "uniform float4 __itervalueoffset1_" << argName << " : register(c" << constant++ << ")";
      shader << ",\n\t\t";
      shader << "uniform float4 __itervalueoffset4_" << argName << " : register(c" << constant++ << ")";
      shader << ",\n\t\t";

      outPass.addConstant( (i+1), "kIteratorConstant_ATIndexofNumer" );
      outPass.addConstant( (i+1), "kIteratorConstant_ATIndexofDenom" );
      outPass.addConstant( (i+1), "kIteratorConstant_ATValueBase" );
      outPass.addConstant( (i+1), "kIteratorConstant_ATValueOffset1" );
      outPass.addConstant( (i+1), "kIteratorConstant_ATValueOffset4" );

      // no real support under address translation yet
   } else {
      // Just output a texcoord for an iterator
      arg->form->getBase()->qualifier &= ~TQ_Iter;
      arg->form->printBase(shader, 0);
      arg->form->getBase()->qualifier = qual;
      shader << argName << " : TEXCOORD" << texcoord++;
      shader <<  ",\n\t\t";

      outPass.addInterpolant( (i+1), "kIteratorInterpolant_Value" );
   }

   //in GLES we need StreamDim for iterators because they are normalised
   //We also need a bias to shift it, equal to (StreamEnd - StreamStart)/2 which the runtime passes to us
   if(!hasDoneStreamDim)
   {
      hasDoneStreamDim=true;
      shader << "#ifdef GL_ES\n\t\t"
             << "uniform float4 StreamDim"
             << " : register (c" << constant++ << ")";
      shader <<  ",\n\t\t";
      outPass.addConstant( (i+1), "StreamDim" );

      shader << "uniform float4 IteratorBias"
             << " : register (c" << constant++ << ")";
      shader <<  ",\n\t\t#endif\n\t\t";

      outPass.addConstant( (i+1), "IteratorBias" );
   }
}


static void
generate_shader_out_arg(std::ostream& shader, Decl *arg,
                        bool& hasDoneIndexofOutput, bool needIndexOfArg,
                        int i, int& texcoord, int &constreg, pass_info& outPass, bool& hasDoneStreamDim)
{
   std::string argName = arg->name->name;

   if (globals.enableGPUAddressTranslation) {
      // index of output should already be available...
   } else if (!hasDoneIndexofOutput && needIndexOfArg) {
      hasDoneIndexofOutput = true;
      shader << "uniform float4 _const_" << argName
             << "_invscalebias" << " : register (c" << constreg++ << ")";
      shader <<  ",\n\t\t";
      shader << "float2 _tex_" << argName << "_pos : TEXCOORD"
             << texcoord++;
      shader <<  ",\n\t\t";

      outPass.addConstant( (i+1), "kOutputConstant_Indexof" );
      outPass.addInterpolant( (i+1), "kOutputInterpolant_Position" );

      if(!hasDoneStreamDim)
      {
         hasDoneStreamDim=true;
	 //In GLES indexof returns normalised cordinates. We need to scale them
	 //based on the size of the stream, so we get them as argument in StreamDim
         shader << "#ifdef GL_ES\n\t\t"
                << "uniform float4 StreamDim"
                << " : register (c" << constreg++ << ")";
         shader <<  ",\n\t\t#endif\n\t\t";
         outPass.addConstant( (i+1), "StreamDim" );
      }
   }
}


static void
generate_reduction_stream_arg(std::ostream& shader, Decl *arg,
                              bool& reductionArgumentComesBeforeStreamArgument,
                              std::vector<int>& reductionStreamArguments,
                              int reductionFactor, int i,
                              int& texcoord, int& samplerreg, pass_info& outPass, 
                              CodeGenTarget target, int &constreg, bool& hasDoneStreamDim, bool& hasDoneOutStreamDim, bool& hasDoneReductionStep)
{
   std::string argName = arg->name->name;
   TypeQual qual = arg->form->getQualifiers();

   if ((qual & TQ_Reduce) != 0 && reductionStreamArguments.size() == 0)
      reductionArgumentComesBeforeStreamArgument = true;

   reductionStreamArguments.push_back(i);

   if (reductionStreamArguments.size() == 2) {
      expandStreamSamplerDecls(shader, argName, 0, 1, arg->form, samplerreg, outPass );
      //In gles we use only one texture which we adjust appropriately
      //so don't generate new additional arguments
      //TODO Implement this for all backends so the entire loop will be gone
      if( target != CODEGEN_GLES )
      for (int r = 2; r < reductionFactor; r++) {
          std::stringstream s;
          s << "__reduce" << r;
          std::string adjustedArgName = s.str();

          shader << "float2 _tex_" << adjustedArgName << "_pos : TEXCOORD"
                << texcoord++;
          shader <<  ",\n\t\t";

          outPass.addInterpolant( 0, r-1 );
      }
   }
   else
   {
     expandStreamSamplerDecls(shader, argName, 0, 0, arg->form, samplerreg, outPass );
   }



   
   if(!hasDoneStreamDim) {
      hasDoneStreamDim= true;
      //In GLES we need the StreamDim to adjust appropriately the normalised coordinates
      shader << "#ifdef GL_ES\n\t\t"
             << "uniform float4 StreamDim"
             << " : register (c" << constreg++ << ")";
      shader <<  ",\n\t\t#endif\n\t\t";
      outPass.addConstant( (i+1), "StreamDim" );
   }

   //TODO This is not required anymore for reductions, but it may be usefull for other stuff
   if(!hasDoneOutStreamDim) {
      hasDoneOutStreamDim= true;
      //In GLES we need the outStreamDim to adjust appropriately the normalised coordinates
      shader << "#ifdef GL_ES\n\t\t"
             << "uniform float4 outStreamDim"
             << " : register (c" << constreg++ << ")";
      shader <<  ",\n\t\t#endif\n\t\t";
      outPass.addConstant( (i+1), "outStreamDim" );
   }

   if(!hasDoneReductionStep) {
      hasDoneReductionStep= true;
      //In GLES we need the ReductionStep to adjust appropriately the indices (coordinates)
      //TODO other GPU backends to be updated as well to use this technique
      shader << "#ifdef GL_ES\n\t\t"
             << "uniform float4 ReductionStep"
             << " : register (c" << constreg++ << ")";
      shader <<  ",\n\t\t#endif\n\t\t";
      outPass.addConstant( (i+1), "ReductionStep" );
   }
}


static void
generate_map_stream_arg(std::ostream& shader, Decl *arg, bool needIndexOfArg, int i,
                        int& texcoord, int& constreg, int& samplerreg, pass_info& outPass, 
                        bool& hasDoneStreamDim)
{
   std::string argName = arg->name->name;

   expandStreamSamplerDecls(shader, argName, (i+1), 0, arg->form, samplerreg, outPass );

   if (globals.enableGPUAddressTranslation) {
      shader << "uniform float4 __streamindexofnumer_" << argName;
      shader << " : register(c" << constreg++ << ")";
      shader << ",\n\t\t";
      shader << "uniform float4 __streamindexofdenom_" << argName;
      shader << " : register(c" << constreg++ << ")";
      shader << ",\n\t\t";
      shader << "uniform float4 __streamlinearize_" << argName;
      shader << " : register(c" << constreg++ << ")";
      shader << ",\n\t\t";
      shader << "uniform float4 __streamtextureshape_" << argName;
      shader << " : register(c" << constreg++ << ")";
      shader << ",\n\t\t";
      shader << "uniform float4 __streamdomainmin_" << argName;
      shader << " : register(c" << constreg++ << ")";
      shader << ",\n\t\t";

      outPass.addConstant( (i+1), "kStreamConstant_ATIndexofNumer" );
      outPass.addConstant( (i+1), "kStreamConstant_ATIndexofDenom" );
      outPass.addConstant( (i+1), "kStreamConstant_ATLinearize" );
      outPass.addConstant( (i+1), "kStreamConstant_ATTextureShape" );
      outPass.addConstant( (i+1), "kStreamConstant_ATDomainMin" );
   } else {
      // Output a texcoord, and optional scale/bias
      if (needIndexOfArg) {
         shader << "uniform float4 _const_" << argName << "_invscalebias"
                << " : register (c" << constreg++ << ")";
         shader <<  ",\n\t\t";
         outPass.addConstant( (i+1), "kStreamConstant_Indexof" );
         
         if(!hasDoneStreamDim) {
            hasDoneStreamDim= true;
	    //In GLES indexof returns normalised cordinates. We need to scale them
	    //based on the size of the stream, so we get them as argument in StreamDim
            shader << "#ifdef GL_ES\n\t\t"
                   << "uniform float4 StreamDim"
                   << " : register (c" << constreg++ << ")";
            shader <<  ",\n\t\t#endif\n\t\t";
            outPass.addConstant( (i+1), "StreamDim" );
         }
      }
      shader << "float2 _tex_" << argName << "_pos : TEXCOORD" << texcoord++;
      shader <<  ",\n\t\t";

      outPass.addInterpolant( (i+1), "kStreamInterpolant_Position" );
   }
}


static void
generate_shader_gather_arg(std::ostream& shader, Decl *arg, int i,
                           int& constreg, int& samplerreg, pass_info& outPass)
{
   std::string argName = arg->name->name;
   int samplerCount = getGatherStructureSamplerCount(arg->form);
   int rank = getGatherRank(arg->form);

   if (globals.enableGPUAddressTranslation) {
      assert(0);
      shader << "uniform _stype" << rank << " " << argName;
      shader << "[" << samplerCount << "] : register (s" << samplerreg << ")";
      samplerreg += samplerCount;

      for(int s = 0; s < samplerCount; s++)
        outPass.addSampler( (i+1), s );

      shader << ",\n\t\t";
      shader << "uniform float4 __gatherlinearize_" << argName;
      shader << " : register(c" << constreg++ << ")";
      shader << ",\n\t\t";
      shader << "uniform float4 __gathertexshape_" << argName;
      shader << " : register(c" << constreg++ << ")";
      shader <<  ",\n\t\t";
      shader << "uniform float4 __gatherdomainmin_" << argName;
      shader << " : register(c" << constreg++ << ")";
      shader <<  ",\n\t\t";

      outPass.addConstant( (i+1), "kGatherConstant_ATLinearize" );
      outPass.addConstant( (i+1), "kGatherConstant_ATTextureShape" );
      outPass.addConstant( (i+1), "kGatherConstant_ATDomainMin" );
   } else {
      // TIM: TODO: handle multi-sampler array for gathers...
      shader << "uniform _stype" << rank << " " << argName;
      shader << "[" << samplerCount << "] : register (s" << samplerreg << ")";
      samplerreg += samplerCount;

      for(int s = 0; s < samplerCount; s++)
        outPass.addSampler( (i+1), s );

      //print the stream type in a comment to be used later from the GLES or other backend
      std::stringstream s;
      s << ",//GL_ES_in ";
      arg->form->printBase(s,0);
      shader << s.str();

      shader <<  "\n\t\t";
      shader << "uniform float4 __gatherconst_" << argName
             << " : register (c" << constreg++ << ")";
      shader <<  ",\n\t\t";

      outPass.addConstant( (i+1), "kGatherConstant_Shape" );
   }
}


static char *
generate_shader_code (Decl **args, int nArgs, const char* functionName,
                      int inFirstOutput, int inOutputCount,
                      bool fullAddressTrans, int reductionFactor,
                      pass_info& outPass, CodeGenTarget target)
{
  std::ostringstream shader;
  std::vector<int> reductionStreamArguments;
  int texcoord, constreg, samplerreg, outputReg, i;
  bool reductionArgumentComesBeforeStreamArgument = false;
  bool isReduction, hasDoneIndexofOutput, hasDoneStreamDim, hasDoneOutStreamDim, hasDoneReductionStep;

  isReduction = false;
  for (i=0; i < nArgs; i++) {
    if ((args[i]->form->getQualifiers() & TQ_Reduce) != 0) {
       isReduction = true;
       break;
    }
  }

  /*
   * Print a whole bunch of boiler-plate stuff at the top
   */

  generate_shader_support(shader);
  generate_shader_structure_definitions(shader);
  generate_shader_subroutines(shader,functionName);

  //Define _reductionFactor as a global constant to be used by reduction index computation
  if (isReduction)
     shader << "static const float _reductionFactor=" << reductionFactor << ";\n\n";

  shader << "void main (\n\t\t";

  /*
   * Print the argument list
   */

  hasDoneIndexofOutput = false;
  hasDoneStreamDim = false;
  hasDoneOutStreamDim = false;
  hasDoneReductionStep = false;
  constreg = texcoord = samplerreg = outputReg = 0;
  for (i=0; i < nArgs; i++) {
     std::string argName = args[i]->name->name;
     TypeQual qual = args[i]->form->getQualifiers();
     bool needIndexOfArg = FunctionProp[functionName].contains(i);

     /* put the output in the argument list */
     if ((qual & TQ_Out) != 0 || (qual & TQ_Reduce) != 0) {
       expandOutputArgumentDecl(shader, argName, isReduction ? 0 : (i+1),
                                 0, args[i]->form,
                                 outputReg, inFirstOutput, inOutputCount, outPass );
     }

     if (args[i]->isStream() || (qual & TQ_Reduce) != 0) {
        if ((qual & TQ_Iter) != 0) {
           generate_shader_iter_arg(shader, args[i], i, texcoord, constreg, outPass, hasDoneStreamDim);
        } else if ((qual & TQ_Out) != 0) {
           generate_shader_out_arg(shader, args[i], hasDoneIndexofOutput,
                                   needIndexOfArg, i, texcoord, constreg, outPass, hasDoneStreamDim);
        } else {
           if (isReduction) {
              assert(!needIndexOfArg && "can't use indexof in a reduction" );
//printf("%s:%d\n", __FUNCTION__, __LINE__);
              generate_reduction_stream_arg(shader, args[i],
                                            reductionArgumentComesBeforeStreamArgument,
                                            reductionStreamArguments,
                                            reductionFactor, i, texcoord,
                                            samplerreg, outPass, target, constreg, hasDoneStreamDim, hasDoneOutStreamDim, hasDoneReductionStep);
           } else {
              generate_map_stream_arg(shader, args[i], needIndexOfArg, i,
                                      texcoord, constreg, samplerreg, outPass, 
                                      hasDoneStreamDim);
           }
        }
     } else if (args[i]->isArray()) {
        generate_shader_gather_arg(shader, args[i], i, constreg, samplerreg, outPass);
     } else {
        shader << "uniform ";
        args[i]->print(shader, true);
        shader << " : register (c" << constreg++ << ")";
        shader <<  ",\n\t\t";

        outPass.addConstant( (i+1), 0 );
     }
  }

  /*
   * Output the bonus arguments.
   *
   * Put them at the end so that the parameter numbering isn't perturbed
   * (especially since there's a different number of bonus arguments with
   * and without address translation).
   */

  if (!globals.enableGPUAddressTranslation) {
    // Add the workspace variable
    shader << "uniform float4 __workspace    : register (c"
           << constreg++ << ")";
  } else {
    shader << "float2 __outputtexcoord : TEXCOORD" << texcoord++;
    shader << ",\n\t\t";
    shader << "float2 __outputaddrinterpolant : TEXCOORD" << texcoord++;
    shader << ",\n\t\t";
    shader << "uniform float2 __outputlinearize : register(c" << constreg++ << ")";
    shader << ",\n\t\t";
    shader << "uniform float4 __outputstride : register(c" << constreg++ << ")";
    shader << ",\n\t\t";
    shader << "uniform float4 __outputinvstride : register(c" << constreg++ << ")";
    shader << ",\n\t\t";
    shader << "uniform float4 __outputinvextent : register(c" << constreg++ << ")";
    shader << ",\n\t\t";
    shader << "uniform float4 __outputdomainmin : register(c" << constreg++ << ")";
    shader << ",\n\t\t";
    shader << "uniform float4 __outputdomainsize : register(c" << constreg++ << ")";
    shader << ",\n\t\t";
    shader << "uniform float4 __outputinvshape : register(c" << constreg++ << ")";
    shader << ",\n\t\t";
    shader << "uniform float __hackconst : register(c" << constreg++ << ")";

    outPass.addInterpolant( 0, "kGlobalInterpolant_ATOutputTex" );
    outPass.addInterpolant( 0, "kGlobalInterpolatn_ATOutputAddress" );
    outPass.addConstant( 0, "kGlobalConstant_ATOutputLinearize" );
    outPass.addConstant( 0, "kGlobalConstant_ATOutputStride" );
    outPass.addConstant( 0, "kGlobalConstant_ATOutputInvStride" );
    outPass.addConstant( 0, "kGlobalConstant_ATOutputInvExtent" );
    outPass.addConstant( 0, "kGlobalConstant_ATOutputDomainMin" );
    outPass.addConstant( 0, "kGlobalConstant_ATOutputDomainSize" );
    outPass.addConstant( 0, "kGlobalConstant_ATOutputInvShape" );
    outPass.addConstant( 0, "kGlobalConstant_ATHackConstant" );
  }

  shader << ") {\n";

  /*
   * Declare the stream variables
   */

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

  //For reductions in GLES we get the texture coordinates of the first texture and adjust it
  //This is always the first 
  if( target == CODEGEN_GLES && isReduction)
  {
     //cgc complains about putting everything in the vec2 constructor so let's do it in two steps
     //We define coordinates and step, which will be used for the indexing and its adjustment
     //We take the coordinates of each element with gl_FragCoord
     //However, cgc doesn't allow its use, so let's define it with _ and 
     //remove it when we generate the kernel at cgc.cpp
     shader << "\tvec4 _gl_FragCoord;\n";
     //The coordinates of the dimension we are reducing (for which reduction
     //step is not 0) need to be scaled by reduction factor, so that for each
     //output point we visit reduction factor samples
     //For the dimension which we are not reducing we will simply use the the glFragCoord
     //TODO: Those expensive conditionals which are the same for each pixel
     //could be avoided by passing this infomation as uniform
     shader << "vec2 CoordScaling;\n";
     shader << "if( ReductionStep.x == 0.0) CoordScaling.x = 1.0; else CoordScaling.x = _reductionFactor;\n";
     shader << "if( ReductionStep.y == 0.0) CoordScaling.y = 1.0; else CoordScaling.y = _reductionFactor;\n";
     shader << "\tvec2 coordinates = CoordScaling.xy*_gl_FragCoord.xy + 0.5 ;\n";
     //We just use the two interpolants in order to obtain the correct step (next element of the reduction)
  }

  if (globals.enableGPUAddressTranslation) {
     // set up output position values
     shader << "\tfloat4 __indexofoutput;\n";
     shader << "\t__calculateoutputpos( __outputaddrinterpolant, "
            << "__outputlinearize, __outputstride, __outputinvstride, "
            << "__outputinvextent, __outputdomainmin, __outputdomainsize, __indexofoutput );\n";
  }

  /*
   * Perform stream fetches
   */

  hasDoneIndexofOutput = false;
  for (i=0; i < nArgs; i++) {
     TypeQual qual = args[i]->form->getQualifiers();
     std::string argName = args[i]->name->name;

     if ((qual & TQ_Iter) != 0)
     {
         if (!globals.enableGPUAddressTranslation) {

             hasDoneStreamDim= true;
             shader << "#ifdef GL_ES\n"
                    //In GLES iterator values are normalised, so scale them by the texture dimensions
                    << "\t" << *args[i]->name << " = "
                    << "StreamDim * " << *args[i]->name << " - IteratorBias ; \n"
                    << "#endif\n";

             continue; /* No texture fetch for iterators */
        }

        if (!fullAddressTrans) {
            shader << "\tfloat4 __indexof_" << argName << " = __indexofoutput;\n";
        } else {
            shader << "\tfloat4 __indexof_" << argName << " = ";
            shader << "__calculateindexof( __indexofoutput, __iterindexofnumer_" << argName
                << ", __iterindexofdenom_" << argName << " );\n";
        }

        shader << "\tfloat4 " << argName << " = __calculateiteratorvalue("
            << "__indexof_" << argName
            << ", __itervaluebase_" << argName
            << ", __itervalueoffset1_" << argName
            << ", __itervalueoffset4_" << argName << ");\n";
     }
     else if (args[i]->isStream() || (qual & TQ_Reduce) != 0) {
        if ((qual & TQ_Out) != 0 ) {
          if (globals.enableGPUAddressTranslation) {
            // should be calculated elsewhere
          } else if (!hasDoneIndexofOutput &&
                     FunctionProp[functionName].contains(i)) {
             hasDoneIndexofOutput= true;
             shader << "#ifndef GL_ES\n"
                    << "\t" << "float4 __indexofoutput = "
                    << "_computeindexof( "
                    << "_tex_" << *args[i]->name << "_pos, "
                    << "floor(float4( _tex_" << *args[i]->name << "_pos*"
                    << "_const_" << *args[i]->name << "_invscalebias.xy + "
                    << "_const_" << *args[i]->name << "_invscalebias.zw,0,0)));\n"
                    << "#else \n"
                    //In GLES texture coordinates are normalised, so first scale by the texture dimensions and then floor
                    << "\t" << "float4 __indexofoutput = "
                    << "floor(StreamDim*_computeindexof( "
                    << "_tex_" << *args[i]->name << "_pos, "
                    << "(float4( _tex_" << *args[i]->name << "_pos*"
                    << "_const_" << *args[i]->name << "_invscalebias.xy + "
                    << "_const_" << *args[i]->name << "_invscalebias.zw,0,0))));\n"
                    << "#endif\n";
          }
        } else {
          if (globals.enableGPUAddressTranslation) {
            if (!fullAddressTrans) {
               shader << "\tfloat4 __indexof_" << argName << " = __indexofoutput;\n";
               shader << "\tfloat2 _tex_" << argName << "_pos = __outputtexcoord;\n";
            } else {
               shader << "\tfloat4 __indexof_" << argName << " = ";
               shader << "__calculateindexof( __indexofoutput, __streamindexofnumer_" << argName << ", __streamindexofdenom_" << argName;
               shader << " );\n";
               shader << "\tfloat2 _tex_" << argName << "_pos = ";
               shader << "__calculatetexpos( __indexof_" << argName << ", ";
               shader << "__streamdomainmin_" << argName << ", ";
               shader << "__streamlinearize_" << argName << ", ";
               shader << "__streamtextureshape_" << argName << " );\n";
            }
          }

          //if it is GLES and we have a reduction we use a single adjusted interpolant (coordinates)
          if( target == CODEGEN_GLES && isReduction)
          {
             //if this is the second fetch (result variable) adjust coordinated again
             if(i == 1)
                shader << "coordinates += ReductionStep.xy;\n";
             expandStreamFetches(shader, args[i]->name->name, args[i]->form, "coordinates/StreamDim.xy", args[i]->name->name.c_str());
          }
          else
             expandStreamFetches(shader, args[i]->name->name, args[i]->form);
          if (!globals.enableGPUAddressTranslation &&
              FunctionProp[functionName].contains(i)) {
             shader << "#ifndef GL_ES\n"
                    << "\t" << "float4 __indexof_" << *args[i]->name << " = "
                    << "_computeindexof( "
                    << "_tex_" << *args[i]->name << "_pos, "
                    << "floor(float4( _tex_" << *args[i]->name << "_pos*"
                    << "_const_" << *args[i]->name << "_invscalebias.xy + "
                    << "_const_" << *args[i]->name << "_invscalebias.zw,0,0)));\n"
                    << "#else \n"
                    //In GLES texture coordinates are normalised, so first scale by the texture dimensions and then floor
                    << "\t" << "float4 __indexof_" << *args[i]->name << " = "
                    << "floor(StreamDim*_computeindexof( "
                    << "_tex_" << *args[i]->name << "_pos, "
                    << "(float4( _tex_" << *args[i]->name << "_pos*"
                    << "_const_" << *args[i]->name << "_invscalebias.xy + "
                    << "_const_" << *args[i]->name << "_invscalebias.zw,0,0))));\n"
                    << "#endif\n";
          }
        }
     }
  }

  /*
   * Print the body of the kernel
   */

//  shader << body << std::endl;
  // TIM: just call the body as a subroutine
  shader << std::endl;

  std::stringstream kernelBodyStream;

  kernelBodyStream << "\t" << functionName << "(\n";
  kernelBodyStream << "\t\t";

  for (i=0; i < nArgs; i++) {
    if( i != 0 )
      kernelBodyStream << ",\n\t\t";
    std::string name = args[i]->name->name;
    if( args[i]->isArray() ) {
      if( globals.enableGPUAddressTranslation )
      {
        kernelBodyStream << name;
        kernelBodyStream << ", __gatherlinearize_" << name;
        kernelBodyStream << ", __gathertexshape_" << name;
        kernelBodyStream << ", __gatherdomainmin_" << name;
      }
      else
      {
        kernelBodyStream << name << ", __gatherconst_" << name;
      }
    } else {
      kernelBodyStream << name;
    }
  }
  std::set<unsigned int>::iterator indexofIterator=
    FunctionProp[ functionName ].begin();
  std::set<unsigned int>::iterator indexofEnd =
    FunctionProp[ functionName ].end();
  for(; indexofIterator != indexofEnd; ++indexofIterator ) {
    if( (args[*indexofIterator]->form->getQualifiers() & TQ_Out) == 0 )
      kernelBodyStream << ",\n\t\t__indexof_" << args[*indexofIterator]->name->name;
    else
      kernelBodyStream << ",\n\n\t__indexofoutput";
  }

  kernelBodyStream << " );\n\n";

  std::string kernelBody = kernelBodyStream.str();

  // if we are doing a reduction, we may want to run the kernel
  // body multiple times to reduce n values...
  if (isReduction) {
     int r;
     assert( reductionStreamArguments.size() == 2 );
     int leftArgumentIndex = reductionStreamArguments[0];
     std::string leftArgumentName = args[leftArgumentIndex]->name->name;
     int rightArgumentIndex = reductionStreamArguments[1];
     std::string rightArgumentName = args[rightArgumentIndex]->name->name;
     Type* leftArgumentForm = args[leftArgumentIndex]->form;

     // do additional fetches...
     for ( r = 2; r < reductionFactor; r++ ) {
        std::stringstream s;
        s << "__reduce" << r;
        std::string argName = s.str();

        shader << "\t";
        leftArgumentForm->printBase( shader, 0);
        shader << " " << argName << ";\n";
        if( target == CODEGEN_GLES )
        {
           shader << "coordinates += ReductionStep.xy;\n";
           expandStreamFetches(shader, leftArgumentName,
                            leftArgumentForm, "coordinates/StreamDim.xy", argName.c_str());
        }
        else
        // do a new fetch for the reduction arg
        expandStreamFetches(shader, leftArgumentName,
                            leftArgumentForm, argName.c_str());
     }

     // save off the right arg...
     shader << "\t";
     leftArgumentForm->printBase(shader,0);
     shader << " __saved = " << rightArgumentName << ";\n";

     // do additional reduction ops
     for( r = 2; r < reductionFactor; r++ ) {
        std::stringstream s;
        s << "__reduce" << r;
        std::string argName = s.str();

        // shuffle stuff around... :)
        shader << "\t" << rightArgumentName << " = " << argName << ";\n";
        shader << kernelBody;
        if (!reductionArgumentComesBeforeStreamArgument)
           shader << "\t" << leftArgumentName << " = "
                  << rightArgumentName << ";\n";
     }

     shader << "\t" << rightArgumentName << " = __saved;\n";
     shader << kernelBody;
  } else {
     shader << kernelBody;
  }

  /*
   * Do any output unpacking
   */

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
 * append_argument_information --
 *
 *      Takes the fp code from the CG compiler and tacks on high level
 *      information from the original kernel function at the bottom.
 */

static char *
append_argument_information (const char *commentstring, char *fpcode,
                             Decl **args, int nArgs,
                             const char* functionName, int firstOutput, int outputCount, bool fullAddressTrans, int reductionFactor )
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
     int dimension = FloatGPUDimension(args[i]->form->getBase()->typemask);

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
  fp << commentstring << "!!fullAddressTrans:" << (fullAddressTrans ? 1 : 0 ) << ":" << std::endl;
  fp << commentstring << "!!reductionFactor:" << reductionFactor << ":" << std::endl;

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
generate_c_code( const std::vector<technique_info>& techniques, const char *name, const char *id)
{
  assert( name );
  assert( id );

  std::string mangledName = std::string("__") + name + "_" + id;

  std::ostringstream out;
  out << std::endl;
  if( techniques.size() == 0 )
  {
    out << "static const void* " << mangledName << " = 0;" << std::endl;
  }
  else
  {
    out << "namespace {" << std::endl;
    out << "\tusing namespace ::brook::desc;" << std::endl;
    out << "\tstatic const gpu_kernel_desc " << mangledName << "_desc = gpu_kernel_desc()";

    for( std::vector<technique_info>::const_iterator i = techniques.begin(); i != techniques.end(); ++i )
    {
      const technique_info& t = (*i);
      //GLES implementations have low limits for textures accesses 
      //so we limit the reduction factor size
      if( (strncmp(id, "gles", 4)==0) && (t.reductionFactor > 7 ) )
        break;

      out << std::endl;
      out << "\t\t.technique( gpu_technique_desc()" << std::endl;

      if( t.reductionFactor >= 2 )
        out << "\t\t\t.reduction_factor(" << t.reductionFactor << ")" << std::endl;

      if( t.outputAddressTranslation )
        out << "\t\t\t.output_address_translation()" << std::endl;
      if( t.inputAddressTranslation )
        out << "\t\t\t.input_address_translation()" << std::endl;

      for( std::vector<pass_info>::const_iterator j = t.passes.begin(); j != t.passes.end(); ++j )
      {
        out << "\t\t\t.pass( gpu_pass_desc(" << std::endl;

        const pass_info& p = (*j);

        const char* code = p.shader.c_str();
        out << "\t\t\t\t\"";

        char c;
        while( (c = *code++) != '\0' )
        {
          if( c == '\n' )
            out << "\\n\"" << std::endl << "\t\t\t\t\"";
          else
            out << c;
        }

        out << "\")" << std::endl;

        std::vector<shader_input_info>::const_iterator k;
        for( k = p.constants.begin(); k != p.constants.end(); ++k )
          out << "\t\t\t\t.constant(" << (*k).argumentIndex << ", " << (*k).componentName << ")" << std::endl;
        for( k = p.samplers.begin(); k != p.samplers.end(); ++k )
          out << "\t\t\t\t.sampler(" << (*k).argumentIndex << ", " << (*k).componentName << ")" << std::endl;
        for( k = p.interpolants.begin(); k != p.interpolants.end(); ++k )
          out << "\t\t\t\t.interpolant(" << (*k).argumentIndex << ", " << (*k).componentName << ")" << std::endl;
        for( k = p.outputs.begin(); k != p.outputs.end(); ++k )
          out << "\t\t\t\t.output(" << (*k).argumentIndex << ", " << (*k).componentName << ")" << std::endl;

        out << "\t\t\t)" << std::endl;
      }

      out << "\t\t)";
    }

    out << ";" << std::endl;
    out << "\tstatic const void* " << mangledName << " = &" << mangledName << "_desc;" << std::endl;

    out << "}" << std::endl;
  }

  return strdup( out.str().c_str() );
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

static char*
generateShaderPass(Decl** args, int nArgs, const char* name, int firstOutput,
                   int outputCount, CodeGenTarget target, bool fullAddressTrans,
                   int reductionFactor, pass_info& outPass)
{

  char* fpcode;
  char* fpcode_with_brccinfo;
  char* shadercode = generate_shader_code( args, nArgs, name, firstOutput, outputCount, fullAddressTrans, reductionFactor, outPass, target );
  if (shadercode) {
     //if (globals.verbose)
     // std::cerr << "\n***Produced this shader:\n" << shadercode << "\n";

    if (globals.keepFiles) {
      std::ofstream out;
      std::string fname =  std::string(globals.shaderoutputname) + "_" + name + ".cg";
      
      out.open(fname.c_str());
      if (out.fail()) {
        std::cerr << "***Unable to open " << fname << "\n";
      } else {
        out << shadercode;
        out.close();
      }
    }
    if (globals.verbose) {
       fprintf(stderr, "Generating %s code for %s outputs [%d, %d).\n",
               CodeGen_TargetName(target), name, firstOutput, firstOutput+outputCount);
    }

    assert (target < CODEGEN_NUM_TARGETS && target >= 0);

    if (shadercompile[target] == NULL)
      return NULL;

    fpcode = shadercompile[target]((std::string(globals.shaderoutputname) + "_" + name).c_str(), 
                                   shadercode, target, 0, true);
    
    if (fpcode==NULL) {
       fprintf (stderr,"for kernel %s.\n",
                name);
    }
    free(shadercode);
  } else {
     fpcode = NULL;
  }
  
  if (fpcode) {
     // if (globals.verbose)
     //   std::cerr << "***Produced this assembly:\n" << fpcode << "\n";
     
     const char* commentString = "##";
     switch(target)
     {
     case CODEGEN_PS20:
     case CODEGEN_PS2B:
     case CODEGEN_PS2A:
     case CODEGEN_PS30:
	 case CODEGEN_GLSL:
	 case CODEGEN_GLES:
     case CODEGEN_CTM:
       commentString = "//";
       break;
     default:
       break;
     }

     // TIM: the argument-info string is obsolete, and should go
     // away once all runtimes parse the new info...
     fpcode_with_brccinfo =
       append_argument_information(commentString,
                                   fpcode, args, nArgs, name, firstOutput, outputCount, fullAddressTrans, reductionFactor );
    free(fpcode);

    // if (globals.verbose)
    //  std::cerr << "***Produced this instrumented assembly:\n"
    //  << fpcode_with_brccinfo << "\n";
  } else {
    fpcode_with_brccinfo = NULL;
  }

  return fpcode_with_brccinfo;
}


static bool
generateShaderTechnique(Decl** args, int nArgs, const char* name,
                        CodeGenTarget target, bool fullAddressTrans,
                        int reductionFactor, technique_info& outTechnique )
{
  bool isReduction = false;
  int outputCount = getShaderOutputCount( nArgs, args, isReduction );
  int maxOutputsPerPass = 1;

  if( (target == CODEGEN_PS2B ||
       target == CODEGEN_PS30 ||
       target == CODEGEN_CTM  ||
       //target == CODEGEN_ARB  ||
       target == CODEGEN_GLSL ||
       target == CODEGEN_GLES ||
       target == CODEGEN_FP40 ||
       globals.arch == GPU_ARCH_X800 ||
       globals.arch == GPU_ARCH_6800 ||
       globals.allowDX9MultiOut) &&
      !isReduction )
    maxOutputsPerPass = 4;

  outTechnique.reductionFactor = reductionFactor;

  if( globals.enableGPUAddressTranslation )
  {
    outTechnique.outputAddressTranslation = true;
    if( fullAddressTrans )
      outTechnique.inputAddressTranslation = true;
  }

  int outputsLeft = outputCount;
  int firstOutput = 0;
  while( outputsLeft > 0 )
  {
    int outputsToWrite = maxOutputsPerPass;
    if( outputsLeft < maxOutputsPerPass )
      outputsToWrite = outputsLeft;

    char* shaderString = NULL;
    pass_info pass;
    while( outputsToWrite > 0 )
    {
      pass_info pass;
      shaderString = generateShaderPass(args, nArgs, name, firstOutput,
                                        outputsToWrite, target,
                                        fullAddressTrans, reductionFactor,
                                        pass);
      if( shaderString )
      {
        pass.shader = shaderString;
        outTechnique.passes.push_back( pass );
        free( shaderString );
        break;
      }

      // try again with fewer outputs
      outputsToWrite--;

      // we have failed if we can't even do one output
      if( outputsToWrite == 0 ) return false;
    }

    firstOutput += outputsToWrite;
    outputsLeft -= outputsToWrite;
  }
  return true;
}


static bool
generateReductionTechniques(Decl** args, int nArgs, const char* name,
                            CodeGenTarget target, bool fullAddressTrans,
                            std::vector<technique_info>& ioTechniques)
{
  bool isReduction = false;
  /*  int outputCount =*/getShaderOutputCount( nArgs, args, isReduction );

  if( !isReduction )
  {
    technique_info technique;
    if( !generateShaderTechnique( args, nArgs, name, target, fullAddressTrans, 0, technique ) )
      return false;
    ioTechniques.push_back( technique );
    return true;
  }

  int reductionFactor = 2;
  while( reductionFactor <= 8 ) // TIM: evil unnamed constant... :)
  {
    technique_info technique;
    if(!generateShaderTechnique( args, nArgs, name, target, fullAddressTrans, reductionFactor, technique ))
    {
      if( reductionFactor == 2 ) return false;
    }
    else
    {
      ioTechniques.push_back( technique );
    }

    reductionFactor++;
  }

  return true;
}

char *
CodeGen_GenerateCode(Type *retType, const char *name,
                     Decl **args, int nArgs, const char *body,
                     CodeGenTarget target)
{
  std::vector<technique_info> techniques;

  if( globals.enableGPUAddressTranslation )
  {
    //TIM: huge hack to get a not address-trans version available
    /*
    //TIM: we'd *like* to do this, but it won't actually work,
    // because subkernel calls will not have gather arguments converted correctly
    globals.enableGPUAddressTranslation = false;
    generateReductionTechniques( args, nArgs, name, target, false, techniques );
    globals.enableGPUAddressTranslation = true;
    */

    // only address-translate input streams
    generateReductionTechniques( args, nArgs, name, target, false, techniques );

    // address-translate the output stream too... ugly
    generateReductionTechniques( args, nArgs, name, target, true, techniques );    
  }
  else
  {
    generateReductionTechniques( args, nArgs, name, target, false, techniques );
  }

  char* c_code = generate_c_code(techniques, name,
                                 CodeGen_TargetName(target));

  // if (globals.verbose)
  //  std::cerr << "***Produced this C code:\n" << c_code;

  return c_code;
}

/*
*	CodeGen_SplitAndEmitCode
*  A simplified entry point for generating shaders that need to be split.
*/
void
CodeGen_SplitAndEmitCode(FunctionDef* inFunctionDef,
                         CodeGenTarget target, std::ostream& inStream ) {


  std::ostringstream shaderStream;

  std::auto_ptr<SplitCompiler> compiler;
  switch( target )
  {
  case CODEGEN_PS20:
  case CODEGEN_PS2B:
  case CODEGEN_PS2A:
  case CODEGEN_PS30:
  case CODEGEN_CTM: {
    std::auto_ptr<SplitCompiler> tmp( new SplitCompilerHLSL() );
    compiler = tmp;
    break;
   }
  case CODEGEN_FP30:
  case CODEGEN_FP40:
  case CODEGEN_GLSL:
  case CODEGEN_GLES:
  case CODEGEN_ARB:{
    std::auto_ptr<SplitCompiler> tmp( new SplitCompilerCg( target ) );
    compiler = tmp;
    break;
   }
  default:
    assert(0);
    break;
  }

  SplitTree splitTree( inFunctionDef, *compiler );

  std::string targetName = CodeGen_TargetName( target );
  std::string functionName = inFunctionDef->FunctionName()->name;
  std::string mangledName = std::string("__") + functionName + "_" + targetName;

  shaderStream << "namespace {" << std::endl;
  shaderStream << "\tusing namespace ::brook::desc;" << std::endl;
  shaderStream << "\tstatic const gpu_kernel_desc " << mangledName << "_desc = gpu_kernel_desc()" << std::endl;

  splitTree.printTechnique( SplitTechniqueDesc(), shaderStream );

  shaderStream << ";" << std::endl;
  shaderStream << "\tstatic const void* " << mangledName << " = &" << mangledName << "_desc;" << std::endl;
  shaderStream << "}" << std::endl;

  inStream << shaderStream.str();
}
