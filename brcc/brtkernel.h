/*
 * brtkernel.h
 *
 *      Header for the various BRT Kernel classes (objects responsible for
 *      compiling and emitting kernels for the various backends).
 */
#ifndef    _BRTKERNEL_H_
#define    _BRTKERNEL_H_

#include "dup.h"
#include "stemnt.h"
#include "brook2cpp.h"

class BRTKernelCode;
typedef Dup<BRTKernelCode> DupableBRTKernelCode;

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
class BRTKernelCode : public DupableBRTKernelCode
{
  public:
    BRTKernelCode(const FunctionDef& _fDef) {
       fDef = (FunctionDef *) _fDef.dup0();
    };

    /*
     * Deleting fDef, even though it's dupped, appears to free memory that
     * has already been freed.  I'm suspicious that one of the dup methods
     * doesn't go deep enough, but haven't chased it down.  --Jeremy.
     */
    virtual ~BRTKernelCode() { delete fDef; };

    BRTKernelCode *dup0() const = 0;
    virtual void printCode(std::ostream& out) const = 0;

    friend std::ostream& operator<< (std::ostream& o, const BRTKernelCode& k);

    FunctionDef *fDef;
};


class BRTFP30KernelCode : public BRTKernelCode
{
  public:
    BRTFP30KernelCode(const FunctionDef& _fDef) : BRTKernelCode(_fDef) {};
   ~BRTFP30KernelCode() { /* Nothing, ~BRTKernelCode() does all the work */ }

    BRTKernelCode *dup0() const { return new BRTFP30KernelCode(*this->fDef); }
    void printCode(std::ostream& out) const;
};


class BRTPS20KernelCode : public BRTKernelCode
{
  public:
    BRTPS20KernelCode(const FunctionDef& fDef);
   ~BRTPS20KernelCode() { /* Nothing, ~BRTKernelCode() does all the work */ }

    BRTKernelCode *dup0() const { return new BRTPS20KernelCode(*this->fDef); }
    void printCode(std::ostream& out) const;

    /* static so it can be passed as a findExpr() callback */
    static Expression *ConvertGathers(Expression *e);
};


class BRTCPUKernelCode : public BRTKernelCode
{
  public:
    BRTCPUKernelCode(const FunctionDef& _fDef) : BRTKernelCode(_fDef) {
       Brook2Cpp_ConvertKernel(this->fDef);
    }
   ~BRTCPUKernelCode() { /* Nothing, ~BRTKernelCode() does all the work */ }

    BRTKernelCode *dup0() const { return new BRTCPUKernelCode(*this->fDef); }
    void printCode(std::ostream& out) const;
};

#endif  /* STEMNT_H */

