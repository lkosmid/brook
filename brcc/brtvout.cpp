#include "brtvout.h"

#include "ctool.h"

VoutFunctionType voutFunctions;

FunctionDef *IdentifyVoutFunc(FunctionDef * fd){
   FunctionType * ft = static_cast<FunctionType* >(fd->decl->form);
   bool foundvout=false;
   std::set<unsigned int> vouts;
   for (int i=0;i<ft->nArgs;++i) {
      if ((ft->args[i]->form->getQualifiers()&TQ_Vout)!=0) {
         foundvout=true;
         vouts.insert(i);
      }
   }
   if (foundvout)
      voutFunctions.insert(VoutFunctionType::
                           value_type(fd->FunctionName()->name,vouts));
   
   return NULL;
}
FunctionDef * TransformHeader (FunctionDef * fd) {
   VoutFunctionType::iterator func
      =voutFunctions.find(fd->FunctionName()->name);
   if (func==voutFunctions.end())
      return NULL;
   FunctionType * ft = static_cast<FunctionType*>(fd->decl->form);
   std::set<unsigned int>::iterator iter =func->second.begin();
   
   for (;iter!=func->second.end();++iter) {
      DeclStemnt * ds = new DeclStemnt (fd->location);
      Decl * tmpVout =ft->args[*iter];
      ft->args[*iter]=ft->args[*iter]->dup();
      ft->args[*iter]->name->name=
         "__"+ft->args[*iter]->name->name+"_stream";
      if (tmpVout->form->type==TT_Stream) {
         tmpVout->form = static_cast<ArrayType*>(tmpVout->form)->subType;
      }
      tmpVout->next=NULL;
      ds->addDecl(tmpVout);
      ds->next = fd->head;
      fd->head = ds;
   }
   Symbol * voutCounter = new Symbol;
   voutCounter->name = "__vout_counter";
   Decl * VoutCounter =  new Decl (voutCounter);
   VoutCounter->form = new BaseType (BT_Float2);
   ft->addArg(VoutCounter);
   return NULL;
}
FunctionDef * TransformVoutToOut (FunctionDef * fd) {
   VoutFunctionType::iterator func
      =voutFunctions.find(fd->FunctionName()->name);
   if (func==voutFunctions.end())
      return NULL;
   FunctionType * ft = static_cast<FunctionType*>(fd->decl->form);
   std::set<unsigned int>::iterator iter =func->second.begin();
   
   for (;iter!=func->second.end();++iter) {
      BaseType * bat = ft->args[*iter]->form->getBase();

      
      bat->qualifier&=(~TQ_Vout);
      bat->qualifier|=TQ_Out;
   }   
   return NULL;
}

