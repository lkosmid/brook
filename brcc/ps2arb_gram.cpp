/* A Bison parser, made from ps2arb_gram.y
   by GNU bison 1.35.  */

#define YYBISON 1  /* Identify Bison output.  */

#define yyparse ps2arb_parse
#define yylex ps2arb_lex
#define yyerror ps2arb_error
#define yylval ps2arb_lval
#define yychar ps2arb_char
#define yydebug ps2arb_debug
#define yynerrs ps2arb_nerrs
# define	PS_NOP	257
# define	PS_NEGATE	258
# define	PS_SWIZZLEMASK	259
# define	PS_COLORREG	260
# define	PS_TEMPREG	261
# define	PS_TEXCOORDREG	262
# define	PS_OUTPUTREG	263
# define	PS_SAMPLEREG	264
# define	PS_CONSTREG	265
# define	PS_TEXKILL	266
# define	PS_SINCOS	267
# define	PS_UNARY_OP	268
# define	PS_BINARY_OP	269
# define	PS_TRINARY_OP	270
# define	PS_OP_FLAGS	271
# define	PS_DCLTEX	272
# define	PS_DCL	273
# define	PS_DEF	274
# define	PS_COMMA	275
# define	PS_MOV	276
# define	PS_COMMENT	277
# define	PS_ENDLESS_COMMENT	278
# define	PS_FLOAT	279
# define	PS_NEWLINE	280
# define	PS_PSHEADER	281

#line 1 "ps2arb_gram.y"


//#define YYDEBUG 1
#ifdef WIN32
#pragma warning(disable:4786)
#pragma warning(disable:4065)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <sstream>
#include "ps2arb.h"
#include "ps2arb_intermediate.h"

using std::map;
using std::string;
using namespace std;//otherwise VC6 dies
extern int ps_lineno;

using namespace ps2arb;

BinOp * MXxYFactory (BinaryFactory * ThreeOrFour, 
                     const InstructionFlags & iflag,
                     const Register & dst, 
                     const Register & src0, 
                     const Register & src1,
                     int y);

static BinOp * M4x4Factory  (const InstructionFlags & iflag, const Register & dst, const Register & src0, const Register & src1) {
	return MXxYFactory(&Dp4Op::factory,iflag,dst,src0,src1,4);
}

static BinOp * M3x4Factory  (const InstructionFlags & iflag, const Register & dst, const Register & src0, const Register & src1) {
	return MXxYFactory(&Dp3Op::factory,iflag,dst,src0,src1,4);
}

static BinOp * M4x3Factory  (const InstructionFlags & iflag, const Register & dst, const Register & src0, const Register & src1) {
	return MXxYFactory(&Dp4Op::factory,iflag,dst,src0,src1,3);
}

static BinOp * M3x3Factory  (const InstructionFlags & iflag, const Register & dst, const Register & src0, const Register & src1) {
	return MXxYFactory(&Dp3Op::factory,iflag,dst,src0,src1,3);
}

static BinOp * M3x2Factory  (const InstructionFlags & iflag, const Register & dst, const Register & src0, const Register & src1) {
	return MXxYFactory(&Dp3Op::factory,iflag,dst,src0,src1,2);
}

static TriOp * ReverseCmpFactory  (const InstructionFlags & iflag, const Register & d, const Register & a, const Register & b, const Register & c) {
	return CmpOp::factory(iflag,d,a,c,b);
}


