#include "brtexpress.h"


BrtGatherExpr::BrtGatherExpr(const IndexExpr *expr) 
   : Expression (ET_BrtGatherExpr, expr->location)
{
  const IndexExpr *p;
  std::vector<Expression *> t;
  int i;

  t.push_back(expr->_subscript->dup0());

  // IAB:  Note that we have to reorder the arguments
  for (p = (IndexExpr *) expr->array; 
       p && p->etype == ET_IndexExpr;
       p = (IndexExpr *) p->array)
     t.push_back(p->_subscript->dup0());

  for (i=t.size()-1; i>=0; i--)
     dims.push_back(t[i]);

  base = (Expression *) p->dup0();
}

void
BrtGatherExpr::print (std::ostream& out) const
{
   out << "tex" << dims.size() << "D(";
   base->print(out);
   out << ",float" << dims.size() << "(";
   for (unsigned int i=0; i<dims.size(); i++) {
      if (i) out << ",";
      dims[i]->print(out);
   }
   out << "))";
}


BrtStreamInitializer::BrtStreamInitializer(const BrtStreamType *type,
					 const Location& loc )
  : Expression( ET_BrtStreamInitializer, loc), l(loc)
{
  t = (BrtStreamType *) type->dup0();
}

BrtStreamInitializer::~BrtStreamInitializer() {
  // Handled by global type list
  //   delete t;
}

Expression *
BrtStreamInitializer::dup0() const {
  return new BrtStreamInitializer(t, l);
}

void
BrtStreamInitializer::print(std::ostream& out) const {
  t->printInitializer(out);
}

