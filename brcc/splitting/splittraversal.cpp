 // splittraversal.cpp
#include "splittraversal.h"

#include "splitnode.h"

void SplitMarkTraversal::traverse( SplitNode* inNode )
{
  inNode->marked = value;
  inNode->traverseChildren( *this );
}

void SplitArgumentTraversal::traverse( SplitNode* inNode )
{
  if( inNode->marked ) return;
  inNode->marked = true;

  inNode->traverseChildren( *this );

  if( !inNode->needsArgument() ) return;

  if( hasOutput )
    stream << ",\n\t\t";
  else
    stream << "\t\t";

  inNode->printArgumentInfo( stream, argumentCounter );
  hasOutput = true;
}

void SplitStatementTraversal::traverse( SplitNode* inNode )
{
  if( inNode->marked ) return;
  inNode->marked = true;

  if( !inNode->needsTemporaryExpression() ) return;

  inNode->traverseChildren( *this );

  stream << "\t";
  stream << inNode->inferredType;
  stream << " ";
  inNode->printTemporaryName( stream );
  stream << " = ";
  inNode->printTemporaryExpression( stream );
  stream << "; // ";
  inNode->printExpression( stream );
  stream << std::endl;
}