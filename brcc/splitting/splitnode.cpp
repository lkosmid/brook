// splitting.cpp
#include "splitting.h"

#include <kerneldesc.hpp>

#include <sstream>

SplitNode::SplitNode()
  : inferredType(kSplitBasicType_Unknown)
{
  _spanningParent = 0;
  _spanningNodeID = 0;
  _spanningSemidominatorID = 0;
  _spanningForestRoot = 0;

  _pdtDominator = 0;

  _rdsFixedMarked = false;
  _rdsFixedUnmarked = false;
  _rdsSplitHere = false;
  _mergeSplitHere = false;

  _isOutput = false;
}

void SplitNode::rdsPrint( const SplitTree& inTree, const SplitCompiler& inCompiler, std::ostream& inStream )
{
  if( marked ) return;
  marked = true;

  for( size_t i = 0; i < _graphChildren.size(); i++ )
    _graphChildren[i]->rdsPrint( inTree, inCompiler, inStream );

  if( isMarkedAsSplit() ) {
    std::cerr << "*** ";
    printExpression( std::cerr );
    std::cerr << std::endl;

    std::vector<SplitNode*> dummy;
    dummy.push_back( this );
    SplitShaderHeuristics unused;
    inCompiler.compile( inTree, dummy, inStream, unused );
  }
}

void SplitNode::addChild( SplitNode* inNode ) {
  _graphChildren.push_back( inNode );
}

void SplitNode::removeChild( SplitNode* inNode )
{
  NodeList::iterator i;
  for( i = _graphChildren.begin(); i != _graphChildren.end(); ++i )
  {
    if( *i != inNode ) continue;

    _graphChildren.erase( i );
    return;
  }
}

void SplitNode::traverseChildren( SplitNodeTraversal& ioTraversal )
{
  ioTraversal( _graphChildren );
}

SplitNode* SplitNode::eval()
{
  if( _spanningForestRoot )
  {
    // v is somewhere in a forest...
    SplitNode* n = this;
    SplitNode* minNode = n;
    while( n->_spanningParent != _spanningForestRoot )
    {
      n = n->_spanningParent;
      if( n->_spanningSemidominatorID < minNode->_spanningSemidominatorID )
        minNode = n;
    }
    return minNode;
  }
  else // this is a root
    return this;
}

void SplitNode::link( SplitNode* w )
{
  // if this is in the forest
  if( _spanningForestRoot )
  {
    w->_spanningForestRoot = _spanningForestRoot;
  }
  else
  {
    // this is the new root
    w->_spanningForestRoot = this;
  }
}

void SplitNode::printTemporaryName( std::ostream& inStream )
{
  inStream << "__temp" << (unsigned int)(this);
}
/*
void SplitNode::printSubFunction( const std::string& inFunctionName, std::ostream& inStream )
{
  SplitMarkTraversal markTraversal(false);
  SplitArgumentTraversal argumentTraversal(inStream);
  SplitStatementTraversal statementTraversal(inStream);
  
  inStream << "float4 " << inFunctionName << "( " << std::endl;
  markTraversal(this);
  argumentTraversal(this);
  inStream << " ) : COLOR0\n{\n";

  SplitStatementTraversal printTraversal(inStream);
  markTraversal(this);
  statementTraversal(this);

  inStream << "\treturn float4(";
  printTemporaryName( inStream );
  switch( inferredType )
  {
  case kSplitBasicType_Float:
    inStream << ", 0, 0, 0";
    break;
  case kSplitBasicType_Float2:
    inStream << ", 0, 0";
    break;
  case kSplitBasicType_Float3:
    inStream << ", 0";
    break;
  default:
    break;
  }
  inStream << ");\n";
  inStream << "}\n";
}*/

void InputSplitNode::printTemporaryExpression( std::ostream& inStream ) {
  if( argumentIndex >= 0 ) // standard arg
    inStream << "arg" << (argumentIndex+1);
  else if( argumentIndex < -1 ) // temporary "arg"
    inStream << "temp" << (-argumentIndex - 1);
  else // global "arg"
    inStream << "global";

  inStream << getComponentTypeName() << componentIndex;
}

void InputSplitNode::printExpression( std::ostream& inStream ) {
  printTemporaryExpression( inStream );
}

void InputSamplerSplitNode::printArgumentInfo( std::ostream& inStream, SplitArgumentCounter& ioCounter )
{
  inStream << "uniform _stype ";
  printExpression( inStream );
  inStream << " : register(s" << ioCounter.samplerCount++ << ")";
}

