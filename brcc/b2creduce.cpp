#ifdef _WIN32
#pragma warning(disable:4786)
//the above warning disables visual studio's annoying habit of warning when using the standard set lib
#endif
#include "ctool.h"
#include <map>
#include <string>
#include <vector>

// o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o+o
bool reduceNeeded (const FunctionDef * fd) {
   if (fd->decl->isReduce()) {
      return false;
   }
   bool ret =false;
   Type * form = fd->decl->form;
   assert (form->isFunction());
   FunctionType* func = static_cast<FunctionType *>(form);
   for (unsigned int i=0;i<func->nArgs;++i) {
      if (func->args[i]->isReduce())
         return true;
   }
   return ret;
}


static std::map <std::string,Expression*>reducenames;
typedef std::map<std::string,Expression*>::value_type reducenameval;
static std::string functionmodifier;
static void (*ModifyAssignExpr)(AssignExpr*ae)=0;
static Expression * (*ModifyFunctionCall)(FunctionCall *, 
                                          unsigned int, 
                                          unsigned int)=0;
static Expression* (*ModifyFutureReduceOperator)(Expression*)=0;

static Expression* DoNothing(Expression * fc) {
   return fc;
}
static Expression* ConvertToNop(Expression*fc) {
   Location l (fc->location);
   delete fc;
   return new UIntConstant (0,l);
}
const std::string dual_reduction_arg="__partial_";

static void DemoteAssignExpr (AssignExpr * ae) {
   ae->aOp=AO_Equal;   
}
Expression *ChangeVariable(Expression * lv) {
   if (lv->etype==ET_Variable) {
      Variable * v = static_cast<Variable*>(lv);
      v->name->name=dual_reduction_arg+v->name->name;
   }
   return lv;
}
static void DuplicateLVal(AssignExpr * ae) {
   Expression * rval = ae->rValue();
   ae->_rightExpr=ChangeVariable(ae->lValue()->dup());
   delete rval;
}
static Expression* CombineReduceStream(FunctionCall *func,
                                       unsigned int reduce,
                                       unsigned int stream) {
   Expression * tmp=func->args[stream];
   func->args[stream]=ChangeVariable(func->args[reduce]->dup());
   delete tmp;
   return func;
}
static Expression* FunctionCallToAssign(FunctionCall *func,
                                        unsigned int reduce,
                                        unsigned int stream) {
      Location l (func->location);
      Expression * mreduce=func->args[reduce]->dup();
      Expression * mstream=func->args[stream]->dup();
      //delete func;
      return new AssignExpr(AO_Equal,
                            mreduce,
                            mstream,
                            l);   
}

static Expression * ConvertPlusTimesGets(Expression * e) {
   if (e->etype==ET_BinaryExpr&&static_cast<BinaryExpr*>(e)->bOp==BO_Assign) {
      
      AssignExpr * ae = static_cast<AssignExpr*>(e);
      if (ae->lValue()->etype==ET_Variable) {
         Variable *v= static_cast<Variable*>(ae->lValue());
         SymEntry * s = v->name->entry;
         if (s->uVarDecl) {
            if (s->uVarDecl->form){ 
               if (s->uVarDecl->isReduce()) {
                  if (reducenames.find(v->name->name)==reducenames.end()) {
                     (*ModifyAssignExpr)(ae);
                     reducenames.insert(reducenameval(v->name->name,e));
                  }else if (reducenames[v->name->name]!=e){
                     return (*ModifyFutureReduceOperator)(ae);
                  }
               }
            }
         }
      }
   }
   return e;
}
static Expression * ConvertReduceToGets(FunctionCall* func, FunctionType * type) {

   Expression * mreduce=NULL;
   Expression * mstream=NULL;
   unsigned int reduceloc=0,streamloc=0;
   std::string reducename;
   for (int i=0;i<type->nArgs;++i) {
      if (type->args[i]->isReduce()) {
         if (func->args[i]->etype==ET_Variable) {
            Variable * v = static_cast<Variable*>(func->args[i]);
            if (reducenames.find(v->name->name)==reducenames.end()) {
               reducename=v->name->name;
               mreduce = func->args[i];
               reduceloc=i;
            }else if (reducenames[v->name->name]!=func) {
               return (*ModifyFutureReduceOperator) (func);
            }
         }
      }
      if (type->args[i]->isStream()) {
         mstream = func->args[i];         
         streamloc=i;
      }
   }
   if (mreduce&&mstream) {
      Expression * e =(*ModifyFunctionCall)(func,reduceloc,streamloc);
      reducenames.insert(reducenameval(reducename,func));
      return e;               
   }
   return func;
}


