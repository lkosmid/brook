/*
 * brtkernel.cpp
 *
 *      Classes reflecting the body of Brook kernels for the different
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

#include "brtkernel.h"
#include "brtexpress.h"
#include "codegen.h"
#include "main.h"

unsigned int getReferenceStream(FunctionDef * fDef) {
   FunctionType * ft= static_cast<FunctionType*>(fDef->decl->form);
   unsigned int ret=0;
   bool found=false;
   for (unsigned int i=0;i<ft->nArgs;++i) {
      if (ft->args[i]->isStream()){
         found=true;
         ret=i;//consolation prize
         if ((ft->args[i]->form->getQualifiers()&TQ_Out)!=0) {
            return i;//jack pot
         }
      }
   }
   if (!found) {
      std::cerr << "Error ";
      fDef->location.printLocation(std::cerr);
      std::cerr << " No stream present in kernel or reduce."<<std::endl;
      return 0;
   }
   return ret;
}
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
bool BRTCPUKernelCode::PrintCPUArg::isGather() {
	return recursiveIsGather(a->form);
}
bool BRTCPUKernelCode::PrintCPUArg::isArrayType() {
      return recursiveIsArrayType(a->form);
}
bool BRTCPUKernelCode::PrintCPUArg::isStream() {
   return a->isStream();
}
void PrintAccessStream(std::ostream &out, 
                       unsigned int index,
                       std::string function, 
                       std::string permissions="") {
   if (function=="getExtents") {
      out << "extents["<<index<<"]";
   }else if(function=="getDimension") {
      out << "dims["<<index<<"]";
   }else if (function=="releaseData") {
      
   }else {
      out << "args["<<index<<"]";
   }
   return;
   out << "reinterpret_cast<brook::Stream*>"<<std::endl;
   indent(out,3);
   out << "(args["<<index<<"])->"<<function;
   out << "("<<permissions<<")";
   
}  
void BRTCPUKernelCode::PrintCPUArg::printDimensionlessGatherStream(std::ostream&out,STAGE s){
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
            PrintAccessStream(out,index,"getData","brook::Stream::READ");
	    out <<", ";
            PrintAccessStream(out,index,"getExtents");
            out<< ");";
            break;
        }
        case USE:
            out << "arg"<<index;
            break;
        case CLEANUP:
           break;
        default:
	  indent(out,1);
          PrintAccessStream(out,index,"releaseData","brook::Stream::READ");
	  out << ";"<<std::endl;
	  break;
        }
}

void BRTCPUKernelCode::PrintCPUArg::ResetNewLine(std::ostream&out,
                                                 bool nDcube,
                                                 unsigned int ref){
	if (a->isReduce())
		return;
	if (!a->isStream())
		return;
	bool isOut = (a->form->getQualifiers()&TQ_Out)!=0;
	if (!nDcube&&(reduceFunc||isOut))
           return;
	if (!nDcube) {
           indent(out,3);
		out << "iter"<<index<<"=getIndexOf(i+mapbegin,";
                out << "extents["<<index<<"],";
                out << "dim, extents["<<ref<<"]);";
		out << std::endl;           
        }else {
           indent(out,3);
		out << "arg"<<index<<" = getIndexOf(i, mapbegin, mapextent,";
                out << "extents["<<index<<"], ";
                out << "dim, extents["<<ref<<"]);";
		out << std::endl;           
        }   
}
void BRTCPUKernelCode::PrintCPUArg::Increment(std::ostream & out, 
                                              bool nDcube, 
                                              unsigned int ref) {
	if (a->isReduce())
		return;
	if (!a->isStream())
		return;
	bool isOut = (a->form->getQualifiers()&TQ_Out)!=0;
	if (!nDcube&&(reduceFunc||isOut)) {
           indent(out,2);
           out << "++arg"<<index<<";"<<std::endl;
	}else if (!nDcube) {
		indent(out,2);		
		out << "if (++ratioiter"<<index<<">=ratio"<<index<<"){";
                out << std::endl;
		indent(out,3);
		out << "ratioiter"<<index<<"=0;"<<std::endl;
		indent(out,3);
                out << "++iter"<<index<<";"<<std::endl;
                indent(out,2);
                out << "}"<<std::endl;
                /*
		out << "if (++iter"<<index<<"%extents["<<index<<"]";
                out << "[dim-1]==0) {";
                out << std::endl;
		indent(out,4);
		out << "iter"<<index<<"=getIndexOf(i+mapbegin,";
                out << "extents["<<index<<"],";
                out << "dim, extents["<<ref<<"]);";
		out << std::endl;
		indent(out,3);
		out << "}"<<std::endl;
		indent(out,2);
		out << "}"<<std::endl;
                */
	}else {
           indent(out,2);
           if(!isOut){
              out << "if (++ratioiter"<<index<<">=ratio"<<index<<"){"<<std::endl;
              indent(out,3);
              out << "ratioiter=0;"<<std::endl;
              indent(out,3);
           }
           out << "++iter"<<index<<";"<<std::endl;
           if (!isOut) {
              indent(out,2);
              out << "}"<<std::endl;			
           }
                /*
		indent(out,!isOut?2:3);
		out << "if (i%mapextent[dim-1]) {"<<std::endl;
		indent(out,!isOut?3:4);
		out << "arg"<<index<<" = getIndexOf(i, mapbegin, mapextent,";
                out << "extents["<<index<<"], ";
                out << "dim, extents["<<ref<<"]);";
		out << std::endl;
		indent(out,!isOut?3:4);
		out << "x"<<index<<" = arg"<<index<<" + delta_x"<<index<<";"<<std::endl;
		indent(out,!isOut?2:3);
		out << "}"<<std::endl;
                */
	}
}
void BRTCPUKernelCode::PrintCPUArg::InitialSet(std::ostream & out, 
                                               bool nDcube, 
                                               unsigned int ref) {
	if (a->isReduce())
		return;
	if (!a->isStream())
		return;
	if (!nDcube&&(reduceFunc||(a->form->getQualifiers()&TQ_Out)!=0)) {
           indent(out,1);
           out << "arg"<<index<<"+=mapbegin;"<<std::endl;
	}else {
           indent(out,1);
           out << "unsigned int ratio"<<index<<" = extents["<<ref<<"]";
           out << "[dim-1]";
           out << "/extents["<<index<<"][dim-1];"<<std::endl;
           indent(out,1);
           out << "unsigned int ratioiter"<<index<<" = 0;"<<std::endl;
           indent (out,1);
           out << "unsigned int iter"<<index<<" = getIndexOf(";
           if (!nDcube) 
              out << "mapbegin,";
           else
              out << "0, mapbegin, mapextent, ";
           out << "extents["<<index<<"], ";
           out << "dim, extents["<<ref<<"]);"<<std::endl;
	}
}
void BRTCPUKernelCode::PrintCPUArg::printArrayStream(std::ostream &out, 
                                                     STAGE s) {
        Type * t=a->form;
        bool isOut = (t->getQualifiers()&TQ_Out)!=0;
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
            if ((tq&TQ_Const)==0&&isOut==false&&(tq&TQ_Reduce)==0){
                out << "const ";//kernels are only allowed to touch out params
            }
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
            if (isStream){
               PrintAccessStream(out,index,"getData",isOut?
                                                     "brook::Stream::WRITE":
                                                     "brook::Stream::READ");


            }else {
               out << "args["<<index<<"];";
            }
            break;
        }
        case USE:{
			if (isStream&&(reduceFunc||isOut)) {
				out <<"*arg"<<index;
			}else if (!isStream) {
                           out << "*arg"<<index;
                        }else {
				out << "*(arg"<<index<<" + iter"<<index<<")";
			}
			break;
        }
	case CLEANUP:
           break;
        default:
          if (isStream) {
              indent(out,1);
              PrintAccessStream(out,index,"releaseData",isOut?
                                                        "brook::Stream::WRITE":
                                                        "brook::Stream::READ");
              out << ";"<<std::endl;
          }
	  break;
        }
}
void BRTCPUKernelCode::PrintCPUArg::printShadowArg(std::ostream&out,STAGE s) {
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
       case CLEANUP:
         break;
       }
}  
   //standard args, not gather or scatter
