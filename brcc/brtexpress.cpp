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
  
  assert(base->etype == ET_Variable);
  Variable *v = (Variable *) base;

  assert(v->name);
  assert(v->name->entry);
  assert(v->name->entry->uVarDecl);
  assert(v->name->entry->uVarDecl->form);
  assert(v->name->entry->uVarDecl->form->isArray());
  ArrayType *a = (ArrayType *) v->name->entry->uVarDecl->form;

  const ArrayType *pp;
  ndims = 1;
  for (pp = a; 
       pp->subType && pp->subType->isArray(); 
       pp = (ArrayType *)pp->subType)
     ndims++;

}

void
BrtGatherExpr::print (std::ostream& out) const
{
   out << "tex" << ndims << "D(";
   base->print(out);
   out << ",float" << ndims << "(";
   for (unsigned int i=0; i<dims.size(); i++) {
      if (i) out << ",";
      dims[i]->print(out);
   }
   out << ")";
   // now scale and modulate by the constant:
   if( ndims == 1 )
   {
     out << "*";
     base->print(out);
     out << "_scalebias.x";
     out << "+";
     base->print(out);
     out << "_scalebias.z";
   }
   else if( ndims == 2 )
   {
     out << "*";
     base->print(out);
     out << "_scalebias.xy";
     out << "+";
     base->print(out);
     out << "_scalebias.zw";
   }
   else
   {
     // TODO: handle the larger cases
     std::cerr << "GPU runtimes can't handle gathers greater than 2D" << std::endl;
   }
   out << ")";
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

