// splittree.cpp
#include "splittree.h"

#include "splitnode.h"
#include "splitbuilder.h"
#include "splitcompiler.h"
#include "splitsubset.h"

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

#include <algorithm>

SplitTree::SplitTree( FunctionDef* inFunctionDef, const SplitCompiler& inCompiler )
  : _resultValue(NULL), _compiler(inCompiler), _pseudoRoot(NULL)
{
  std::cout << "$$$$$ creating a split tree for " << inFunctionDef->FunctionName()->name << std::endl;

  build( inFunctionDef );
}

SplitTree::SplitTree( FunctionDef* inFunctionDef, const SplitCompiler& inCompiler, const std::vector<SplitNode*>& inArguments )
: _resultValue(NULL), _compiler(inCompiler), _pseudoRoot(NULL)
{
  build( inFunctionDef, inArguments );
}

SplitTree::~SplitTree()
{
}

//static std::ofstream dumpFile;

void SplitTree::printTechnique( const SplitTechniqueDesc& inTechniqueDesc, std::ostream& inStream )
{
//  dumpFile.open( "dump.txt" );

//  preRdsMagic();

  rdsSearch();
//  rdsSubdivide();

  // TIM: we need to split somewhere
  for( size_t i = 0; i < _outputList.size(); i++ )
  {
    assert( _outputList[i]->_splitHere );
  }

//  std::cerr << "printing!!!" << std::endl;
  // assign "registers" to all split nodes
  {for( NodeList::iterator i = _dagOrderNodeList.begin(); i != _dagOrderNodeList.end(); ++i ) {
    (*i)->setTemporaryID( 0 );
  }}
  int outputID = 0;
  {for( NodeList::iterator i = _outputList.begin(); i != _outputList.end(); ++i ) {
    (*i)->setTemporaryID( _outputArgumentIndices[outputID++] );
  }}
  int temporaryID = 1;
  {for( NodeList::iterator i = _dagOrderNodeList.begin(); i != _dagOrderNodeList.end(); ++i )
  {
    if( (*i)->isMarkedAsSplit() && ((*i)->getTemporaryID() == 0) )
      (*i)->setTemporaryID( -(temporaryID++) );
  }}
  int temporaryCount = temporaryID-1;

  // go recursively through the nodes
  // and print out anything that we split at...
  inStream << "\t.technique( gpu_technique_desc()" << std::endl;
  if( temporaryCount )
  {
    inStream << "\t\t.temporaries(" << temporaryCount << ")" << std::endl;
  }

  unmark( SplitNode::kMarkBit_Printed );

  _pseudoRoot->rdsPrint( *this, _compiler, inStream );

  inStream << "\t)";

//  dumpFile.close();
}

void SplitTree::unmark( int inMarkBit ) const
{
  SplitNode::MarkBit markBit = (SplitNode::MarkBit)(inMarkBit);
  for( NodeList::const_iterator i = _dagOrderNodeList.begin(); i != _dagOrderNodeList.end(); ++i )
    (*i)->unmark( markBit );
  _outputPositionInterpolant->unmark( markBit );
}

void SplitTree::preRdsMagic()
{
  preRdsMagic( _pseudoRoot );
}

void SplitTree::preRdsMagic( SplitNode* inNode )
{
//  std::cerr << "pre-RDS magic for :";
//  inNode->dump( std::cerr );
//  std::cerr << std::endl;

  // we only collect MR nodes worth considering...
  if( inNode->_graphParents.size() > 1 && !inNode->isTrivial() )
    inNode->_isMRNodeWorthConsidering = true;

  if( rdsCompile( inNode ) )
  {
//    std::cerr << "succesful compile... its magic!" << std::endl;

    // it all can be done in one pass,
    // so there is no need to ever
    // look into this subtree
    inNode->_isMagic = true;
  }
  else
  {
//    std::cerr << "iterating over it's children, of which there are " << inNode->_pdtChildren.size() << std::endl;

    // we might have to split this node
    // so we had better make sure
    // that it's MR descendants
    // get added to our listing...
    for( NodeList::iterator c = inNode->_pdtChildren.begin(); c != inNode->_pdtChildren.end(); ++c )
      preRdsMagic( *c );
  }
}

