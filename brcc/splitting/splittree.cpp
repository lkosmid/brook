// splittree.cpp
#include "splittree.h"

#include "splitnode.h"
#include "splitbuilder.h"
#include "splitcompiler.h"
#include "splitsubset.h"

#include <iostream>
#include <string>
#include <sstream>

#include <algorithm>

SplitTree::SplitTree( FunctionDef* inFunctionDef )
{
  build( inFunctionDef );
}

SplitTree::~SplitTree()
{
}

void SplitTree::printTechnique( const SplitTechniqueDesc& inTechniqueDesc, const SplitCompiler& inCompiler, std::ostream& inStream )
{
  // TIM: hack to make temporaries have a shared position stream
  _outputPositionInterpolant = new InputInterpolantSplitNode( 0, 0, kSplitBasicType_Float2 );

  rdsSearch( inCompiler );
  rdsSubdivide( inCompiler );

  // TIM: we need to split somewhere
  for( size_t i = 0; i < _outputList.size(); i++ )
    _outputList[i]->_rdsSplitHere = true;

  std::cerr << "printing!!!" << std::endl;

  // assign temporary ids to any non-output split nodes
  int temporaryID = 1;
  for( size_t i = 0; i < _rdsNodeList.size(); i++ )
  {
    SplitNode* node = _rdsNodeList[i];
    if( node->isMarkedAsSplit() && !(node->isOutputNode()) )
      node->setTemporaryID( temporaryID++ );
  }
  int temporaryCount = temporaryID-1;

  // go recursively through the nodes
  // and print out anything that we split at...
  inStream << "\t.technique( gpu_technique_desc()" << std::endl;
  if( temporaryCount )
  {
    inStream << "\t\t.temporaries(" << temporaryCount << ")" << std::endl;
  }

  for( size_t i = 0; i < _outputList.size(); i++ )
    _outputList[i]->rdsPrint( *this, inCompiler, inStream );
  inStream << "\t)";
}

class SplitRDSUnmarkTraversal : public SplitNodeTraversal
{
public:
  void traverse( SplitNode* inNode ) {
    inNode->rdsUnmark();
  }
};

void SplitTree::rdsSearch( const SplitCompiler& inCompiler )
{
  std::cerr << "search" << std::endl;

  SplitRDSUnmarkTraversal unmark;

  for( size_t i = 0; i < _multiplyReferencedNodes.size(); i++ )
  {
    SplitNode* m = _multiplyReferencedNodes[i];

    unmark( _outputList );
    m->_rdsFixedUnmarked = false;
    m->_rdsFixedMarked = true;
    rdsSubdivide( inCompiler );
    float costSave = 0.0f; // getPartitionCost();

    unmark( _outputList );
    m->_rdsFixedMarked = false;
    m->_rdsFixedUnmarked = true;
    rdsSubdivide( inCompiler );
    float costRecompute = 0.0f; // getPartitionCost();

    if( costSave < costRecompute ) {
      m->_rdsFixedUnmarked = false;
      m->_rdsFixedMarked = true;
    }
  }
}

class SplitRDSMergeTraversal : public SplitNodeTraversal
{
public:
  SplitRDSMergeTraversal( SplitTree& inTree, SplitNode* inRoot, const SplitCompiler& inCompiler )
    : _tree(inTree), _root(inRoot), _compiler(inCompiler) {}

  void traverse( SplitNode* inNode )
  {
    if( inNode->isRDSSplit() && (inNode != _root) )
      return;

    inNode->traverseChildren( *this );
    _tree.rdsMerge( inNode, _compiler );
  }

private:
  SplitTree& _tree;
  SplitNode* _root;
  const SplitCompiler& _compiler;
};

void SplitTree::rdsSubdivide( const SplitCompiler& inCompiler )
{
  for( size_t i = 0; i < _outputList.size(); i++ )
    rdsSubdivide( _outputList[i], inCompiler );
}

void SplitTree::rdsSubdivide( SplitNode* t, const SplitCompiler& inCompiler )
{
  std::cerr << "subdivide" << std::endl;

  if( rdsCompile( t, inCompiler ) ) return;

  for( size_t i = 0; i < t->_pdtChildren.size(); i++ )
  {
    SplitNode* k = t->_pdtChildren[i];
    rdsSubdivide( k, inCompiler );
    if( k->_graphParents.size() <= 1 ) continue;
    
    // if it's multiply-referenced
    if( k->_rdsFixedMarked )
      k->_rdsSplitHere = true;
    else if( k->_rdsFixedUnmarked )
      k->_rdsSplitHere = false;
    else
    {
      // let's assume that we never decide
      // to save, that's a pretty bad heuristic, though :)
      k->_rdsSplitHere = false;
    }
  }

  SplitRDSMergeTraversal merge( *this, t, inCompiler );
  merge( t );
}

