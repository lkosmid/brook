// splitcompilerhlsl.cpp
#include "splitcompilerhlsl.h"

#include "splitting.h"
#include "../fxc.h"

#include <string>
#include <sstream>

void SplitCompilerHLSL::printHeaderCode( std::ostream& inStream ) const
{
  inStream
//    << "#ifdef USERECT\n"
//    << "#define _stype   samplerRECT\n"
//    << "#define _sfetch  texRECT\n"
//    << "#define __sample1(s,i) texRECT((s),float2(i,0))\n"
//    << "#define __sample2(s,i) texRECT((s),(i))\n"
//    << "#define _computeindexof(a,b) float4(a, 0, 0)\n"
//    << "#else\n"
    << "#define _stype   sampler\n"
    << "#define _sfetch  tex2D\n"
    << "#define __sample1(s,i) tex1D((s),(i))\n"
    << "#define __sample2(s,i) tex2D((s),(i))\n";
//    << "#define _computeindexof(a,b) (b)\n"
//    << "#endif\n\n";

}

void SplitCompilerHLSL::compileShader(
  const std::string& inHighLevelCode, std::ostream& outLowLevelCode, const SplitConfiguration& inConfiguration, SplitShaderHeuristics& outHeuristics ) const
{
  ShaderResourceUsage usage;

  bool shouldValidate = inConfiguration.validateShaders;

  char* assemblerBuffer = compile_fxc( inHighLevelCode.c_str(), CODEGEN_PS20, &usage, shouldValidate );

  if( assemblerBuffer == NULL )
  {
    outHeuristics.valid = false;
    outHeuristics.cost = 0;
    outHeuristics.recompute = true;
    return;
  }

  int textureInstructionCount = usage.textureInstructionCount;
  int arithmeticInstructionCount = usage.arithmeticInstructionCount;
  int samplerCount = usage.samplerRegisterCount;
  int interpolantCount = usage.interpolantRegisterCount;
  int constantCount = usage.constantRegisterCount;
  int temporaryCount = usage.temporaryRegisterCount;
  int outputCount = usage.outputRegisterCount;

  if( !shouldValidate )
  {
    if( (textureInstructionCount > inConfiguration.maximumTextureInstructionCount)
      || (arithmeticInstructionCount > inConfiguration.maximumArithmeticInstructionCount)
      || (samplerCount > inConfiguration.maximumSamplerCount)
      || (interpolantCount > inConfiguration.maximumSamplerCount)
      || (constantCount > inConfiguration.maximumConstantCount)
      || (temporaryCount > inConfiguration.maximumTemporaryCount)
      || (outputCount > inConfiguration.maximumOutputCount) )
    {
      std::cout << "compile failed because of configuration limits..." << std::endl;
      outHeuristics.valid = false;
      outHeuristics.cost = 0;
      outHeuristics.recompute = true;
      return;
    }
  }

  int passCost = inConfiguration.passCost;
  int textureInstructionCost = inConfiguration.textureInstructionCost;
  int arithmeticInstructionCost = inConfiguration.arithmeticInstructionCost;
  int samplerCost = inConfiguration.samplerCost;
  int interpolantCost = inConfiguration.interpolantCost;
  int constantCost = inConfiguration.constantCost;
  int temporaryCost = inConfiguration.temporaryCost;
  int outputCost = inConfiguration.outputCost;

  int shaderCost = passCost
    + textureInstructionCost*textureInstructionCount
    + arithmeticInstructionCost*arithmeticInstructionCount
    + samplerCost*samplerCount
    + interpolantCost*interpolantCount
    + constantCost*constantCount
    + temporaryCost*temporaryCount
    + outputCost*outputCount;

  bool shouldRecompute = true;
  if( textureInstructionCount*2 > inConfiguration.maximumTextureInstructionCount )
    shouldRecompute = false;
  if( arithmeticInstructionCount*2 > inConfiguration.maximumArithmeticInstructionCount )
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
