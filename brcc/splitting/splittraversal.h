// splittraversal.h
#ifndef __SPLITTRAVERSAL_H__
#define __SPLITTRAVERSAL_H__

#include <iostream>

class SplitNode;

class SplitNodeTraversal
{
public:
  void operator()( SplitNode* inNode ) {
    traverse( inNode );
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
    : samplerCount(0), constantCount(0), texcoordCount(0) {}

  int samplerCount;
  int constantCount;
  int texcoordCount;
};

class SplitArgumentTraversal :
  public SplitNodeTraversal
{
public:
  SplitArgumentTraversal( std::ostream& inStream )
    : stream(inStream), hasOutput(false) {}

  void traverse( SplitNode* inNode );

private:
  SplitArgumentCounter argumentCounter;
  std::ostream& stream;
  bool hasOutput;
};

class SplitStatementTraversal :
  public SplitNodeTraversal
{
public:
  SplitStatementTraversal( std::ostream& inStream )
    : stream(inStream) {}
  void traverse( SplitNode* inNode );

private:
  std::ostream& stream;
};

#endif