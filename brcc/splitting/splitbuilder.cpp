// splitbuilder.cpp
#include "splitbuilder.h"

#include "splitnode.h"
#include "splittree.h"
#include "splitcompiler.h"

static SplitBasicType getInferredType( BaseType* inType )
{
  switch(inType->typemask)
  {
  case BT_Float:
    return kSplitBasicType_Float;
    break;
  case BT_Float2:
    return kSplitBasicType_Float2;
    break;
  case BT_Float3:
    return kSplitBasicType_Float3;
    break;
  case BT_Float4:
    return kSplitBasicType_Float4;
    break;
  default:
    return kSplitBasicType_Unknown;
    break;
  }
}

SplitTreeBuilder::SplitTreeBuilder( SplitTree& ioTree )
: tree(ioTree), compiler(ioTree.getComplier())
{
}

SplitNode* SplitTreeBuilder::addArgument( Decl* inDeclaration, int inArgumentIndex )
{
  std::string name = inDeclaration->name->name;
  Type* type = inDeclaration->form;
  TypeQual quals = type->getQualifiers();
  
  SplitBasicType inferredType = getInferredType( type->getBase() );

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

void SplitTreeBuilder::addVariable( const std::string& inName, Type* inForm )
{
  // TIM: TODO: actually create something to represent the binding... :(
  nodeMap[inName] = NULL;
}


SplitNode* SplitTreeBuilder::addConstant( Constant* inConstant )
{
  SplitNode* result = new BrtConstantSplitNode( inConstant );
  return result;
}

SplitNode* SplitTreeBuilder::addConstant( int inValue )
{
  SplitNode* result = new BrtConstantSplitNode( inValue );
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
  GatherArgumentSplitNode* stream = inStream->isGatherArgument();
  assert(stream);
  InputSamplerSplitNode* sampler = stream->getSampler();

  // TIM: for now
  assert( inIndices.size() == 1 );
  SplitNode* index = inIndices[0]->getValueNode();

  SplitNode* textureCoordinate = index;

  if( compiler.mustScaleAndBiasGatherIndices() )
  {
    SplitNode* scaled = addBinaryOp( BO_Mult, index, stream->getScale() );
    SplitNode* biased = addBinaryOp( BO_Plus, scaled, stream->getBias() );
    textureCoordinate = biased;
  }

  SplitNode* result = new TextureFetchSplitNode( sampler, textureCoordinate );
  return result;
}

SplitNode* SplitTreeBuilder::addConstructor( BaseType* inType, const std::vector<SplitNode*>& inArguments )
{
  std::vector<SplitNode*> argumentValues;
  for( std::vector<SplitNode*>::const_iterator i = inArguments.begin(); i != inArguments.end(); ++i )
    argumentValues.push_back( (*i)->getValueNode() );

  SplitBasicType inferredType = getInferredType( inType );
  SplitNode* result = new ConstructorSplitNode( inferredType, argumentValues );
  return result;
}

SplitNode* SplitTreeBuilder::addConstructor( SplitBasicType inType, SplitNode* inX, SplitNode* inY, SplitNode* inZ, SplitNode* inW )
{
  std::vector<SplitNode*> argumentValues;
  if( inX )
    argumentValues.push_back( inX->getValueNode() );
  if( inY )
    argumentValues.push_back( inY->getValueNode() );
  if( inZ )
    argumentValues.push_back( inZ->getValueNode() );
  if( inW )
    argumentValues.push_back( inW->getValueNode() );

  SplitNode* result = new ConstructorSplitNode( inType, argumentValues );
  return result;
}

SplitNode* SplitTreeBuilder::addIndexof( const std::string& inName )
{
  SplitNode* variable = findVariable( inName );
  StreamArgumentSplitNode* stream = variable->isStreamArgument();
  return stream->getIndexofNode();
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