void SplitTree::rdsSearch()
{

  // TIM: this is a pretty good early
  // exit, but it assumes a single output...
  /*if( _outputList.size() == 1 )
  {
    if( rdsCompile( _outputList[0] ) )
      return;
  }*/

  {for( NodeList::iterator j = _dagOrderNodeList.begin(); j != _dagOrderNodeList.end(); ++j )
  {
    (*j)->_wasConsidered = false;
    (*j)->_wasSaved = false;
    (*j)->_wasConsideredSave = false;
    (*j)->_wasSavedSave = false;
    (*j)->_wasConsideredRecompute = false;
    (*j)->_wasSavedRecompute = false;
  }}

  float bestCost;

  size_t index = 0;
  size_t count = _multiplyReferencedNodes.size();
  for( NodeList::iterator i = _multiplyReferencedNodes.begin(); i != _multiplyReferencedNodes.end(); ++i )
  {
    SplitNode* node = *i;

    std::cout << "search step is considering node " << (void*)node <<
      " - number " << index++ << " of " << count << std::endl;

    bool trySave = true;
    bool tryRecompute = true;
    float saveCost = 0;
    float recomputeCost = 0;

    if( i != _multiplyReferencedNodes.begin() )
    {
      if( !node->_wasConsideredSave && !node->_wasConsideredRecompute )
      {
        // this node has no real impact, it never got looked at
        std::cout << "skipping !!!" << std::endl;
        continue; 
      }

      if( node->_wasConsideredSave
        && node->_wasConsideredRecompute
        && (node->_wasSavedSave == node->_wasSavedRecompute) )
      {
        if( node->_wasSavedSave )
        {
          std::cout << "skipping save" << std::endl;
          trySave = false;
          saveCost = bestCost;
        }
        else
        {
          std::cout << "skipping recompute" << std::endl;
          tryRecompute = false;
          recomputeCost = bestCost;
        }
      }
    }
/*
    {for( NodeList::iterator j = _dagOrderNodeList.begin(); j != _dagOrderNodeList.end(); ++j )
    {
      (*j)->_wasConsidered = false;
      (*j)->_wasSaved = false;
    }}*/

    if( trySave )
    {
      {for( NodeList::iterator j = _dagOrderNodeList.begin(); j != _dagOrderNodeList.end(); ++j )
      {
        (*j)->_wasConsidered = false;
        (*j)->_wasSaved = false;
      }}

      node->_rdsFixedMarked = true;
      node->_rdsFixedUnmarked = false;
      saveCost = rdsCompileConfiguration();

      {for( NodeList::iterator j = _dagOrderNodeList.begin(); j != _dagOrderNodeList.end(); ++j )
      {
        (*j)->_wasConsideredSave = (*j)->_wasConsidered;
        (*j)->_wasSavedSave = (*j)->_wasSaved;
      }}
    }
//    dumpFile << "####### save cost is " << saveCost << std::endl;

    if( tryRecompute )
    {
      {for( NodeList::iterator j = _dagOrderNodeList.begin(); j != _dagOrderNodeList.end(); ++j )
      {
        (*j)->_wasConsidered = false;
        (*j)->_wasSaved = false;
      }}

      node->_rdsFixedMarked = false;
      node->_rdsFixedUnmarked = true;
      recomputeCost = rdsCompileConfiguration();

      {for( NodeList::iterator j = _dagOrderNodeList.begin(); j != _dagOrderNodeList.end(); ++j )
      {
        (*j)->_wasConsideredRecompute = (*j)->_wasConsidered;
        (*j)->_wasSavedRecompute = (*j)->_wasSaved;
      }}
    }
//    dumpFile << "####### recompute cost is " << saveCost << std::endl;

    if( saveCost < recomputeCost )
    {
      bestCost = saveCost;
      node->_rdsFixedUnmarked = false;
      node->_rdsFixedMarked = true;

//      dumpFile << "####### @final decision is to save " << (void*)node << std::endl << "  ";
//      node->dump( dumpFile );
//      dumpFile << std::endl;
    }
    else
    {
      bestCost = recomputeCost;
      node->_rdsFixedUnmarked = true;
      node->_rdsFixedMarked = false;

//      dumpFile << "####### final decision is to recompute " << (void*)node << std::endl << "  ";
//      node->dump( dumpFile );
//      dumpFile << std::endl;
    }
  }

  // use the resulting configuration
  // for one final compile pass
  rdsCompileConfiguration();
}

float SplitTree::rdsCompileConfiguration()
{

  rdsSubdivide();
  return getPartitionCost();
}

void SplitTree::rdsMerge( SplitNode* n, SplitShaderHeuristics& outHeuristics )
{
  assert( n );

//  dumpFile << "MERGE " << (void*)n << std::endl << "   ";
//  n->dump( dumpFile );
//  dumpFile << std::endl;

  // unvisit nodes
  unmark( SplitNode::kMarkBit_Merged );

  size_t childCount = n->getGraphChildCount();
  for( size_t i = 0; i < childCount; i++ )
  {
    SplitNode* child = n->getIndexedGraphChild(i);
    rdsMergeRec( child );
  }

  rdsTryMerge( n, outHeuristics );
}

