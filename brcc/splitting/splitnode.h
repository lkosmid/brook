// splitnode.h
#ifndef __SPLITNODE_H__
#define __SPLITNODE_H__

#include "../stemnt.h"
#include "splittraversal.h"
#include "splittypes.h"
#include "splitcompiler.h"

class GatherArgumentSplitNode;
class StreamArgumentSplitNode;
class SplitTreeBuilder;
class SplitArgumentCounter;
class SplitTree;
class SplitCompiler;
class OutputSplitNode;
class IndexofableSplitNode;

class SplitNode
{
public:
  SplitNode();
  virtual ~SplitNode() {}

  // print a function that will evaluate this node's value
  void printSubFunction( const std::string& inFunctionName, std::ostream& inStream );

  bool isRDSSplit() {
    return _rdsSplitHere;
  }

  void rdsUnmark() {
    _rdsSplitHere = false;
  }

  void rdsPrint( const SplitTree& inTree, const SplitCompiler& inCompiler, std::ostream& inStream );

  void markAsOutput() {
    _isOutput = true;
  }
  void unmarkAsOutput() {
    _isOutput = false;
  }
  bool isMarkedAsOutput() {
    return _isOutput;
  }

  bool isMarkedAsSplit() {
    return _rdsSplitHere || _mergeSplitHere;
  }

  int getTemporaryID() {
    return _temporaryID;
  }
  void setTemporaryID( int inID ) {
    _temporaryID = inID;
  }

  const SplitShaderHeuristics& getHeuristics() {
    return _heuristics;
  }
  void setHeuristics( const SplitShaderHeuristics& inHeuristics ) {
    _heuristics = inHeuristics;
  }

  // print an expression to get this node's value
  virtual void dump( std::ostream& inStream ) {
    printExpression( inStream );
  }

  virtual void printTemporaryName( std::ostream& inStream );
  virtual bool needsArgument() { return false; }
  virtual void printArgumentInfo( std::ostream& inStream, SplitArgumentCounter& ioCounter ) {}
  virtual bool needsTemporaryExpression() { return true; }
  virtual bool needsTemporaryVariable() { return true; }
  virtual void printTemporaryExpression( std::ostream& inStream ) = 0;
  virtual void printExpression( std::ostream& inStream ) = 0;

  virtual bool needsAnnotation() { return false; }
  virtual void printAnnotationInfo( std::ostream& inStream ) {}

  virtual void assign( SplitNode* inValue ) {
    throw 1;
  }

  virtual GatherArgumentSplitNode* isGatherArgument() { return 0; }
  virtual StreamArgumentSplitNode* isStreamArgument() { return 0; }
  virtual OutputSplitNode* isOutputNode() { return 0; }
  virtual IndexofableSplitNode* isIndexofable() { return 0; }

  // whacky stuff to let arguments pass their value off to another node:
  virtual SplitNode* getValueNode() { return this; }

  // TIM: total break in protection... :)
  bool marked;
  SplitBasicType inferredType;

  void getChildren( std::vector<SplitNode*>& outResult ) {
    outResult = _graphChildren;
  }

  static bool nodeIdLess( SplitNode* a, SplitNode* b ) {
    return a->_spanningNodeID < b->_spanningNodeID;
  }

  void traverseChildren( SplitNodeTraversal& ioTraversal );

  virtual bool isTrivial() { return false; }

protected:
  void addChild( SplitNode* inNode );
  void removeChild( SplitNode* inNode );

private:
  // data for calculating immediate dominators...
  friend class SplitDominatorDFSTraversal;
  friend class SplitTree;

  typedef std::vector<SplitNode*> NodeList;

  SplitNode* eval();
  void link( SplitNode* w );
  
  NodeList _graphParents;
  NodeList _graphChildren;


  SplitNode* _pdtDominator;
  NodeList _pdtChildren;
  
  SplitNode* _spanningParent;
  NodeList _spanningChildren;

  size_t _spanningNodeID;
  size_t _spanningSemidominatorID;
  SplitNode* _spanningForestRoot;
  NodeList _spanningBucket;

  bool _rdsFixedMarked;
  bool _rdsFixedUnmarked;
  bool _rdsSplitHere;
  bool _mergeSplitHere;
  bool _isOutput;

  int _temporaryID;

  SplitShaderHeuristics _heuristics;
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
  bool needsAnnotation() { return true; }

protected:
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
  virtual void printAnnotationInfo( std::ostream& inStream );

  virtual const char* getComponentTypeName() { return "sampler"; }

  virtual bool isTrivial() { return true; }
};

