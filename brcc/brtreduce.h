/*
 * brtreduce.h
 *
 *      Header for the various BRT Reduce classes (objects responsible for
 *      compiling and emitting reducers for the various backends).
 */
#ifndef    _BRTREDUCE_H_
#define    _BRTREDUCE_H_

#include "dup.h"
#include "stemnt.h"
#include "brtkernel.h"
#include "b2ctransform.h"

class BRTFP30ReduceCode : public BRTFP30KernelCode
{
  public:
    BRTFP30ReduceCode(const FunctionDef& _fDef) : BRTFP30KernelCode(_fDef) {};
   ~BRTFP30ReduceCode() { /* Nothing, ~BRTKernelCode() does all the work */ }

    BRTKernelCode *dup0() const { return new BRTFP30ReduceCode(*this->fDef); }
    void printCode(std::ostream& out) const;
};

class BRTFP40ReduceCode : public BRTFP40KernelCode
{
  public:
    BRTFP40ReduceCode(const FunctionDef& _fDef) : BRTFP40KernelCode(_fDef) {};
   ~BRTFP40ReduceCode() { /* Nothing, ~BRTKernelCode() does all the work */ }

    BRTKernelCode *dup0() const { return new BRTFP40ReduceCode(*this->fDef); }
    void printCode(std::ostream& out) const;
};

class BRTARBReduceCode : public BRTARBKernelCode
{
  public:
    BRTARBReduceCode(const FunctionDef& _fDef) : BRTARBKernelCode(_fDef) {};
   ~BRTARBReduceCode() { /* Nothing, ~BRTKernelCode() does all the work */ }

    BRTKernelCode *dup0() const { return new BRTARBReduceCode(*this->fDef); }
    void printCode(std::ostream& out) const;
};


class BRTPS20ReduceCode : public BRTPS20KernelCode
{
  public:
    BRTPS20ReduceCode(const FunctionDef& fDef);
   ~BRTPS20ReduceCode() { /* Nothing, ~BRTKernelCode() does all the work */ }

    BRTKernelCode *dup0() const { return new BRTPS20ReduceCode(*this->fDef); }
    void printCode(std::ostream& out) const;
};


class BRTCPUReduceCode : public BRTCPUKernelCode
{
  public:
    BRTCPUReduceCode(const FunctionDef& _fDef) : BRTCPUKernelCode(_fDef) {}
   ~BRTCPUReduceCode() { /* Nothing, ~BRTReduceCode() does all the work */ }

    BRTKernelCode *dup0() const { 
       return new BRTCPUReduceCode(*this->fDef); 
    }
    void printCode(std::ostream& out) const;
};

#endif  /* STEMNT_H */

