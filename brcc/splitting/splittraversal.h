// splittraversal.h
#ifndef __SPLITTRAVERSAL_H__
#define __SPLITTRAVERSAL_H__

#include <iostream>
#include <vector>

class SplitNode;

class SplitNodeTraversal
{
public:
  void operator()( SplitNode* inNode ) {
    traverse( inNode );
  }

  void operator()( const std::vector<SplitNode*>& inNodes ) {
    for( std::vector<SplitNode*>::const_iterator i = inNodes.begin(); i != inNodes.end(); ++i )
      traverse( *i );
  }

  virtual void traverse( SplitNode* inNode ) = 0;
};

class SplitMarkTraversal :
  public SplitNodeTraversal
{
public:
  SplitMarkTraversal( bool inValue )
    : value(inValue) {}

  void traverse( SplitNode* inNode );

private:
  bool value;
};

class SplitArgumentCounter
{
public:
  SplitArgumentCounter()
    : samplerCount(0), constantCount(0), texcoordCount(0), outputCount(0) {}

  int samplerCount;
  int constantCount;
  int texcoordCount;
  int outputCount;
};

class SplitArgumentTraversal :
  public SplitNodeTraversal
{
public:
  SplitArgumentTraversal( std::ostream& inStream, SplitNode* inOutputPosition )
    : stream(inStream), hasOutput(false), outputPosition(inOutputPosition) {}

  void traverse( SplitNode* inNode );

private:
  SplitArgumentCounter argumentCounter;
  std::ostream& stream;
  SplitNode* outputPosition;
  bool hasOutput;
};

class SplitStatementTraversal :
  public SplitNodeTraversal
{
public:
  SplitStatementTraversal( std::ostream& inStream, SplitNode* inOutputPosition )
    : stream(inStream), outputPosition(inOutputPosition) {}
  void traverse( SplitNode* inNode );

private:
  std::ostream& stream;
  SplitNode* outputPosition;
};

class SplitAnnotationTraversal :
  public SplitNodeTraversal
{
public:
  SplitAnnotationTraversal( std::ostream& inStream, SplitNode* inOutputPosition )
    : stream(inStream), outputPosition(inOutputPosition) {}
  void traverse( SplitNode* inNode );

private:
  std::ostream& stream;
  SplitNode* outputPosition;
};

#endif