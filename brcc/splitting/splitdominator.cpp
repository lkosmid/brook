// splitdominator.cpp

#include "splitting.h"

class SplitDominatorDFSTraversal :
  public SplitNodeTraversal
{
public:
  SplitDominatorDFSTraversal( std::vector<SplitNode*>& ioNodeList )
    : _id(1), _parent(0), _nodeList(ioNodeList)
  {}

  void traverse( SplitNode* inNode )
  {
    if( _parent ) {
      inNode->_graphParents.push_back( _parent );
      _parent->_spanningChildren.push_back( inNode );
    }

    if( inNode->_spanningNodeID > 0 )
      return;

    inNode->_spanningParent = _parent;
    inNode->_spanningNodeID = _id++;
    inNode->_spanningSemidominatorID = inNode->_spanningNodeID;
    _nodeList.push_back( inNode );

    SplitDominatorDFSTraversal subTraversal( _id, inNode, _nodeList );
    inNode->traverseChildren( subTraversal );

    _id = subTraversal._id;
  }

private:
  SplitDominatorDFSTraversal( size_t inID, SplitNode* inParent, std::vector<SplitNode*>& ioNodeList )
    : _id(inID), _parent(inParent), _nodeList(ioNodeList)
  {}

  size_t _id;
  SplitNode* _parent;
  std::vector<SplitNode*>& _nodeList;
};

void SplitTree::buildDominatorTree()
{
  std::cerr << "buildDominatorTree" << std::endl;

  for( NodeMap::iterator i = outputValues.begin(); i != outputValues.end(); ++i )
    _outputList.push_back( (*i).second );

  // build the immediate dominator info...
  // step 1 - dfs
  SplitDominatorDFSTraversal dominatorDFS( _rdsNodeList );
  dominatorDFS( _outputList );

  // step 2 and 3, build semidominators
  {for( size_t i = _rdsNodeList.size(); i > 1; i-- )
  {
    SplitNode* w = _rdsNodeList[i-1];
    // step 2
    for( std::vector<SplitNode*>::iterator j = w->_graphParents.begin(); j != w->_graphParents.end(); ++j )
    {
      SplitNode* u = (*j)->eval();
      if( u->_spanningSemidominatorID < w->_spanningSemidominatorID )
        w->_spanningSemidominatorID = u->_spanningSemidominatorID;
    }
    _rdsNodeList[ w->_spanningSemidominatorID-1 ]->_spanningBucket.push_back( w );
    w->_spanningParent->link( w );

    // step 3
    SplitNode* parent = w->_spanningParent;
    for( std::vector<SplitNode*>::iterator k = parent->_spanningBucket.begin(); k != parent->_spanningBucket.end(); ++k )
    {
      SplitNode* v = *k;
      SplitNode* u = v->eval();
      v->_pdtDominator = (u->_spanningSemidominatorID < v->_spanningSemidominatorID) ? u : w->_spanningParent;
    }
    parent->_spanningBucket.clear();
  }}

  // step 4
  {for( size_t i = 2; i < _rdsNodeList.size(); i++ )
  {
    SplitNode* w = _rdsNodeList[i];
    if( w->_pdtDominator != _rdsNodeList[w->_spanningSemidominatorID-1])
      w->_pdtDominator = w->_pdtDominator->_pdtDominator;
  }}
  {for( size_t i = 0; i < _outputList.size(); i++ ){
    _outputList[i]->_pdtDominator = 0;
  }}
  // we have dominator info... 
  // now we need to prune it to the Partial Dominator Tree,
  // and the list of MR nodes...
  {for( size_t i = 0; i < _rdsNodeList.size(); i++ )
  {
    SplitNode* n = _rdsNodeList[i];

    // we add it to the partial tree if it is multiply-referenced
    // or has some multiply-referenced descendants (which would
    // have already been added).
    if( n->_graphParents.size() > 1 )
    {
      _multiplyReferencedNodes.push_back( n );
      n->_pdtDominator->_pdtChildren.push_back( n );
    }
    else if( n->_pdtChildren.size() != 0 )
    {
      n->_pdtDominator->_pdtChildren.push_back( n );
    }
  }}
}
