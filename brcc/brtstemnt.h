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

    virtual void printCode(std::ostream& out) const = 0;
    void printStub(std::ostream& out) const;
};


class BRTGPUKernelDef : public BRTKernelDef
{
  public:
    BRTGPUKernelDef(const FunctionDef& fDef) : BRTKernelDef(fDef) {};
   ~BRTGPUKernelDef() { /* Nothing, ~FunctionDef() does all the work */ }

    /* Just pass ourselves (as a FunctionDef descendant) to our constructor */
    Statement *dup0() const { return new BRTGPUKernelDef(*this); }

    void printCode(std::ostream& out) const;
};


class BRTCPUKernelDef;
extern void compileCpp(BRTCPUKernelDef *cpuDef);

class BRTCPUKernelDef : public BRTKernelDef
{
  public:
    BRTCPUKernelDef(const FunctionDef& fDef) : BRTKernelDef(fDef) {
       compileCpp(this);
    }
   ~BRTCPUKernelDef() { /* Nothing, ~FunctionDef() does all the work */ }

    /* Just pass ourselves (as a FunctionDef descendant) to our constructor */
    Statement *dup0() const { return new BRTCPUKernelDef(*this); }

    void printCode(std::ostream& out) const;
};

#endif  /* STEMNT_H */

