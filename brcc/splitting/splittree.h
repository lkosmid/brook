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
  SplitTree( FunctionDef* inFunctionDef );
  virtual ~SplitTree();

  void printTechnique( const SplitTechniqueDesc& inTechniqueDesc, const SplitCompiler& inCompiler, std::ostream& inStream );

  void printShaderFunction( std::ostream& inStream );

  // print a shader function def for the given subset of outputs
  void printShaderFunction( const std::vector<SplitNode*>& inOutputs, std::ostream& inStream ) const;
  void printArgumentAnnotations( const std::vector<SplitNode*>& inOutputs, std::ostream& inStream ) const;

  void rdsMerge( SplitNode* n, const SplitCompiler& inCompiler );

private:
  void build( FunctionDef* inFunctionDef );

  void buildDominatorTree();

  void rdsSearch( const SplitCompiler& inCompiler );
  void rdsSubdivide( const SplitCompiler& inCompiler );
  void rdsSubdivide( SplitNode* t, const SplitCompiler& inCompiler );
  std::vector<SplitNode*> _rdsNodeList;

  bool rdsCompile( SplitNode* inNode, const SplitCompiler& inCompiler );

  friend class SplitTreeBuilder;
  typedef std::map< std::string, SplitNode* > NodeMap;
  NodeMap outputValues;

  std::vector<SplitNode*> _outputList;
  std::vector<SplitNode*> _multiplyReferencedNodes;

  SplitNode* _outputPositionInterpolant;
};

#endif
