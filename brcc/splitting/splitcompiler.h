// splitcompiler.h
#ifndef __SPLITCOMPILER_H__
#define __SPLITCOMPILER_H__

#ifdef _WIN32
#pragma warning(disable:4786)
//debug symbol warning
#endif

#include <vector>
#include <iostream>

class SplitTree;
class SplitNode;

class SplitCompiler
{
public:
  virtual void compile( const SplitTree& inTree, const std::vector<SplitNode*>& inOutputs, std::ostream& inStream ) const = 0;
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
