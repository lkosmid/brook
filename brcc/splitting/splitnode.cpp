// splitting.cpp
#include "splitting.h"

void SplitNode::printTemporaryName( std::ostream& inStream )
{
  inStream << "__temp" << (unsigned int)(this);
}

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
}

void InputSplitNode::printTemporaryExpression( std::ostream& inStream ) {
  inStream << "arg" << argumentIndex << getComponentTypeName() << componentIndex;
}

void InputSplitNode::printExpression( std::ostream& inStream ) {
  inStream << "arg" << argumentIndex << getComponentTypeName() << componentIndex;
}

void InputSamplerSplitNode::printArgumentInfo( std::ostream& inStream, SplitArgumentCounter& ioCounter )
{
  inStream << "uniform _stype ";
  printExpression( inStream );
  inStream << " : register(s" << ioCounter.samplerCount++ << ")";
}

void InputConstantSplitNode::printArgumentInfo( std::ostream& inStream, SplitArgumentCounter& ioCounter )
{
  inStream << "uniform " << inferredType << " ";
  printExpression( inStream );
  inStream << " : register(c" << ioCounter.constantCount++ << ")";
}

void InputInterpolantSplitNode::printArgumentInfo( std::ostream& inStream, SplitArgumentCounter& ioCounter )
{
  inStream << inferredType << " ";
  printExpression( inStream );
  inStream << " : TEXCOORD" << ioCounter.texcoordCount++;
}

void ArgumentSplitNode::printTemporaryExpression( std::ostream& inStream )
{
  inStream << "arg" << argumentIndex;
}

void ArgumentSplitNode::printExpression( std::ostream& inStream )
{
  inStream << name;
}

StreamArgumentSplitNode::StreamArgumentSplitNode( const std::string& inName, SplitBasicType inType, int inArgumentIndex, SplitTreeBuilder& ioBuilder )
  : ArgumentSplitNode( inName, inType, inArgumentIndex )
{
  sampler = new InputSamplerSplitNode( argumentIndex, 0, inferredType );
  interpolant = new InputInterpolantSplitNode( argumentIndex, 0, kSplitBasicType_Float2 );
  value = new TextureFetchSplitNode( sampler, interpolant );
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
  : ArgumentSplitNode( inName, inType, inArgumentIndex )
{
  value = new InputConstantSplitNode( argumentIndex, 0, inType );
}

BrtConstantSplitNode::BrtConstantSplitNode( Constant* inValue )
  : value(inValue)
{
  switch(value->ctype)
  {
  case CT_Int:
  case CT_UInt:
  case CT_Float:
    inferredType = kSplitBasicType_Float;
    break;
  }
}

void BrtConstantSplitNode::printTemporaryExpression( std::ostream& inStream )
{
  value->print( inStream );
}

void BrtConstantSplitNode::printExpression( std::ostream& inStream )
{
  value->print( inStream );
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

BrtBinaryOpSplitNode::BrtBinaryOpSplitNode( BinaryOp inOperation, SplitNode* inLeft, SplitNode* inRight )
: operation(inOperation), left(inLeft), right(inRight)
{
  // TIM: simple assumption - always choose the larger of the two...
  // this is actually a HACK
  SplitBasicType leftType = left->inferredType;
  SplitBasicType rightType = right->inferredType;
  int maximumType = (leftType > rightType) ? leftType : rightType;
  inferredType = (SplitBasicType)(maximumType);
}

void BrtBinaryOpSplitNode::printTemporaryExpression( std::ostream& inStream )
{
  std::ostream& out = inStream;

  left->printTemporaryName( inStream );

  switch( operation )
  {
  case BO_Plus:
    out << "+";
    break;
  case BO_Minus:
    out << "-";
    break;
  case BO_Mult:
    out << "*";
    break;
  case BO_Div:
    out << "/";
    break;
  case BO_Mod:
    out << "%";
    break;
  case BO_Shl:
    out << "<<";
    break;
  case BO_Shr:
    out << ">>";
    break;
  case BO_BitAnd:
    out << "&";
    break;
  case BO_BitXor:
    out << "^";
    break;
  case BO_BitOr:
    out << "|";
    break;
  case BO_And:
    out << "&&";
    break;
  case BO_Or:
    out << "||";
    break;
  case BO_Comma:
    out << ",";
    break;
  case BO_Member:
    out << ".";
    break;
  case BO_PtrMember:
    out << "->";
    break;
  default:
    //  case BO_Index        // x[y]
  case BO_Assign:      // An AssignExpr
  case BO_Rel:         // A RelExpr
    break;
  }
  right->printTemporaryName( inStream );
}

void BrtBinaryOpSplitNode::printExpression( std::ostream& inStream )
{
  std::ostream& out = inStream;

  left->printExpression( inStream );

  switch( operation )
  {
  case BO_Plus:
    out << "+";
    break;
  case BO_Minus:
    out << "-";
    break;
  case BO_Mult:
    out << "*";
    break;
  case BO_Div:
    out << "/";
    break;
  case BO_Mod:
    out << "%";
    break;
  case BO_Shl:
    out << "<<";
    break;
  case BO_Shr:
    out << ">>";
    break;
  case BO_BitAnd:
    out << "&";
    break;
  case BO_BitXor:
    out << "^";
    break;
  case BO_BitOr:
    out << "|";
    break;
  case BO_And:
    out << "&&";
    break;
  case BO_Or:
    out << "||";
    break;
  case BO_Comma:
    out << ",";
    break;
  case BO_Member:
    out << ".";
    break;
  case BO_PtrMember:
    out << "->";
    break;
  default:
    //  case BO_Index        // x[y]
  case BO_Assign:      // An AssignExpr
  case BO_Rel:         // A RelExpr
    break;
  }
  right->printExpression( inStream );
}

TextureFetchSplitNode::TextureFetchSplitNode( InputSamplerSplitNode* inSampler, SplitNode* inTextureCoordinate )
{
  sampler = inSampler;
  textureCoordinate = inTextureCoordinate;

  inferredType = inSampler->inferredType;
}


TextureFetchSplitNode::TextureFetchSplitNode( SplitNode* inStream, const std::vector<SplitNode*>& inIndices, SplitTreeBuilder& ioBuilder )
{
  GatherArgumentSplitNode* stream = inStream->isGatherArgument();
  assert(stream);

  sampler = stream->getSampler();

  // TIM: for now
  assert( inIndices.size() == 1 );
  SplitNode* index = inIndices[0]->getValueNode();

  SplitNode* scaled = ioBuilder.addBinaryOp( BO_Mult, index, stream->getScale() );
  SplitNode* biased = ioBuilder.addBinaryOp( BO_Plus, scaled, stream->getBias() );

  textureCoordinate = biased;

  inferredType = stream->inferredType;
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
}

void TextureFetchSplitNode::printExpression( std::ostream& inStream )
{
  sampler->printExpression( inStream );
  inStream << "[ ";
  textureCoordinate->printExpression( inStream );
  inStream << " ]";
}