void InputSamplerSplitNode::printAnnotationInfo( std::ostream& inStream ) {
  inStream << ".sampler(" << (argumentIndex+1) << "," << componentIndex << ")";
}

void InputConstantSplitNode::printArgumentInfo( std::ostream& inStream, SplitArgumentCounter& ioCounter )
{
  inStream << "uniform " << inferredType << " ";
  printExpression( inStream );
  inStream << " : register(c" << ioCounter.constantCount++ << ")";
}

void InputConstantSplitNode::printAnnotationInfo( std::ostream& inStream ) {
  inStream << ".constant(" << (argumentIndex+1) << "," << componentIndex << ")";
}

void InputInterpolantSplitNode::printArgumentInfo( std::ostream& inStream, SplitArgumentCounter& ioCounter )
{
  inStream << inferredType << " ";
  printExpression( inStream );
  inStream << " : TEXCOORD" << ioCounter.texcoordCount++;
}

void InputInterpolantSplitNode::printAnnotationInfo( std::ostream& inStream ) {
  inStream << ".interpolant(" << (argumentIndex+1) << "," << componentIndex << ")";
}

void OutputSplitNode::dump( std::ostream& inStream )
{
  printTemporaryName( inStream );
  inStream << " = ";
  printExpression( inStream );
}

void OutputSplitNode::printTemporaryName( std::ostream& inStream ) {
  inStream << "arg" << (argumentIndex+1) << "output" << componentIndex;
}

void OutputSplitNode::printTemporaryExpression( std::ostream& inStream ) {
  inStream << "float4(";
  value->printTemporaryName( inStream );
  switch( value->inferredType )
  {
  case kSplitBasicType_Float:
    inStream << ",0,0,0)";
    break;
  case kSplitBasicType_Float2:
    inStream << ",0,0)";
    break;
  case kSplitBasicType_Float3:
    inStream << ",0)";
    break;
  case kSplitBasicType_Float4:
    inStream << ")";
    break;
  }
}

void OutputSplitNode::printExpression( std::ostream& inStream ) {
  value->printExpression( inStream );
}

void OutputSplitNode::printArgumentInfo( std::ostream& inStream, SplitArgumentCounter& ioCounter )
{
  inStream << "out float4 ";
  printTemporaryName( inStream );
  inStream << " : COLOR" << ioCounter.outputCount++;
}

void OutputSplitNode::printAnnotationInfo( std::ostream& inStream ) {
  inStream << ".output(" << (argumentIndex+1) << "," << componentIndex << ")";
}

void OutputSplitNode::traverseChildren( SplitNodeTraversal& ioTraversal ) {
  ioTraversal( value );
}

void LocalVariableSplitNode::assign( SplitNode* inValue )
{
  SplitNode* value = inValue->getValueNode();
  if( value->inferredType != inferredType )
  {
    std::vector<SplitNode*> constructorArgs;
    constructorArgs.push_back( value );
    value = new ConstructorSplitNode( inferredType, constructorArgs );
  }
  if( _value )
    removeChild( _value );
  _value = value;
  addChild( _value );
}

void LocalVariableSplitNode::printTemporaryExpression( std::ostream& inStream )
{
  assert( _value );
  _value->printTemporaryExpression( inStream );
}

void LocalVariableSplitNode::printExpression( std::ostream& inStream )
{
  inStream << _name;
}

void ArgumentSplitNode::printTemporaryExpression( std::ostream& inStream )
{
  inStream << "arg" << argumentIndex;
}

void ArgumentSplitNode::printExpression( std::ostream& inStream )
{
  inStream << name;
}

void AssignableArgumentSplitNode::assign( SplitNode* inValue )
{
  SplitNode* value = inValue->getValueNode();
  if( value->inferredType != inferredType )
  {
    std::vector<SplitNode*> constructorArgs;
    constructorArgs.push_back( value );
    value = new ConstructorSplitNode( inferredType, constructorArgs );
  }
  if( _assignedValue )
    removeChild( _assignedValue );
  _assignedValue = value;
  addChild( _assignedValue );
}

void AssignableArgumentSplitNode::printTemporaryExpression( std::ostream& inStream )
{
  if( _assignedValue )
    _assignedValue->printTemporaryExpression( inStream );
  else
    ArgumentSplitNode::printTemporaryExpression( inStream );
}

IteratorArgumentSplitNode::IteratorArgumentSplitNode( const std::string& inName, SplitBasicType inType, int inArgumentIndex )
  : AssignableArgumentSplitNode( inName, inType, inArgumentIndex )
{
  _value = new InputInterpolantSplitNode( argumentIndex, 0, inferredType );
}

