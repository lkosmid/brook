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

SplitTree::SplitTree( FunctionDef* inFunctionDef, const SplitCompiler& inCompiler )
  : _compiler(inCompiler)
{
  build( inFunctionDef );
}

SplitTree::SplitTree( FunctionDef* inFunctionDef, const SplitCompiler& inCompiler, const std::vector<SplitNode*>& inArguments )
  : _compiler(inCompiler)
{
  build( inFunctionDef, inArguments );
}

SplitTree::~SplitTree()
{
}

void SplitTree::printTechnique( const SplitTechniqueDesc& inTechniqueDesc, std::ostream& inStream )
{
  // TIM: hack to make temporaries have a shared position stream
  _outputPositionInterpolant = new InputInterpolantSplitNode( -1, 0, kSplitBasicType_Float2 );

  rdsSearch();
  rdsSubdivide();

  // TIM: we need to split somewhere
  for( size_t i = 0; i < _outputList.size(); i++ )
    _outputList[i]->_rdsSplitHere = true;

//  std::cerr << "printing!!!" << std::endl;

  // assign temporary ids to any non-output split nodes
  int temporaryID = 1;
  {for( size_t i = 0; i < _rdsNodeList.size(); i++ )
  {
    SplitNode* node = _rdsNodeList[i];
    if( node->isMarkedAsSplit() && !(node->isOutputNode()) )
      node->setTemporaryID( temporaryID++ );
  }}
  int temporaryCount = temporaryID-1;

  // go recursively through the nodes
  // and print out anything that we split at...
  inStream << "\t.technique( gpu_technique_desc()" << std::endl;
  if( temporaryCount )
  {
    inStream << "\t\t.temporaries(" << temporaryCount << ")" << std::endl;
  }

  SplitMarkTraversal unmark(false);
  unmark( _outputList );
  unmark( _outputPositionInterpolant );

  {for( size_t i = 0; i < _outputList.size(); i++ ){
    _outputList[i]->rdsPrint( *this, _compiler, inStream );
  }}
  inStream << "\t)";
}

class SplitRDSUnmarkTraversal : public SplitNodeTraversal
{
public:
  void traverse( SplitNode* inNode ) {
    inNode->rdsUnmark();
  }
};

void SplitTree::rdsSearch()
{
//  std::cerr << "search" << std::endl;

  SplitRDSUnmarkTraversal unmark;

  for( size_t i = 0; i < _multiplyReferencedNodes.size(); i++ )
  {
    SplitNode* m = _multiplyReferencedNodes[i];

    std::cerr << "deciding whether to save ";
    m->dump( std::cerr );
    std::cerr << std::endl;

    unmark( _outputList );
    m->_rdsFixedUnmarked = false;
    m->_rdsFixedMarked = true;
    rdsSubdivide();
    float costSave = getPartitionCost();
    std::cerr << "save cost = " << costSave << std::endl;

    unmark( _outputList );
    m->_rdsFixedMarked = false;
    m->_rdsFixedUnmarked = true;
    rdsSubdivide();
    float costRecompute = getPartitionCost();
    std::cerr << "save recompute = " << costSave << std::endl;

    if( costSave < costRecompute ) {
      m->_rdsFixedUnmarked = false;
      m->_rdsFixedMarked = true;
    }
  }
}

class SplitRDSMergeTraversal : public SplitNodeTraversal
{
public:
  SplitRDSMergeTraversal( SplitTree& inTree, SplitNode* inRoot )
    : _tree(inTree), _root(inRoot) {}

  void traverse( SplitNode* inNode )
  {
    if( inNode->isRDSSplit() && (inNode != _root) )
      return;

    inNode->traverseChildren( *this );
    _tree.rdsMerge( inNode );
  }

private:
  SplitTree& _tree;
  SplitNode* _root;
};

void SplitTree::rdsSubdivide()
{
  for( size_t i = 0; i < _outputList.size(); i++ )
    rdsSubdivide( _outputList[i] );
}

void SplitTree::rdsSubdivide( SplitNode* t )
{
//  std::cerr << "subdivide" << std::endl;

  if( rdsCompile( t ) ) return;

  for( size_t i = 0; i < t->_pdtChildren.size(); i++ )
  {
    SplitNode* k = t->_pdtChildren[i];
    rdsSubdivide( k );
    if( k->_graphParents.size() <= 1 ) continue;
    
    // if it's multiply-referenced
    if( k->_rdsFixedMarked )
      k->_rdsSplitHere = true;
    else if( k->_rdsFixedUnmarked )
      k->_rdsSplitHere = false;
    else
    {
      k->_rdsSplitHere = !k->getHeuristics().recompute;
    }
  }

  SplitRDSMergeTraversal merge( *this, t );
  merge( t );
}

