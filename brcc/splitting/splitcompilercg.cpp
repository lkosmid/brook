// splitcompilercg.cpp
#include "splitcompilercg.h"

#include "splitting.h"
#include "../cgc.h"

#include <string>
#include <sstream>

void SplitCompilerCg::printHeaderCode( std::ostream& inStream ) const
{
  inStream
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

}

void SplitCompilerCg::compileShader( const std::string& inHighLevelCode, std::ostream& outLowLevelCode ) const
{
  char* assemblerBuffer = compile_cgc( inHighLevelCode.c_str(), _target );
  if( assemblerBuffer == NULL )
    throw SplitCompilerError( "Cg compilation failed" );

  outLowLevelCode << assemblerBuffer;
  free( assemblerBuffer );
}
