/*
 * brtstemnt.h
 *
 *      Brook extensions to cTool's stemnt.h for kernel definitions.
 */
#ifndef    _BRTSTEMNT_H_
#define    _BRTSTEMNT_H_

#include "stemnt.h"
#include "brook2cpp.h"

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
class BRTKernelDef : public FunctionDef
{
  public:
    BRTKernelDef(const FunctionDef& fDef);
   ~BRTKernelDef();

    /* Pass ourselves (as a FunctionDef) to our own constructor */
    Statement *dup0() const { return new BRTKernelDef(*this); };
    void print(std::ostream& out, int level) const;

    void printStub(std::ostream& out) const;

    bool CheckSemantics(void) const;
};

#endif  /* STEMNT_H */

