#include <set>
#include <fstream>
#include <string>
#include "ctool.h"
#include "main.h"
#include <iostream>

template <class ConverterFunctor> void ConvertToT (Expression * expression);

BinaryOp TranslatePlusGets (AssignOp ae) {
				BinaryOp bo=BO_Assign;
				switch (ae) {
				case AO_Equal:
					break;
				case AO_PlusEql:
					bo=BO_Plus;
					break;
				case AO_MinusEql:
					bo= BO_Minus;
					break;
				case AO_DivEql:
					bo=BO_Div;
					break;
				case AO_ModEql:
					bo=BO_Mod;
					break;
				case AO_ShlEql:
					bo = BO_Shl;
					break;
				case AO_ShrEql:
					bo = BO_Shr;
					break;
				case AO_BitAndEql:
					bo = BO_BitAnd;
					break;
				case AO_BitXorEql:
					bo = BO_BitXor;
					break;
				case AO_BitOrEql:
					bo = BO_BitOr;
					break;
				}
				return bo;
}
bool looksLikeMask (std::string s) {
	if (s.length()<=4) {
		for (unsigned int i=0;i<s.length();++i) {
			if (s[i]!='x'&&s[i]!='y'&&s[i]!='z'&&s[i]!='w'&&
				s[i]!='r'&&s[i]!='g'&&s[i]!='b'&&s[i]!='a'){
				return false;
			}
		}
		return true;
	}
	return false;
}
int translateSwizzle (char mask) {
	switch (mask) {
	case 'y':
	case 'g':
		return 1;
	case 'z':
	case 'b':
		return 2;
	case 'w':
	case 'a':
		return 3;
	default:
		return 0;
	}
}
std::string translateMask (int swizzle) {
	switch (swizzle) {
	case 3:
		return "maskW";
	case 2:
		return "maskZ";
	case 1:
		return "maskY";
	default:
		return "maskX";
	}
}
class MaskExpr : public BinaryExpr {
  public:
	std::string mask;
    MaskExpr( Expression *lExpr, Expression *rExpr, std::string mask, const Location& l ):
		BinaryExpr(BO_Member,lExpr,rExpr,l) {
		this->mask=mask;
	}
	~MaskExpr(){}
	
    Expression *lValue() const { return leftExpr(); }
    Expression *rValue() const { return rightExpr(); }

    int precedence() const { return 16;/*maybe left expr's prec*/ }

    Expression *dup0()  const {
		MaskExpr *ret = new MaskExpr(_leftExpr->dup(),_rightExpr->dup(),mask, location);
		ret->type = type;  
		return ret;
	}
	
    void print(std::ostream& out) const{
		if (lValue()->precedence()<precedence())
			out << "(" << *lValue() << ")";
		else
			out << *lValue();
		out << ".mask"<< mask.length()<<"(";
		int commaprecedence = BinaryExpr(BO_Comma,NULL,NULL,location).precedence();		
		if (rValue()->precedence()<commaprecedence/*comma*/)
			out << "(" <<*rValue() <<")";
		else
			out << *rValue();
		for (unsigned int i=0;i<4&&i<mask.length();++i) {
			out << ",";
			out << translateMask(translateSwizzle(mask[i]));
		}
		out << ")";
	}

};
class NewQuestionColon :public TrinaryExpr {public:
	NewQuestionColon (Expression * c, Expression * t, Expression * f, const Location &l):TrinaryExpr(c,t,f,l) {}
	Expression * dup0() const {return new NewQuestionColon(condExpr()->dup(),trueExpr()->dup(),falseExpr()->dup(),location);}
	int precedence() const {return 3;}
	void print(std::ostream &out) const {
		int memberprecedence = BinaryExpr(BO_Member,NULL,NULL,location).precedence();
		if (condExpr()->precedence()<memberprecedence)/*member*/ 
			out << "(";
		out<<*condExpr();
		if (condExpr()->precedence()<memberprecedence) /*member*/
			out << ")";		
		out<<".questioncolon(";
		out<<*trueExpr();
		out << ",";
		out << *falseExpr();
		out << ")";
	}
		
};
class QuestionColonConverter{public:
	Expression * operator()(Expression * e) {
		TrinaryExpr * te;
		NewQuestionColon * ret=NULL;
		if (e->etype==ET_TrinaryExpr&& (te = dynamic_cast<TrinaryExpr*>(e))) {
			ret = new NewQuestionColon(te->condExpr()->dup(),
									   te->trueExpr()->dup(),
									   te->falseExpr()->dup(),
									   te->location);
		}
		return ret;
	}
};





