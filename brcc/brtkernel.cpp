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
    
    // Check to see if the expression is from a gather stream
    IndexExpr *p = (IndexExpr *) expr;
    for (p = (IndexExpr *) p->array; 
         p && p->etype == ET_IndexExpr;
         p = (IndexExpr *) p->array);
    
    // If things have gone horribly wrong
    if (!p) return expr;
    if (p->etype != ET_Variable) return expr;
    
    Variable *v = (Variable *) p;
    assert(v->name->entry);
    
    if (v->name->entry->type != ParamDeclEntry)
      return expr;
    // XXX Daniel: BrtGatherExpr asserts that it is
    //             indeed an array, not a TT_Stream
    if (v->name->entry->uVarDecl)
      if (v->name->entry->uVarDecl->form)
        if (v->name->entry->uVarDecl->form->type!=TT_Array)
          return expr;
    gather = new BrtGatherExpr((IndexExpr *) expr);
    
    // IAB: XXX For some reason I can't delete expr!!!
    //delete expr; 
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

extern bool recursiveIsGather(Type*);
bool recursiveIsArrayType(Type * form) {
   if ((form->getQualifiers()&TQ_Reduce)!=0) {
      return form->type==TT_Array;
   }
   return form->type==TT_Stream
      &&(static_cast<ArrayType*>(form)->subType->type==TT_Array);
}


// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
class PrintCPUArg {
    Decl * a;
    unsigned int index;
public:
     bool isGather() {
       return recursiveIsGather(a->form);
    }
    bool isArrayType() {
      return recursiveIsArrayType(a->form);
    }
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
#if 0
    bool isDimensionless() {
        if (a->form->type==TT_Array) {
            return isDimensionlessHelper(static_cast<ArrayType*>
                                         (a->form)->subType);
        }
        return false;
    }
#endif
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
            indent(out,3);
            out << "(";
            Symbol nothing;
            nothing.name="";
            cgt.printSubtype(out,&nothing,true,0);
            out<<"*)";
            out <<"reinterpret_cast<brook::Stream*>"<<std::endl;
	    indent(out,3);
            out << "(args["<<index<<"])->getData(brook::Stream::READ), ";
	    out <<std::endl;
            indent(out,3);
            out<<"reinterpret_cast<brook::Stream*>"<<std::endl;
	    indent(out,3);
            out<< "(args["<<index<<"])->getExtents());";
            break;
        }
        case USE:
            out << "arg"<<index;
            break;
        case CLEANUP:
	  indent(out,1);
	  out << "reinterpret_cast<brook::Stream*>"<<std::endl;
	  indent(out,3);
	  out << "(args["<<index<<"])->releaseData(brook::Stream::READ);";
	  out << std::endl;
	  break;
        }
    }

   //The following function is obsolete for now since static sized
   //arrays are no longer helpful
    void printArrayStream(std::ostream &out, STAGE s) {
        Type * t=a->form;
	//temporarily dissect type.
        assert (t->type==TT_Stream||t->type==TT_Array);
        bool isStream=false;
        if (t->type==TT_Stream) {
           t=static_cast<ArrayType*>(t)->subType;
           isStream=true;
        }
        switch (s) {
        case HEADER:{
            TypeQual tq= t->getQualifiers();            
            if ((tq&TQ_Const)==0&&(tq&TQ_Out)==0&&(tq&TQ_Reduce)==0){
                out << "const ";//kernels are only allowed to touch out params
            }
			Type * tmp = a->form;
			t->printType(out,a->name,true,0);
 
			
            break;
        }
        case DEF:{
            Symbol s;
            if (t->type==TT_Base)
                s=getSymbol(std::string("*arg")+tostring(index));
            else
                s=getSymbol(std::string("(*arg")+tostring(index)+")");
            t->printType(out,&s,true,0);
            out << " = (";
            s=(t->type==TT_Base)?getSymbol("*"):getSymbol("(*)");
            t->printType(out,&s,true,0);
            out << ")";
			out<<"args["<<index<<"];";
            break;
        }
        case USE:{
            out <<"*arg"<<index;
            if (isStream)
               out <<"++";
            break;
        }
	case CLEANUP:
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
            if ((tq&TQ_Const)==0&&(tq&TQ_Out)==0&&(tq&TQ_Reduce)==0){
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
    void printCPU(std::ostream & out,STAGE s){
        if(isGather())
	  printDimensionlessGatherStream(out,s);
	else if (isArrayType())
	  printArrayStream(out,s);
        else
	  printNormalArg(out,s);
    }

};



std::string whiteout (std::string s) {
   for (unsigned int i=0;i<s.length();++i) {
      s[i]=' ';
   }
   return s;
}
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
    std::string myvoid("void  ");
    out << myvoid;//we don't want to automatically print this for it would say "kernel void" which means Nothing
    Symbol enhanced_name;
    enhanced_name.name = "__"+fDef->decl->name->name + "_cpu_inner";
    func->printBefore(out,&enhanced_name,0);
    out << " (";
    std::vector<PrintCPUArg> myArgs;
    {for (int i=0;i<func->nArgs;++i) {
        myArgs.push_back(PrintCPUArg(func->args[i],i));
    }}
    {
       std::string long_name(whiteout(myvoid+enhanced_name.name+" ("));
       for (unsigned int i=0;i<myArgs.size();++i) {
          if (i!=0)
             out << ","<<std::endl<<long_name;
          myArgs[i].printCPU(out,PrintCPUArg::HEADER);
    }}
    out << ")";    
    fDef->Block::print(out,0);
    //we don't want to automatically print this for it would say "kernel void" which means Nothing
    enhanced_name.name = "__"+fDef->decl->name->name + "_cpu";
    out << myvoid;
    func->printBefore(out,&enhanced_name,0);
    out << " (";
    std::string long_name (whiteout(myvoid + enhanced_name.name+" ("));
    out << "const std::vector<void *>&args,"<<std::endl;
    out << long_name<<"unsigned int mapbegin, "<<std::endl;
    out << long_name<< "unsigned int mapend) {"<<std::endl;
    {for (unsigned int i=0;i<myArgs.size();++i) {
        indent(out,1);
        myArgs[i].printCPU(out,PrintCPUArg::DEF);
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
        myArgs[i].printCPU(out,PrintCPUArg::USE);
    }}
    out<< ");"<<std::endl;
    {for (unsigned int i=0;i<myArgs.size();++i) {
        myArgs[i].printCPU(out,PrintCPUArg::CLEANUP);
    }}    
    indent(out,1);out <<"}"<<std::endl;
    out << "}"<<std::endl;   
}
