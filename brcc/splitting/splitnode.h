// splitnode.h
#ifndef __SPLITNODE_H__
#define __SPLITNODE_H__

#include "../stemnt.h"
#include "splittraversal.h"
#include "splittypes.h"

class GatherArgumentSplitNode;
class SplitTreeBuilder;
class SplitArgumentCounter;

class SplitNode
{
public:
  SplitNode()
    : inferredType(kSplitBasicType_Unknown) {}
  virtual ~SplitNode() {}

  // print a function that will evaluate this node's value
  void printSubFunction( const std::string& inFunctionName, std::ostream& inStream );

  virtual void traverseChildren( SplitNodeTraversal& ioTraversal ) {}

  // print an expression to get this node's value
  void printTemporaryName( std::ostream& inStream );
  virtual bool needsArgument() { return false; }
  virtual void printArgumentInfo( std::ostream& inStream, SplitArgumentCounter& ioCounter ) {}
  virtual bool needsTemporaryExpression() { return true; }
  virtual void printTemporaryExpression( std::ostream& inStream ) = 0;
  virtual void printExpression( std::ostream& inStream ) = 0;

  virtual GatherArgumentSplitNode* isGatherArgument() { return 0; }

  // whacky stuff to let arguments pass their value off to another node:
  virtual SplitNode* getValueNode() { return this; }

  // TIM: total break in protection... :)
  bool marked;
  SplitBasicType inferredType;
};

class InputSplitNode : public SplitNode
{
public:
  InputSplitNode( int inArgumentIndex, int inComponentIndex )
    : argumentIndex(inArgumentIndex), componentIndex(inComponentIndex) {}

  virtual const char* getComponentTypeName() = 0;

  void printTemporaryExpression( std::ostream& inStream );
  void printExpression( std::ostream& inStream );

  bool needsArgument() { return true; }

private:
  int argumentIndex;
  int componentIndex;
};

class InputSamplerSplitNode : public InputSplitNode
{
public:
  InputSamplerSplitNode( int inArgumentIndex, int inComponentIndex, SplitBasicType inType )
    : InputSplitNode( inArgumentIndex, inComponentIndex ) { inferredType = inType; }

  virtual bool needsTemporaryExpression() { return false; }

  virtual void printArgumentInfo( std::ostream& inStream, SplitArgumentCounter& ioCounter );

  virtual const char* getComponentTypeName() { return "sampler"; }
};

class InputConstantSplitNode : public InputSplitNode
{
public:
  InputConstantSplitNode( int inArgumentIndex, int inComponentIndex, SplitBasicType inType )
    : InputSplitNode( inArgumentIndex, inComponentIndex ) { inferredType = inType; }

  virtual void printArgumentInfo( std::ostream& inStream, SplitArgumentCounter& ioCounter );

  virtual const char* getComponentTypeName() { return "constant"; }
};

class InputInterpolantSplitNode : public InputSplitNode
{
public:
  InputInterpolantSplitNode( int inArgumentIndex, int inComponentIndex, SplitBasicType inType )
    : InputSplitNode( inArgumentIndex, inComponentIndex ) { inferredType = inType; }

    virtual void printArgumentInfo( std::ostream& inStream, SplitArgumentCounter& ioCounter );

    virtual const char* getComponentTypeName() { return "texcoord"; }
};

class ArgumentSplitNode : public SplitNode
{
public:
  ArgumentSplitNode( const std::string& inName, SplitBasicType inType, int inArgumentIndex )
    : name(inName), argumentIndex(inArgumentIndex) { inferredType = inType; }

    void printTemporaryExpression( std::ostream& inStream );
    void printExpression( std::ostream& inStream );

protected:
  std::string name;
  int argumentIndex;
};

class ReduceArgumentSplitNode : public ArgumentSplitNode
{
public:
  ReduceArgumentSplitNode( const std::string& inName, SplitBasicType inType, int inArgumentIndex )
    : ArgumentSplitNode( inName, inType, inArgumentIndex ) {}
};