std::set<Expression *> ArrayBlacklist;//set of items we do not want to change to int1 objects.
void ArrayBlackmailer(Expression * e) {
	ArrayBlacklist.insert(e);
}
void BlackmailType(Type * t);
template <class T> void BlackmailT(Type * t) {
	T * k;
	k = dynamic_cast<T *>(t);
	if (k) {
		BlackmailType(k->subType);
	}
}
void BlackmailType (Type * t) {
	if (!t)
		return;
	BaseType * base = dynamic_cast<BaseType *>(t);

	if (base) {
		if (base->typemask&BT_Char) {
			base->typeName = new Symbol;
			base->typeName->name = "bool1";
			base->typemask=BT_UserType;
		}else if (base->typemask&BT_Float) {
			base->typeName = new Symbol;
			base->typeName->name = "float1";
			base->typemask=BT_UserType;			
		}else if (base->typemask&BT_Double) {
			base->typeName = new Symbol;
			base->typeName->name = "float1";
			base->typemask=BT_UserType;			
		}else if (base->typemask&BT_Long||base->typemask&BT_Int) {
			base->typeName = new Symbol;
			base->typeName->name = "int1";
			base->typemask=BT_UserType;			
		} else if (base->typemask&BT_UserType) {
			if (base->typeName->name=="int") {
				base->typeName = new Symbol;
				base->typeName->name ="int1";
			}
		}
	}
	ArrayType *at ;
	if (t->type==TT_Array&&(at = dynamic_cast<ArrayType *>(t))) {
		at->size->findExpr(ArrayBlackmailer);
		ArrayBlackmailer(at->size);
		
		BlackmailType(at->subType);
	}
	BlackmailT<PtrType>(t);
	BlackmailT<BitFieldType>(t);
	StreamType * st;
/*
	if (st = dynamic_cast<StreamType *>(t)) {
		st->size->findExpr(ArrayBlackmailer);
		ArrayBlackmailer(st->size);		
		BlackmailType(st->subType);
	}
*/
	FunctionType * ft;
	if (ft = dynamic_cast<FunctionType *>(t)) {
		ft->findExpr(ArrayBlackmailer);
	}
	
	
}

void FindTypesDecl (Statement * s) {
	DeclStemnt * ds;
	if ((s->type==ST_TypedefStemnt||s->type==ST_DeclStemnt)&&(ds=dynamic_cast<DeclStemnt*>(s))) {
		//ds->findExpr(ArrayBlackmailer);
		for (unsigned int i=0;i<ds->decls.size();++i) {
			Type * t = ds->decls[i]->form;
			BlackmailType(t);
		}
	}
	FunctionDef * fd;
	if (s->isFuncDef() && (fd = dynamic_cast<FunctionDef *>(s))) {
		BlackmailType(fd->decl->form);
	}
	
}
FunctionDef * FindFunctionTypes(FunctionDef * fd) {
		BlackmailType(fd->decl->form);
		fd->findStemnt(FindTypesDecl);
		return NULL;
}

/*FunctionDef * FindTypesFunc (FunctionDef * fun){
	
	return NULL;
	}*/



