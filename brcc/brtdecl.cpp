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
  out << "__BRTStream ";
  if (name) 
    out << *name;

  // TIM: add initializer as constructor
  out << "(\"";
  base->printBase(out, 0);
  out << "\",";
  for (unsigned int i=0; i<dims.size(); i++) {
    dims[i]->print(out);
    out << ",";
  }
  out << "-1)";
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
  out << "__BRTCreateStream(\"";
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

    for (unsigned int i=0; i<dims.size(); i++) {
       dims[i] = (cb)(dims[i]);
       dims[i]->findExpr(cb);
    }
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


CPUGatherType::CPUGatherType(const ArrayType &t,bool copy_on_write):ArrayType(*static_cast<ArrayType*>(t.dup())){
	dimension=1;
	this->copy_on_write=copy_on_write;
	if (subType->type==TT_Array) {
		CPUGatherType * at = new CPUGatherType(*static_cast<ArrayType*>(subType),copy_on_write);
		subType = at;
		dimension = at->dimension+1;
	}
}
Type * CPUGatherType::dup0()const {
	return new CPUGatherType (*this);
}
void CPUGatherType::printSubtype(std::ostream&out,Symbol *name,bool showBase,int level) const {
	const Type *t = this;
	while (t->type==TT_Array) {
		t = static_cast<const ArrayType*> (t)->subType;
	}
	t->printType(out,name,showBase,level);
	
}
void CPUGatherType::printType(std::ostream &out, Symbol * name, bool showBase, int level) const
{

	printBefore(out,name,level);
	printAfter(out);	
	
}
void CPUGatherType::printBefore(std::ostream & out, Symbol *name, int level) const {
	if (!copy_on_write) {
		out << "BrtArray<";
		Symbol nothing;nothing.name="";
		printSubtype(out,&nothing,true,level);
		out << ", "<<dimension <<"  , false";		
	}else {
		out << "Array"<<dimension<<"d<";
		
		printBase(out,level);
		
		
		const Type * t = this;
		for (unsigned int i=0;i<dimension&&i<3;i++) {
			if (i!=0)
			out <<"	 ";
			out <<", ";
			const ArrayType *a =static_cast<const ArrayType *>(t);
			a->size->print(out);
			t = a->subType;			
		}
	}
	out << "> "; 
	out << *name;
}
void CPUGatherType::printAfter(std::ostream &out) const{
		//nothing happens
		//you fail to obtain anything
		//...
}