Expression *ChangeFirstReduceFunction (Expression * e) {
   if (e->etype==ET_FunctionCall) {
      FunctionCall * fc = static_cast<FunctionCall*>(e);
      Expression * k =fc->function;
      if (k->etype==ET_Variable) {
         Variable * callname =static_cast<Variable*>(k);
         Symbol * sym = callname->name;
         SymEntry * s= sym->entry;
         if (s->uVarDecl) {
            if (s->uVarDecl->form){ 
               if (s->uVarDecl->isReduce()) {
                  if (s->uVarDecl->form->type==TT_Function) {
                     FunctionType* func = 
                        static_cast<FunctionType*>(s->uVarDecl->form);
                     return ConvertReduceToGets(fc,func);
                  
                  }
               }else if (sym->name.find(functionmodifier)==std::string::npos){
                  std::string tmp = sym->name;
                  for (unsigned int i=0;i<fc->nArgs();++i) {
                     if (fc->args[i]->etype==ET_Variable) {
                        Variable * v = static_cast<Variable*>(fc->args[i]);                        
                        if (v->name->entry&&v->name->entry->uVarDecl) {
                           if (v->name->entry->uVarDecl->isReduce()) {
                              callname->name->name = tmp+functionmodifier;
                              // change kernels taking in a reduction
                              // to be the appropriate combine function
                              // other kernels might not even have such a 
                              // construct as they may not be reduce funcs
                              break;
                           }
                        }
                     }
                  }

               }
            }
         }
      }
   }
   return e;
}


void FindFirstReduceFunctionCall (Statement * s) {

   s->findExpr(&ChangeFirstReduceFunction);
   s->findExpr(&ConvertPlusTimesGets);
}
void BrookReduce_ConvertKernel(FunctionDef *fDef) {
   reducenames.clear();
   functionmodifier="__base";
   ModifyAssignExpr= &DemoteAssignExpr;
   ModifyFunctionCall=&FunctionCallToAssign;
   ModifyFutureReduceOperator=&DoNothing;
   fDef->findStemnt(&FindFirstReduceFunctionCall);
   //   fDef->decl->name = fDef->decl->name->dup();
   fDef->decl->name->name+=functionmodifier;
}

void BrookCombine_ConvertKernel(FunctionDef *fDef) {
   reducenames.clear();
   functionmodifier="__combine";
   ModifyAssignExpr= &DuplicateLVal;
   ModifyFunctionCall=&CombineReduceStream;
   ModifyFutureReduceOperator=&ConvertToNop;
   fDef->findStemnt(&FindFirstReduceFunctionCall);
   //   fDef->decl->name = fDef->decl->name->dup();
   fDef->decl->name->name+=functionmodifier;
   if (1) {
      FunctionType*  func=static_cast<FunctionType*>(fDef->decl->form);
      std::vector <Decl *>AdditionalDecl;
      unsigned int i;
      for (i=0;i<func->nArgs;++i) {
         if (func->args[i]->isReduce()) {
            AdditionalDecl.push_back(func->args[i]->dup());
            AdditionalDecl.back()->name->name=
               dual_reduction_arg+
               AdditionalDecl.back()->name->name;
         }
      }
      for (i=0;i<AdditionalDecl.size();++i) {
         func->addArg(AdditionalDecl[i]);
      }
   }
}