void SplitTree::rdsTryMerge( SplitNode* n, SplitShaderHeuristics& outHeuristics )
{
  assert( n );

//  dumpFile << "TRY MERGE " << (void*)n << std::endl;
//  n->dump( dumpFile );
//  dumpFile << std::endl;

  // first try to merge with all children
  if( rdsCompile( n, outHeuristics ) )
    return;

//  dumpFile << "whole thing didn't work, trying to split" << std::endl;

  // count the number of unsaved kids
  size_t childCount = n->getGraphChildCount();
  NodeList unsavedChildren;
  for( size_t i = 0; i < childCount; i++ )
  {
    SplitNode* child = n->getIndexedGraphChild(i);
    if( !child->isMarkedAsSplit() )
      unsavedChildren.push_back( child );
  }
  size_t unsavedChildCount = unsavedChildren.size();

  assert( unsavedChildCount > 0 );

  size_t subsetSize = unsavedChildCount;
  while( subsetSize-- > 0 )
  {
    // try to do merges with the given subset size
//    dumpFile << "trying merges of " << subsetSize << " of the " << unsavedChildCount << " children" << std::endl;
    if( rdsMergeSome( n, unsavedChildren, subsetSize, outHeuristics ) )
      return;
  }

  assert( false );
}

bool SplitTree::rdsMergeSome( SplitNode* n, const NodeList& inUnsavedChildren, size_t inSubsetSize, SplitShaderHeuristics& outHeuristics )
{
  const NodeList& unsavedChildren = inUnsavedChildren;
  size_t unsavedChildCount = unsavedChildren.size();
  size_t subsetSize = inSubsetSize;

  std::vector< size_t > validSubsets;
  std::vector< SplitShaderHeuristics > validHeuristics;

  SplitSubsetGenerator generator( subsetSize, unsavedChildCount );
  while( generator.hasMore() )
  {
    size_t subsetBitfield = generator.getNext();

//    dumpFile << "subset chosen was " << subsetBitfield << std::endl;

    for( size_t i = 0; i < unsavedChildCount; i++ )
      unsavedChildren[i]->_splitHere = (subsetBitfield & (1 << i)) == 0;

    SplitShaderHeuristics subsetHeuristics;
    if( rdsCompile( n, subsetHeuristics ) )
    {
//      dumpFile << "subset " << subsetBitfield << " was valid (cost = " << subsetHeuristics.cost << ")" << std::endl;

      validSubsets.push_back( subsetBitfield );
      validHeuristics.push_back( subsetHeuristics );
    }
  }

  size_t validSubsetCount = validSubsets.size();
  
  if( validSubsetCount == 0 )
    return false;

  size_t bestSubset = validSubsets[0];
  SplitShaderHeuristics bestHeuristics = validHeuristics[0];

  for( size_t i = 1; i < validSubsetCount; i++ )
  {
    size_t otherSubset = validSubsets[i];
    SplitShaderHeuristics otherHeuristics = validHeuristics[i];

    if( otherHeuristics.cost < bestHeuristics.cost )
    {
      bestSubset = otherSubset;
      bestHeuristics = otherHeuristics;
    }
  }

//  dumpFile << "subset " << bestSubset << " was chosen (cost = " << bestHeuristics.cost << ")" << std::endl;

  // set the state of the child nodes to reflect
  // the chosen subset
  for( size_t i = 0; i < unsavedChildCount; i++ )
  {
    if( (bestSubset & (1 << i)) == 0 )
    {
      unsavedChildren[i]->_splitHere = true;

//      dumpFile << "merge step decided to @save " << (void*)(unsavedChildren[i]) << std::endl << "  ";
//      unsavedChildren[i]->dump( dumpFile );
//      dumpFile << std::endl;
    }
    else
      unsavedChildren[i]->_splitHere = false;
  }

  outHeuristics = bestHeuristics;
  return true;
}

