#ifndef BRTEXPRESS_H
#define BRTEXPRESS_H

#include "express.h"
#include "brtdecl.h"

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

#endif  /* BRTEXPRESS_H */
