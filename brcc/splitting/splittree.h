// splittree.h
#ifndef __SPLITTREE_H__
#define __SPLITTREE_H__
#ifdef _WIN32
#pragma warning(disable:4786)
//debug symbol warning
#endif

#include <string>
#include <map>
#include <vector>

class FunctionDef;
class SplitNode;
class SplitTechniqueDesc;
class SplitCompiler;

class SplitTree
{
public:
  SplitTree( FunctionDef* inFunctionDef, const SplitCompiler& inCompiler );
  SplitTree( FunctionDef* inFunctionDef, const SplitCompiler& inCompiler, const std::vector<SplitNode*>& inArguments );
  virtual ~SplitTree();

  void printTechnique( const SplitTechniqueDesc& inTechniqueDesc, std::ostream& inStream );

  void printShaderFunction( std::ostream& inStream );

  // print a shader function def for the given subset of outputs
  void printShaderFunction( const std::vector<SplitNode*>& inOutputs, std::ostream& inStream ) const;
  void printArgumentAnnotations( const std::vector<SplitNode*>& inOutputs, std::ostream& inStream ) const;

  void rdsMerge( SplitNode* n );

  const SplitCompiler& getComplier() {
    return _compiler;
  }

  SplitNode* getResultValue() {
    return _resultValue;
  }

  SplitNode* getOutputPositionInterpolant() {
    return _outputPositionInterpolant;
  }

private:
  void build( FunctionDef* inFunctionDef );
  void build( FunctionDef* inFunctionDef, const std::vector<SplitNode*>& inArguments );

  void buildDominatorTree();
  void dumpDominatorTree();
  void dumpDominatorTree( std::ostream& inStream, SplitNode* inNode, int inLevel = 0 );

  void preRdsMagic();
  void preRdsMagic( SplitNode* inNode );

  void rdsSearch();
  void rdsSubdivide();
  void rdsSubdivide( SplitNode* t );
  std::vector<SplitNode*> _rdsNodeList;

  bool rdsCompile( SplitNode* inNode );

  float getPartitionCost();

  typedef std::vector< SplitNode* > NodeList;

  friend class SplitTreeBuilder;
  typedef std::map< std::string, SplitNode* > NodeMap;
  NodeMap outputValues;

  typedef std::vector< SplitNode* > NodeList;
  NodeList _outputList;
  NodeList _multiplyReferencedNodes;

  SplitNode* _outputPositionInterpolant;
  SplitNode* _resultValue;

  const SplitCompiler& _compiler;
};

#endif