void SplitTree::rdsMerge( SplitNode* n, const SplitCompiler& inCompiler )
{
  std::cerr << "merge" << std::endl;

  size_t childCount = n->_graphChildren.size();

  std::cerr << "children are:" << std::endl;
  for( size_t i = 0; i < childCount; i++ )
  {
    n->_graphChildren[i]->printExpression( std::cerr );
    std::cerr << std::endl;
  }

  for( size_t d = childCount+1; d-- > 0; )
  {
    std::vector<size_t> validSubsets;

    std::cerr << "childCount = " << d << " out of " << childCount << std::endl;

    SplitSubsetGenerator generator( d, childCount );
    while( generator.hasMore() )
    {
      size_t subsetBitfield = generator.getNext();

      std::cerr << "subset = " << subsetBitfield << std::endl;

      for( size_t i = 0; i < childCount; i++ )
        n->_graphChildren[i]->_mergeSplitHere = (subsetBitfield & (1 << i)) == 0;

      if( rdsCompile( n, inCompiler ) ) {
        std::cerr << "subset " << subsetBitfield << " was valid" << std::endl;
        validSubsets.push_back( subsetBitfield );
      }
    }

    if( validSubsets.size() == 0 )
      continue;

    size_t bestSubset;
    if( validSubsets.size() == 1 )
    {
      bestSubset = validSubsets[0];
    }
    else
    {
      // supposed to use a heuristic... this is a very bad one :)
      bestSubset = validSubsets[0];
    }

    // mark children according to that subset
    // and we are done!!!
    std::cerr << "subset " << bestSubset << " was chosen" << std::endl;
    for( size_t i = 0; i < childCount; i++ )
      n->_graphChildren[i]->_rdsSplitHere = (bestSubset & (1 << i)) == 0;

    return;
  }

  std::cerr << "empty subset was chosen" << std::endl;
  for( size_t i = 0; i < childCount; i++ )
    n->_graphChildren[i]->_rdsSplitHere = true;

  // we couldn't manage any of them...
  // mark all children for saving,
  // and then return
}

bool SplitTree::rdsCompile( SplitNode* inNode, const SplitCompiler& inCompiler )
{
  try
  {
    std::vector<SplitNode*>  outputVector;
    outputVector.push_back( inNode );

    std::ostringstream nullStream;
    inCompiler.compile( *this, outputVector, nullStream );

    return true;
  }
  catch( SplitCompilerError& )
  {
    return false;
  }
}

void SplitTree::printShaderFunction( const std::vector<SplitNode*>& inOutputs, std::ostream& inStream ) const
{
  SplitMarkTraversal unmark(false);
  SplitArgumentTraversal printArguments(inStream,_outputPositionInterpolant);
  SplitStatementTraversal printStatements(inStream,_outputPositionInterpolant);

  for( size_t i = 0; i < _rdsNodeList.size(); i++ )
    _rdsNodeList[i]->unmarkAsOutput();
  _outputPositionInterpolant->unmarkAsOutput();
  for( size_t i = 0; i < inOutputs.size(); i++ )
    inOutputs[i]->markAsOutput();

  // create the wrapper for the function
  inStream << "void main(" << std::endl;

  unmark( inOutputs );
  unmark( _outputPositionInterpolant );
  printArguments( inOutputs );

  inStream << " ) {" << std::endl;

  unmark( inOutputs );
  unmark( _outputPositionInterpolant );
  printStatements( inOutputs );

  inStream << "}" << std::endl;
}

void SplitTree::printArgumentAnnotations( const std::vector<SplitNode*>& inOutputs, std::ostream& inStream ) const
{
  SplitMarkTraversal unmark(false);
  SplitAnnotationTraversal printAnnotations(inStream,_outputPositionInterpolant);

  unmark( inOutputs );
  unmark( _outputPositionInterpolant );
  printAnnotations( inOutputs );
}

void SplitTree::build( FunctionDef* inFunctionDef )
{
  Decl* functionDecl = inFunctionDef->decl;
  assert( functionDecl->form->type == TT_Function );
  FunctionType* functionType = ((FunctionType*)functionDecl->form);
  Statement* headStatement = inFunctionDef->head;

  SplitTreeBuilder builder( *this );
  unsigned int i;
  for( i = 0; i < functionType->nArgs; i++ )
  {
    Decl* argumentDecl = functionType->args[i];
    builder.addArgument( argumentDecl, i );
  }

  Statement* statement = headStatement;
  while( statement )
  {
    statement->buildSplitTree( builder );

    statement = statement->next;
  }

  for( i = 0; i < functionType->nArgs; i++ )
  {
    Decl* argumentDecl = functionType->args[i];
    Type* argumentType = argumentDecl->form;
    std::string name = argumentDecl->name->name;

    if( (argumentType->getQualifiers() & TQ_Out) != 0 )
    {
      SplitNode* outputValue = builder.findVariable( name );
      // TIM: we assume here that each output is single-valued... :(
      SplitNode* outputNode = new OutputSplitNode( outputValue, i, 0 );
      outputValues[name] = outputNode;
    }
  }

  buildDominatorTree();
}