StreamArgumentSplitNode::StreamArgumentSplitNode( const std::string& inName, SplitBasicType inType, int inArgumentIndex, SplitTreeBuilder& ioBuilder )
  : AssignableArgumentSplitNode( inName, inType, inArgumentIndex )
{
  sampler = new InputSamplerSplitNode( argumentIndex, 0, inferredType );
  interpolant = new InputInterpolantSplitNode( argumentIndex, 0, kSplitBasicType_Float2 );
  indexofConstant = new InputConstantSplitNode( argumentIndex, ::brook::desc::kStreamConstant_Indexof, kSplitBasicType_Float4 );
  value = new TextureFetchSplitNode( sampler, interpolant );

  // compute the indexof expression...
  // TIM: for now we are *really* bad and assume it is always the ps2.0 way :)

  indexofNode = ioBuilder.addConstructor( kSplitBasicType_Float4,
    ioBuilder.addBinaryOp( "+",
      ioBuilder.addBinaryOp( "*", interpolant, ioBuilder.addMember(indexofConstant,"xy") ),
      ioBuilder.addMember( indexofConstant, "zw" ) ),
    ioBuilder.addConstant( 0 ),
    ioBuilder.addConstant( 0 ) );
}

GatherArgumentSplitNode::GatherArgumentSplitNode( const std::string& inName, SplitBasicType inType, int inArgumentIndex, SplitTreeBuilder& ioBuilder )
  : ArgumentSplitNode( inName, inType, inArgumentIndex )
{
  sampler = new InputSamplerSplitNode( argumentIndex, 0, inferredType );
  gatherConstant = new InputConstantSplitNode( argumentIndex, 0, kSplitBasicType_Float4 );
  scale = ioBuilder.addMember( gatherConstant, "xy" );
  bias = ioBuilder.addMember( gatherConstant, "zw" );
}

ConstantArgumentSplitNode::ConstantArgumentSplitNode( const std::string& inName, SplitBasicType inType, int inArgumentIndex )
  : AssignableArgumentSplitNode( inName, inType, inArgumentIndex )
{
  value = new InputConstantSplitNode( argumentIndex, 0, inType );
}

BrtConstantSplitNode::BrtConstantSplitNode( Constant* inValue )
{
  std::ostringstream s;
  inValue->print( s );
  value = s.str();

  switch(inValue->ctype)
  {
  case CT_Int:
  case CT_UInt:
  case CT_Float:
    inferredType = kSplitBasicType_Float;
    break;
  }
}

BrtConstantSplitNode::BrtConstantSplitNode( int inValue )
{
  std::ostringstream s;
  s << inValue;
  value = s.str();
  inferredType = kSplitBasicType_Float;
}

void BrtConstantSplitNode::printTemporaryExpression( std::ostream& inStream )
{
  inStream << value;
}

void BrtConstantSplitNode::printExpression( std::ostream& inStream )
{
  inStream << value;
}

BrtMemberSplitNode::BrtMemberSplitNode( SplitNode* inValue, const std::string& inName )
: value(inValue), name(inName)
{
  // TIM: for now we assume all member access is for swizzles
  size_t nameLength = name.size();
  switch( nameLength )
  {
  case 1:
    inferredType = kSplitBasicType_Float;
    break;
  case 2:
    inferredType = kSplitBasicType_Float2;
    break;
  case 3:
    inferredType = kSplitBasicType_Float3;
    break;
  case 4:
    inferredType = kSplitBasicType_Float4;
    break;
  }
  addChild( inValue );
}

void BrtMemberSplitNode::printTemporaryExpression( std::ostream& inStream )
{
  value->printTemporaryName( inStream );
  inStream << "." << name;
}

void BrtMemberSplitNode::printExpression( std::ostream& inStream )
{
  value->printExpression( inStream );
  inStream << "." << name;
}

UnaryOpSplitNode::UnaryOpSplitNode( const std::string& inOperation, SplitNode* inOperand )
  : _operation(inOperation), _operand(inOperand)
{
  inferredType = inOperand->inferredType;
  addChild( _operand );
}

void UnaryOpSplitNode::printTemporaryExpression( std::ostream& inStream )
{
  inStream << _operation;
  _operand->printTemporaryName( inStream );
}

void UnaryOpSplitNode::printExpression( std::ostream& inStream )
{
  inStream << _operation;
  _operand->printExpression( inStream );
}

