#ifdef _WIN32
#pragma warning(disable:4786)
//the above warning disables visual studio's annoying habit of warning when using the standard set lib
#endif

#include "ctool.h"
#include "brook2cpp.h"


std::map <std::string, functionProperties> FunctionProp;
static std::string currentName;
static bool indofchanged=false;
Expression *indexOfKeyword (Expression* e) {
   if (e->etype==ET_Variable) {
      Variable * v = static_cast<Variable*>(e);
      if (v->name->name=="indexof") {
         FunctionProp[currentName].p|=FP_INDEXOF;
      }else if (v->name->name=="linearindexof") {
         FunctionProp[currentName].p|=FP_LINEARINDEXOF;
      }
   }
   return e;
}
FunctionDef * needIndexOf (FunctionDef * fd) {
   if (fd->decl->storage==ST_Kernel) {
      currentName = fd->FunctionName()->name;
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
            if ((FunctionProp[currentName].p&FunctionProp[v->name->name].p)!=FunctionProp[v->name->name].p) {
               indofchanged=true;
               FunctionProp[currentName].p|=FunctionProp[v->name->name].p;
               //printf ("found %s is linked to %s",currentName.c_str(),v->name->name.c_str());
            }
         }
      }
   }
   return e;
}
FunctionDef * recursiveNeedIndexOf(FunctionDef*  fd) {
   if (fd->decl->storage==ST_Kernel) {
      currentName = fd->FunctionName()->name;
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
            if (fp.p!=FP_NONE) {
               if (fp.p&FP_INDEXOF) {
                  Symbol * s = new Symbol;
                  s->name="indexof";
                  s->entry=mk_vardecl("indexof",IndexOf);
                  fc->addArg(new Variable(s,e->location));                  
               }
               if (fp.p&FP_LINEARINDEXOF) {
                  Symbol * s = new Symbol;
                  s->name="linearindexof";
                  s->entry=mk_vardecl("linearindexof",IndexOf);
                  fc->addArg(new Variable(s,e->location));
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