void SplitTree::rdsMergeRec( SplitNode* n )
{
  assert( n );

//  dumpFile << "MERGE REC " << (void*)n << "   " << std::endl;
//  n->dump( dumpFile );
//  dumpFile << std::endl;

  if( n->isMarkedAsSplit() )
  {
//    dumpFile << "ignored as it was a split" << std::endl;
    return;
  }

  if( n->isPDTNode() )
  {
//    dumpFile << "ignored as it was a PDT node" << std::endl;
    return;
  }

  if( n->isMarked( SplitNode::kMarkBit_Merged ) )
  {
//    dumpFile << "ignored as it was an already-considered node" << std::endl;
    return;
  }

  n->mark( SplitNode::kMarkBit_Merged );

  size_t childCount = n->getGraphChildCount();
  
  // leaf nodes had better always work...
  if( childCount == 0 )
  {
    if( !rdsCompile( n ) )
      assert( false );
  }

  // first merge the subtrees
  for( size_t i = 0; i < childCount; i++ )
  {
    SplitNode* child = n->getIndexedGraphChild(i);
    rdsMergeRec( child );
  }

  SplitShaderHeuristics unused;
  rdsTryMerge( n, unused );

//  dumpFile << "MERGE REC DONE " << (void*)n << std::endl;
}

void SplitTree::rdsSubdivide()
{
  for( NodeList::iterator i = _dagOrderNodeList.begin(); i != _dagOrderNodeList.end(); ++i )
    (*i)->rdsUnmark();
  _outputPositionInterpolant->rdsUnmark();

  SplitShaderHeuristics unused;
  rdsSubdivide( _pseudoRoot, unused );
}

void SplitTree::rdsSubdivide( SplitNode* t, SplitShaderHeuristics& outHeuristics )
{
  assert( t );
  assert( t->isPDTNode() );

//  dumpFile << "SUBDIVIDE " << (void*)t << "   " << std::endl;
//  t->dump( dumpFile );
//  dumpFile << std::endl;

  // if it fits in a single pass, just do that...
  if( rdsCompile( t, outHeuristics ) )
  {
//    dumpFile << "it worked in a single pass!!" << std::endl;
    return;
  }

  size_t childCount = t->getPDTChildCount();
  for( size_t i = 0; i < childCount; i++ )
  {
    SplitNode* child = t->getIndexedPDTChild(i);

    // subdivide the child
    SplitShaderHeuristics childHeuristics;
    rdsSubdivide( child, childHeuristics );

    // try again to compile the current node
    if( rdsCompile( t, outHeuristics ) )
    {
//      dumpFile << "early exist after child subdivide" << std::endl;
      return;
    }

    // make save/recompute decision
    // for the child node
    rdsDecideSave( child, childHeuristics );
  }

  // last chance - compile again and hope it works
  if( rdsCompile( t, outHeuristics ) )
  {
//    dumpFile << "early exist after all children subdivided" << std::endl;
    return;
  }

//  dumpFile << "have to apply merging to this subtree" << std::endl;
  // otherwise we need to apply merging
  rdsMerge( t, outHeuristics );
}

void SplitTree::rdsDecideSave( SplitNode* n, const SplitShaderHeuristics& inHeuristics )
{
  if( !n->isMultiplyReferenced() )
    return;

//  dumpFile << "subdivide is deciding whether to save " << (void*)n << std::endl;
//  n->dump( dumpFile );
//  dumpFile << std::endl;

  if( n->_rdsFixedMarked )
  {
//    dumpFile << "fixed as saved" << std::endl;
    n->_splitHere = true;
  }
  else if( n->_rdsFixedUnmarked )
  {
//    dumpFile << "fixed as unsaved" << std::endl;
    n->_splitHere = false;
  }
  else
  {
    n->_splitHere = !( inHeuristics.recompute );

    n->_wasConsidered = true;
    n->_wasSaved = n->_splitHere;

//    dumpFile << "heuristic decided to " << (n->_splitHere ? "@save" : "recompute" ) << std::endl;
  }
}

bool SplitTree::rdsCompile( SplitNode* inNode )
{
  SplitShaderHeuristics heuristics;
  rdsCompile( inNode, heuristics );
  return heuristics.valid;
}

bool SplitTree::rdsCompile( SplitNode* inNode, SplitShaderHeuristics& outHeuristics )
{
  if( inNode == _pseudoRoot )
  {
    // TIM: the pseudo root will only
    // compile succesfully if all
    // of its children are splits
    bool valid = true;
    size_t childCount = _pseudoRoot->getGraphChildCount();
    for( size_t i = 0; i < childCount; i++ )
    {
      if( !_pseudoRoot->getIndexedGraphChild(i)->isMarkedAsSplit() )
        valid = false;
    }

    outHeuristics.cost = 0;
    outHeuristics.recompute = true;
    outHeuristics.valid = valid;
    return valid;
  }

  // TIM: we really need to handle this better:
  if( inNode->getGraphChildCount() == 0 )
  {
    outHeuristics.cost = 0;
    outHeuristics.recompute = true;
    outHeuristics.valid = true;
    return true;
  }

  std::vector<SplitNode*>  outputVector;
  outputVector.push_back( inNode );

  std::ostringstream nullStream;
  _compiler.compile( *this, outputVector, nullStream, outHeuristics );

  return outHeuristics.valid;
}

