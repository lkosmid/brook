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


  if( inNode->isMarkedAsOutput() )
  {
    inNode->traverseChildren( *this );

    if( hasOutput )
      stream << ",\n\t\t";
    else
      stream << "\t\t";

    // ignore other goo, and assume it needs an output
    stream << "out float4 ";
    inNode->printTemporaryName( stream );
    stream << " : COLOR" << argumentCounter.outputCount++;
    hasOutput = true;
    return;
  }

  if( inNode->isMarkedAsSplit() )
  {
    traverse( outputPosition );

    if( hasOutput )
      stream << ",\n\t\t";
    else
      stream << "\t\t";

    // ignore other goo, and assume it is a saved value...
    stream << "uniform _stype ";
    inNode->printTemporaryName( stream );
    stream << "_saved : register(s" << argumentCounter.samplerCount++ << ")";
    hasOutput = true;
    return;
  }

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

  if( inNode->isMarkedAsOutput() )
  {
    inNode->traverseChildren( *this );

    inNode->printTemporaryName( stream );
    stream << " = float4(";
    inNode->printTemporaryExpression( stream );
    switch( inNode->inferredType )
    {
    case kSplitBasicType_Float:
      stream << ",0,0,0";
      break;
    case kSplitBasicType_Float2:
      stream << ",0,0";
      break;
    case kSplitBasicType_Float3:
      stream << ",0";
      break;
    case kSplitBasicType_Float4:
      break;
    }
    stream << ");" << std::endl;
    return;
  }

  if( inNode->isMarkedAsSplit() )
  {
    traverse( outputPosition );

    stream << inNode->inferredType;
    stream << " ";
    inNode->printTemporaryName( stream );
    stream << " = ";

    // TIM: fake it for now
    stream << "__sample2(";
    inNode->printTemporaryName( stream );
    stream << "_saved, ";
    outputPosition->printTemporaryName( stream );
    stream << ");" << std::endl;
    return;
  }

  inNode->traverseChildren( *this );

  stream << "\t";
  if( inNode->needsTemporaryVariable() )
  {
    // if it needs us to declare the var, do it...
    stream << inNode->inferredType;
    stream << " ";
  }
  inNode->printTemporaryName( stream );
  stream << " = ";
  inNode->printTemporaryExpression( stream );
  stream << "; // ";
  inNode->printExpression( stream );
  stream << std::endl;
}

void SplitAnnotationTraversal::traverse( SplitNode* inNode )
{
  if( inNode->marked ) return;
  inNode->marked = true;


  if( inNode->isMarkedAsOutput() )
  {
    inNode->traverseChildren( *this );

    if( OutputSplitNode* output = inNode->isOutputNode() )
    {
      stream << "\t\t\t";
      inNode->printAnnotationInfo( stream );
      stream << "\n";
    }
    else
    {
      stream << "\t\t\t";
      stream << ".output(-" << inNode->getTemporaryID() << ",0)";
      stream << "\n";
    }
    return;
  }

  if( inNode->isMarkedAsSplit() )
  {
    traverse( outputPosition );

    stream << "\t\t\t";
    stream << ".sampler(-" << inNode->getTemporaryID() << ",0)";
    stream << "\n";
    // mark temporary sampler...
    return;
  }

  inNode->traverseChildren( *this );

  if( !inNode->needsAnnotation() ) return;

  stream << "\t\t\t";
  inNode->printAnnotationInfo( stream );
  stream << "\n";
}