static map<string,VoidFactory*> createVoidFactory() {
	map<string,VoidFactory*> ret;
	ret.insert(map<string,VoidFactory*>::value_type("texkill",&KilOp::factory));
	return ret;
}
static map<string,UnaryFactory*> createUnaryFactory() {
	map<string,UnaryFactory*> ret;
	ret.insert(map<string,UnaryFactory*>::value_type("abs",&AbsOp::factory));
	ret.insert(map<string,UnaryFactory*>::value_type("exp",&ExpOp::factory));
	ret.insert(map<string,UnaryFactory*>::value_type("log",&LogOp::factory));
	ret.insert(map<string,UnaryFactory*>::value_type("frc",&FrcOp::factory));
	ret.insert(map<string,UnaryFactory*>::value_type("rcp",&RcpOp::factory));
	ret.insert(map<string,UnaryFactory*>::value_type("rsq",&RsqOp::factory));
	ret.insert(map<string,UnaryFactory*>::value_type("nrm",&NrmOp::factory));
	ret.insert(map<string,UnaryFactory*>::value_type("mov",&MovOp::factory));
	ret.insert(map<string,UnaryFactory*>::value_type("sincos",&ScsOp::factory));
	return ret;
}
static map<string,BinaryFactory*> createBinaryFactory() {
	map<string,BinaryFactory*> ret;
	ret.insert(map<string,BinaryFactory*>::value_type("add",&AddOp::factory));
	ret.insert(map<string,BinaryFactory*>::value_type("sub",&SubOp::factory));
	ret.insert(map<string,BinaryFactory*>::value_type("mul",&MulOp::factory));
	ret.insert(map<string,BinaryFactory*>::value_type("crs",&XpdOp::factory));
	ret.insert(map<string,BinaryFactory*>::value_type("dp3",&Dp3Op::factory));
	ret.insert(map<string,BinaryFactory*>::value_type("dp4",&Dp4Op::factory));
	ret.insert(map<string,BinaryFactory*>::value_type("pow",&PowOp::factory));
	ret.insert(map<string,BinaryFactory*>::value_type("min",&MinOp::factory));
	ret.insert(map<string,BinaryFactory*>::value_type("max",&MaxOp::factory));
	ret.insert(map<string,BinaryFactory*>::value_type("m3x2",&M3x2Factory));
	ret.insert(map<string,BinaryFactory*>::value_type("m3x3",&M3x3Factory));
	ret.insert(map<string,BinaryFactory*>::value_type("m3x4",&M3x4Factory));
	ret.insert(map<string,BinaryFactory*>::value_type("m4x3",&M4x3Factory));
	ret.insert(map<string,BinaryFactory*>::value_type("m4x4",&M4x4Factory));
	ret.insert(map<string,BinaryFactory*>::value_type("texld",&TexldOp::factory));
	ret.insert(map<string,BinaryFactory*>::value_type("texldp",&TexldpOp::factory));
	ret.insert(map<string,BinaryFactory*>::value_type("texldb",&TexldbOp::factory));
	return ret;
}
static map<string,TrinaryFactory*> createTrinaryFactory() {
	map<string,TrinaryFactory*> ret;
	ret.insert(map<string,TrinaryFactory*>::value_type("cmp",&ReverseCmpFactory));
	ret.insert(map<string,TrinaryFactory*>::value_type("lrp",&LrpOp::factory));
	ret.insert(map<string,TrinaryFactory*>::value_type("mad",&MadOp::factory));
	ret.insert(map<string,TrinaryFactory*>::value_type("dp2add",&Dp2addOp::factory));
	return ret;
}

static std::map<string,VoidFactory*> void_factory=createVoidFactory();
static std::map<string,UnaryFactory*> unary_factory=createUnaryFactory();
static std::map<string,BinaryFactory*> binary_factory=createBinaryFactory();
static std::map<string,TrinaryFactory*> trinary_factory=createTrinaryFactory();

#ifdef WIN32
#pragma warning( disable : 4102 ) 
#endif

extern int yylex(void);
static void yyerror (char *s) {
  fprintf (stderr, "Error Line %d: %s\n", ps_lineno,s);
}


#line 120 "ps2arb_gram.y"
#ifndef YYSTYPE
typedef union {
  char *s;
  float f;
  struct Register{
    char * swizzlemask;
    char * negate;
    char * reg;
  } reg;
} yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif
#line 129 "ps2arb_gram.y"


/* so about this function...it's nice cus it's centrally called
** --however there might be some argument that we're redoing the parsing
** phase of PS2 and that we should put this in the individual sections
** that know that ps2registers are being used...  static Register
*/
static Register
createPS2Register (const YYSTYPE::Register &r) {
	if (r.reg[0]!=0) {
		Symbol s;
		unsigned int index = atoi (r.reg+1);
		switch (r.reg[0]) {
		case 't':
			s.Set(Symbol::TEXCOORD,index);
			break;
		case 's':
			s.Set(Symbol::SAMPLE,index);
			break;
		case 'v':
			s.Set(Symbol::COLOR,index);
			break;
		case 'o':
			switch ( r.reg[1]) {
			case 'D':
			case 'd':
				s.Set (Symbol::OUTPUTDEPTH,0);
				break;
			case 'c':
			default:
				index = atoi(r.reg+2);
				s.Set(Symbol::OUTPUTCOLOR,index);
				break;
			}
			//output
			break;
		case 'c':
			s.Set(Symbol::CONST,index);
			break;
		case 'r':
		default:
			s.Set(Symbol::TEMP,index);
			break;			
		}
		iLanguage->SpecifySymbol (r.reg,s);
	}else{
		fprintf (stderr,"register %s not properly specified:",r.reg);
	}
	return Register(r.reg,r.swizzlemask,r.negate);
}

