/*
 * brtstemnt.h
 *
 *      Brook extensions to cTool's stemnt.h for kernel definitions.
 */
#ifndef    _BRTSCATTER_H_
#define    _BRTSCATTER_H_

#include "stemnt.h"
#include "b2ctransform.h"

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
class BRTScatterDef : public FunctionDef
{
  public:
    BRTScatterDef(const FunctionDef& fDef);
   ~BRTScatterDef();

    /* Pass ourselves (as a FunctionDef) to our own constructor */
    Statement *dup0() const { return new BRTScatterDef(*static_cast<const FunctionDef*>(this)); };
    void print(std::ostream& out, int level) const;
};

#endif  /* STEMNT_H */

