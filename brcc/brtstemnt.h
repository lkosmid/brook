/*
 * brtstemnt.h
 *
 *      Brook extensions to cTool's stemnt.h for kernel definitions.
 */
#ifndef    _BRTSTEMNT_H_
#define    _BRTSTEMNT_H_

#include "stemnt.h"

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
class BRTKernelDef : public FunctionDef
{
  public:
    BRTKernelDef(const FunctionDef& fDef);
   ~BRTKernelDef();

    Statement *dup0() const;
    void print(std::ostream& out, int level) const;
};

#endif  /* STEMNT_H */

