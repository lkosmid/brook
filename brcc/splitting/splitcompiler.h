// splitcompiler.h
#ifndef __SPLITCOMPILER_H__
#define __SPLITCOMPILER_H__

#ifdef _WIN32
#pragma warning(disable:4786)
//debug symbol warning
#endif

#include <vector>
#include <iostream>
#include <string>
class SplitTree;
class SplitNode;

class SplitCompiler
{
public:
  void compile( const SplitTree& inTree, const std::vector<SplitNode*>& inOutputs, std::ostream& inStream ) const;

  // TIM: complete hack, even for me
  virtual bool mustScaleAndBiasGatherIndices() const { return false; }

protected:
  virtual void printHeaderCode( std::ostream& inStream ) const {};
  virtual void printFooterCode( std::ostream& inStream ) const {};
  virtual void compileShader( const std::string& inHighLevelCode, std::ostream& inAssemblerStream ) const = 0;

  void printStringConstant( const std::string& inString, const std::string& inPrefix, std::ostream& inStream ) const;
};

class SplitCompilerError
{
public:
  SplitCompilerError( const char* inMessage )
    : message(inMessage)
  {}

  const char* getMessage() {
    return message;
  }

private:
  const char* message;
};

#endif