char * incRegister (string s) {
	if (s.length()) {
		char c=*s.begin();
		s = s.substr(1);
		int which=1+strtol(s.c_str(),NULL,10);
		char out [1024];
		sprintf(out,"%c%d",c,which);
		return strdup(out);
	}	
	return strdup("");
}

BinOp * MXxYFactory  (BinaryFactory * ThreeOrFour, const InstructionFlags & iflag, const Register & dst, const Register & src0, const Register & src1, int y) {
	YYSTYPE cinc;
	char * tmp=NULL;
	Register destination(dst);
	cinc.reg.swizzlemask="";
	cinc.reg.negate="";
	cinc.reg.reg = incRegister(src1);
	destination.swizzle="x";
	iLanguage->AddInst((*ThreeOrFour)(iflag,destination,src0,src1));
	destination.swizzle="y";
	BinOp * ret =(*ThreeOrFour)(iflag,destination,src0,createPS2Register(cinc.reg));
	if (y>2) {
		iLanguage->AddInst(ret);
		cinc.reg.reg = incRegister(tmp = cinc.reg.reg);
		free(tmp);	
		destination.swizzle="z";
		ret = (*ThreeOrFour)(iflag,destination,src0,createPS2Register(cinc.reg));
		if (y>3) {
			iLanguage->AddInst(ret);
			cinc.reg.reg = incRegister(tmp = cinc.reg.reg);
			free(tmp);
			destination.swizzle="w";
			ret= (*ThreeOrFour)(iflag,destination,src0,createPS2Register(cinc.reg));
		}
	}
	free(cinc.reg.reg);
	return ret;
}

static string strtoupper (string s) {
	for (string::iterator i =s.begin();i!=s.end();++i) {
		*i= toupper(*i);
	}
	return s;
}

InstructionFlags AdjustInstructionFlags (std::string flags) {
	flags = strtoupper(flags);
	bool pp = flags.find("_PP")!=string::npos;	
	bool sat = flags.find("_SAT")!=string::npos;
	return InstructionFlags(pp?InstructionFlags::PP:InstructionFlags::FULL,sat);
}

#define DEFINERETVAL YYSTYPE ret

#ifndef YYDEBUG
# define YYDEBUG 0
#endif



#define	YYFINAL		99
#define	YYFLAG		-32768
#define	YYNTBASE	28

/* YYTRANSLATE(YYLEX) -- Bison token number corresponding to YYLEX. */
#define YYTRANSLATE(x) ((unsigned)(x) <= 281 ? yytranslate[x] : 52)

/* YYTRANSLATE[YYLEX] -- Bison token number corresponding to YYLEX. */
static const char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27
};

#if YYDEBUG
static const short yyprhs[] =
{
       0,     0,     7,    11,    12,    16,    18,    19,    22,    25,
      36,    38,    40,    42,    44,    46,    48,    58,    66,    72,
      77,    78,    85,    89,    95,    97,    98,   100,   101,   103,
     105,   108,   110,   112,   115,   117,   118,   122,   124,   126,
     128,   130,   132,   135,   137,   140,   142,   144,   145,   147
};
static const short yyrhs[] =
{
      51,    27,    49,    29,    30,    50,     0,    31,    49,    29,
       0,     0,    32,    49,    30,     0,    32,     0,     0,    19,
      43,     0,    18,    10,     0,    20,    11,    21,    25,    21,
      25,    21,    25,    21,    25,     0,    35,     0,    34,     0,
      33,     0,    37,     0,    38,     0,    39,     0,    16,    40,
      45,    21,    47,    21,    47,    21,    47,     0,    15,    40,
      45,    21,    47,    21,    47,     0,    14,    40,    45,    21,
      47,     0,    21,    47,    21,    47,     0,     0,    13,    40,
      45,    21,    47,    36,     0,    12,    40,    47,     0,    22,
      40,    44,    21,    47,     0,    17,     0,     0,     5,     0,
       0,     6,     0,     8,     0,    42,    41,     0,    45,     0,
       9,     0,     7,    41,     0,     4,     0,     0,    46,    48,
      41,     0,     6,     0,     8,     0,    10,     0,    11,     0,
       7,     0,    26,    49,     0,    26,     0,    23,    49,     0,
      23,     0,    24,     0,     0,    49,     0,     0
};

