/*
 * b2cindex
 * constructs a DAG of functions that need indexof any particular stream
 */
#ifdef _WIN32
#pragma warning(disable:4786)
//the above warning disables visual studio's annoying habit of warning when using the standard set lib
#endif

#include "ctool.h"
#include "brtexpress.h"
#include "b2ctransform.h"


std::map <std::string, functionProperties> FunctionProp;
static std::string currentName;
static std::map<std::string,unsigned int> currentArgs;
static bool indofchanged=false;
Expression *indexOfKeyword (Expression* e) {
   if (e->etype==ET_BrtIndexofExpr) {
      Variable * v = static_cast<BrtIndexofExpr*>(e)->expr;
      SymEntry * se = v->name->entry;
      if (se&&currentArgs.find(v->name->name)!=currentArgs.end()
          &&((se->uVarDecl
              &&se->uVarDecl->isStream())
             ||!se->uVarDecl)) {
         
         FunctionProp[currentName].insert(currentArgs[v->name->name]);
      }else {
         std::cerr << "Error ";
         v->location.printLocation(std::cerr);
         std::cerr<< ": Argument to indexof not a stream\n";
         exit(1);
      }
   }
   return e;
}
void addCurrentArgs(FunctionDef * fd) {
   FunctionType * f = static_cast<FunctionType*>(fd->decl->form);
   currentArgs.clear();
   for (int i=0;i<f->nArgs;++i) {
      currentArgs[f->args[i]->name->name]=i;
   }
}
FunctionDef * needIndexOf (FunctionDef * fd) {
   if (fd->decl->storage==ST_Kernel) {
      currentName = fd->FunctionName()->name;
      addCurrentArgs (fd);
      fd->findExpr (&indexOfKeyword);
   }
   return NULL;
}
Expression * callIndexOf(Expression * e) {
   if (e->etype==ET_FunctionCall) {
      FunctionCall*fc = static_cast<FunctionCall * > (e);
      if (fc->function->etype==ET_Variable) {
         Variable*  v= static_cast<Variable *>(fc->function);
         if (FunctionProp.find(v->name->name)!=FunctionProp.end()){
            functionProperties::iterator i=FunctionProp[v->name->name].begin();
            functionProperties::iterator end=FunctionProp[v->name->name].end();
            for (;i!=end;++i) {
               unsigned int j=*i;
               
               bool appropriate=fc->args[j]->etype==ET_Variable;
               if (appropriate) {
                  Variable * v=static_cast<Variable*>(fc->args[j]);
                  if (currentArgs.find(v->name->name)!=currentArgs.end()) {
                     unsigned int newarg=currentArgs[v->name->name];
                     if (!FunctionProp[currentName].contains(newarg)) {
                        FunctionProp[currentName].insert(newarg);
                        indofchanged=true;
                     }
                  }else {
                     appropriate=false;
                  }
               }
               if (!appropriate) {
                  std::cerr<< "Error ";
                  fc->location.printLocation(std::cerr);
                  std::cerr<<": Variable not passed in to function requiring index of stream\n";
               }
            }
         } 
      }
   }
   return e;
}
FunctionDef * recursiveNeedIndexOf(FunctionDef*  fd) {
   if (fd->decl->storage==ST_Kernel) {
      currentName = fd->FunctionName()->name;
      addCurrentArgs (fd);
      fd->findExpr(&callIndexOf);
   }
   return NULL;
}
Decl* getIndexOfDecl(std::string name, BaseTypeSpec baset) {
   Decl * ret = new Decl( new BaseType(baset));
   ret->name= new Symbol;
   ret->name->name=name;
   return ret;
}
Expression * changeFunctionCallToPassIndexOf(Expression* e) {
   if (e->etype==ET_FunctionCall) {
      
      FunctionCall * fc = static_cast<FunctionCall*>(e);
      if (fc->function->etype==ET_Variable) {
         Variable * v = static_cast<Variable*>(fc->function); 
         //         fprintf (stderr,"functionc all %s",v->name->name.c_str());
         if (FunctionProp.find(v->name->name)!=FunctionProp.end()) {
            functionProperties fp = FunctionProp[v->name->name];
            static Decl *IndexOf=getIndexOfDecl("indexof",BT_Float4);
            static Decl *LinearIndexOf=getIndexOfDecl("linearindexof",BT_Int);
            functionProperties::iterator i=fp.begin();
            functionProperties::iterator end=fp.end();
            for (;i!=end;++i) {
               unsigned int j=*i;
               bool appropriate=fc->args[j]->etype==ET_Variable;
               Variable * v=NULL;
               if (appropriate) {
                  v=static_cast<Variable*>(fc->args[j]);
                  SymEntry * se=  v->name->entry;
                  if (se&&se->uVarDecl)
                     if (!se->uVarDecl->isStream())
                        appropriate=false;                    
               }
               if (appropriate) {
                  Symbol * s = new Symbol;
                  s->name="__indexof_"+v->name->name;
                  s->entry=mk_vardecl(s->name,IndexOf);
                  fc->addArg(new Variable(s,e->location));                  
               }else {
                  std::cerr << "Error ";
                  fc->location.printLocation(std::cerr);
                  std::cerr<< ": Argument "<<j+1<<" to function using indexof";
                  std::cerr<< " not a stream identifier\n";
                  exit(1);
                  
               }
            }
         }
      }
   }
   return e;
   }
FunctionDef * changeFunctionCallForIndexOf (FunctionDef * fd) {
   if (fd->decl->storage==ST_Kernel) {
      fd->findExpr(&changeFunctionCallToPassIndexOf);
   }
   return NULL;
}

void 
Brook2Cpp_IdentifyIndexOf (TransUnit * tu) {
   tu->findFunctionDef(&needIndexOf);
   do {
      indofchanged=false;
      tu->findFunctionDef(&recursiveNeedIndexOf);
   }while (indofchanged);
}