class NewIndexExpr :public IndexExpr {public:
	NewIndexExpr (Expression * a, Expression * s,const Location &l):IndexExpr(a,s,l) {} //Not *the* tr1nity with a/s/l check onjoin.
	Expression * dup0() const {return new NewIndexExpr(array->dup(),_subscript->dup(),location);}
	void print(std::ostream &out) const {
    if (array->precedence() < precedence())
    {
        out << "(";
        array->print(out);
        out << ")";
    }
    else
        array->print(out);

    out << "[(int)";
	int castprecedence= CastExpr(NULL,NULL,location).precedence();
	if (_subscript->precedence()<=castprecedence)
		out <<"(";
    _subscript->print(out);
	if (_subscript->precedence()<=castprecedence)
		out <<")";	
    out << "]";

	}
		
};
class IndexExprConverter{public:
	Expression * operator()(Expression * e) {
		IndexExpr *ie;
		IndexExpr * ret=NULL;
		if (e->etype==ET_IndexExpr&&(ie=dynamic_cast<IndexExpr*>(e))) {
			ret = new NewIndexExpr (ie->array->dup(),ie->_subscript->dup(),e->location);
			ret->array->findExpr(ConvertToT<IndexExprConverter>);
			ret->_subscript->findExpr(ConvertToT<IndexExprConverter>);			
		}
		CastExpr * ce;
		if (e->etype==ET_CastExpr&&(ce=dynamic_cast<CastExpr*>(e))) {
			BlackmailType(ce->castTo);
		}
		return ret;
	}
};
class NewIntConstant:public IntConstant {public:
	NewIntConstant(long val, const Location &l):IntConstant(val,l){}
	virtual void print (std::ostream&out) const{
		out << "int1(";
		(this)->IntConstant::print(out);
		out << ")";
	}
};
class NewUIntConstant:public UIntConstant {public:
	NewUIntConstant(unsigned int val, const Location &l):UIntConstant(val,l){}
	virtual void print (std::ostream&out) const{
		out << "int1(";
		(this)->UIntConstant::print(out);
		out << ")";
	}
};
class NewCharConstant:public CharConstant {public:
	NewCharConstant(char val, const Location &l):CharConstant(val,l){}
	virtual void print (std::ostream&out) const{
		out << "bool1(";
		(this)->CharConstant::print(out);
		out << ")";
	}
};
class NewFloatConstant:public FloatConstant {public:
	NewFloatConstant(double val, const Location &l):FloatConstant(val,l){}
	virtual void print (std::ostream&out) const{
		out << "float1(";
		(this)->FloatConstant::print(out);
		out << ")";
	}
};
class NewArrayConstant:public ArrayConstant {public:
	NewArrayConstant(const Location &l):ArrayConstant(l){}
	virtual void print (std::ostream&out) const{
		out << "float"<<items.size()<<"(";
		for (unsigned int i=0;i<items.size();++i) {
			items[i]->print(out);
			if (i!=items.size()-1) {
				out <<", ";
			}
		}
		out << ")";
	}
};

class ConstantExprConverter{public:
	Expression * operator()(Expression * e) {
		Constant *con;
		Constant * ret=NULL;
		if (e->etype==ET_Constant&&(con=dynamic_cast<Constant*>(e))) {
			switch (con->ctype) {
			case CT_Char:
			{
				CharConstant * cc = dynamic_cast<CharConstant *>(con);
				ret = new NewCharConstant(cc->ch,cc->location);
				break;
			}
			case CT_Int:
			{
				IntConstant * cc = dynamic_cast<IntConstant *>(con);
				ret = new NewIntConstant(cc->lng,cc->location);				
				break;
			}
			case CT_UInt:
			{
				UIntConstant * cc = dynamic_cast<UIntConstant *>(con);
				ret = new NewUIntConstant(cc->ulng,cc->location);								
				break;
			}
			case CT_Float:
			{
				FloatConstant * cc = dynamic_cast<FloatConstant *>(con);
				ret = new NewFloatConstant(cc->doub,cc->location);												
				break;
			}
			case CT_Array:
			{
				ArrayConstant * ac = dynamic_cast<ArrayConstant *>(con);
				ArrayConstant *aret=  new NewArrayConstant (ac->location);
				ret=aret;
				for (unsigned int i=0;i<ac->items.size();++i){
					Expression * expr=ac->items[i]->dup();
					expr->findExpr(ConvertToT<ConstantExprConverter>);
					aret->addElement(expr);					
				}
				break;
			}
			}
		}
		return ret;
	}
};


