// splitcompiler.h
#ifndef __SPLITCOMPILER_H__
#define __SPLITCOMPILER_H__

#ifdef _WIN32
#pragma warning(disable:4786)
//debug symbol warning
#endif

#include <vector>
#include <set>
#include <iostream>
#include <string>
class SplitTree;
class SplitNode;

class SplitShaderHeuristics
{
public:
  bool valid; // whether this shader can be compiled
  float cost; // the cost of this shader (if valid)
  bool recompute; // should we recompute this shader, rather than save it?
};

class SplitCompiler
{
public:
  typedef std::set< SplitNode* > NodeSet;

  void compile( const SplitTree& inTree, const NodeSet& inOutputs, std::ostream& inStream, SplitShaderHeuristics& outHeuristics, bool forReal = false ) const;

  // TIM: complete hack, even for me
  virtual bool mustScaleAndBiasGatherIndices() const { return false; }

protected:
  virtual void printHeaderCode( std::ostream& inStream ) const {};
  virtual void printFooterCode( std::ostream& inStream ) const {};
  virtual void compileShader( const std::string& inHighLevelCode, std::ostream& inAssemblerStream, SplitShaderHeuristics& outHeuristics ) const = 0;

  void printStringConstant( const std::string& inString, const std::string& inPrefix, std::ostream& inStream ) const;
};

#endif