BrtBinaryOpSplitNode::BrtBinaryOpSplitNode( const std::string& inOperation, SplitNode* inLeft, SplitNode* inRight )
  : operation(inOperation), left(inLeft), right(inRight)
{
  // TIM: simple assumption - always choose the larger of the two...
  // this is actually a HACK
  SplitBasicType leftType = left->inferredType;
  SplitBasicType rightType = right->inferredType;
  int maximumType = (leftType > rightType) ? leftType : rightType;
  inferredType = (SplitBasicType)(maximumType);

  addChild( left );
  addChild( right );
}

void BrtBinaryOpSplitNode::printTemporaryExpression( std::ostream& inStream )
{
  left->printTemporaryName( inStream );
  inStream << " " << operation << " ";
  right->printTemporaryName( inStream );
}

void BrtBinaryOpSplitNode::printExpression( std::ostream& inStream )
{
  left->printExpression( inStream );
  inStream << " " << operation << " ";
  right->printExpression( inStream );
}

TextureFetchSplitNode::TextureFetchSplitNode( InputSamplerSplitNode* inSampler, SplitNode* inTextureCoordinate )
{
  sampler = inSampler;
  textureCoordinate = inTextureCoordinate;

  inferredType = inSampler->inferredType;

  addChild( textureCoordinate );
  addChild( sampler );
}

void TextureFetchSplitNode::printTemporaryExpression( std::ostream& inStream )
{
  switch(textureCoordinate->inferredType)
  {
  case kSplitBasicType_Float:
    inStream << "__sample1";
    break;
  case kSplitBasicType_Float2:
    inStream << "__sample2";
    break;
  default:
    inStream << "__sampleN";
    break;
  }

  inStream << "(";
  sampler->printExpression( inStream );
  inStream << ", ";
  textureCoordinate->printTemporaryName( inStream );
  inStream << " )";

  switch( inferredType )
  {
  case kSplitBasicType_Float:
    inStream << ".x";
    break;
  case kSplitBasicType_Float2:
    inStream << ".xy";
    break;
  case kSplitBasicType_Float3:
    inStream << ".xyz";
    break;
  case kSplitBasicType_Float4:
    inStream << ".xyzw";
    break;
  }
}

void TextureFetchSplitNode::printExpression( std::ostream& inStream )
{
  sampler->printExpression( inStream );
  inStream << "[ ";
  textureCoordinate->printExpression( inStream );
  inStream << " ]";
}

void ConstructorSplitNode::printTemporaryExpression( std::ostream& inStream )
{
  inStream << inferredType << "( ";
  for( std::vector<SplitNode*>::const_iterator i = arguments.begin(); i != arguments.end(); ++i )
  {
    if( i != arguments.begin() )
      inStream << ", ";
    (*i)->printTemporaryName( inStream );
  }
  inStream << " )";
}

void ConstructorSplitNode::printExpression( std::ostream& inStream )
{
  inStream << inferredType << "( ";
  for( std::vector<SplitNode*>::const_iterator i = arguments.begin(); i != arguments.end(); ++i )
  {
    if( i != arguments.begin() )
      inStream << ", ";
    (*i)->printExpression( inStream );
  }
  inStream << " )";
}


FunctionCallSplitNode::FunctionCallSplitNode( const std::string& inName, const std::vector<SplitNode*>& inArguments )
  : _name(inName), _arguments(inArguments)
{
  for( size_t i = 0; i < _arguments.size(); i++ )
  {
    _arguments[i] = _arguments[i]->getValueNode();
    addChild( _arguments[i] );
  }

  // TIM: must try to infer the type from the arguments!!!!
  if( _name == "dot" )
  {
    assert( _arguments.size() == 2 );
    inferredType = kSplitBasicType_Float;
  }
  else if( _name == "sqrt" )
  {
    assert( _arguments.size() == 1 );
    inferredType = _arguments[0]->inferredType;
  }
  else if( _name == "cross" )
  {
    assert( _arguments.size() == 2 );
    inferredType = kSplitBasicType_Float3;
  }
}

void FunctionCallSplitNode::printTemporaryExpression( std::ostream& inStream )
{
  inStream << _name << "( ";
  for( NodeList::const_iterator i = _arguments.begin(); i != _arguments.end(); ++i )
  {
    if( i != _arguments.begin() )
      inStream << ", ";
    (*i)->printTemporaryName( inStream );
  }
  inStream << " )";
}

void FunctionCallSplitNode::printExpression( std::ostream& inStream )
{
  inStream << _name << "( ";
  for( NodeList::const_iterator i = _arguments.begin(); i != _arguments.end(); ++i )
  {
    if( i != _arguments.begin() )
      inStream << ", ";
    (*i)->printExpression( inStream );
  }
  inStream << " )";
}
