// splitcompilerhlsl.cpp
#include "splitcompilerhlsl.h"

#include "splitting.h"
#include "../fxc.h"

#include <string>
#include <sstream>

void SplitCompilerHLSL::compile(
  const SplitTree& inTree, const std::vector<SplitNode*>& inOutputs, std::ostream& inStream ) const
{
  std::ostringstream bodyStream;

  // print boilerplate code
  bodyStream
    << "#ifdef USERECT\n"
    << "#define _stype   samplerRECT\n"
    << "#define _sfetch  texRECT\n"
    << "#define __sample1(s,i) texRECT((s),float2(i,0))\n"
    << "#define __sample2(s,i) texRECT((s),(i))\n"
    << "#define _computeindexof(a,b) float4(a, 0, 0)\n"
    << "#else\n"
    << "#define _stype   sampler\n"
    << "#define _sfetch  tex2D\n"
    << "#define __sample1(s,i) tex1D((s),(i))\n"
    << "#define __sample2(s,i) tex2D((s),(i))\n"
    << "#define _computeindexof(a,b) (b)\n"
    << "#endif\n\n";

  // print the actual body of the function
  // for the given set of outputs
  inTree.printShaderFunction( inOutputs, bodyStream );

  // actually compile the damn thing...
  char* assemblerBuffer = compile_fxc( bodyStream.str().c_str(), CODEGEN_PS20 );
  if( assemblerBuffer == NULL )
  {
    std::cerr << "********\n\n" << bodyStream.str() << "\n*********" << std::endl;
    throw SplitCompilerError( "HLSL compilation failed" );
  }
  std::string assemblerCode = assemblerBuffer;
  free( assemblerBuffer );

  // now we spit out the annotated version of things:
  inStream << "\t\t.pass( gpu_pass_desc(" << std::endl;
  printStringConstant( assemblerCode, "\t\t\t", inStream );
  inStream << " )" << std::endl;

  inTree.printArgumentAnnotations( inOutputs, inStream );

  inStream << "\t\t)" << std::endl;
}

void SplitCompilerHLSL::printStringConstant( const std::string& inString, const std::string& inPrefix, std::ostream& inStream ) const
{
  const char* s = inString.c_str();

  inStream << inPrefix << "\"";

  char c;
  while( (c = *s++) != '\0' )
  {
    if( c == '\n' )
      inStream << "\\n\"" << std::endl << inPrefix << "\"";
    else
      inStream << c;
  }

  inStream << "\"";
}
