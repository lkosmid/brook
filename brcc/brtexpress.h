/*
 * brtexpress.h
 * Gather, stream, and indexof exprs get transformed into these
 */

#ifndef BRTEXPRESS_H
#define BRTEXPRESS_H

#include "express.h"
#include "brtdecl.h"


// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
class BrtGatherExpr : public Expression
{
public:
   BrtGatherExpr (const IndexExpr *expr);
   ~BrtGatherExpr () {/* do nothing*/}

   void print (std::ostream& out) const;
   
   int ndims;

   Expression *base;
   ExprVector dims;
};

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
class BrtStreamInitializer : public Expression
{
  public:

  BrtStreamInitializer(const BrtStreamType *t, const Location& l );
  ~BrtStreamInitializer();
  
  Expression *dup0() const;
  void print(std::ostream& out) const;
  
  BrtStreamType *t;
  Location l;
};

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
class BrtIndexofExpr : public Expression
{
  public:
    BrtIndexofExpr( Variable *operand, const Location& l );
   ~BrtIndexofExpr();

    int precedence() const { return 15; }

    Expression *dup0() const;
    void print(std::ostream& out) const;

    void findExpr( fnExprCallback cb );

    Variable  *expr;          // want the size of this expression.
};


#endif  /* BRTEXPRESS_H */
