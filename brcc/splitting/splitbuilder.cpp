// splitbuilder.cpp
#include "splitbuilder.h"

#include "splitnode.h"

SplitTreeBuilder::SplitTreeBuilder( SplitTree& ioTree )
: tree(ioTree)
{
}

SplitNode* SplitTreeBuilder::addArgument( Decl* inDeclaration, int inArgumentIndex )
{
  std::string name = inDeclaration->name->name;
  Type* type = inDeclaration->form;
  TypeQual quals = type->getQualifiers();

  BaseType* baseType = type->getBase();
  
  SplitBasicType inferredType = kSplitBasicType_Unknown;
  switch(baseType->typemask)
  {
    case BT_Float:
      inferredType = kSplitBasicType_Float;
      break;
    case BT_Float2:
      inferredType = kSplitBasicType_Float2;
      break;
    case BT_Float3:
      inferredType = kSplitBasicType_Float3;
      break;
    case BT_Float4:
      inferredType = kSplitBasicType_Float4;
      break;
  }

  SplitNode* result = NULL;
  if( (quals & TQ_Reduce) != 0 ) // reduction arg
  {
    result = new ReduceArgumentSplitNode( name, inferredType, inArgumentIndex );
  }
  if( (quals & TQ_Iter) != 0 ) // iterator arg
  {
    result = new IteratorArgumentSplitNode( name, inferredType, inArgumentIndex );
  }
  if( (quals & TQ_Out) != 0 ) // output arg
  {
    result = new OutputArgumentSplitNode( name, inferredType, inArgumentIndex );
  }
  else if( type->isStream() ) // non-reduce stream
  {
    result = new StreamArgumentSplitNode( name, inferredType, inArgumentIndex, *this );
  }
  else if( type->isArray() ) // gather stream
  {
    result = new GatherArgumentSplitNode( name, inferredType, inArgumentIndex, *this );
  }
  else // non-stream constant
  {
    result = new ConstantArgumentSplitNode( name, inferredType, inArgumentIndex );
  }

  nodeMap[name] = result;
  return result;
}


SplitNode* SplitTreeBuilder::addConstant( Constant* inConstant )
{
  SplitNode* result = new BrtConstantSplitNode( inConstant );
  return result;
}

SplitNode* SplitTreeBuilder::addMember( SplitNode* inValue, const std::string& inName )
{
  SplitNode* result = new BrtMemberSplitNode( inValue->getValueNode(), inName );
  return result;
}

SplitNode* SplitTreeBuilder::addBinaryOp( BinaryOp inOperation, SplitNode* inLeft, SplitNode* inRight )
{
  SplitNode* result = new BrtBinaryOpSplitNode( inOperation, inLeft->getValueNode(), inRight->getValueNode() );
  return result;
}

SplitNode* SplitTreeBuilder::addGather( SplitNode* inStream, const std::vector<SplitNode*> inIndices )
{
  SplitNode* result = new TextureFetchSplitNode( inStream->getValueNode(), inIndices, *this );
  return result;
}

SplitNode* SplitTreeBuilder::findVariable( const std::string& inName )
{
  NodeMap::iterator i = nodeMap.find( inName );
  if( i != nodeMap.end() )
    return (*i).second;
  else
  {
    std::cerr << "Undefined variable found in split-tree build process." << std::endl;
    return NULL;
  }
}

SplitNode* SplitTreeBuilder::assign( const std::string& inName, SplitNode* inValue )
{
  nodeMap[inName] = inValue->getValueNode();
  return inValue;
}