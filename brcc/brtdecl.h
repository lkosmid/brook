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
  
  BaseType       *base;
  ExprVector     dims;
};

#endif  /* BRTDECL_H */