Statement * ModifyReturnStemnt (Statement * ste, 
                                FunctionType * vt, 
                                Decl * vout_counter,
                                const Location & location) {
   Block * block = new Block(location);
   Block * subblock = new Block(location);
   
   Symbol * vcounter=  vout_counter->name->dup();
   vcounter->name=vout_counter->name->name;
   vcounter->entry=mk_paramdecl(vcounter->name,vout_counter);

   Symbol * Eks = new Symbol;Eks->name = "x";
   Symbol * Why = new Symbol;Why->name = "y";
   block->add(new IfStemnt (new RelExpr(RO_GrtrEql,
                                        new BinaryExpr(BO_Member,
                                                       new Variable
                                                       (vcounter,
                                                        location),
                                                       new Variable 
                                                       (Eks,
                                                        location),
                                                       location),
                                       new IntConstant(0,location),
                                       location),
                           subblock,
                           location));                           
   if (ste)
      block->add(ste->dup());
   return block;
}
static 
void TransformBlockStemnt (Block *fd, FunctionType * ft, Decl * vout_counter) {
   if (!fd)
      return;
   Statement * ste=fd->head;
   Statement** lastSte=&fd->head;
   for (;ste;ste=ste->next) {
      Statement * newstemnt=NULL;
      if (ste->type==ST_ReturnStemnt) {
         //newstemnt = ModifyReturnStemnt (ste,ft,vout_counter,ste->location);
      }
      if (ste->type==ST_ExpressionStemnt) {
         ExpressionStemnt * es = static_cast<ExpressionStemnt*>(ste);
         if (es->expression->etype==ET_FunctionCall) {
            FunctionCall* fc=static_cast<FunctionCall*>(es->expression);
            if (fc->args.size()==1
                &&fc->function->etype==ET_Variable
                &&static_cast<Variable*>(fc->function)->name->name=="push") {
               if (fc->args[0]->etype!=ET_Variable) {
                  std::cerr<<"Error: ";
                  fc->args[0]->location.printLocation(std::cerr);
                  std::cerr<< " Push called without specific vout stream.\n";
                  return;
               }
               std::string voutname=static_cast<Variable*>
                  (fc->args[0])->name->name;
               Decl * streamDecl=NULL;
               for (unsigned int i=0;i<ft->nArgs;++i) {
                  if (ft->args[i]->name->name
                      ==std::string("__")+voutname+"_stream") {
                     streamDecl=ft->args[i];
                     break;
                  }
               }
               if (streamDecl==NULL) {
                  std::cerr<<"Error: ";
                  fc->args[0]->location.printLocation(std::cerr);
                  std::cerr<<" Push called on var that is not a vout arg.\n";
                  return;
               }
               Symbol * Eks = new Symbol; Eks->name="x";
               Symbol * counter=vout_counter->name->dup();
               counter->name=vout_counter->name->name;
               counter->entry=mk_paramdecl(counter->name,vout_counter);
               Symbol * stream=new Symbol;
               stream->name="__"+voutname+"_stream";
               stream->entry=mk_paramdecl(stream->name,streamDecl);
               Block * AssignStream = new Block(fc->location);
               AssignStream->add
                  (new ExpressionStemnt
                   (new AssignExpr(AO_Equal,
                                   new Variable(stream,fc->location),
                                   fc->args[0]->dup(),
                                   fc->location),
                    fc->location));
               
               newstemnt=new IfStemnt
                  (new RelExpr(RO_Equal,
                               new FloatConstant(-1,fc->location),
                               new AssignExpr(AO_MinusEql,
                                              new BinaryExpr
                                              (BO_Member,
                                               new Variable (counter,
                                                             fc->location),
                                               new Variable (Eks,
                                                             fc->location),
                                               fc->location),
                                              new IntConstant(1,fc->location),
                                              fc->location),
                               fc->location),
                   AssignStream,
                   fc->location);
               
            }
         }
         if (newstemnt) {
               if (ste==fd->tail) {
                  fd->tail=newstemnt;
               }
               *lastSte=newstemnt;
               newstemnt->next= ste->next;
               //delete ste;
         }
      }
      lastSte=&ste->next;
   }

}
void TransAndFindBlockPush (Block * b, FunctionType * ft, Decl * vout_counter);
static 
void ModifyStemnt (Statement * ste, FunctionType *ft,Decl *vout_counter){
   if (!ste)
      return;
      switch (ste->type) {
      case ST_IfStemnt:{
         IfStemnt * myif = static_cast<IfStemnt*>(ste);
         ModifyStemnt(myif->thenBlk,ft,vout_counter);
         ModifyStemnt(myif->elseBlk,ft,vout_counter);
         break;
      }
      case ST_SwitchStemnt:
         ModifyStemnt(static_cast<SwitchStemnt*>(ste)->block,
                      ft,
                      vout_counter);
         break;
      case ST_ForStemnt: {
         ModifyStemnt(static_cast<ForStemnt*>(ste)->block,
                      ft,
                      vout_counter);
         break;
      }
      case ST_WhileStemnt:
         ModifyStemnt(static_cast<WhileStemnt*>(ste)->block,
                      ft,
                      vout_counter);         
         break;
      case ST_DoWhileStemnt:
         ModifyStemnt(static_cast<DoWhileStemnt*>(ste)->block,
                      ft,
                      vout_counter);         
         break;
      case ST_Block:
         TransAndFindBlockPush (static_cast<Block*>(ste),ft,vout_counter);
         break;
      default:
         break;
      }

}
 
void TransAndFindBlockPush (Block * b, FunctionType * ft, Decl * vout_counter){
   if (!b)
      return;
   Statement * ste;
   TransformBlockStemnt(b,ft,vout_counter);
   for (ste=b->head;ste;ste=ste->next) {
      ModifyStemnt(ste,ft,vout_counter);
   }   
}
FunctionDef* TransformVoutPush(FunctionDef*fd) {
   VoutFunctionType::iterator func
      =voutFunctions.find(fd->FunctionName()->name);
   if (func==voutFunctions.end())
      return NULL;
   FunctionType * ft = static_cast<FunctionType*>(fd->decl->form);
   Decl * vout_counter = ft->args[ft->nArgs-1];
   TransAndFindBlockPush(fd,ft,vout_counter);
   Location tmp(fd->location);
   Block * AssignInf=NULL;
   if (fd->head) {
       AssignInf= new Block(fd->head->location);
       Statement *temp = fd->head;
       AssignInf->next=  fd->head->next;
       fd->head->next= NULL;
       AssignInf->add(fd->head);
       fd->head= AssignInf;
   }else AssignInf = new Block (fd->location);

   //if (fd->tail)
   //   fd->add(ModifyReturnStemnt(NULL,ft,vout_counter,fd->tail->location));
   //else
   //   fd->add(ModifyReturnStemnt(NULL,ft,vout_counter,fd->location));
   return NULL;   
}

void transform_vout (TransUnit * tu) {
   tu->findFunctionDef (IdentifyVoutFunc);
   tu->findFunctionDef (TransformHeader);
   tu->findFunctionDef(TransformVoutToOut);
   tu->findFunctionDef(TransformVoutPush);
   //transform push calls
   //transform function calls
   

}




