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

private:
  void build( FunctionDef* inFunctionDef );

  void buildDominatorTree();

  void rdsSearch();
  void rdsSubdivide();
  void rdsSubdivide( SplitNode* t );
  std::vector<SplitNode*> _rdsNodeList;

  bool rdsCompile( SplitNode* inNode );

  float getPartitionCost();

  friend class SplitTreeBuilder;
  typedef std::map< std::string, SplitNode* > NodeMap;
  NodeMap outputValues;

  std::vector<SplitNode*> _outputList;
  std::vector<SplitNode*> _multiplyReferencedNodes;

  SplitNode* _outputPositionInterpolant;

  const SplitCompiler& _compiler;
};

#endif
