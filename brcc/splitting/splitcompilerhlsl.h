// splitcompilerhlsl.h
#ifndef __SPLITCOMPILERHLSL_H__
#define __SPLITCOMPILERHLSL_H__

#ifdef _WIN32
#pragma warning(disable:4786)
//debug symbol warning
#endif

#include "splitcompiler.h"

class SplitCompilerHLSL
  : public SplitCompiler
{
public:
  void compile( const SplitTree& inTree, const std::vector<SplitNode*>& inOutputs, std::ostream& inStream ) const;

private:
  void printStringConstant( const std::string& inString, const std::string& inPrefix, std::ostream& inStream ) const;
};

#endif
