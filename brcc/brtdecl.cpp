/*
 * brtdecl.cpp --
 * Stream types and gather types are actually printed and dealt with here.
 */

#include <cassert>
#include <cstring>

#include "decl.h"
#include "express.h"
#include "stemnt.h"

#include "token.h"
#include "gram.h"
#include "project.h"

#include "brtdecl.h"

BrtStreamType::BrtStreamType(const ArrayType *t)
  : Type(TT_BrtStream)
{
  const ArrayType *p;

  // First find the base type of the array;
  for (p = t; 
       p->subType && p->subType->isArray(); 
       p = (ArrayType *)p->subType)
    dims.push_back(p->size->dup0());
  dims.push_back(p->size->dup0());

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
  out << "(";
  
  if (base->getBase()->typemask&BT_Float4) {
    out << "__BRTFLOAT4";
  }else if (base->getBase()->typemask&BT_Float3) {
    out << "__BRTFLOAT3";
  }else if (base->getBase()->typemask&BT_Float2) {
    out << "__BRTFLOAT2";
  }else if (base->getBase()->typemask&BT_Float) {
    out << "__BRTFLOAT";
  }else {
    std::cerr << "Warning: Unsupported stream type ";
    base->printBase(std::cerr,0);
    std::cerr << std::endl;
    out << "__BRTFLOAT";
  }
  out << ",";
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

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
BrtIterType::BrtIterType(const ArrayType *stream, const FunctionCall *f)
  : Type(TT_BrtIter)
{
  const ArrayType *p;
  ExprVector::const_iterator i;

  assert(f->function->etype == ET_Variable);
  assert(strcmp(((Variable *) f->function)->name->name.c_str(),"BRTIter") == 0);

  // Drill down to the find the baseType and inflate our dimensions
  for (p = stream;
       p->subType && p->subType->isArray();
       p = (ArrayType *)p->subType) {
    dims.push_back(p->size->dup0());
  }
  dims.push_back(p->size->dup0());

  assert(p->subType && p->subType->isBaseType());
  base = (BaseType *) p->subType->dup0();

  /*
   * Now fish the min / max out of 'f'.
   *
   * We impose the following constraints on dimensions
   *   - We support float, float2, float3, or float4 1-D streams
   *   - We float2 2-D streams
   */
  assert(f->args.size() == 2 * FloatDimension(base->typemask));
  assert(dims.size() == 1 ||
         (dims.size() == 2 && base->typemask == BT_Float2));
  for (i = f->args.begin(); i != f->args.end(); i++) {
     args.push_back((*i)->dup0());
  }
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
BrtIterType::~BrtIterType()
{
   ExprVector::iterator i;

   /* base is taken care of when the global type list is destroyed */

   for (i = dims.begin(); i != dims.end(); i++) {
      delete *i;
   }
   for (i = args.begin(); i != args.end(); i++) {
      delete *i;
   }
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
Type*
BrtIterType::dup0() const
{
   ExprVector::const_iterator i;

   BrtIterType *r = new BrtIterType();
   r->base = (BaseType *) base->dup0();

   for (i = dims.begin(); i != dims.end(); i++) {
      r->dims.push_back((*i)->dup0());
   }
   for (i = args.begin(); i != args.end(); i++) {
      r->args.push_back((*i)->dup0());
   }
   return r;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
BrtIterType::printType( std::ostream& out, Symbol *name,
                        bool showBase, int level ) const
{
  ExprVector::const_iterator i;

  out << "__BRTIter ";

  if (name)
    out << *name;

  // TIM: add initializer as constructor
  out << "(";

  if (base->getBase()->typemask&BT_Float4) {
    out << "__BRTFLOAT4, ";
  }else if (base->getBase()->typemask&BT_Float3) {
    out << "__BRTFLOAT3, ";
  }else if (base->getBase()->typemask&BT_Float2) {
    out << "__BRTFLOAT2, ";
  }else if (base->getBase()->typemask&BT_Float) {
    out << "__BRTFLOAT, ";
  }else {
    std::cerr << "Warning: Unsupported stream type ";
    base->printBase(std::cerr,0);
    std::cerr << std::endl;
    out << "__BRTFLOAT, ";
  }

  /* Now print the dimensions */
  for (i = dims.begin(); i != dims.end(); i++) {
    (*i)->print(out);
    out << ",";
  }
  out << " -1, ";

  /* Now print the min / max */
  for (i = args.begin(); i != args.end(); i++) {
    (*i)->print(out);
    out << ", ";
  }
  out << "-1)";
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
BrtIterType::printForm(std::ostream& out) const
{
    out << "-BrtIter Type ";
    if (base)
        base->printBase(out, 0);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
BrtIterType::printInitializer(std::ostream& out) const
{
  out << "__BRTCreateIter(\"";
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
BrtIterType::findExpr( fnExprCallback cb )
{
    if (base) base->findExpr(cb);

    for (unsigned int i=0; i<dims.size(); i++) {
       dims[i] = (cb)(dims[i]);
       dims[i]->findExpr(cb);
    }
}

CPUGatherType::CPUGatherType(const ArrayType &t,bool copy_on_write) {
	dimension=0;
	at = &t;
	this->copy_on_write=copy_on_write;
	subtype = at;
	while (subtype->type==TT_Array) {
		dimension +=1;
		subtype = static_cast<const ArrayType*>(subtype)->subType;
	}
}
void CPUGatherType::printSubtype(std::ostream&out,Symbol *name,bool showBase,int level) const {
    subtype->printType(out,name,showBase,level);
	
}
void CPUGatherType::printType(std::ostream &out, Symbol * name, bool showBase, int level) const
{
	printBefore(out,name,level);
	printAfter(out);	
	
}
void CPUGatherType::printBefore(std::ostream & out, Symbol *name, int level) const {
	if (!copy_on_write) {
		out << "__BrtArray<";
		Symbol nothing;nothing.name="";
		printSubtype(out,&nothing,true,level);
		out << ", "<<dimension <<"  , false";		
	}else {
		out << "Array"<<dimension<<"d<";
		
		at->printBase(out,level);
		
		
		const Type * t = at;
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


