// splitbuilder.h
#ifndef __SPLITBUILDER_H__
#define __SPLITBUILDER_H__
#ifdef _WIN32
#pragma warning(disable:4786)
//debug symbol warning
#endif
#include <string>
#include <map>
#include "../stemnt.h"

#include "splittypes.h"

class SplitTree;

class SplitTreeBuilder
{
public:
  SplitTreeBuilder( SplitTree& ioTree );

  SplitNode* addArgument( Decl* inDeclaration, int inArgumentIndex );

  SplitNode* addConstant( Constant* inConstant );
  SplitNode* addConstant( int inValue );
  SplitNode* addMember( SplitNode* inValue, const std::string& inName );
  SplitNode* addBinaryOp( BinaryOp inOperation, SplitNode* inLeft, SplitNode* inRight );
  SplitNode* addGather( SplitNode* inStream, const std::vector<SplitNode*> inIndices );
  SplitNode* addConstructor( BaseType* inType, const std::vector<SplitNode*>& inArguments );
  SplitNode* addConstructor( SplitBasicType inType, SplitNode* inX = 0, SplitNode* inY = 0, SplitNode* inZ = 0, SplitNode* inW = 0 );
  SplitNode* addIndexof( const std::string& inName );
  SplitNode* findVariable( const std::string& inName );
  SplitNode* assign( const std::string& inName, SplitNode* inValue );

private:
  typedef std::map< std::string, SplitNode* > NodeMap;
  NodeMap nodeMap;
  SplitTree& tree;
};

#endif