#endif

#if YYDEBUG
/* YYRLINE[YYN] -- source line where rule number YYN was defined. */
static const short yyrline[] =
{
       0,   253,   255,   256,   258,   259,   260,   263,   267,   298,
     310,   310,   310,   310,   310,   310,   313,   323,   332,   340,
     342,   345,   353,   360,   368,   372,   377,   381,   386,   387,
     390,   399,   405,   414,   423,   427,   432,   441,   441,   441,
     441,   441,   444,   450,   456,   462,   469,   473,   476,   480
};
#endif


#if (YYDEBUG) || defined YYERROR_VERBOSE

/* YYTNAME[TOKEN_NUM] -- String name of the token TOKEN_NUM. */
static const char *const yytname[] =
{
  "$", "error", "$undefined.", "PS_NOP", "PS_NEGATE", "PS_SWIZZLEMASK", 
  "PS_COLORREG", "PS_TEMPREG", "PS_TEXCOORDREG", "PS_OUTPUTREG", 
  "PS_SAMPLEREG", "PS_CONSTREG", "PS_TEXKILL", "PS_SINCOS", "PS_UNARY_OP", 
  "PS_BINARY_OP", "PS_TRINARY_OP", "PS_OP_FLAGS", "PS_DCLTEX", "PS_DCL", 
  "PS_DEF", "PS_COMMA", "PS_MOV", "PS_COMMENT", "PS_ENDLESS_COMMENT", 
  "PS_FLOAT", "PS_NEWLINE", "PS_PSHEADER", "program", "declarations", 
  "instructions", "declaration", "instruction", "trinary_op", "binary_op", 
  "unary_op", "optional_dualreg", "sincos", "texkill", "mov", 
  "optional_flags", "optionalwritemask", "colorortexreg", "declreg", 
  "movreg", "dstreg", "optionalnegate", "srcreg", "readablereg", 
  "newlines", "optendlesscomment", "optnewlines", 0
};
#endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives. */
static const short yyr1[] =
{
       0,    28,    29,    29,    30,    30,    30,    31,    31,    31,
      32,    32,    32,    32,    32,    32,    33,    34,    35,    36,
      36,    37,    38,    39,    40,    40,    41,    41,    42,    42,
      43,    44,    44,    45,    46,    46,    47,    48,    48,    48,
      48,    48,    49,    49,    49,    49,    50,    50,    51,    51
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN. */
static const short yyr2[] =
{
       0,     6,     3,     0,     3,     1,     0,     2,     2,    10,
       1,     1,     1,     1,     1,     1,     9,     7,     5,     4,
       0,     6,     3,     5,     1,     0,     1,     0,     1,     1,
       2,     1,     1,     2,     1,     0,     3,     1,     1,     1,
       1,     1,     2,     1,     2,     1,     1,     0,     1,     0
};

/* YYDEFACT[S] -- default rule to reduce with in state S when YYTABLE
   doesn't specify something else to do.  Zero means the default is an
   error. */
static const short yydefact[] =
{
      49,    45,    43,    48,     0,    44,    42,     0,     3,     0,
       0,     0,     6,     0,     8,    28,    29,    27,     7,     0,
      25,    25,    25,    25,    25,    25,    47,     5,    12,    11,
      10,    13,    14,    15,     3,    26,    30,     0,    24,    35,
       0,     0,     0,     0,     0,    46,     1,     6,     2,     0,
      34,     0,    22,    27,     0,     0,     0,     0,    32,     0,
      31,     4,     0,    37,    41,    38,    39,    40,    27,    33,
      35,    35,    35,    35,    35,     0,    36,    20,    18,     0,
       0,    23,     0,    35,    21,    35,    35,     0,     0,    17,
       0,     0,    35,    35,     9,    19,    16,     0,     0,     0
};

static const short yydefgoto[] =
{
      97,    12,    26,    13,    27,    28,    29,    30,    84,    31,
      32,    33,    39,    36,    17,    18,    59,    54,    51,    52,
      68,     3,    46,     4
};

static const short yypact[] =
{
      -2,    -2,    -2,-32768,   -14,-32768,-32768,    -2,   -15,    37,
      40,    17,    19,    -2,-32768,-32768,-32768,    44,-32768,    29,
      34,    34,    34,    34,    34,    34,    28,    -2,-32768,-32768,
  -32768,-32768,-32768,-32768,   -15,-32768,-32768,    30,-32768,    49,
      47,    47,    47,    47,    18,-32768,-32768,    19,-32768,    35,
  -32768,     8,-32768,    44,    36,    38,    39,    41,-32768,    42,
  -32768,-32768,    33,-32768,-32768,-32768,-32768,-32768,    44,-32768,
      49,    49,    49,    49,    49,    43,-32768,    45,-32768,    46,
      48,-32768,    50,    49,-32768,    49,    49,    51,    52,-32768,
      53,    54,    49,    49,-32768,-32768,-32768,    61,    65,-32768
};

static const short yypgoto[] =
{
  -32768,    55,    21,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
  -32768,-32768,    15,   -51,-32768,-32768,-32768,     1,-32768,   -63,
  -32768,    -1,-32768,-32768
};


#define	YYLAST		89


static const short yytable[] =
{
       5,     6,    69,     9,    10,    11,     8,    77,    78,    79,
      80,    81,    34,     7,    63,    64,    65,    76,    66,    67,
      88,     1,    89,    90,     2,    53,    47,    58,    19,    95,
      96,    20,    21,    22,    23,    24,    40,    41,    42,    43,
      44,    25,    55,    56,    57,    60,    15,    14,    16,    35,
      37,    38,    45,    50,    53,    49,    62,    70,    75,    71,
      72,    98,    73,    74,    82,    99,    83,    85,    61,    86,
       0,     0,    91,    92,    93,    87,     0,     0,     0,    94,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    48
};

static const short yycheck[] =
{
       1,     2,    53,    18,    19,    20,     7,    70,    71,    72,
      73,    74,    13,    27,     6,     7,     8,    68,    10,    11,
      83,    23,    85,    86,    26,     7,    27,     9,    11,    92,
      93,    12,    13,    14,    15,    16,    21,    22,    23,    24,
      25,    22,    41,    42,    43,    44,     6,    10,     8,     5,
      21,    17,    24,     4,     7,    25,    21,    21,    25,    21,
      21,     0,    21,    21,    21,     0,    21,    21,    47,    21,
      -1,    -1,    21,    21,    21,    25,    -1,    -1,    -1,    25,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    34
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/share/bison/bison.simple"

/* Skeleton output parser for bison,

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software
   Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* This is the parser code that is written into each bison parser when
   the %semantic_parser declaration is not specified in the grammar.
   It was written by Richard Stallman by simplifying the hairy parser
   used when %semantic_parser is specified.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

#if ! defined (yyoverflow) || defined (YYERROR_VERBOSE)

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || defined (YYERROR_VERBOSE) */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYLTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
# if YYLSP_NEEDED
  YYLTYPE yyls;
# endif
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAX (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# if YYLSP_NEEDED
#  define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE) + sizeof (YYLTYPE))	\
      + 2 * YYSTACK_GAP_MAX)
# else
#  define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAX)
# endif

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAX;	\
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif


#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");			\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).

   When YYLLOC_DEFAULT is run, CURRENT is set the location of the
   first token.  By default, to implement support for ranges, extend
   its range to the last symbol.  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)       	\
   Current.last_line   = Rhs[N].last_line;	\
   Current.last_column = Rhs[N].last_column;
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#if YYPURE
# if YYLSP_NEEDED
#  ifdef YYLEX_PARAM
#   define YYLEX		yylex (&yylval, &yylloc, YYLEX_PARAM)
#  else
#   define YYLEX		yylex (&yylval, &yylloc)
#  endif
# else /* !YYLSP_NEEDED */
#  ifdef YYLEX_PARAM
#   define YYLEX		yylex (&yylval, YYLEX_PARAM)
#  else
#   define YYLEX		yylex (&yylval)
#  endif
# endif /* !YYLSP_NEEDED */
#else /* !YYPURE */
# define YYLEX			yylex ()
#endif /* !YYPURE */


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)
/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
#endif /* !YYDEBUG */