class IteratorArgumentSplitNode : public ArgumentSplitNode
{
public:
  IteratorArgumentSplitNode( const std::string& inName, SplitBasicType inType, int inArgumentIndex )
    : ArgumentSplitNode( inName, inType, inArgumentIndex ) {}
};

class OutputArgumentSplitNode : public ArgumentSplitNode
{
public:
  OutputArgumentSplitNode( const std::string& inName, SplitBasicType inType, int inArgumentIndex )
    : ArgumentSplitNode( inName, inType, inArgumentIndex ) {}
};

class StreamArgumentSplitNode : public ArgumentSplitNode
{
public:
  StreamArgumentSplitNode( const std::string& inName, SplitBasicType inType, int inArgumentIndex, SplitTreeBuilder& ioBuilder );
  
  virtual SplitNode* getValueNode() { return value; }

private:
  InputSamplerSplitNode* sampler;
  InputInterpolantSplitNode* interpolant;
  SplitNode* value;
};

class GatherArgumentSplitNode : public ArgumentSplitNode
{
public:
  GatherArgumentSplitNode( const std::string& inName, SplitBasicType inType, int inArgumentIndex, SplitTreeBuilder& ioBuilder );
  InputSamplerSplitNode* getSampler() { return sampler; }
  SplitNode* getScale() { return scale; }
  SplitNode* getBias() { return bias; }

  virtual GatherArgumentSplitNode* isGatherArgument() { return this; }

private:
  InputSamplerSplitNode* sampler;
  InputConstantSplitNode* gatherConstant;
  SplitNode* scale;
  SplitNode* bias;
};

class ConstantArgumentSplitNode : public ArgumentSplitNode
{
public:
  ConstantArgumentSplitNode( const std::string& inName, SplitBasicType inType, int inArgumentIndex );

  virtual SplitNode* getValueNode() { return value; }

private:
  InputConstantSplitNode* value;
};

class BrtConstantSplitNode :
  public SplitNode
{
public:
  BrtConstantSplitNode( Constant* inValue );
  virtual void printTemporaryExpression( std::ostream& inStream );
  virtual void printExpression( std::ostream& inStream );

private:
  Constant* value;
};

class BrtMemberSplitNode :
  public SplitNode
{
public:
  BrtMemberSplitNode( SplitNode* inValue, const std::string& inName );
  virtual void printTemporaryExpression( std::ostream& inStream );
  virtual void printExpression( std::ostream& inStream );

  virtual void traverseChildren( SplitNodeTraversal& ioTraversal ) {
    ioTraversal( value );
  }

private:
  SplitNode* value;
  std::string name;
};

class BrtBinaryOpSplitNode :
  public SplitNode
{
public:
  BrtBinaryOpSplitNode( BinaryOp inOperation, SplitNode* inLeft, SplitNode* inRight );
  virtual void printTemporaryExpression( std::ostream& inStream );
  virtual void printExpression( std::ostream& inStream );

  virtual void traverseChildren( SplitNodeTraversal& ioTraversal ) {
    ioTraversal( left );
    ioTraversal( right );
  }

private:
   BinaryOp operation;
   SplitNode* left;
   SplitNode* right; 
};

class TextureFetchSplitNode :
  public SplitNode
{
public:
  TextureFetchSplitNode( InputSamplerSplitNode* inSampler, SplitNode* inTextureCoordinate );
  TextureFetchSplitNode( SplitNode* inStream, const std::vector<SplitNode*>& inIndices, SplitTreeBuilder& ioBuilder );
  virtual void printTemporaryExpression( std::ostream& inStream );
  virtual void printExpression( std::ostream& inStream );

  virtual void traverseChildren( SplitNodeTraversal& ioTraversal ) {
    ioTraversal( sampler );
    ioTraversal( textureCoordinate );
  }

private:
  InputSamplerSplitNode* sampler;
  SplitNode* textureCoordinate;
//  SplitNode* stream;
//  typedef std::vector<SplitNode*> NodeList;
//  NodeList indices;
};

#endif