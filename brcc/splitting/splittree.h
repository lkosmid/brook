// splittree.h
#ifndef __SPLITTREE_H__
#define __SPLITTREE_H__
#ifdef _WIN32
#pragma warning(disable:4786)
//debug symbol warning
#endif

#include <string>
#include <map>
#include <set>
#include <vector>

class FunctionDef;
class SplitNode;
class SplitTechniqueDesc;
class SplitCompiler;
class SplitShaderHeuristics;

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
  friend class SplitTreeBuilder;
  typedef std::map< std::string, SplitNode* > NodeMap;
  typedef std::vector< SplitNode* > NodeList;
  typedef std::set< SplitNode* > NodeSet;

  void build( FunctionDef* inFunctionDef );
  void build( FunctionDef* inFunctionDef, const std::vector<SplitNode*>& inArguments );

  void buildDominatorTree();
  void dominatorDFS( SplitNode* inNode, SplitNode* inParent, size_t& ioID );
  void dumpDominatorTree();
  void dumpDominatorTree( std::ostream& inStream, SplitNode* inNode, int inLevel = 0 );

  void preRdsMagic();
  void preRdsMagic( SplitNode* inNode );

  void rdsSearch();
  float rdsCompileConfiguration();
  void rdsSubdivide();
  void rdsSubdivide( SplitNode* t, SplitShaderHeuristics& outHeuristics );
  void rdsDecideSave( SplitNode* n, const SplitShaderHeuristics& inHeuristics );

  void rdsMerge( SplitNode* n, SplitShaderHeuristics& outHeuristics );
  void rdsTryMerge( SplitNode* n, SplitShaderHeuristics& outHeuristics );
  bool rdsMergeSome( SplitNode* n, const NodeList& inUnsavedChildren, size_t inSubsetSize, SplitShaderHeuristics& outHeuristics );
  void rdsMergeRec( SplitNode* n );

  void unmark( int inMarkBit ) const;

  NodeList _rdsNodeList;

  bool rdsCompile( SplitNode* inNode );
  bool rdsCompile( SplitNode* inNode, SplitShaderHeuristics& outHeuristics );

  float getPartitionCost();


  SplitNode* _pseudoRoot;

  NodeList _outputList;
  std::vector<int> _outputArgumentIndices;
  NodeList _multiplyReferencedNodes;

  NodeList _dagOrderNodeList;

  SplitNode* _outputPositionInterpolant;
  SplitNode* _resultValue;

  const SplitCompiler& _compiler;
};

#endif