/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif

#ifdef YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif
#endif

#line 315 "/usr/share/bison/bison.simple"


/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
#  define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL
# else
#  define YYPARSE_PARAM_ARG YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
# endif
#else /* !YYPARSE_PARAM */
# define YYPARSE_PARAM_ARG
# define YYPARSE_PARAM_DECL
#endif /* !YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
# ifdef YYPARSE_PARAM
int yyparse (void *);
# else
int yyparse (void);
# endif
#endif

/* YY_DECL_VARIABLES -- depending whether we use a pure parser,
   variables are global, or local to YYPARSE.  */

#define YY_DECL_NON_LSP_VARIABLES			\
/* The lookahead symbol.  */				\
int yychar;						\
							\
/* The semantic value of the lookahead symbol. */	\
YYSTYPE yylval;						\
							\
/* Number of parse errors so far.  */			\
int yynerrs;

#if YYLSP_NEEDED
# define YY_DECL_VARIABLES			\
YY_DECL_NON_LSP_VARIABLES			\
						\
/* Location data for the lookahead symbol.  */	\
YYLTYPE yylloc;
#else
# define YY_DECL_VARIABLES			\
YY_DECL_NON_LSP_VARIABLES
#endif


/* If nonreentrant, generate the variables here. */

#if !YYPURE
YY_DECL_VARIABLES
#endif  /* !YYPURE */

