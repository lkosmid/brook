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
#include "b2ctransform.h"
#include "codegen.h"

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
    virtual void printInnerCode(std::ostream&out)const=0;
    friend std::ostream& operator<< (std::ostream& o, const BRTKernelCode& k);
    FunctionDef *fDef;
    bool standAloneKernel()const;
    virtual void onlyPrintInner(std::ostream &out)const{}
};


class BRTGPUKernelCode : public BRTKernelCode
{
public:
   BRTGPUKernelCode(const FunctionDef& fDef);
   ~BRTGPUKernelCode() { /* Nothing, ~BRTKernelCode() does all the work */ }
   
   void printInnerCode(std::ostream&out)const;
   /* static so it can be passed as a findExpr() callback */
   static Expression *ConvertGathers(Expression *e);
   
   void printCodeForType(std::ostream& out, 
                         CodeGenTarget target) const;
   
   virtual BRTKernelCode *dup0() const = 0;
   virtual void printCode(std::ostream& out) const = 0;
};

class BRTFP30KernelCode : public BRTGPUKernelCode
{
  public:
    BRTFP30KernelCode(const FunctionDef& fDef) : BRTGPUKernelCode(fDef) {}
   ~BRTFP30KernelCode() { /* Nothing, ~BRTKernelCode() does all the work */ }

    BRTKernelCode *dup0() const { return new BRTFP30KernelCode(*this->fDef); }
    void printCode(std::ostream& out) const;
};

class BRTARBKernelCode : public BRTGPUKernelCode
{
  public:
    BRTARBKernelCode(const FunctionDef& fDef) : BRTGPUKernelCode(fDef) {}
   ~BRTARBKernelCode() { /* Nothing, ~BRTKernelCode() does all the work */ }

    BRTKernelCode *dup0() const { return new BRTARBKernelCode(*this->fDef); }
    void printCode(std::ostream& out) const;
};

class BRTPS20KernelCode : public BRTGPUKernelCode
{
  public:
   BRTPS20KernelCode(const FunctionDef& fDef) : BRTGPUKernelCode(fDef) {}
   ~BRTPS20KernelCode() { /* Nothing, ~BRTKernelCode() does all the work */ }
   
   BRTKernelCode *dup0() const { return new BRTPS20KernelCode(*this->fDef); }
   void printCode(std::ostream& out) const;
};


class BRTCPUKernelCode : public BRTKernelCode
{
  public:
	class PrintCPUArg {
		Decl * a;
		unsigned int index;
		bool shadowOutput;
		bool reduceFunc;
	public:
		Decl * getDecl(){return a;}
		bool isGather();
		bool isArrayType();
                bool isStream();
		PrintCPUArg(Decl * arg,unsigned int index, bool shadow, bool reduceFunc)
				:a(arg),index(index){
			this->reduceFunc=reduceFunc;
			shadowOutput=shadow;
		}
		enum STAGE {HEADER,DEF,CLEANUP};
		bool useShadowOutput()const ;
                void Increment(std::ostream&out, bool nDcube, unsigned int ref,std::string fname);
                void Use (std::ostream&out, bool nDcube, unsigned int ref);
                void ResetNewLine(std::ostream&out,bool nDcube,unsigned int ref,std::string fname);
		void InitialSet(std::ostream&out, bool nDcube, unsigned int ref);
		void printDimensionlessGatherStream(std::ostream&out,STAGE s);
		void printArrayStream(std::ostream &out, STAGE s);
		void printShadowArg(std::ostream&out,STAGE s);
		void printNormalArg(std::ostream&out,STAGE s);
		void printCPUVanilla(std::ostream & out,STAGE s);
		void printCPU(std::ostream & out, STAGE s);
	};
	
    BRTCPUKernelCode(const FunctionDef& _fDef) : BRTKernelCode(_fDef) {}
   ~BRTCPUKernelCode() { /* Nothing, ~BRTKernelCode() does all the work */ }

    BRTKernelCode *dup0() const { 
       return new BRTCPUKernelCode(*this->fDef); 
    }
    static std::vector<PrintCPUArg>getPrintableArgs(FunctionDef * fDef,
                                                    bool shadowOutput);
    void incrementIndexOf(std::ostream &out)const;
    void incrementAllLocals(std::ostream &out,
                            bool nDcube,
                            std::vector<PrintCPUArg>)const;
   void initializeIndexOf(std::ostream &out,
                          unsigned int ref,
                          bool nDcube)const;
    void printIndexOfCallingArgs(std::ostream & out)const;
    void printCombineCode(std::ostream& out)const;
    void printCombineInnerLoop(std::ostream & out) const;
    void printTightLoop(std::ostream&out, 
                        FunctionDef * fDef, 
                        std::vector<PrintCPUArg> args,
                        bool reduceneeded)const;
    void printNdTightLoop(std::ostream&out, 
                          FunctionDef * fDef, 
                          std::vector<PrintCPUArg> args,
                          bool reduceneeded)const;
    void printCode(std::ostream& out) const;
    void printInnerCode(std::ostream&out)const;
    static void printInnerFunction(std::ostream&out,
                                   std::string fullname,
                                   FunctionDef*fDef,
                                   bool shadowOutput,
                                   std::string origname);
   void onlyPrintInner(std::ostream &out)const;
};

#endif  /* STEMNT_H */

