#include <cassert>
#include <cstring>

#include "decl.h"
#include "express.h"
#include "stemnt.h"

#include "token.h"
#include "gram.h"
#include "project.h"

#include "brtdecl.h"

BrtStreamType::BrtStreamType()
  : Type(TT_BrtStream)
{
}

BrtStreamType::BrtStreamType(const ArrayType *t)
  : Type(TT_BrtStream)
{
  const ArrayType *p;

  // First find the base type of the array;
  for (p = t; 
       p->subType && p->subType->isArray(); 
       p = (ArrayType *)p->subType)
    dims.push_back(t->size->dup0());
  dims.push_back(t->size->dup0());

  assert (p->subType);
  assert (p->subType->isBaseType());

  base = (BaseType *) p->subType->dup0();
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
BrtStreamType::~BrtStreamType()
{
    // Handled by deleting the global type list
    // delete subType;
    for (unsigned int i=0; i<dims.size(); i++)
      delete dims[i];
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
Type*
BrtStreamType::dup0() const
{
  BrtStreamType *r = new BrtStreamType();
  r->base = (BaseType *) base->dup0();

  for (unsigned int i=0; i<dims.size(); i++)
    r->dims.push_back(dims[i]->dup0());
  
  return r;
}

void 
BrtStreamType::printType( std::ostream& out, Symbol *name,
			  bool showBase, int level ) const
{
  out << "__BrookStream ";
  if (name) 
    out << *name;
}

void
BrtStreamType::printForm(std::ostream& out) const
{
    out << "-BrtStream Type ";
    if (base)
        base->printBase(out, 0);
}

void
BrtStreamType::printInitializer(std::ostream& out) const
{
  out << "__BrtCreateStream(\"";
  base->printBase(out, 0);
  out << "\",";
  for (unsigned int i=0; i<dims.size(); i++) {
    dims[i]->print(out);
    out << ",";
  }
  out << "-1)";
} 
  

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
BrtStreamType::findExpr( fnExprCallback cb )
{
    if (base)
        base->findExpr(cb);

    for (unsigned int i=0; i<dims.size(); i++)
      dims[i]->findExpr(cb);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
bool
BrtStreamType::lookup( Symbol* sym ) const
{
  if (base)
    return base->lookup(sym);
  else
    return false;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
TypeQual
BrtStreamType::getQualifiers( void )
{
   return base->getQualifiers();
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
BaseType *
BrtStreamType::getBase( void )
{
   return base->getBase();
}


CPUGatherType::CPUGatherType(const ArrayType &t,bool toplevel):ArrayType(*static_cast<ArrayType*>(t.dup())){
	this->toplevel=toplevel;
	dimension=1;
	if (subType->type==TT_Array) {
		CPUGatherType * at = new CPUGatherType(*static_cast<ArrayType*>(subType),false);
		subType = at;
		dimension = at->dimension+1;
	}
}
Type * CPUGatherType::dup0()const {
	return new CPUGatherType (*this);
}
void CPUGatherType::printBefore(std::ostream & out, Symbol *name, int level) const {
	Symbol nothing;
	nothing.name="";
	out << "Array"<<dimension<<"d<";
	subType->getBase()->printBefore(out,&nothing,0);
	const Type * t = this;
	for (unsigned int i=0;i<dimension&&i<3;i++) {
		if (i!=0)
			out <<", ";
		const ArrayType *a =static_cast<const ArrayType *>(t);
		a->size->print(out);
		t = a->subType;			
	}
	out << "> "; 
	out << *name;
}
void CPUGatherType::printAfter(std::ostream &out) const{
		//nothing happens
		//you fail to obtain anything
		//...
}