int
yyparse (YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  /* If reentrant, generate the variables here. */
#if YYPURE
  YY_DECL_VARIABLES
#endif  /* !YYPURE */

  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yychar1 = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack. */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;

#if YYLSP_NEEDED
  /* The location stack.  */
  YYLTYPE yylsa[YYINITDEPTH];
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;
#endif

#if YYLSP_NEEDED
# define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
# define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  YYSIZE_T yystacksize = YYINITDEPTH;


  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
#if YYLSP_NEEDED
  YYLTYPE yyloc;
#endif

  /* When reducing, the number of symbols on the RHS of the reduced
     rule. */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;
#if YYLSP_NEEDED
  yylsp = yyls;
#endif
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  */
# if YYLSP_NEEDED
	YYLTYPE *yyls1 = yyls;
	/* This used to be a conditional around just the two extra args,
	   but that might be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);
	yyls = yyls1;
# else
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);
# endif
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);
# if YYLSP_NEEDED
	YYSTACK_RELOCATE (yyls);
# endif
# undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
#if YYLSP_NEEDED
      yylsp = yyls + yysize - 1;
#endif

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yychar1 = YYTRANSLATE (yychar);

#if YYDEBUG
     /* We have to keep this `#if YYDEBUG', since we use variables
	which are defined only if `YYDEBUG' is set.  */
      if (yydebug)
	{
	  YYFPRINTF (stderr, "Next token is %d (%s",
		     yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise
	     meaning of a token, for further debugging info.  */
# ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
# endif
	  YYFPRINTF (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %d (%s), ",
	      yychar, yytname[yychar1]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#if YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to the semantic value of
     the lookahead token.  This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

#if YYLSP_NEEDED
  /* Similarly for the default location.  Let the user run additional
     commands if for instance locations are ranges.  */
  yyloc = yylsp[1-yylen];
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
#endif

#if YYDEBUG
  /* We have to keep this `#if YYDEBUG', since we use variables which
     are defined only if `YYDEBUG' is set.  */
  if (yydebug)
    {
      int yyi;

      YYFPRINTF (stderr, "Reducing via rule %d (line %d), ",
		 yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (yyi = yyprhs[yyn]; yyrhs[yyi] > 0; yyi++)
	YYFPRINTF (stderr, "%s ", yytname[yyrhs[yyi]]);
      YYFPRINTF (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif

  switch (yyn) {

case 7:
#line 264 "ps2arb_gram.y"
{	
	iLanguage->AddDecl(new DeclareRegisterUsage(createPS2Register(yyvsp[0].reg)));
;
    break;}
case 8:
#line 268 "ps2arb_gram.y"
{
	Symbol::TEXTARGET tt=Symbol::UNSPECIFIED;
	switch (yyvsp[-1].s[4]) {
           case '2':
              tt=Symbol::TEX2D;
	   break;
	   case '1':
              tt=Symbol::TEX1D;
	   break;
	   case 'c':
              tt=Symbol::CUBE;
	   break;
	   case '3':
	   case 'v':
	   default:
              tt=Symbol::TEX3D;	
	   break;
        }

	Symbol s;
	s.Set(Symbol::SAMPLE,atoi(yyvsp[0].s+1),tt);
	iLanguage->SpecifySymbol(yyvsp[0].s,s);
	YYSTYPE tmp;
        tmp.reg.reg=yyvsp[0].s; 
        tmp.reg.swizzlemask="";
        tmp.reg.negate="";
	iLanguage->AddDecl(
            new DeclareSampleRegister(
               createPS2Register(tmp.reg)));
;
    break;}
case 9:
#line 299 "ps2arb_gram.y"
{       
        YYSTYPE tmp;
        tmp.reg.reg=yyvsp[-8].s; 
        tmp.reg.swizzlemask="";
        tmp.reg.negate="";
	iLanguage->AddDecl(
           new DefineConstantRegister(
             createPS2Register(tmp.reg),
		yyvsp[-6].f, yyvsp[-4].f, yyvsp[-2].f, yyvsp[0].f));
;
    break;}
case 15:
#line 311 "ps2arb_gram.y"
{;
    break;}
case 16:
#line 314 "ps2arb_gram.y"
{
        iLanguage->AddInst((*trinary_factory[yyvsp[-8].s])(
	     AdjustInstructionFlags(yyvsp[-7].s),
             createPS2Register(yyvsp[-6].reg),
             createPS2Register(yyvsp[-4].reg),
             createPS2Register(yyvsp[-2].reg),
             createPS2Register(yyvsp[0].reg)));
;
    break;}
case 17:
#line 324 "ps2arb_gram.y"
{
        iLanguage->AddInst((*binary_factory[yyvsp[-6].s])(
             AdjustInstructionFlags(yyvsp[-5].s),
             createPS2Register(yyvsp[-4].reg),
             createPS2Register(yyvsp[-2].reg),
             createPS2Register(yyvsp[0].reg)));
;
    break;}
case 18:
#line 333 "ps2arb_gram.y"
{
        iLanguage->AddInst((*unary_factory[yyvsp[-4].s])(
             AdjustInstructionFlags(yyvsp[-3].s),
             createPS2Register(yyvsp[-2].reg),
             createPS2Register(yyvsp[0].reg)));
;
    break;}
case 19:
#line 341 "ps2arb_gram.y"
{;
    break;}
case 20:
#line 343 "ps2arb_gram.y"
{;
    break;}
case 21:
#line 346 "ps2arb_gram.y"
{
        iLanguage->AddInst((*unary_factory[yyvsp[-5].s])(
             AdjustInstructionFlags(yyvsp[-4].s),
             createPS2Register(yyvsp[-3].reg),
             createPS2Register(yyvsp[-1].reg)));
;
    break;}
case 22:
#line 354 "ps2arb_gram.y"
{
        iLanguage->AddInst((*void_factory[yyvsp[-2].s])(
             AdjustInstructionFlags(yyvsp[-1].s),
             createPS2Register(yyvsp[0].reg)));
;
    break;}
case 23:
#line 361 "ps2arb_gram.y"
{
        iLanguage->AddInst((*unary_factory[yyvsp[-4].s])(
             AdjustInstructionFlags(yyvsp[-3].s),
             createPS2Register(yyvsp[-2].reg),
             createPS2Register(yyvsp[0].reg)));
;
    break;}
case 24:
#line 369 "ps2arb_gram.y"
{
        yyval.s=yyvsp[0].s;
;
    break;}
case 25:
#line 373 "ps2arb_gram.y"
{
        yyval.s="";
;
    break;}
case 26:
#line 378 "ps2arb_gram.y"
{
        yyval.s=yyvsp[0].s;
;
    break;}
case 27:
#line 382 "ps2arb_gram.y"
{
        yyval.s="";
;
    break;}
case 30:
#line 391 "ps2arb_gram.y"
{
        DEFINERETVAL; 
        ret.reg.reg=yyvsp[-1].s;
        ret.reg.swizzlemask=yyvsp[0].s;
        ret.reg.negate="";
        yyval.reg=ret.reg;
;
    break;}
case 31:
#line 400 "ps2arb_gram.y"
{
        DEFINERETVAL;
        ret.reg = yyvsp[0].reg;
        yyval.reg = ret.reg;
;
    break;}
case 32:
#line 406 "ps2arb_gram.y"
{
        DEFINERETVAL;
        ret.reg.reg = yyvsp[0].s;
        ret.reg.swizzlemask = "";
        ret.reg.negate = "";
        yyval.reg = ret.reg;
;
    break;}
case 33:
#line 415 "ps2arb_gram.y"
{
        DEFINERETVAL;
        ret.reg.reg=yyvsp[-1].s;
        ret.reg.swizzlemask = yyvsp[0].s;
        ret.reg.negate="";
        yyval.reg=ret.reg;
;
    break;}
case 34:
#line 424 "ps2arb_gram.y"
{
        yyval.s=yyvsp[0].s;
;
    break;}
case 35:
#line 428 "ps2arb_gram.y"
{
        yyval.s="";
;
    break;}
case 36:
#line 433 "ps2arb_gram.y"
{
        DEFINERETVAL;
        ret.reg.negate = yyvsp[-2].s;
        ret.reg.reg = yyvsp[-1].s;
        ret.reg.swizzlemask = yyvsp[0].s;
        yyval.reg=ret.reg
;
    break;}
case 42:
#line 445 "ps2arb_gram.y"
{
        iLanguage->AddCommentOrNewline(
           new Newline((int)yyvsp[-1].f));
        yyval.f = yyvsp[-1].f +yyvsp[0].f;
;
    break;}
case 43:
#line 451 "ps2arb_gram.y"
{
        iLanguage->AddCommentOrNewline(
           new Newline((int)yyvsp[0].f));
        yyval.f = yyvsp[0].f;
;
    break;}
case 44:
#line 457 "ps2arb_gram.y"
{
        iLanguage->AddCommentOrNewline(
           new Comment(yyvsp[-1].s));
        yyval.f = 1 + yyvsp[0].f;
;
    break;}
case 45:
#line 463 "ps2arb_gram.y"
{
        iLanguage->AddCommentOrNewline(
          new Comment(yyvsp[0].s));
        yyval.f = 1; 
;
    break;}
case 46:
#line 470 "ps2arb_gram.y"
{
        iLanguage->AddCommentOrNewline(new Comment(yyvsp[0].s));
;
    break;}
case 48:
#line 477 "ps2arb_gram.y"
{
        yyval.f = yyvsp[0].f;
;
    break;}
case 49:
#line 481 "ps2arb_gram.y"
{
        yyval.f = 0;
;
    break;}
}

#line 705 "/usr/share/bison/bison.simple"


  yyvsp -= yylen;
  yyssp -= yylen;
#if YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "state stack now");
      while (yyssp1 != yyssp)
	YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;
#if YYLSP_NEEDED
  *++yylsp = yyloc;
#endif

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("parse error, unexpected ") + 1;
	  yysize += yystrlen (yytname[YYTRANSLATE (yychar)]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "parse error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[YYTRANSLATE (yychar)]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx)
		      {
			const char *yyq = ! yycount ? ", expecting " : " or ";
			yyp = yystpcpy (yyp, yyq);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yycount++;
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exhausted");
	}
      else
#endif /* defined (YYERROR_VERBOSE) */
	yyerror ("parse error");
    }
  goto yyerrlab1;


/*--------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action |
`--------------------------------------------------*/
yyerrlab1:
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;
      YYDPRINTF ((stderr, "Discarding token %d (%s).\n",
		  yychar, yytname[yychar1]));
      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;


/*-------------------------------------------------------------------.
| yyerrdefault -- current state does not do anything special for the |
| error token.                                                       |
`-------------------------------------------------------------------*/
yyerrdefault:
#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */

  /* If its default is to accept any token, ok.  Otherwise pop it.  */
  yyn = yydefact[yystate];
  if (yyn)
    goto yydefault;
#endif


/*---------------------------------------------------------------.
| yyerrpop -- pop the current state because it cannot handle the |
| error token                                                    |
`---------------------------------------------------------------*/
yyerrpop:
  if (yyssp == yyss)
    YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#if YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "Error: state stack now");
      while (yyssp1 != yyssp)
	YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

/*--------------.
| yyerrhandle.  |
`--------------*/
yyerrhandle:
  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;
#if YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

/*---------------------------------------------.
| yyoverflowab -- parser overflow comes here.  |
`---------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}
#line 485 "ps2arb_gram.y"





