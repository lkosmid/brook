// splittree.h
#ifndef __SPLITTREE_H__
#define __SPLITTREE_H__
#ifdef _WIN32
#pragma warning(disable:4786)
//debug symbol warning
#endif

#include <string>
#include <map>

class FunctionDef;
class SplitNode;

class SplitTree
{
public:
  SplitTree( FunctionDef* inFunctionDef );
  virtual ~SplitTree();

  void printShaderFunction( std::ostream& inStream );

private:
  void build( FunctionDef* inFunctionDef );

  friend class SplitTreeBuilder;
  typedef std::map< std::string, SplitNode* > NodeMap;
  NodeMap outputValues;
};

#endif
