/*
 * brtdecl.h  --
 * Stream type and gather type are operated on here
 */

#ifndef    BRTDECL_H
#define    BRTDECL_H

#include <cstdlib>
#include <iostream>
#include <vector>
#include <cassert>

#include "symbol.h"
#include "callback.h"
#include "location.h"

#include "dup.h"

#include "decl.h"
#include "express.h"

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
class BrtStreamType : public Type
{
public:
  BrtStreamType(const ArrayType *t);
  BrtStreamType();
  
  ~BrtStreamType();
  
  Type* dup0() const;    // deep-copy
  
  Type* extend(Type *extension) {assert (0); return NULL;}
  
  void printBase( std::ostream& out, int level ) const {assert (0);}
  void printBefore( std::ostream& out, Symbol *name, int level) const {assert (0);}
  void printAfter( std::ostream& out ) const {assert (0);}
  
  void printType( std::ostream& out, Symbol *name,
		  bool showBase, int level ) const;

  void printForm(std::ostream& out) const;

  void printInitializer(std::ostream& out) const;

  void registerComponents() { }
  
  void findExpr( fnExprCallback cb );
  
  bool lookup( Symbol* sym ) const;

  TypeQual getQualifiers( void );
  BaseType *getBase( void );
 
  BaseType       *base;
  ExprVector     dims;
};

class CPUGatherType{public:
    const Type * at;
    const Type * subtype;
    bool copy_on_write;
    unsigned int dimension;
	CPUGatherType(const ArrayType &t,bool copy_on_write);
	Type * dup0()const;
	void printType(std::ostream & out, Symbol *name, bool showBase, int level) const;
	void printBefore(std::ostream & out, Symbol *name, int level) const;
	void printAfter(std::ostream &out)const;
	void printSubtype(std::ostream &out,Symbol *name, bool showBase,int level)const;
};



#endif  /* BRTDECL_H */

