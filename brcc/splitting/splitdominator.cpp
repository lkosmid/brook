// splitdominator.cpp

#include "splitting.h"

#include <fstream>

class SplitDominatorDFSTraversal :
  public SplitNodeTraversal
{
public:
  SplitDominatorDFSTraversal( std::vector<SplitNode*>& ioNodeList, std::vector<SplitNode*>& ioDagOrderList )
    : _id(1), _parent(0), _nodeList(ioNodeList), _dagOrderNodeList(ioDagOrderList)
  {}

  void traverse( SplitNode* inNode )
  {

    if( _parent ) {
      inNode->_graphParents.push_back( _parent );
    }

    if( inNode->_spanningNodeID > 0 )
      return;

//    std::cerr << "dominator DFS id (" << _id << ") : ";
//    inNode->dump( std::cerr );
//    std::cerr << std::endl;

    inNode->_spanningParent = _parent;
//    _parent->_spanningChildren.push_back( inNode );

    inNode->_spanningNodeID = _id++;
    inNode->_spanningSemidominatorID = inNode->_spanningNodeID;
    _nodeList.push_back( inNode );

//    std::cerr << "{";
    SplitDominatorDFSTraversal subTraversal( _id, inNode, _nodeList, _dagOrderNodeList );
    inNode->traverseChildren( subTraversal );
//    std::cerr << "}";

    _dagOrderNodeList.push_back( inNode );

    _id = subTraversal._id;
  }

private:
  SplitDominatorDFSTraversal( size_t inID, SplitNode* inParent, std::vector<SplitNode*>& ioNodeList, std::vector<SplitNode*>& ioDagOrderList )
    : _id(inID), _parent(inParent), _nodeList(ioNodeList), _dagOrderNodeList(ioDagOrderList)
  {}

  size_t _id;
  SplitNode* _parent;
  std::vector<SplitNode*>& _nodeList;
  std::vector<SplitNode*>& _dagOrderNodeList;
};

void SplitTree::buildDominatorTree()
{
//  std::cerr << "buildDominatorTree" << std::endl;

  for( NodeMap::iterator i = outputValues.begin(); i != outputValues.end(); ++i )
    _outputList.push_back( (*i).second );
  if( _resultValue )
    _outputList.push_back( _resultValue );

//  std::cerr << "step 1" << std::endl;

  // build the immediate dominator info...
  // step 1 - dfs
  SplitDominatorDFSTraversal dominatorDFS( _rdsNodeList, _dagOrderNodeList );
  dominatorDFS( _outputList );

//  std::cerr << "steps 2,3" << std::endl;
  // step 2 and 3, build semidominators
  {for( size_t i = _rdsNodeList.size(); i > 0; i-- )
  {
    SplitNode* w = _rdsNodeList[i-1];

//    std::cerr << "operating on ";
//    w->dump( std::cerr );
//    std::cerr << std::endl;

//    std::cerr << "step 2" << std::endl;
    // step 2
    for( std::vector<SplitNode*>::iterator j = w->_graphParents.begin(); j != w->_graphParents.end(); ++j )
    {
      SplitNode* v = *j;
      SplitNode* u = v->eval();

//      std::cerr << "u is ";
//      u->dump( std::cerr );
//      std::cerr << std::endl;

      if( u->_spanningSemidominatorID < w->_spanningSemidominatorID )
        w->_spanningSemidominatorID = u->_spanningSemidominatorID;
    }
    _rdsNodeList[ w->_spanningSemidominatorID-1 ]->_spanningBucket.insert( w );

    SplitNode* parent = w->_spanningParent;
    if( parent )
    {
      parent->link( w );

//      std::cerr << "step 3" << std::endl;
      // step 3
      for( NodeSet::iterator k = parent->_spanningBucket.begin(); k != parent->_spanningBucket.end(); ++k )
      {
        SplitNode* v = *k;
        SplitNode* u = v->eval();
        v->_pdtDominator = (u->_spanningSemidominatorID < v->_spanningSemidominatorID) ? u : parent;
      }
      parent->_spanningBucket.clear();
    }
  }}

//  std::cerr << "step 4" << std::endl;
  // step 4
  {for( size_t i = 0; i < _rdsNodeList.size(); i++ )
  {
    SplitNode* w = _rdsNodeList[i];
    if( !w->_spanningParent )
      w->_pdtDominator = 0;
    else if( w->_pdtDominator != _rdsNodeList[w->_spanningSemidominatorID-1])
      w->_pdtDominator = w->_pdtDominator->_pdtDominator;
  }}

//  std::cerr << "step PDT" << std::endl;
  // we have dominator info... 
  // now we need to prune it to the Partial Dominator Tree,
  // and the list of MR nodes...
  // we iterate in reverse order, because the
  // roots are at the beginning of the list...
  {for( size_t i = 0; i < _dagOrderNodeList.size(); i++ )
  {
    SplitNode* n = _dagOrderNodeList[i];

    // we add it to the partial tree if it is multiply-referenced
    // or has some multiply-referenced descendants (which would
    // have already been added).
    // TIM: we ignore nodes that are taken to be "trivial"
    // (that is, those that should never be saved)
    if( n->_graphParents.size() > 1 /*&& !n->isTrivial()*/ )
    {
      _multiplyReferencedNodes.push_back( n );
      if( n->_pdtDominator )
        n->_pdtDominator->_pdtChildren.push_back( n );
    }
    else if( n->_pdtChildren.size() != 0 )
    {
      if( n->_pdtDominator )
        n->_pdtDominator->_pdtChildren.push_back( n );
    }
  }}

//  dumpDominatorTree();
}

void SplitTree::dumpDominatorTree()
{
  std::ofstream dumpFile("dominator_dump.txt");

  size_t outputCount = _outputList.size();
  for( size_t o = 0; o < outputCount; o++ )
  {
    SplitNode* output = _outputList[o];
    dumpDominatorTree( dumpFile, output );
  }
}

void SplitTree::dumpDominatorTree( std::ostream& inStream, SplitNode* inNode, int inLevel )
{
  for( int l = 0; l < inLevel; l++ )
    inStream << "   ";
  
  inNode->dump( inStream );
  inStream << std::endl;

  size_t childCount = inNode->_pdtChildren.size();
  for( size_t c = 0; c < childCount; c++ )
  {
    SplitNode* child = inNode->_pdtChildren[c];
    assert( child->_pdtDominator == inNode );
    dumpDominatorTree( inStream, child, inLevel+1 );
  }
}
