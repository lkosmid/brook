/*
 * brtkernel.cpp
 *
 *      Classes reflecting the body of Brook kernels for the different
 *      backends.  Each one knows how to build itself from a function
 *      definition and then how to emit C++ for itself.
 */
#include <cstring>
#include <cassert>
#include <sstream>

#include "brtkernel.h"
#include "brtexpress.h"
#include "codegen.h"
#include "main.h"


// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
std::ostream&
operator<< (std::ostream& o, const BRTKernelCode& k) {
   k.printCode(o);
   return o;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
BRTFP30KernelCode::printCode(std::ostream& out) const
{
   FunctionType *fType;
   std::ostringstream wrapOut;
   char *fpcode;

   fDef->Block::print(wrapOut, 0);
   if (globals.verbose) {
      std::cerr << "***Wrapping\n";
      fDef->decl->print(std::cerr, true);
      std::cerr << std::endl << wrapOut.str() << "\n***\n";
   }

   assert (fDef->decl->form->type == TT_Function);
   fType = (FunctionType *) fDef->decl->form;

   fpcode = CodeGen_FP30GenerateCode(fType->subType,
                                     fDef->FunctionName()->name.c_str(),
                                     fType->args, fType->nArgs,
                                     wrapOut.str().c_str());
   out << fpcode;
   free(fpcode);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
BRTPS20KernelCode::BRTPS20KernelCode(const FunctionDef& _fDef)
   : BRTKernelCode(_fDef)
{
   fDef->findExpr(ConvertGathers);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
Expression *
BRTPS20KernelCode::ConvertGathers (Expression *expr) {
  BrtGatherExpr *gather;
  if (expr->etype == ET_IndexExpr) {

     if (globals.verbose) {
        std::cerr << "Found Index Expr: " << expr << std::endl;
     }

    gather = new BrtGatherExpr((IndexExpr *) expr);

    //delete expr;  // IAB: XXX For some reason I can't delete expr!!!
    return gather;
  }
  return expr;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
BRTPS20KernelCode::printCode(std::ostream& out) const
{
   FunctionType *fType;
   std::ostringstream wrapOut;
   char *fpcode;

   fDef->Block::print(wrapOut, 0);
   if (globals.verbose) {
      std::cerr << "***Wrapping\n";
      fDef->decl->print(out, true);
      std::cerr << std::endl << wrapOut.str() << "\n***\n";
   }

   assert (fDef->decl->form->type == TT_Function);
   fType = (FunctionType *) fDef->decl->form;

   fpcode = CodeGen_PS20GenerateCode(fType->subType,
                                     fDef->FunctionName()->name.c_str(),
                                     fType->args, fType->nArgs,
                                     wrapOut.str().c_str());
   out << fpcode;
   free(fpcode);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void printType (std::ostream & out, 
                Type * t, 
                bool addIndirection, 
                std::string name ="") {
    Symbol sym;
    sym.name=name;
    if (addIndirection)
	sym.name=std::string("*")+sym.name;
    t->printBase(out,0);
    t->printBefore(out,&sym,0);
    t->printAfter(out);
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

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
class PrintCPUArg {
    Decl * a;
    unsigned int index;
public:

    PrintCPUArg(Decl * arg,unsigned int index):a(arg),index(index){}
    enum STAGE {HEADER,DEF,USE,CLEANUP};
   
   //this function determines if the size actually is specified for this dim
    bool isDimensionlessHelper(Type * t) {
        if (t->type==TT_Array) {
            ArrayType* a = static_cast<ArrayType*>(t);
            Expression * size=  a->size;
	    if (size==NULL)
		return true;
            if (size->etype==ET_VoidExpr)
                return true;
            return isDimensionlessHelper (a->subType);
        }
        return false; 
    }
   // this function figures that all arrays have a chance to have all dims specified
    bool isDimensionless() {
        if (a->form->type==TT_Array) {
            return isDimensionlessHelper(static_cast<ArrayType*>
                                         (a->form)->subType);
        }
        return false;
    }

    bool isArray() {
        return (a->form->type==TT_Array);
    }

    void printDimensionlessGatherStream(std::ostream&out,STAGE s){
        ArrayType * t = static_cast<ArrayType*>(a->form);
        switch (s) {
        case HEADER:{
            Symbol name=getSymbol("&"+a->name->name);
            out << "const ";
            CPUGatherType(*t,false).printType(out,
                                              &name,
                                              true,
                                              false);
            break;
        }
        case DEF:{
            CPUGatherType cgt (*t,false);        
            Symbol arg1;arg1.name="arg"+tostring(index);
            
            cgt.printType(out,&arg1,false,0);
            out << "("<<std::endl;
            indent(out,2);
            out << "(";
            Symbol nothing;
            nothing.name="";
            cgt.printSubtype(out,&nothing,true,0);
            out<<"*)";
            out <<"reinterpret_cast<brook::Stream*>";
            out << "(args["<<index<<"])->getData(brook::Stream::READ), ";
	    out <<std::endl;
            indent(out,2);
            out<<"reinterpret_cast<brook::Stream*>";
            out<< "(args["<<index<<"])->getExtents());";
            break;
        }
        case USE:
            out << "arg"<<index;
            break;
        case CLEANUP:
	  indent(out,1);
	  out << "reinterpret_cast<brook::Stream*>";
	  out << "(args["<<index<<"])->releaseData(brook::Stream::READ);";
	  out << std::endl;
	  break;
        }
    }

   //The following function is obsolete for now since static sized
   //arrays are no longer helpful
    void printDimensionalGatherStream(std::ostream &out, STAGE s) {
	printDimensionlessGatherStream(out,s);
	return;
	//past this point is dead code.
        Type * t=a->form;//obsolete! not allowed to index with float4
        switch (s) {
        case HEADER:{//obsolete! not allowed to index with float4
            TypeQual tq= t->getQualifiers();            
            if ((tq&TQ_Const)==0&&(tq&TQ_Out)==0){
                out << "const ";//kernels are only allowed to touch out params
            }
            a->print(out,0);
            break;
        }
        case DEF:{//obsolete! not allowed to index with float4
            t=static_cast<ArrayType*>(t)->subType;
            Symbol s;
            if (t->type==TT_Base)
                s=getSymbol(std::string("*arg")+tostring(index));
            else
                s=getSymbol(std::string("(*arg")+tostring(index)+")");
            t->printType(out,&s,true,0);
            out << " = (";
            s=(t->type==TT_Base)?getSymbol("*"):getSymbol("(*)");
            t->printType(out,&s,true,0);
            out << ")"<<std::endl;
            indent(out,2);
            out <<"reinterpret_cast<brook::Stream *>(args["<<index<<"])";
	    indent(out,3);
	    out << "->getData(brook::Stream::READ);";
            break;
        }
        case USE:{//obsolete! not allowed to index with float4
            out <<"arg"<<index;
            break;
        }
	case CLEANUP:
	  indent(out,1);
	  out << "reinterpret_cast<brook::Stream*>";
	  out << "(args["<<index<<"])->releaseData(brook::Stream::READ);";
	  out << std::endl;
	  break;
        }
    }

   //standard args, not gather or scatter
    void printNormalArg(std::ostream&out,STAGE s){
        Type * t = a->form;
        TypeQual tq= t->getQualifiers();
        bool isStream = (t->type==TT_Stream);        
        switch(s) {
        case HEADER:{
            if ((tq&TQ_Const)==0&&(tq&TQ_Out)==0){
                out << "const ";//kernels are only allowed to touch out params
            }
            Symbol name=getSymbol(a->name->name);
            name=getSymbol("&"+a->name->name);            
            if (isStream)
                t = static_cast<ArrayType*>(t)->subType;
            t->printType(out,&name,true,0);
            break;
        }
        case DEF:
            if (isStream) {
                t=static_cast<ArrayType*>(t)->subType;
            }
            printType(out,t,true,"arg"+tostring(index));
            out << " = (";
            printType(out,t,true);
            out << ")args["<<index<<"];";
            if (isStream)
                    out<<" arg"<<index<<"+=mapbegin;";
            break;
        case USE:
                out <<"*arg"<<index;
                if (isStream)
                    out <<"++";
            break;
	case CLEANUP:
	  break;
        }
    }
    
   //redirects call
    void printCPUFunctionArg(std::ostream & out){
        if(isArray())
            if (isDimensionless())
                printDimensionlessGatherStream(out,HEADER);
            else
                printDimensionalGatherStream(out,HEADER);
        else
            printNormalArg(out,HEADER);
    }

   //redirects call
    void printInternalDef(std::ostream &out){
        if(isArray())
            if (isDimensionless())
                printDimensionlessGatherStream(out,DEF);
            else
                printDimensionalGatherStream(out,DEF);
        else
            printNormalArg(out,DEF);
    }

   //redirects call
    void printInternalUse(std::ostream &out){
        if(isArray())
            if (isDimensionless())
                printDimensionlessGatherStream(out,USE);
            else
                printDimensionalGatherStream(out,USE);
        else
            printNormalArg(out,USE);
    }
    void printInternalCleanup(std::ostream &out){
        if(isArray())
            if (isDimensionless())
                printDimensionlessGatherStream(out,CLEANUP);
            else
                printDimensionalGatherStream(out,CLEANUP);
        else
            printNormalArg(out,CLEANUP);
    }
};




// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
BRTCPUKernelCode::printCode(std::ostream& out) const
{
    bool copy_on_write=false;
    bool dims_specified=false;        
  /* We've already transformed everything, so just print ourselves */
  Type * form = fDef->decl->form;
  assert (form->isFunction());
  FunctionType* func = static_cast<FunctionType *>(form->dup());
    
    out << "void ";//we don't want to automatically print this for it would say "kernel void" which means Nothing
    Symbol enhanced_name;
    enhanced_name.name = "__"+fDef->decl->name->name + "_cpu_inner";
    func->printBefore(out,&enhanced_name,0);
    out << "(";
    std::vector<PrintCPUArg> myArgs;
    {for (int i=0;i<func->nArgs;++i) {
        myArgs.push_back(PrintCPUArg(func->args[i],i));
    }}
    {for (unsigned int i=0;i<myArgs.size();++i) {
        if (i!=0)
            out << ", ";
        myArgs[i].printCPUFunctionArg(out);
    }}
    out << ")";    
    fDef->Block::print(out,0);
    out << "void ";//we don't want to automatically print this for it would say "kernel void" which means Nothing
    enhanced_name.name = "__"+fDef->decl->name->name + "_cpu";
    func->printBefore(out,&enhanced_name,0);
    out << "(const std::vector<void *>&args, ";
    out <<  "unsigned int mapbegin, ";
    out <<  "unsigned int mapend) {"<<std::endl;
    {for (unsigned int i=0;i<myArgs.size();++i) {
        indent(out,1);
        myArgs[i].printInternalDef(out);
        out << std::endl;
    }}
    indent(out,1);
    out << "for (unsigned int i=mapbegin;i<mapend;++i) {"<<std::endl;
    indent(out,2);out << "__" <<fDef->decl->name->name<<"_cpu_inner (";
    out<<std::endl;
    {for (unsigned int i=0;i<myArgs.size();++i) {
        if (i!=0)
            out <<","<<std::endl;
        indent(out,3);
        myArgs[i].printInternalUse(out);
    }}
    out<< ");"<<std::endl;
    {for (unsigned int i=0;i<myArgs.size();++i) {
        myArgs[i].printInternalCleanup(out);
    }}    
    indent(out,1);out <<"}"<<std::endl;
    out << "}"<<std::endl;   
}
