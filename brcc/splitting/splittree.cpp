// splittree.cpp
#include "splittree.h"

#include "splitnode.h"
#include "splitbuilder.h"

#include <iostream>
#include <string>


SplitTree::SplitTree( FunctionDef* inFunctionDef )
{
  build( inFunctionDef );
}

SplitTree::~SplitTree()
{
}

void SplitTree::printShaderFunction( std::ostream& inStream )
{
  // TIM: better hope there's only one output :)
  for( NodeMap::iterator i = outputValues.begin(); i != outputValues.end(); i++ )
  {
    (*i).second->printSubFunction( "main", inStream );
  }
}

void SplitTree::build( FunctionDef* inFunctionDef )
{
  Decl* functionDecl = inFunctionDef->decl;
  assert( functionDecl->form->type == TT_Function );
  FunctionType* functionType = ((FunctionType*)functionDecl->form);
  Statement* headStatement = inFunctionDef->head;

  SplitTreeBuilder builder( *this );

  for( unsigned int i = 0; i < functionType->nArgs; i++ )
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

  for( unsigned int i = 0; i < functionType->nArgs; i++ )
  {
    Decl* argumentDecl = functionType->args[i];
    Type* argumentType = argumentDecl->form;
    std::string name = argumentDecl->name->name;

    if( (argumentType->getQualifiers() & TQ_Out) != 0 )
    {
      SplitNode* outputValue = builder.findVariable( name );
      outputValues[name] = outputValue;
    }
  }
}