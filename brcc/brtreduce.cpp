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


// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
static std::string tostring(unsigned int i) {
    char c[1024];
    c[1023]=0;
    sprintf(c,"%d",i);
    return std::string(c);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
static Symbol getSymbol(std::string in) {
    Symbol name;
    name.name =in;
    return name;
}

extern bool recursiveIsGather(Type*);

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
extern std::string whiteout (std::string s);


// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void BRTCPUReduceCode::printCode(std::ostream& out) const
{
    bool copy_on_write=false;
    bool dims_specified=false;        
  /* We've already transformed everything, so just print ourselves */
 
    std::vector<PrintCPUArg> myArgs;
    FunctionDef * fDef = static_cast<FunctionDef*>(this->fDef->dup());
    
    Brook2Cpp_ConvertKernel(fDef);

    printInnerFunction (out,
                        "__"+fDef->decl->name->name+"_cpu_inner",
                        fDef,
                        myArgs,
                        false,
                        this->fDef->decl->name->name);    
   //we don't want to automatically print this for it would say "reduce void" which means Nothing
    Symbol enhanced_name;
    enhanced_name.name = "__"+fDef->decl->name->name + "_cpu";
    std::string myvoid("void  ");
    out << myvoid;
    fDef->decl->form->printBefore(out,&enhanced_name,0);
    out << " (";
    std::string long_name (whiteout(myvoid + enhanced_name.name+" ("));
    out << "const std::vector<void *>&args,"<<std::endl;
    out << long_name<<"unsigned int mapbegin, "<<std::endl;
    out << long_name<< "unsigned int mapend,"<<std::endl;
    out << long_name<< "unsigned int dim,"<<std::endl;
    out << long_name<< "const unsigned int *extents) {"<<std::endl;
    {for (unsigned int i=0;i<myArgs.size();++i) {
        indent(out,1);
        myArgs[i].printCPU(out,PrintCPUArg::DEF);
        out << std::endl;
    }}
    initializeIndexOf(out);
	indent(out,1); out << "unsigned int i=mapbegin;"<<std::endl;
	indent(out,1); out << "if (i<mapend) {"<<std::endl;
	//out = in
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
	indent(out,2);
	if (output&&input) {
		if (output->isArray()) {
			Symbol nothing;nothing.name="";
			out << "memcpy (*arg"<<outint<<", *arg"<<inint<<"++,sizeof(";
			output->form->printType(out,&nothing,true,0);
			out <<"));"<<std::endl;
		}else{
			out <<"*arg"<<outint<<" = *arg"<<inint<<"++;"<<std::endl;
		}
		indent(out,2);
		out <<"i++;"<<std::endl;		
	}
	indent(out,1); out <<"}"<<std::endl;
    indent(out,1);
    out << "for (;i<mapend;++i) {";
    out << std::endl;
    indent(out,2);out << "__" <<fDef->decl->name->name<<"_cpu_inner (";
    out<<std::endl;
    {for (unsigned int i=0;i<myArgs.size();++i) {
        if (i!=0)
            out <<","<<std::endl;
        indent(out,3);
        myArgs[i].printCPU(out,PrintCPUArg::USE);
    }}
    printIndexOfCallingArgs(out);
    out<< ");"<<std::endl;
    incrementIndexOf(out);
    indent(out,1);out <<"}"<<std::endl;
    {for (unsigned int i=0;i<myArgs.size();++i) {
        myArgs[i].printCPU(out,PrintCPUArg::CLEANUP);
    }}    
    out << "}"<<std::endl;   
    delete fDef;
}
