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

bool compileCpp() {
	Project proj;
	TransUnit * tu = proj.parse(globals.sourcename,false,NULL,false,NULL,NULL,NULL);
	if (tu) {
		std::ofstream out;
//		tu->findFunctionDef (ConvertToMask);
		tu->findStemnt(FindMask);
		tu->findStemnt (FindSwizzle);
		tu->findStemnt (FindQuestionColon);
		tu->findStemnt (FindIndexExpr);

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