void SplitTree::rdsMerge( SplitNode* n )
{
//  std::cerr << "merge" << std::endl;

  size_t childCount = n->_graphChildren.size();

/*  std::cerr << "children are:" << std::endl;
  for( size_t i = 0; i < childCount; i++ )
  {
    n->_graphChildren[i]->printExpression( std::cerr );
    std::cerr << std::endl;
  }*/

  for( size_t d = childCount+1; d-- > 0; )
  {
    std::vector<size_t> validSubsets;
    std::vector<SplitShaderHeuristics> validHeuristics;

//    std::cerr << "childCount = " << d << " out of " << childCount << std::endl;

    SplitSubsetGenerator generator( d, childCount );
    while( generator.hasMore() )
    {
      size_t subsetBitfield = generator.getNext();

//      std::cerr << "subset = " << subsetBitfield << std::endl;

      for( size_t i = 0; i < childCount; i++ )
        n->_graphChildren[i]->_mergeSplitHere = (subsetBitfield & (1 << i)) == 0;

     if( rdsCompile( n ) ) {
//        std::cerr << "subset " << subsetBitfield << " was valid" << std::endl;
        validSubsets.push_back( subsetBitfield );
        validHeuristics.push_back( n->getHeuristics() );
      }
    }

    if( validSubsets.size() == 0 )
      continue;

    size_t bestSubset;
    SplitShaderHeuristics bestHeuristics;
    bestSubset = validSubsets[0];
    bestHeuristics = validHeuristics[0];

    for( size_t j = 1; j < validSubsets.size(); j++ )
    {
      SplitShaderHeuristics h = validHeuristics[j];

      if( h.cost < bestHeuristics.cost )
      {
        bestHeuristics = h;
        bestSubset = validSubsets[j];
      }
    }

    // mark children according to that subset
    // and we are done!!!
//    std::cerr << "subset " << bestSubset << " was chosen" << std::endl;
    for( size_t i = 0; i < childCount; i++ )
      n->_graphChildren[i]->_rdsSplitHere = (bestSubset & (1 << i)) == 0;

    n->setHeuristics( bestHeuristics );

    return;
  }

//  std::cerr << "empty subset was chosen" << std::endl;
  {for( size_t i = 0; i < childCount; i++ ){
    n->_graphChildren[i]->_rdsSplitHere = true;
  }}

  // TIM: force heuristic update?
  rdsCompile( n );

  // we couldn't manage any of them...
  // mark all children for saving,
  // and then return
}

bool SplitTree::rdsCompile( SplitNode* inNode )
{
  // TIM: we really need to handle this better:
  if( inNode->isTrivial() )
  {
    SplitShaderHeuristics heuristics;
    heuristics.cost = 0;
    heuristics.recompute = false;
    heuristics.valid = true;
    return true;
  }

  std::vector<SplitNode*>  outputVector;
  outputVector.push_back( inNode );

  SplitShaderHeuristics heuristics;

  std::ostringstream nullStream;
  _compiler.compile( *this, outputVector, nullStream, heuristics );

  /* TIM: if you want to see the output at the point of failure...
  static size_t errorNumber = 0;
  if( !heuristics.valid )
  {
    std::cerr << "****" << std::endl;
    inNode->dump( std::cerr );
    std::cerr << "****" << std::endl;

    if( ++errorNumber == 2 )
      throw 1;
  }*/

  if( heuristics.valid )
    inNode->setHeuristics( heuristics );

  return heuristics.valid;
}

float SplitTree::getPartitionCost()
{
  SplitMarkTraversal unmark(false);


  unmark( _outputList );
  unmark( _outputPositionInterpolant );

  class AccumulateCostTraversal : public SplitNodeTraversal
  {
  public:
    float _cost;
    virtual void traverse( SplitNode* inNode ) {

      if( inNode->marked )
        return;
      inNode->marked = true;

      inNode->traverseChildren( *this );

      if( inNode->isMarkedAsSplit() )
      {
        _cost += inNode->getHeuristics().cost;
      }
    }
  };

  AccumulateCostTraversal accumulate;
  accumulate._cost = 0;
  accumulate( _outputList );

  return accumulate._cost;
}

void SplitTree::printShaderFunction( const std::vector<SplitNode*>& inOutputs, std::ostream& inStream ) const
{
  SplitMarkTraversal unmark(false);
  SplitArgumentTraversal printArguments(inStream,_outputPositionInterpolant);
  SplitStatementTraversal printStatements(inStream,_outputPositionInterpolant);

  for( size_t i = 0; i < _rdsNodeList.size(); i++ )
    _rdsNodeList[i]->unmarkAsOutput();
  _outputPositionInterpolant->unmarkAsOutput();
  {for( size_t i = 0; i < inOutputs.size(); i++ ){
    inOutputs[i]->markAsOutput();
  }}

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

static FunctionType* getFunctionType( FunctionDef* inFunctionDef )
{
  Decl* functionDecl = inFunctionDef->decl;
  assert( functionDecl->form->type == TT_Function );
  return ((FunctionType*)functionDecl->form);
  Statement* headStatement = inFunctionDef->head;
}

void SplitTree::build( FunctionDef* inFunctionDef, const std::vector<SplitNode*>& inArguments )
{
  FunctionType* functionType = getFunctionType( inFunctionDef );

  SplitTreeBuilder builder( *this );

  std::cerr << "function args: " << functionType->nArgs << " args passed: " << inArguments.size();

  assert( functionType->nArgs == inArguments.size() );

  unsigned int i;
  for( i = 0; i < functionType->nArgs; i++ )
  {
    Decl* argumentDecl = functionType->args[i];
    builder.addArgument( argumentDecl, i, inArguments[i] );
  }

  Statement* statement = inFunctionDef->head;
  while( statement )
  {
    statement->buildSplitTree( builder );

    statement = statement->next;
  }

  // we were called with arguments
  // thus we don't deal with creating
  // output nodes, or with building
  // the dominator tree...
}

void SplitTree::build( FunctionDef* inFunctionDef )
{
  FunctionType* functionType = getFunctionType( inFunctionDef );
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
//  std::cerr << "done" << std::endl;
}