float SplitTree::getPartitionCost()
{
  // TIM: print it out for my edification :)
  for( NodeList::iterator i = _dagOrderNodeList.begin(); i != _dagOrderNodeList.end(); ++i )
  {
    SplitNode* node = *i;

    if( !node->isMarkedAsSplit() ) continue;

//    std::cerr << "%%%%%%%%%%%%%%%\n";
//    node->dump( std::cerr );
//    std::cerr << std::endl;
  }

  float totalCost = 0;
  SplitShaderHeuristics heuristics;
  for( NodeList::iterator i = _dagOrderNodeList.begin(); i != _dagOrderNodeList.end(); ++i )
  {
    SplitNode* node = *i;

    if( !node->isMarkedAsSplit() ) continue;

    bool valid = rdsCompile( node, heuristics );
    assert( valid );

    totalCost += heuristics.cost;
  }

  return totalCost;
}

void SplitTree::printShaderFunction( const std::vector<SplitNode*>& inOutputs, std::ostream& inStream ) const
{
  SplitArgumentTraversal printArguments(inStream,_outputPositionInterpolant);
  SplitStatementTraversal printStatements(inStream,_outputPositionInterpolant);

  for( size_t i = 0; i < _dagOrderNodeList.size(); i++ )
    _dagOrderNodeList[i]->unmarkAsOutput();
  _outputPositionInterpolant->unmarkAsOutput();
  {for( size_t i = 0; i < inOutputs.size(); i++ ){
    inOutputs[i]->markAsOutput();
  }}

  // create the wrapper for the function
  inStream << "void main(" << std::endl;

  unmark( SplitNode::kMarkBit_SubPrinted );
  printArguments( inOutputs );

  inStream << " ) {" << std::endl;

  unmark( SplitNode::kMarkBit_SubPrinted );
  printStatements( inOutputs );

  inStream << "}" << std::endl;
}

void SplitTree::printArgumentAnnotations( const std::vector<SplitNode*>& inOutputs, std::ostream& inStream ) const
{
  SplitAnnotationTraversal printAnnotations(inStream,_outputPositionInterpolant);

  unmark( SplitNode::kMarkBit_SubPrinted );
  printAnnotations( inOutputs );
}

static FunctionType* getFunctionType( FunctionDef* inFunctionDef )
{
  Decl* functionDecl = inFunctionDef->decl;
  assert( functionDecl->form->type == TT_Function );
  return ((FunctionType*)functionDecl->form);
}

void SplitTree::build( FunctionDef* inFunctionDef, const std::vector<SplitNode*>& inArguments )
{
  // TIM: hack to make temporaries have a shared position stream
  _outputPositionInterpolant = new InputInterpolantSplitNode( -1, 0, kSplitBasicType_Float2 );

  FunctionType* functionType = getFunctionType( inFunctionDef );

  SplitTreeBuilder builder( *this );

//  std::cerr << "function args: " << functionType->nArgs << " args passed: " << inArguments.size();

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

  _resultValue = builder.getResultValue();

  // we were called with arguments
  // thus we don't deal with creating
  // output nodes, or with building
  // the dominator tree...
}

void SplitTree::build( FunctionDef* inFunctionDef )
{
  // TIM: hack to make temporaries have a shared position stream
  _outputPositionInterpolant = new InputInterpolantSplitNode( -1, 0, kSplitBasicType_Float2 );

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

  _pseudoRoot = new SplitRootNode();

  _resultValue = builder.getResultValue();

  for( i = 0; i < functionType->nArgs; i++ )
  {
    Decl* argumentDecl = functionType->args[i];
    Type* argumentType = argumentDecl->form;
    std::string name = argumentDecl->name->name;

    if( (argumentType->getQualifiers() & TQ_Out) != 0 )
    {
      SplitNode* outputValue = builder.findVariable( name )->getValueNode();

      _pseudoRoot->addChild( outputValue );

      _outputList.push_back( outputValue );
      _outputArgumentIndices.push_back( (i+1) );
    }
  }

  if( _resultValue )
  {
    _pseudoRoot->addChild( _resultValue );
    _outputList.push_back( _resultValue );
    _outputArgumentIndices.push_back( 0 );
  }

  buildDominatorTree();
//  std::cerr << "done" << std::endl;
}
