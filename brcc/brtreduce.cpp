/*
 * brtreduce.cpp
 *
 *      Classes reflecting the body of Brook reduces for the different
 *      backends.  Each one knows how to build itself from a function
 *      definition and then how to emit C++ for itself.
 */
#ifdef _WIN32
#pragma warning(disable:4786)
//the above warning disables visual studio's annoying habit of warning when using the standard set lib
#endif

#include <cstring>
#include <cassert>
#include <sstream>

#include "brtreduce.h"
#include "brtexpress.h"
#include "codegen.h"
#include "main.h"


// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
BRTFP30ReduceCode::printCode(std::ostream& out) const
{
	this->BRTFP30KernelCode::printCode(out);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
BRTPS20ReduceCode::BRTPS20ReduceCode(const FunctionDef& _fDef)
		: BRTPS20KernelCode(_fDef)//converts gathers
{
}


void
BRTPS20ReduceCode::printCode(std::ostream& out) const
{
	this->BRTPS20KernelCode::printCode(out);
}




extern bool recursiveIsGather(Type*);

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
extern std::string whiteout (std::string s);


// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void BRTCPUReduceCode::printCode(std::ostream& out) const
{
  /* We've already transformed everything, so just print ourselves */
 
    FunctionDef * fDef = static_cast<FunctionDef*>(this->fDef->dup());
    
    Brook2Cpp_ConvertKernel(fDef);
    std::vector<PrintCPUArg> myArgs=getPrintableArgs(fDef,false);

    ////// Print Normal Inner Function ///////
    printInnerFunction (out,
                        "__"+fDef->decl->name->name+"_cpu_inner",
                        fDef,
                        false,
                        this->fDef->decl->name->name);    
    Symbol enhanced_name;
    std::string myvoid("void  ");
    ////// Print Base Case Inner Function ///////    
    enhanced_name.name = "__"+fDef->decl->name->name+"__base_cpu_inner";
    out << myvoid<<enhanced_name.name<<" (";
    {       
       std::string long_name(whiteout(myvoid+enhanced_name.name+" ("));
       for (unsigned int i=0;i<myArgs.size();++i) {
          if (i!=0)
             out << ","<<std::endl<<long_name;
          myArgs[i].printCPU(out,PrintCPUArg::HEADER);
       }
    }
    out << ") {";
    Decl * output=NULL;Decl *input=NULL;
    unsigned int outint=0,inint=0;
    out << std::endl;
    {for (unsigned int i=0;i<myArgs.size();++i) {
       if (myArgs[i].getDecl()->isReduce()){
          outint=i;output=myArgs[i].getDecl();
       }else if (myArgs[i].getDecl()->isStream() &&
                 (myArgs[i].getDecl()->form->getQualifiers()&TQ_Out)==0) {
          inint=i;input=myArgs[i].getDecl();
       }
    }}
    indent(out,1);
    if (output&&input) {
       if (output->isArray()) {
          Symbol nothing;nothing.name="";
          out << "memcpy ("<<output->name->name<<", ";
          out << input->name->name<<",sizeof(";
          output->form->printType(out,&nothing,true,0);
          out <<"));"<<std::endl;
       }else{
          out <<output->name->name<<" = ";
          out <<input->name->name<<";"<<std::endl;
       }
    }
    out << "}" << std::endl;
    ////// Print Combiner Inner Function ///////
    Symbol combinereduction;combinereduction.name="__combiner";
    if (output&&input) {

       enhanced_name.name = "__"+fDef->decl->name->name+"__combine_cpu_inner";
       out << myvoid<<enhanced_name.name<<" (";

       {       
          std::string long_name(whiteout(myvoid+enhanced_name.name+" ("));
          unsigned int i=0;
          for (;i<myArgs.size();++i) {
             if (i!=0)
                out << ","<<std::endl<<long_name;
             myArgs[i].printCPU(out,PrintCPUArg::HEADER);
          }
          if (i!=0)
             out << ","<<std::endl<<long_name;
          Symbol * tmp = output->name;
          output->name= &combinereduction;
          myArgs[outint].printCPU(out,PrintCPUArg::HEADER);
          output->name=tmp;          
       }
       out << ") {"<<std::endl;
       indent(out,1);
       out << "__"<<fDef->decl->name->name <<"_cpu_inner (";
       for (unsigned int i=0;i<myArgs.size();++i) {
          if (i!=0) out << ", ";
          if (i!=inint) {
             out << myArgs[i].getDecl()->name->name;
          }else {
             out << combinereduction.name;
          }
       }
       out << ");"<<std::endl;
       out << "}"<<std::endl;           
    }
    printTightLoop(out,fDef,myArgs,true);
    printNdTightLoop(out,fDef,myArgs,true);
    printCombineCode(out);
}
