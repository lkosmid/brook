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

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
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
   bool shadowOutput;
public:
     bool isGather() {
       return recursiveIsGather(a->form);
    }
    bool isArrayType() {
      return recursiveIsArrayType(a->form);
    }
    PrintCPUArg(Decl * arg,unsigned int index, bool shadow)
       :a(arg),index(index){
       shadowOutput=shadow;
    }
    enum STAGE {HEADER,DEF,USE,CLEANUP};
  
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
    void printShadowArg(std::ostream&out,STAGE s) {
       Type * t = a->form;
       bool isStream = (t->type==TT_Stream);        
       switch(s) {
       case HEADER:
          printCPUVanilla(out,s);
          break;
       case DEF:
          if (isStream)
             t=static_cast<ArrayType*>(t)->subType;
          printType(out,t,false,"arg"+tostring(index));
          out << ";";
          break;
       case USE:
          out << "arg"<<index;
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
    void printCPUVanilla(std::ostream & out,STAGE s){
        if(isGather())
	  printDimensionlessGatherStream(out,s);
	else if (isArrayType())
	  printArrayStream(out,s);
        else
	  printNormalArg(out,s);
    }
   void printCPU(std::ostream & out, STAGE s) {
      Type * t = a->form;
      if (shadowOutput&&(t->getQualifiers()&TQ_Out)!=0) {
         printShadowArg(out,s);
      }else {
         printCPUVanilla(out,s);
      }
   }

};

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
std::string whiteout (std::string s) {
   for (unsigned int i=0;i<s.length();++i) {
      s[i]=' ';
   }
   return s;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void printInnerFunction (std::ostream & out,
                         std::string fullname,
                         FunctionDef *fDef, 
                         std::vector<PrintCPUArg>&myArgs,
                         bool shadowOutput,
                         std::string origname) {
   Type * form = fDef->decl->form;
    assert (form->isFunction());
    FunctionType* func = static_cast<FunctionType *>(form->dup());
    std::string myvoid("void  ");
    out << myvoid;//we don't want to automatically print this for it would say "kernel void" which means Nothing
    Symbol enhanced_name;
    enhanced_name.name = fullname;
    func->printBefore(out,&enhanced_name,0);
    out << " (";
    {for (int i=0;i<func->nArgs;++i) {
        myArgs.push_back(PrintCPUArg(func->args[i],i,shadowOutput));
    }}
    {
       unsigned int i;
       std::string long_name(whiteout(myvoid+enhanced_name.name+" ("));
       for (i=0;i<myArgs.size();++i) {
          if (i!=0)
             out << ","<<std::endl<<long_name;
          myArgs[i].printCPU(out,PrintCPUArg::HEADER);
       }
       fprintf (stderr, "Check %s %d\n",origname.c_str(),FunctionProp[origname].p);
       if (FunctionProp[origname].p&FP_INDEXOF) {
          if (i!=0)
             out << ","<<std::endl<<long_name;
          out << "const __BrtFloat4 &indexof";
       }
       if (FunctionProp[origname].p&FP_LINEARINDEXOF) {
          if (i!=0)
             out << ","<<std::endl<<long_name;
          out << "const __BrtInt1 &linearindexof";
          
       }
    }
    out << ")";    
    fDef->Block::print(out,0);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void BRTCPUKernelCode::printCombineCode(std::ostream &out) const{
    FunctionDef * fDef = static_cast<FunctionDef*>(this->fDef->dup());   
    Brook2Cpp_ConvertKernel(fDef);
    BrookCombine_ConvertKernel(fDef);

    std::vector <PrintCPUArg> myArgs;
    printInnerFunction (out,
                        "__"+fDef->decl->name->name+"_cpu_inner",
                        fDef,
                        myArgs,
                        true,
                        this->fDef->decl->name->name);    
    Symbol enhanced_name;
    enhanced_name.name = "__"+fDef->decl->name->name + "_cpu";
    out << "void  ";
    fDef->decl->form->printBefore(out,&enhanced_name,0);
    out << " (const std::vector<void *>&args, unsigned int mapbegin) {";
    out << std::endl;
    {for (unsigned int i=0;i<myArgs.size();++i) {
        indent(out,1);
        myArgs[i].printCPU(out,PrintCPUArg::DEF);
        out << std::endl;
    }}    
    indent(out,2);out<< "__" <<fDef->decl->name->name<<"_cpu_inner (";
    out << std::endl;
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
    out << "}"<<std::endl;   

}


// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void BRTCPUKernelCode::printCode(std::ostream& out) const
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
    bool reduceneeded=reduceNeeded(fDef);
    
    if (reduceneeded){
       FunctionDef * baseCase = static_cast<FunctionDef*>(this->fDef->dup());
       std::vector<PrintCPUArg>temp;
       Brook2Cpp_ConvertKernel(baseCase);
       BrookReduce_ConvertKernel(baseCase);
       printInnerFunction (out,
                           "__"+baseCase->decl->name->name+"_cpu_inner",
                           baseCase,
                           temp,
                           false,
                           this->fDef->decl->name->name);
       delete baseCase;
    }
   //we don't want to automatically print this for it would say "kernel void" which means Nothing
    Symbol enhanced_name;
    enhanced_name.name = "__"+fDef->decl->name->name + "_cpu";
    std::string myvoid("void  ");
    out << myvoid;
    fDef->decl->form->printBefore(out,&enhanced_name,0);
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

    if (reduceneeded) {
       indent(out,1); out << "if (mapbegin!=0&&mapbegin<mapend) {"<<std::endl;
       indent(out,2);out<< "__" <<fDef->decl->name->name<<"__base_cpu_inner (";
       out << std::endl;
       {for (unsigned int i=0;i<myArgs.size();++i) {
          if (i!=0)
             out <<","<<std::endl;
          indent(out,3);
          myArgs[i].printCPU(out,PrintCPUArg::USE);
       }}
       out << ");"<<std::endl;
       indent(out,2); out << "mapbegin+=1;"<<std::endl;;
       indent(out,1); out <<"}"<<std::endl;
    }
    indent(out,1);
    out << "for (unsigned int i=mapbegin";
    out <<";i<mapend;++i) {"<<std::endl;
    indent(out,2);out << "__" <<fDef->decl->name->name<<"_cpu_inner (";
    out<<std::endl;
    {for (unsigned int i=0;i<myArgs.size();++i) {
        if (i!=0)
            out <<","<<std::endl;
        indent(out,3);
        myArgs[i].printCPU(out,PrintCPUArg::USE);
    }}
    out<< ");"<<std::endl;
    indent(out,1);out <<"}"<<std::endl;
    {for (unsigned int i=0;i<myArgs.size();++i) {
        myArgs[i].printCPU(out,PrintCPUArg::CLEANUP);
    }}    
    out << "}"<<std::endl;   
    delete fDef;
    if (reduceneeded)
       printCombineCode(out);
}
