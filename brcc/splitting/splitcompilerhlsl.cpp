// splitcompilerhlsl.cpp
#include "splitcompilerhlsl.h"

#include "splitting.h"
#include "../fxc.h"

#include <string>
#include <sstream>

void SplitCompilerHLSL::printHeaderCode( std::ostream& inStream ) const
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

void SplitCompilerHLSL::compileShader( const std::string& inHighLevelCode, std::ostream& outLowLevelCode, SplitShaderHeuristics& outHeuristics ) const
{
  ShaderResourceUsage usage;

  char* assemblerBuffer = compile_fxc( inHighLevelCode.c_str(), CODEGEN_PS20, &usage );
  if( assemblerBuffer == NULL )
  {
    outHeuristics.valid = false;
    outHeuristics.cost = 0.0f;
    outHeuristics.recompute = true;
    return;
  }

  int textureInstructionCount = usage.textureInstructionCount;
  int arithmeticInstructionCount = usage.arithmeticInstructionCount;
  int samplerCount = usage.samplerRegisterCount;
  int interpolantCount = usage.interpolantRegisterCount;
  int constantCount = usage.constantRegisterCount;
  int temporaryCount = usage.temporaryRegisterCount;

  static const float passCost = 100.0f;
  static const float textureInstructionCost = 0.0f;
  static const float arithmeticInstructionCost = 0.0f;
//  static const float passCost = 15.7f;
//  static const float textureInstructionCost = 1.36f;
//  static const float arithmeticInstructionCost = 1.0f;
  static const float samplerCost = 0.0f;
  static const float interpolantCost = 0.0f;
  static const float constantCost = 0.0f;
  static const float temporaryCost = 0.0f;

  float shaderCost = passCost
    + textureInstructionCost*textureInstructionCount
    + arithmeticInstructionCost*arithmeticInstructionCount
    + samplerCost*samplerCount
    + interpolantCost*interpolantCount
    + constantCost*constantCount
    + temporaryCost*temporaryCount;

  bool shouldRecompute = true;
  if( textureInstructionCount > 8 )
    shouldRecompute = false;
  if( arithmeticInstructionCount > 32 )
    shouldRecompute = false;
//  if( samplerCount > 8 )
//    shouldRecompute = false;
//  if( interpolantCount > 4 )
//    shouldRecompute = false;
//  if( constantCount > 4 )
//    shouldRecompute = false;
//  if( temporaryCount > 8 )
//    shouldRecompute = false;

  outHeuristics.valid = true;
  outHeuristics.cost = shaderCost;
  outHeuristics.recompute = shouldRecompute;

  outLowLevelCode << assemblerBuffer;
  free( assemblerBuffer );
}