class InputConstantSplitNode : public InputSplitNode
{
public:
  InputConstantSplitNode( int inArgumentIndex, int inComponentIndex, SplitBasicType inType )
    : InputSplitNode( inArgumentIndex, inComponentIndex ) { inferredType = inType; }

  virtual void printArgumentInfo( std::ostream& inStream, SplitArgumentCounter& ioCounter );
  virtual void printAnnotationInfo( std::ostream& inStream );

  virtual const char* getComponentTypeName() { return "constant"; }

  virtual bool isTrivial() { return true; }
};

class InputInterpolantSplitNode : public InputSplitNode
{
public:
  InputInterpolantSplitNode( int inArgumentIndex, int inComponentIndex, SplitBasicType inType )
    : InputSplitNode( inArgumentIndex, inComponentIndex ) { inferredType = inType; }

    virtual void printArgumentInfo( std::ostream& inStream, SplitArgumentCounter& ioCounter );
    virtual void printAnnotationInfo( std::ostream& inStream );

    virtual const char* getComponentTypeName() { return "texcoord"; }

    virtual bool isTrivial() { return true; }
};

class OutputSplitNode : public SplitNode
{
public:
  OutputSplitNode( SplitNode* inValue, int inArgumentIndex, int inComponentIndex )
    : value(inValue), argumentIndex(inArgumentIndex), componentIndex(inComponentIndex)
  {
    // all outputs are float4 currently
    inferredType = kSplitBasicType_Float4;
    addChild( value );
  }

  void dump( std::ostream& inStream );
  void printTemporaryName( std::ostream& inStream );
  void printTemporaryExpression( std::ostream& inStream );
  void printExpression( std::ostream& inStream );

  void printArgumentInfo( std::ostream& inStream, SplitArgumentCounter& ioCounter );
  void printAnnotationInfo( std::ostream& inStream );

  void traverseChildren( SplitNodeTraversal& ioTraversal );

  bool needsArgument() { return false; }
  bool needsAnnotation() { return true; }
  bool needsTemporaryVariable() { return false; }

  virtual OutputSplitNode* isOutputNode() { return this; }

private:
  SplitNode* value;
  int argumentIndex;
  int componentIndex;
};

class LocalVariableSplitNode : public SplitNode
{
public:
  LocalVariableSplitNode( const std::string& inName, SplitBasicType inType )
    : _name(inName), _value(0)
  {
    inferredType = inType;
  }

  void assign( SplitNode* inValue );
  virtual SplitNode* getValueNode() {
    return _value ? _value->getValueNode() : this;
  }

  void printTemporaryExpression( std::ostream& inStream );
  void printExpression( std::ostream& inStream );

  virtual void dump( std::ostream& inStream ) {
    inStream << "var{";
    printExpression( inStream );
    inStream << "}";
  }

private:
  std::string _name;
  SplitNode* _value;
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

class AssignableArgumentSplitNode : public ArgumentSplitNode
{
public:
  AssignableArgumentSplitNode( const std::string& inName, SplitBasicType inType, int inArgumentIndex )
    : ArgumentSplitNode( inName, inType, inArgumentIndex ), _assignedValue(0)
  {}

  void assign( SplitNode* inValue );
  virtual SplitNode* getValueNode() {
    return _assignedValue ? _assignedValue : this;
  }

  void printTemporaryExpression( std::ostream& inStream );

protected:
  SplitNode* _assignedValue;
};

class IndexofableSplitNode : public AssignableArgumentSplitNode
{
public:
  IndexofableSplitNode( const std::string& inName, SplitBasicType inType, int inArgumentIndex )
    : AssignableArgumentSplitNode( inName, inType, inArgumentIndex ), _indexofValue(0)
  {}

  virtual IndexofableSplitNode* isIndexofable() { return this; }

  virtual SplitNode* getIndexofValue() {
    return _indexofValue;
  }

protected:
  SplitNode* _indexofValue;
};

class ReduceArgumentSplitNode : public AssignableArgumentSplitNode
{
public:
  ReduceArgumentSplitNode( const std::string& inName, SplitBasicType inType, int inArgumentIndex )
    : AssignableArgumentSplitNode( inName, inType, inArgumentIndex ) {}
};

class IteratorArgumentSplitNode : public AssignableArgumentSplitNode
{
public:
  IteratorArgumentSplitNode( const std::string& inName, SplitBasicType inType, int inArgumentIndex );

  virtual SplitNode* getValueNode() { return _assignedValue ? _assignedValue : _value; }

