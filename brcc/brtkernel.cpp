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
#include <stdio.h>
//someone removed the above header...it is needed for linux
#include <cstring>
#include <cassert>
#include <sstream>

#include "brtkernel.h"
#include "brtexpress.h"
#include "codegen.h"
#include "main.h"

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
std::string whiteout (std::string s) {
   for (unsigned int i=0;i<s.length();++i) {
      s[i]=' ';
   }
   return s;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
// checks if a function is a gather stream
extern bool recursiveIsGather(Type*);

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
// This function finds out which argument to the function def is the
// one that will decide how other streams stretch. For maps that's the out
// for reductions that's the input.
unsigned int getReferenceStream(FunctionDef * fDef) {
   FunctionType * ft= static_cast<FunctionType*>(fDef->decl->form);
   unsigned int ret=0;
   bool found=false;
   for (int i=0;i<ft->nArgs;++i) {
      if (ft->args[i]->isReduce())
         continue;
      if (ft->args[i]->isStream()){
         found=true;
         ret=i;//consolation prize (reduction? maybe)
         if ((ft->args[i]->form->getQualifiers()&TQ_Out)!=0) {
            return i;//jack pot (map!)
         }
      }
   }
   if (!found) {
      std::cerr << fDef->location << "Error: ";
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
BRTGPUKernelCode::BRTGPUKernelCode(const FunctionDef& _fDef)
   : BRTKernelCode(_fDef)
{
   fDef->findExpr(ConvertGathers);
}

static Variable * NewGatherArg (Variable * v) {
   Symbol * s = new Symbol;
   s->name = v->name->name+"_scalebias";
   return new Variable(s,v->location);
   
}


// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
// This function prints the code of an internally callable kernel
// from within another kernel.
void BRTGPUKernelCode::printInnerCode (std::ostream&out) const {
   int i;
   std::string myvoid("void  ");
   FunctionType * ft = static_cast<FunctionType *>(fDef->decl->form);
   out << myvoid;
   out << fDef->decl->name->name<< " (";
   std::string blank (whiteout(myvoid + fDef->decl->name->name +" ("));
   for (i=0;i<ft->nArgs;++i) {
      if (i!=0) {
         out << ","<<std::endl<< blank;
      }
      Symbol * nam = ft->args[i]->name;
      Type * t = ft->args[i]->form;
      if (recursiveIsGather(t)) {
         out << "_stype "<<nam->name <<","<<std::endl;
         out << blank << "float4 "<<nam->name<<"_scalebias";
      }else {
         if (ft->args[i]->isStream()) {
            t = static_cast<ArrayType *>(t)->subType;
         }
         t->printType(out,nam, true,0);
      }
   }
   std::set<unsigned int>::iterator iter=
      FunctionProp[ fDef->decl->name->name].begin();
   std::set<unsigned int>::iterator iterend = 
      FunctionProp[ fDef->decl->name->name].end();
   for (;iter!=iterend;++iter,++i) {
      if (i!=0)
         out << ","<<std::endl<<blank;
      out << "float4 __indexof_"<<ft->args[*iter]->name->name;
   }
   out << ")"<<std::endl;
   fDef->Block::print(out,0);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
// This function converts gathers into scale and bias expressions
// This function converts function calls' gathers into two args.
// This function adds the indexof items to function calls requriing indexof
Expression *
BRTGPUKernelCode::ConvertGathers (Expression *expr) {
  BrtGatherExpr *gather;

  /* Check function calls inside of kernels */
  if (expr->etype == ET_FunctionCall) {

     //now we have to convert gathers that are passed into functions
     FunctionCall * fc = static_cast<FunctionCall*>(expr);
     if (fc->function->etype==ET_Variable) {
        Variable * function = static_cast<Variable*>(fc->function);
        if (function->name->entry && function->name->entry->uVarDecl) {
           if (function->name->entry->uVarDecl->isKernel() &&
               !function->name->entry->uVarDecl->isReduce()) {

              std::set<unsigned int>::iterator iter=
                 FunctionProp[function->name->name].begin();
              std::set<unsigned int>::iterator iterend = 
                 FunctionProp[function->name->name].end();

              for ( ; iter!=iterend; ++iter) {
                 if (fc->args[*iter]->etype != ET_Variable) {
                    std::cerr<<"Error: ";
                    fc->args[*iter]->location.printLocation(std::cerr);
                    std::cerr<< "Argument "<<*iter+1<<" not a stream where";
                    std::cerr<< "indexof used in subfunction";
                 } else {
                    Variable * v = static_cast<Variable*>(fc->args[*iter]);
                    if (v->name->entry &&
                        v->name->entry->uVarDecl){
                       if (v->name->entry->uVarDecl->isStream()) {
                          Decl * indexofDecl 
                             = new Decl(new BaseType(BT_Float4));
                          
                          Symbol * indexofS = new Symbol;
                          indexofS->name = "__indexof_"+v->name->name;
                          indexofS->entry = mk_vardecl(indexofS->name,
                                                       indexofDecl);
                          fc->addArg(new Variable(indexofS,v->location));
                       } else {
                          std::cerr<< "Error: ";
                          v->location.printLocation(std::cerr);
                          std::cerr<<" Argument "<<*iter+1<<" not a stream";
                          std::cerr<< "where indexof used in subfunction";
                       }
                    }
                 }
              }

              int i;
              for (i=0;i<fc->nArgs();++i) {
                 if (fc->args[i]->etype==ET_Variable){
                    Variable * v = static_cast<Variable*>(fc->args[i]);
                    if (v->name->entry&&v->name->entry->uVarDecl) {
                       if(recursiveIsGather(v->name->entry->uVarDecl->form)) {
                          ++i;
                          fc->args.insert(fc->args.begin()+i,NewGatherArg(v));
                       }
                    }
                 }
              }
           }
        }
     }
     return expr;
  }


  /* Convert gather expressions: a[i][j] */
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
BRTFP30KernelCode::printCode(std::ostream& out) const
{
   printCodeForType(out, false);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
BRTPS20KernelCode::printCode(std::ostream& out) const
{
   printCodeForType(out, true);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
void
BRTGPUKernelCode::printCodeForType(std::ostream& out, 
                                   bool ps20_not_fp30) const
{
   FunctionType *fType;
   std::ostringstream wrapOut;
   char *fpcode;

   fDef->Block::print(wrapOut, 0);
   if (globals.verbose) {
      std::cerr << "***Wrapping***\n";
      fDef->decl->print(out, true);
      std::cerr << std::endl << wrapOut.str() << "\n**********\n";
   }

   assert (fDef->decl->form->type == TT_Function);
   fType = (FunctionType *) fDef->decl->form;

   fpcode = CodeGen_GenerateCode(fType->subType,
                                 fDef->FunctionName()->name.c_str(),
                                 fType->args, fType->nArgs,
                                 wrapOut.str().c_str(),
                                 ps20_not_fp30);
   out << fpcode;
   free(fpcode);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
// This function prints out the type of a variable from a stream passed in
// it may optionally add indirection.
static void printType (std::ostream & out, 
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

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
// The cpu args are already translated since all call sites are changed
// to be postfixed with __cpu_inner
void BRTCPUKernelCode::printInnerCode (std::ostream&out) const {
   //nothing happens: you fail to obtain anything!
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
bool BRTCPUKernelCode::PrintCPUArg::isArrayType() {
      return recursiveIsArrayType(a->form);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
bool BRTCPUKernelCode::PrintCPUArg::isStream() {
   return a->isStream();
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
// This function used to do a reinterpret cast to the stream that was on
// the argument list.  With stream->stream reductions this became obsolete
// and such arguments are now passed in as the extents, dims, and args, arrays
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

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
// This function prints the header (prototype), tight loop definition, and 
// cleanup for a dimensionless gather stream.(all gather streams are so marked)
void BRTCPUKernelCode::
     PrintCPUArg::printDimensionlessGatherStream(std::ostream&out,STAGE s){
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
        case CLEANUP:
           break;
        default:
	  indent(out,1);
          PrintAccessStream(out,index,"releaseData","brook::Stream::READ");
	  out << ";"<<std::endl;
	  break;
        }
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
// This function prints the code that must occur when the output stream
// has overrun its bounds in the base (last) dimension. each input stream
// must recalculate its position (not known due to potential stretch)
// in the ndcube or entire map, as the case may be.
void BRTCPUKernelCode::PrintCPUArg::ResetNewLine(std::ostream&out,
                                                 bool nDcube,
                                                 unsigned int ref){
	if (a->isReduce())
		return;
	if (!a->isStream())
		return;
	bool isIter = (a->form->getQualifiers()&TQ_Iter)!=0;
        
	if (!nDcube&&(ref==index||isIter))
           return;
	if (!nDcube) {
           indent(out,3);
		out << "iter"<<index<<"=getIndexOf(i+mapbegin,";
                out << "extents["<<index<<"],";
                out << "dim, extents["<<ref<<"]);";
		out << std::endl;           
        }else {
           indent(out,3);
		out << "iter"<<index<<" = getIndexOf(i, mapbegin, mapextents,";
                out << "extents["<<index<<"], ";
                out << "dim, extents["<<ref<<"]);";
		out << std::endl;           
        }   
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
// This function prints out the code that will increment a single variable
// Only streams must be incremented. output streams are always incremented
// input streams are only incremented if a ratio point has been hit.
// simple output streams (no nd cube) may be incremented with a ++ operator
void BRTCPUKernelCode::PrintCPUArg::Increment(std::ostream & out, 
                                              bool nDcube, 
                                              unsigned int ref) {
	if (a->isReduce())
		return;
	if (!a->isStream())
		return;
        bool isIter = (a->form->getQualifiers()&TQ_Iter)!=0;
	if (!nDcube&&(isIter||ref==index)) {
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
	}else {
           indent(out,2);
           if(ref!=index){
              out << "if (++ratioiter"<<index<<">=ratio"<<index<<"){";
              out << std::endl;
              indent(out,3);
              out << "ratioiter"<<index<<"=0;"<<std::endl;
              indent(out,3);
           }
           out << "++iter"<<index<<";"<<std::endl;
           if (ref!=index) {
              indent(out,2);
              out << "}"<<std::endl;			
           }
	}
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
// This function sets up each argument depending on what type it is.
// simple outputs for non nD case may just add the starting offset.
// more complex outputs require the indexOf function call and need
// the ratio variables to be initialized so they may be later used.
void BRTCPUKernelCode::PrintCPUArg::InitialSet(std::ostream & out, 
                                               bool nDcube, 
                                               unsigned int ref) {
	if (a->isReduce())
		return;
	if (!a->isStream())
		return;
        bool isIter=(a->form->getQualifiers()&TQ_Iter)!=0;
	if (!nDcube&&(isIter||ref==index)) {
           indent(out,1);
           out << "arg"<<index<<"+=mapbegin;"<<std::endl;
	}else {
           if (ref!=index) {
              indent(out,1);
              out << "unsigned int ratio"<<index<<" = extents["<<ref<<"]";
              out << "[dim-1]";
              out << "/extents["<<index<<"][dim-1];"<<std::endl;
              indent(out,1);
              out << "unsigned int ratioiter"<<index<<" = 0;"<<std::endl;
           }
           indent (out,1);
           out << "unsigned int iter"<<index<<" = getIndexOf(";
           if (!nDcube) 
              out << "mapbegin,";
           else
              out << "0, mapbegin, mapextents, ";
           out << "extents["<<index<<"], ";
           out << "dim, extents["<<ref<<"]);"<<std::endl;
	}
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
// This function prints out the usage of a particular variable when
// the inner function is called from the tight loop.
// most items are dereferenced when passed.
// input streams and outputs for the nD case must have iterX added to them.
void BRTCPUKernelCode::PrintCPUArg::Use(std::ostream &out, 
                                        bool nDcube,
                                        unsigned int ref) {
   bool isIter = (a->form->getQualifiers()&TQ_Iter)!=0;
   bool isReduceArg=(a->form->getQualifiers()&TQ_Reduce)!=0;
   if (useShadowOutput()||isGather()) {
      out << "arg"<<index;
   }else {
      if (isStream()&&(ref==index||isIter)&&!nDcube) {
         out <<"*arg"<<index;
      }else if (isReduceArg||!isStream()) {
         out << "*arg"<<index;
      }else {
         out << "*(arg"<<index<<" + iter"<<index<<")";
      }			
   }
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
// This function prints the Header, tight loop definition and cleanup
// for any given argument of type ArrayStream (a stream of float2[10] for ex.)
// Arrays have pass by ref semantics in C++, but are just passed const here.
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

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
// When combining we do not wish to write to the output stream once more.
// This is only necessary for a kernel with a reduce.
// This will print all out variables in the combine functions as locals.
// so as not to corrupt the legitimate output.
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
       case CLEANUP:
         break;
       }
}  

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
// This function prints out streams of basic types and basic type constants.
// It prints the header (function prototype), definition in tight loop, and 
// cleanup.  Outputs must be passed by reference as indicaed... others are
// const.
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
    
// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
// This function redirects the call to the specific printer based on type
void BRTCPUKernelCode::PrintCPUArg::printCPUVanilla(std::ostream & out,
                                                    STAGE s){
        if(isGather())
	  printDimensionlessGatherStream(out,s);
	else if (isArrayType())
	  printArrayStream(out,s);
        else
	  printNormalArg(out,s);
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+oo+
// only combines are shadowed.
bool BRTCPUKernelCode::PrintCPUArg::useShadowOutput()const {
   return shadowOutput&&(a->form->getQualifiers()&TQ_Out)!=0;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
// This function calls the appropriate redirect function. Ironic, huh?
void BRTCPUKernelCode::PrintCPUArg::printCPU(std::ostream & out, 
                                             STAGE s) {
      if (useShadowOutput()) {
         printShadowArg(out,s);
      }else {
         printCPUVanilla(out,s);
      }
}


// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
// This function trasnforms the arguments in the decl into a PrintCPUArg
// class, which has all the utility functions described above...
// and redirectors and so forth.
std::vector<BRTCPUKernelCode::PrintCPUArg> 
BRTCPUKernelCode::getPrintableArgs (FunctionDef * fDef,bool shadowOutput) {
    Type * form = fDef->decl->form;
    assert (form->isFunction());
    FunctionType* func = static_cast<FunctionType *>(form->dup());
    std::vector<PrintCPUArg> myArgs;
    {for (int i=0;i<func->nArgs;++i) {
        myArgs.push_back(PrintCPUArg(func->args[i],
                                     i,
                                     shadowOutput,
                                     fDef->decl->isReduce()));
    }}  
    return myArgs;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
// This function prints the function that is called from the inner loop
// of the CPU. This function may also be called from within other kernels.
void BRTCPUKernelCode::printInnerFunction (std::ostream & out,
                                           std::string fullname,
                                           FunctionDef *fDef, 
                                           bool shadowOutput,
                                           std::string origname) {
    Type * form = fDef->decl->form;
    assert (form->isFunction());
    FunctionType* func = static_cast<FunctionType *>(form->dup());
    std::string myvoid("void  ");
    out << myvoid;
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
// This prints an altered inner loop for the combine function.
// This inner loop will use the combiner second input instead of the stream
// argument to the first reduce function. Other reduce functions are not called
void BRTCPUKernelCode::printCombineInnerLoop(std::ostream &out)const {
   if (!globals.multiThread) return;//only print if multithreading.
    FunctionDef * fDef = static_cast<FunctionDef*>(this->fDef->dup());   
    Brook2Cpp_ConvertKernel(fDef);
    BrookCombine_ConvertKernel(fDef);
    std::vector <PrintCPUArg> myArgs = getPrintableArgs (fDef,true);
    printInnerFunction (out,
                        "__"+fDef->decl->name->name+"_cpu_inner",
                        fDef,
                        true,
                        this->fDef->decl->name->name);    
    delete fDef;

}
// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
// This prints the combine caller (if necessary) which sets up locals and calls
// the combine above.  
void BRTCPUKernelCode::printCombineCode(std::ostream &out) const{
   if (!globals.multiThread) return;//only print if multithreading.
    FunctionDef * fDef = static_cast<FunctionDef*>(this->fDef->dup());   
    Brook2Cpp_ConvertKernel(fDef);
    BrookCombine_ConvertKernel(fDef);
    std::vector <PrintCPUArg> myArgs = getPrintableArgs (fDef,true);        
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
    unsigned int reference_stream = getReferenceStream(this->fDef);
    {for (unsigned int i=0;i<myArgs.size();++i) {
        indent(out,1);
        myArgs[i].printCPU(out,PrintCPUArg::DEF);
        out << std::endl;
    }}
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
        myArgs[i].Use(out,false,reference_stream);
    }}
    printIndexOfCallingArgs(out);
    out<< ");"<<std::endl;
    {for (unsigned int i=0;i<myArgs.size();++i) {
        myArgs[i].printCPU(out,PrintCPUArg::CLEANUP);
    }}    
    out << "}"<<std::endl;   
    delete fDef;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
// This function prints indexof args in headers as necessary for the given 
// function.
// The FunctionProp map determines the necessity by looking at the dag.
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

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
// This function prints indexof definitions as necessary for the given function
// The FunctionProp map determines the necessity by looking at the dag.
void BRTCPUKernelCode::initializeIndexOf(std::ostream&out)const {
   std::string name = this->fDef->decl->name->name;
   functionProperties::iterator it=FunctionProp[name].begin();
   functionProperties::iterator end=FunctionProp[name].end();
   for (;it!=end;++it) {
      indent(out,1);
      out << ";"<<std::endl<<"__BrtFloat4 indexof"<<*it;
      out << " = computeIndexOf(mapbegin, ";
      PrintAccessStream(out,*it,"getDimension");
      out << ", ";
      PrintAccessStream(out,*it,"getExtents");
      out << ");";
      out <<std::endl;
   }
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
// This function prints indexof incrementation as necessary for the given func
// The FunctionProp map determines the necessity by looking at the dag.
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

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
// This function increments any local variables (given in myArgs) appropriately
// It merely redirects the calls to the myArgs incrementation functions.
// Then it checks the output to see if a newline is reached and recomputes
// indexof if that is the case
void BRTCPUKernelCode::incrementAllLocals(std::ostream&out,
                                          bool nDcube,
                                          std::vector<PrintCPUArg> myArgs
                                          ) const{
       indent(out,2); out << "i++;"<<std::endl;
       incrementIndexOf(out);
       unsigned int reference_stream = getReferenceStream(this->fDef);
       {for (unsigned int i=0;i<myArgs.size();++i) {
          myArgs[i].Increment(out,nDcube,reference_stream);
       }}
       indent(out,2);
       if (nDcube) {
          out << "if (i%newline==0) {"<<std::endl;
       }else {
          out << "if ((mapbegin+i)%newline==0) {"<<std::endl;
       }
       {for (unsigned int i=0;i<myArgs.size();++i) {
          myArgs[i].ResetNewLine(out,nDcube,reference_stream);
       }}          
       indent(out,2);
       out << "}"<<std::endl;
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
// This function will print out the tight loop for the given function.
// This works on maps and reduce to single value.
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
    out << long_name<< "unsigned int mapextent) {"<<std::endl;
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
    if (reduceneeded) {
       indent(out,1); out << "if (mapextent) {"<<std::endl;
       
       indent(out,2);out<< "__" <<fDef->decl->name->name<<"__base_cpu_inner (";
       out << std::endl;
       {for (unsigned int i=0;i<myArgs.size();++i) {
          if (i!=0)
             out <<","<<std::endl;
          indent(out,3);
          myArgs[i].Use(out,false,reference_stream);
       }}
       printIndexOfCallingArgs(out);
       out << ");"<<std::endl;
       incrementAllLocals(out,false,myArgs);
       indent(out,1); out <<"}"<<std::endl;
    }
    indent(out,1);
    out << "while (i<mapextent) {";
    out << std::endl;
    indent(out,2);out << "__" <<fDef->decl->name->name<<"_cpu_inner (";
    out<<std::endl;
    {for (unsigned int i=0;i<myArgs.size();++i) {
        if (i!=0)
            out <<","<<std::endl;
        indent(out,3);
        myArgs[i].Use(out,false,reference_stream);
    }}
    printIndexOfCallingArgs(out);
    out<< ");"<<std::endl;
    incrementAllLocals(out,false,myArgs);
    indent(out,1);out <<"}"<<std::endl;
    {for (unsigned int i=0;i<myArgs.size();++i) {
        myArgs[i].printCPU(out,PrintCPUArg::CLEANUP);
    }}    
    out << "}"<<std::endl;     
}

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
// This function prints the tight loop that is used for stream->stream
// Reductions.
void BRTCPUKernelCode::printNdTightLoop(std::ostream&out, 
                                        FunctionDef * fDef, 
                                        std::vector<PrintCPUArg> myArgs,
                                        bool reduceneeded)const{ 
    Symbol enhanced_name;
    enhanced_name.name = "__"+fDef->decl->name->name + "_ndcpu";
    std::string myvoid("void  ");
    out << myvoid;
    fDef->decl->form->printBefore(out,&enhanced_name,0);
    out << " (";
    std::string long_name (whiteout(myvoid + enhanced_name.name+" ("));
    out << "const std::vector<void *>&args,"<<std::endl;
    out << long_name << "const std::vector<const unsigned int *>&extents,";
    out <<std::endl;
    out << long_name << "const std::vector<unsigned int>&dims,"<<std::endl;
    out << long_name<< "const unsigned int *mapbegin, "<<std::endl;
    out << long_name<< "const unsigned int *mapextents) {"<<std::endl;
    {for (unsigned int i=0;i<myArgs.size();++i) {
        indent(out,1);
        myArgs[i].printCPU(out,PrintCPUArg::DEF);
        out << std::endl;
    }}
    unsigned int reference_stream = getReferenceStream(this->fDef);
    indent(out,1);
    out << "unsigned int dim=dims["<<reference_stream<<"];"<<std::endl;
    indent(out,1);
    out << "unsigned int newline=mapextents[dim-1];"<<std::endl;
    indent(out,1);
    out << "unsigned int mapextent = mapextents[0], i=1;"<<std::endl;
    indent(out,1);
    out << "for (;i<dim;++i) mapextent*=mapextents[i];"<<std::endl;
    out << std::endl;
    {for (unsigned int i=0;i<myArgs.size();++i) {
          myArgs[i].InitialSet(out,true,reference_stream);
    }}

    initializeIndexOf(out);
    indent(out,1); out << "i=0; "<<std::endl;
    if (reduceneeded) {
       indent(out,1); out << "if (mapextent) {"<<std::endl;
       
       indent(out,2);out<< "__" <<fDef->decl->name->name<<"__base_cpu_inner (";
       out << std::endl;
       {for (unsigned int i=0;i<myArgs.size();++i) {
          if (i!=0)
             out <<","<<std::endl;
          indent(out,3);
          myArgs[i].Use(out,true,reference_stream);
       }}
       printIndexOfCallingArgs(out);
       out << ");"<<std::endl;
       incrementAllLocals(out,true,myArgs);
       indent(out,1); out <<"}"<<std::endl;
    }
    indent(out,1);
    out << "while (i<mapextent) {";
    out << std::endl;
    indent(out,2);out << "__" <<fDef->decl->name->name<<"_cpu_inner (";
    out<<std::endl;
    {for (unsigned int i=0;i<myArgs.size();++i) {
        if (i!=0)
            out <<","<<std::endl;
        indent(out,3);
        myArgs[i].Use(out,true,reference_stream);
    }}
    printIndexOfCallingArgs(out);
    out<< ");"<<std::endl;
    incrementAllLocals(out,true,myArgs);
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
       printCombineInnerLoop(out);
    }
   //we don't want to automatically print this for it would say "kernel void" which means Nothing
    printTightLoop(out,fDef,myArgs,reduceneeded);
    delete fDef;
    if (reduceneeded) {
       printNdTightLoop(out,fDef,myArgs,reduceneeded);
       printCombineCode(out);
    }
}