class SwizzleConverter{public:
	Expression * operator()(Expression * e) {
		BinaryExpr * be;
		if (e->etype==ET_BinaryExpr&& (be = dynamic_cast<BinaryExpr*>(e))) {
			if (be->op()==BO_Member) {
				Variable * vswiz;
				if (be->rightExpr()->etype==ET_Variable&&(vswiz = dynamic_cast<Variable*>(be->rightExpr()))) {
					if (looksLikeMask(vswiz->name->name)) {
//						printf ("swizzlefound %s",vswiz->name->name.c_str());
						unsigned int len =vswiz->name->name.length();
						char swizlength [2]={len+'0',0};
						std::string rez=std::string("swizzle")+swizlength+"<";
						for (unsigned int i=0;i<len;++i) {
							char swizchar [3] =  {'0'+translateSwizzle(vswiz->name->name[i]),i==len-1?'\0':',','\0'};
							rez+=swizchar;
						}
						rez+=">()";
						vswiz->name->name=rez;
					}
				}
			}
		}
		return NULL;
	}
};
class MaskConverter{public:
Expression * operator () (Expression * e) {
	AssignExpr * ae;
	BinaryExpr * ret =NULL;
	Variable * vmask=NULL;
	if (e->etype == ET_BinaryExpr&&(ae= dynamic_cast<AssignExpr *> (e))) {
		//now lets identify what expression is to the left... if it's a dot then we go!
		BinaryExpr * lval;
		if (ae->lValue()->etype==ET_BinaryExpr&& (lval = dynamic_cast<BinaryExpr*>(ae->lValue()))) {
			if (lval->op()==BO_Member) {
				if (lval->rightExpr()->etype==ET_Variable&&(vmask = dynamic_cast<Variable*>(lval->rightExpr()))) {
					if (looksLikeMask(vmask->name->name)) {
						//printf ("mask detected %s\n",vmask->name->name.c_str());
						BinaryOp bo =TranslatePlusGets (ae->op());
						if (bo!=BO_Assign) {
							//now we	 		need to move the lval to the right
							ae->_rightExpr = new BinaryExpr(bo,ae->lValue()->dup(),ae->_rightExpr,ae->location);
							ae->aOp=AO_Equal;
						}
						ret = new MaskExpr (lval->leftExpr()->dup(),ae->rValue()->dup(),vmask->name->name,lval->location);
					}
				}
			}
		}
	}
	return ret;
	
}};
class ChainExpression:public Expression {
public:
	ChainExpression (Expression * e):Expression(e->etype,e->location) {
		next = e;
	}
	virtual void print (std::ostream & out)const{
		next->print(out);
	}
	virtual Expression * dup0() const {
		return new ChainExpression(next);
	}
    virtual void findExpr( fnExprCallback cb ) { next->findExpr(cb); }	
};
template <class ConverterFunctor> void ConvertToT (Expression * expression) {
	if (ArrayBlacklist.find(expression)==ArrayBlacklist.end()) {
		Expression * e = ConverterFunctor()(expression);
		if (e) {
//		(*expression)=ChainExpression(e);
			Expression * k = new ChainExpression(e);//memory leak--but how else are we to guarantee the integrity of our expression..it's a lost cause unless we have some way of assinging to the passed in expression without rewriting the whole loop.
			char location[sizeof(Location)];
			memcpy (&location[0],&expression->location,sizeof(Location));
			memcpy (expression,k,sizeof(Expression));//DANGEROUS but we don't have access to the code
			memcpy (&expression->location,&location[0],sizeof(Location));
			e->findExpr(ConvertToT<MaskConverter>);
			
		}
	}
}
void FindMask (Statement * s) {
	s->findExpr(ConvertToT<MaskConverter>);
}
void FindSwizzle (Statement * s) {
	s->findExpr(ConvertToT<SwizzleConverter>);
}
void FindQuestionColon (Statement * s) {
	s->findExpr(ConvertToT<QuestionColonConverter>);
}
void FindIndexExpr (Statement * s) {
	s->findExpr(ConvertToT<IndexExprConverter>);
}
void FindConstantExpr (Statement * s) {
	s->findExpr(ConvertToT<ConstantExprConverter>);
}

bool compileCpp() {
	Project proj;
	TransUnit * tu = proj.parse(globals.sourcename,false,NULL,false,NULL,NULL,NULL);
	if (tu) {
		std::ofstream out;
		
		tu->findStemnt (FindTypesDecl);		
		tu->findStemnt(FindMask);
		tu->findStemnt (FindTypesDecl);				
		tu->findStemnt (FindSwizzle);
		tu->findStemnt (FindTypesDecl);				
		tu->findStemnt (FindQuestionColon);
		tu->findStemnt (FindTypesDecl);				
		tu->findStemnt (FindIndexExpr);
		tu->findStemnt (FindTypesDecl);				
		tu->findStemnt (FindConstantExpr);
		tu->findStemnt (FindTypesDecl);				


		std::string s (globals.coutputname);
		s+="pp";
		out.open(s.c_str());
		if (out.fail()) {
			std::cerr << "***Unable to open " << s << "\n";
		}
		out << *tu << std::endl;
		out.close();
		
	}else {
      std::cerr << "***Unable to parse " << globals.sourcename << std::endl;
	  return false;
	}
	return true;
}