void BRTCPUKernelCode::PrintCPUArg::printNormalArg(std::ostream&out,STAGE s){
        Type * t = a->form;
        TypeQual tq= t->getQualifiers();
        bool isOut = (tq&TQ_Out)!=0;
        bool isStream = (t->type==TT_Stream);        
        switch(s) {
        case HEADER:{
            if ((tq&TQ_Const)==0&&isOut==false&&(tq&TQ_Reduce)==0){
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
            out << ")";
            if (isStream) {
               PrintAccessStream(out,index,"getData",isOut?
                                 "brook::Stream::WRITE":
                                 "brook::Stream::READ");
               out << ";";
            }else {
               out << "args["<<index<<"];";
            }
            break;
        case USE:
			if (isStream&&(reduceFunc||isOut)) {
				out <<"*arg"<<index;
			}else if (!isStream) {
                           out << "*arg"<<index;
                        }else{
                           out << "*(arg"<<index<<" + iter"<<index<<")";
			}
			break;
	case CLEANUP:
           break;
        default:
           if (isStream) {
              indent(out,1);
              PrintAccessStream(out,index,"releaseData",isOut?
                                                        "brook::Stream::WRITE":
                                                        "brook::Stream::READ");
              out << ";"<<std::endl;
           }
	  break;
        }
}
    
   //redirects call
void BRTCPUKernelCode::PrintCPUArg::printCPUVanilla(std::ostream & out,
                                                    STAGE s){
        if(isGather())
	  printDimensionlessGatherStream(out,s);
	else if (isArrayType())
	  printArrayStream(out,s);
        else
	  printNormalArg(out,s);
}
void BRTCPUKernelCode::PrintCPUArg::printCPU(std::ostream & out, 
                                             STAGE s) {
      Type * t = a->form;
      if (shadowOutput&&(t->getQualifiers()&TQ_Out)!=0) {
         printShadowArg(out,s);
      }else {
         printCPUVanilla(out,s);
      }
}



// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
std::string whiteout (std::string s) {
   for (unsigned int i=0;i<s.length();++i) {
      s[i]=' ';
   }
   return s;
}
std::vector<BRTCPUKernelCode::PrintCPUArg> 
BRTCPUKernelCode::getPrintableArgs (FunctionDef * fDef,bool shadowOutput) {
    Type * form = fDef->decl->form;
    assert (form->isFunction());
    FunctionType* func = static_cast<FunctionType *>(form->dup());
    std::vector<PrintCPUArg> myArgs;
    {for (int i=0;i<func->nArgs;++i) {
        myArgs.push_back(PrintCPUArg(func->args[i],i,shadowOutput,fDef->decl->isReduce()));
    }}  
    return myArgs;
}
// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void BRTCPUKernelCode::printInnerFunction (std::ostream & out,
                                           std::string fullname,
                                           FunctionDef *fDef, 
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
    std::vector<PrintCPUArg>myArgs=getPrintableArgs(fDef,shadowOutput);
    {
       unsigned int i;
       std::string long_name(whiteout(myvoid+enhanced_name.name+" ("));
       for (i=0;i<myArgs.size();++i) {
          if (i!=0)
             out << ","<<std::endl<<long_name;
          myArgs[i].printCPU(out,PrintCPUArg::HEADER);
       }
       //       fprintf (stderr, "Check %s %d\n",origname.c_str(),FunctionProp[origname].p);
       for (unsigned int j=0;j<myArgs.size();++j,++i) {
          if (FunctionProp[origname].contains(j)) {
             if (i!=0)
                out << ","<<std::endl<<long_name;
             out << "const __BrtFloat4 &__indexof_";
             out << myArgs[j].getDecl()->name->name;
          }
       }
    }
    out << ")";    
    fDef->Block::print(out,0);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void BRTCPUKernelCode::printCombineCode(std::ostream &out, bool print_inner) const{
    FunctionDef * fDef = static_cast<FunctionDef*>(this->fDef->dup());   
    Brook2Cpp_ConvertKernel(fDef);
    BrookCombine_ConvertKernel(fDef);

    std::vector <PrintCPUArg> myArgs = getPrintableArgs (fDef,true);
    if (print_inner) {
      printInnerFunction (out,
                          "__"+fDef->decl->name->name+"_cpu_inner",
                          fDef,
                          true,
                          this->fDef->decl->name->name);    
    }
    
    
    std::string enhanced_name= "__"+fDef->decl->name->name + "_cpu";
    std::string myvoid ("void  ");
    out << myvoid;
    out << enhanced_name;

    std::string whiteOut= whiteout(myvoid + enhanced_name+" (");
    out << " (const std::vector<void *>&args,"<<std::endl;
    out << whiteOut << "const std::vector<const unsigned int *>&extents,";
    out <<std::endl;
    out << whiteOut << "const std::vector<unsigned int>&dims,"<<std::endl;
    out << whiteOut << "unsigned int mapbegin) {"<<std::endl;
    indent(out,1);
    out << "unsigned int i= 0;"<<std::endl;
    initializeIndexOf(out);
    {for (unsigned int i=0;i<myArgs.size();++i) {
        indent(out,1);
        myArgs[i].printCPU(out,PrintCPUArg::DEF);
        out << std::endl;
    }}
    unsigned int reference_stream = getReferenceStream(this->fDef);
    indent(out,1);
    out << "unsigned int dim=dims["<<reference_stream<<"];"<<std::endl;
    {for (unsigned int i=0;i<myArgs.size();++i) {
          myArgs[i].InitialSet(out,false,reference_stream);
    }}
    indent(out,2);out<< "__" <<fDef->decl->name->name<<"_cpu_inner (";
    out << std::endl;
    {for (unsigned int i=0;i<myArgs.size();++i) {
        if (i!=0)
            out <<","<<std::endl;
        indent(out,3);
        myArgs[i].printCPU(out,PrintCPUArg::USE);
    }}
    printIndexOfCallingArgs(out);
    out<< ");"<<std::endl;
    {for (unsigned int i=0;i<myArgs.size();++i) {
        myArgs[i].printCPU(out,PrintCPUArg::CLEANUP);
    }}    
    out << "}"<<std::endl;   

}
void BRTCPUKernelCode::printIndexOfCallingArgs(std::ostream & out)const {
   std::string name = this->fDef->decl->name->name;
   functionProperties::iterator it=FunctionProp[name].begin();
   functionProperties::iterator end=FunctionProp[name].end();
   for (;it!=end;++it) {
      out << ","<<std::endl;
      indent(out,3);
      out << "indexof"<<*it;
   }
}

void BRTCPUKernelCode::initializeIndexOf(std::ostream&out)const {
   std::string name = this->fDef->decl->name->name;
   functionProperties::iterator it=FunctionProp[name].begin();
   functionProperties::iterator end=FunctionProp[name].end();
   for (;it!=end;++it) {
      indent(out,1);
      /*
      out << "unsigned int dim"<<*it<<"=";
      PrintAccessStream(out,*it,"getDimension");
      out << ";"<<std::endl<<"const unsigned int * extents"<<*it<<"=";
      PrintAccessStream(out,*it,"getExtents");
      */
      out << ";"<<std::endl<<"__BrtFloat4 indexof"<<*it;
      out << " = computeIndexOf(mapbegin, ";
      PrintAccessStream(out,*it,"getDimension");
      out << ", ";
      PrintAccessStream(out,*it,"getExtents");
      out << ");";
      out <<std::endl;
   }
}

void BRTCPUKernelCode::incrementIndexOf(std::ostream&out)const {
   std::string name = this->fDef->decl->name->name;
   functionProperties::iterator it=FunctionProp[name].begin();
   functionProperties::iterator end=FunctionProp[name].end();
   for (;it!=end;++it) {
      indent(out,2);
      out<<"incrementIndexOf (indexof"<<*it<<", ";
      PrintAccessStream(out,*it,"getDimension");
      out << ", ";
      PrintAccessStream(out,*it,"getExtents");
      out << ");";
      out<<std::endl;
   }
}
void BRTCPUKernelCode::incrementAllLocals(std::ostream&out,
                                          std::vector<PrintCPUArg> myArgs
                                          ) const{
       indent(out,2); out << "i++;"<<std::endl;
       incrementIndexOf(out);
       unsigned int reference_stream = getReferenceStream(this->fDef);
       {for (unsigned int i=0;i<myArgs.size();++i) {
          myArgs[i].Increment(out,false,reference_stream);
       }}
       indent(out,2);
       out << "if (i%newline==0) {"<<std::endl;
       {for (unsigned int i=0;i<myArgs.size();++i) {
          myArgs[i].ResetNewLine(out,false,reference_stream);
       }}          
       indent(out,2);
       out << "}";
}
void BRTCPUKernelCode::printTightLoop(std::ostream&out, 
                                      FunctionDef * fDef, 
                                      std::vector<PrintCPUArg> myArgs,
                                      bool reduceneeded)const{ 
    Symbol enhanced_name;
    enhanced_name.name = "__"+fDef->decl->name->name + "_cpu";
    std::string myvoid("void  ");
    out << myvoid;
    fDef->decl->form->printBefore(out,&enhanced_name,0);
    out << " (";
    std::string long_name (whiteout(myvoid + enhanced_name.name+" ("));
    out << "const std::vector<void *>&args,"<<std::endl;
    out << long_name << "const std::vector<const unsigned int *>&extents,";
    out <<std::endl;
    out << long_name << "const std::vector<unsigned int>&dims,"<<std::endl;
    out << long_name<<"unsigned int mapbegin, "<<std::endl;
    out << long_name<< "unsigned int mapend) {"<<std::endl;
    {for (unsigned int i=0;i<myArgs.size();++i) {
        indent(out,1);
        myArgs[i].printCPU(out,PrintCPUArg::DEF);
        out << std::endl;
    }}
    unsigned int reference_stream = getReferenceStream(this->fDef);
    indent(out,1);
    out << "unsigned int dim=dims["<<reference_stream<<"];"<<std::endl;
    indent(out,1);
    out << "unsigned int newline=extents["<<reference_stream<<"][dim-1];";
    out << std::endl;
    {for (unsigned int i=0;i<myArgs.size();++i) {
          myArgs[i].InitialSet(out,false,reference_stream);
    }}

    initializeIndexOf(out);
    indent(out,1); out << "unsigned int i=0; "<<std::endl;
    indent(out,1); out << "unsigned int iend=mapend-mapbegin;"<<std::endl;
    if (reduceneeded) {
       indent(out,1); out << "if (iend) {"<<std::endl;
       
       indent(out,2);out<< "__" <<fDef->decl->name->name<<"__base_cpu_inner (";
       out << std::endl;
       {for (unsigned int i=0;i<myArgs.size();++i) {
          if (i!=0)
             out <<","<<std::endl;
          indent(out,3);
          myArgs[i].printCPU(out,PrintCPUArg::USE);
       }}
       printIndexOfCallingArgs(out);
       out << ");"<<std::endl;
       incrementAllLocals(out,myArgs);
       indent(out,1); out <<"}"<<std::endl;
    }
    indent(out,1);
    out << "while (i<iend) {";
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
    incrementAllLocals(out,myArgs);
    indent(out,1);out <<"}"<<std::endl;
    {for (unsigned int i=0;i<myArgs.size();++i) {
        myArgs[i].printCPU(out,PrintCPUArg::CLEANUP);
    }}    
    out << "}"<<std::endl;     
}
// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void BRTCPUKernelCode::printCode(std::ostream& out) const
{
  /* We've already transformed everything, so just print ourselves */
 

    FunctionDef * fDef = static_cast<FunctionDef*>(this->fDef->dup());
    
    Brook2Cpp_ConvertKernel(fDef);
    std::vector<PrintCPUArg> myArgs=getPrintableArgs(fDef,false);
    printInnerFunction (out,
                        "__"+fDef->decl->name->name+"_cpu_inner",
                        fDef,
                        false,
                        this->fDef->decl->name->name);    
    bool reduceneeded=reduceNeeded(fDef);
    
    if (reduceneeded){
       FunctionDef * baseCase = static_cast<FunctionDef*>(this->fDef->dup());
       Brook2Cpp_ConvertKernel(baseCase);
       BrookReduce_ConvertKernel(baseCase);
       printInnerFunction (out,
                           "__"+baseCase->decl->name->name+"_cpu_inner",
                           baseCase,
                           false,
                           this->fDef->decl->name->name);
       delete baseCase;
    }
   //we don't want to automatically print this for it would say "kernel void" which means Nothing

    printTightLoop(out,fDef,myArgs,reduceneeded);
    delete fDef;
    if (reduceneeded)
       printCombineCode(out,true);
}