  virtual bool isTrivial() { return true; }

private:
  InputInterpolantSplitNode* _value;
};

class OutputArgumentSplitNode : public IndexofableSplitNode
{
public:
  OutputArgumentSplitNode( const std::string& inName, SplitBasicType inType, int inArgumentIndex, SplitTreeBuilder& ioBuilder );

private:
  InputConstantSplitNode* indexofConstant;
};

class StreamArgumentSplitNode : public IndexofableSplitNode
{
public:
  StreamArgumentSplitNode( const std::string& inName, SplitBasicType inType, int inArgumentIndex, SplitTreeBuilder& ioBuilder );
  
  virtual SplitNode* getValueNode() { return _assignedValue ? _assignedValue : value; }

  virtual StreamArgumentSplitNode* isStreamArgument() { return this; }

  // print an expression to get this node's value
  virtual void dump( std::ostream& inStream ) {
    inStream << "stream{";
    printExpression( inStream );
    inStream << "}";
  }

private:
  InputSamplerSplitNode* sampler;
  InputInterpolantSplitNode* interpolant;
  InputConstantSplitNode* indexofConstant;
  SplitNode* value;
//  SplitNode* indexofNode;
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

class ConstantArgumentSplitNode : public AssignableArgumentSplitNode
{
public:
  ConstantArgumentSplitNode( const std::string& inName, SplitBasicType inType, int inArgumentIndex );

  virtual SplitNode* getValueNode() { return _assignedValue ? _assignedValue : value; }

private:
  InputConstantSplitNode* value;
};

class BrtConstantSplitNode :
  public SplitNode
{
public:
  BrtConstantSplitNode( Constant* inValue );
  BrtConstantSplitNode( int inValue );

  virtual void printTemporaryExpression( std::ostream& inStream );
  virtual void printExpression( std::ostream& inStream );

  virtual bool isTrivial() { return true; }

private:
  std::string value;
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

class UnaryOpSplitNode :
  public SplitNode
{
public:
  UnaryOpSplitNode( const std::string& inOperation, SplitNode* inOperand );
  virtual void printTemporaryExpression( std::ostream& inStream );
  virtual void printExpression( std::ostream& inStream );

private:
  std::string _operation;
  SplitNode* _operand; 
};

class BrtBinaryOpSplitNode :
  public SplitNode
{
public:
  BrtBinaryOpSplitNode( const std::string& inOperation, SplitNode* inLeft, SplitNode* inRight );
  virtual void printTemporaryExpression( std::ostream& inStream );
  virtual void printExpression( std::ostream& inStream );

  virtual void traverseChildren( SplitNodeTraversal& ioTraversal ) {
    ioTraversal( left );
    ioTraversal( right );
  }

private:
  std::string operation;
  SplitNode* left;
  SplitNode* right; 
};

class TextureFetchSplitNode :
  public SplitNode
{
public:
  TextureFetchSplitNode( InputSamplerSplitNode* inSampler, SplitNode* inTextureCoordinate );
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

class ConstructorSplitNode :
  public SplitNode
{
public:
  ConstructorSplitNode( SplitBasicType inType, const std::vector<SplitNode*>& inArguments )
    : arguments(inArguments)
  {
    inferredType = inType;
    for( size_t i = 0; i < arguments.size(); i++ )
    {
      arguments[i] = arguments[i]->getValueNode();
      addChild( arguments[i] );
    }
  }

  virtual void printTemporaryExpression( std::ostream& inStream );
  virtual void printExpression( std::ostream& inStream );

  virtual void traverseChildren( SplitNodeTraversal& ioTraversal ) {
    ioTraversal( arguments );
  }

private:
  std::vector<SplitNode*> arguments;
};

class CastSplitNode :
  public SplitNode
{
public:
  CastSplitNode( SplitBasicType inType, SplitNode* inValue )
    : _value(inValue->getValueNode())
  {
    inferredType = inType;
    addChild(_value);
  }

  virtual void printTemporaryExpression( std::ostream& inStream );
  virtual void printExpression( std::ostream& inStream );

private:
  SplitNode* _value;
};

class FunctionCallSplitNode :
  public SplitNode
{
public:
  FunctionCallSplitNode( const std::string& inName, const std::vector<SplitNode*>& inArguments );

  virtual void printTemporaryExpression( std::ostream& inStream );
  virtual void printExpression( std::ostream& inStream );

private:
  std::string _name;
  std::vector<SplitNode*> _arguments;
};

class ConditionalSplitNode :
  public SplitNode
{
public:
  ConditionalSplitNode( SplitNode* inCondition, SplitNode* inConsequent, SplitNode* inAlternate );

  virtual void printTemporaryExpression( std::ostream& inStream );
  virtual void printExpression( std::ostream& inStream );

private:
  SplitNode* _condition;
  SplitNode* _consequent;
  SplitNode* _alternate;
};

#endif