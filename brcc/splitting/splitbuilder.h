// splitbuilder.h
#ifndef __SPLITBUILDER_H__
#define __SPLITBUILDER_H__

#include <map>
#include "../stemnt.h"

class SplitTree;

class SplitTreeBuilder
{
public:
  SplitTreeBuilder( SplitTree& ioTree );

  SplitNode* addArgument( Decl* inDeclaration, int inArgumentIndex );

  SplitNode* addConstant( Constant* inConstant );
  SplitNode* addMember( SplitNode* inValue, const std::string& inName );
  SplitNode* addBinaryOp( BinaryOp inOperation, SplitNode* inLeft, SplitNode* inRight );
  SplitNode* addGather( SplitNode* inStream, const std::vector<SplitNode*> inIndices );
  SplitNode* findVariable( const std::string& inName );
  SplitNode* assign( const std::string& inName, SplitNode* inValue );

private:
  typedef std::map< std::string, SplitNode* > NodeMap;
  NodeMap nodeMap;
  SplitTree& tree;
};

#endif