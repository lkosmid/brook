/* A Bison parser, made by GNU Bison 1.875b.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.

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

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     IDENT = 258,
     TAG_NAME = 259,
     LABEL_NAME = 260,
     TYPEDEF_NAME = 261,
     STRING = 262,
     LSTRING = 263,
     CHAR_CONST = 264,
     LCHAR_CONST = 265,
     INUM = 266,
     RNUM = 267,
     PP_DIR = 268,
     PP_LINE = 269,
     INVALID = 270,
     CONST = 271,
     VOLATILE = 272,
     OUT = 273,
     REDUCE = 274,
     VOUT = 275,
     ITER = 276,
     KERNEL = 277,
     AUTO = 278,
     EXTRN = 279,
     REGISTR = 280,
     STATIC = 281,
     TYPEDEF = 282,
     VOID = 283,
     CHAR = 284,
     SHORT = 285,
     INT = 286,
     LONG = 287,
     DOUBLE = 288,
     SGNED = 289,
     UNSGNED = 290,
     FLOAT = 291,
     FLOAT2 = 292,
     FLOAT3 = 293,
     FLOAT4 = 294,
     ENUM = 295,
     STRUCT = 296,
     UNION = 297,
     BREAK = 298,
     CASE = 299,
     CONT = 300,
     DEFLT = 301,
     DO = 302,
     ELSE = 303,
     IF = 304,
     FOR = 305,
     GOTO = 306,
     RETURN = 307,
     SWITCH = 308,
     WHILE = 309,
     PLUS_EQ = 310,
     MINUS_EQ = 311,
     STAR_EQ = 312,
     DIV_EQ = 313,
     MOD_EQ = 314,
     B_AND_EQ = 315,
     B_OR_EQ = 316,
     B_XOR_EQ = 317,
     L_SHIFT_EQ = 318,
     R_SHIFT_EQ = 319,
     EQUAL = 320,
     LESS_EQ = 321,
     GRTR_EQ = 322,
     NOT_EQ = 323,
     RPAREN = 324,
     RBRCKT = 325,
     LBRACE = 326,
     RBRACE = 327,
     SEMICOLON = 328,
     COMMA = 329,
     ELLIPSIS = 330,
     LB_SIGN = 331,
     DOUB_LB_SIGN = 332,
     BACKQUOTE = 333,
     AT = 334,
     ATTRIBUTE = 335,
     ALIGNED = 336,
     PACKED = 337,
     CDECL = 338,
     MODE = 339,
     FORMAT = 340,
     NORETURN = 341,
     COMMA_OP = 342,
     ASSIGN = 343,
     EQ = 344,
     COMMA_SEP = 345,
     COLON = 346,
     QUESTMARK = 347,
     OR = 348,
     AND = 349,
     B_OR = 350,
     B_XOR = 351,
     B_AND = 352,
     COMP_EQ = 353,
     GRTR = 354,
     LESS = 355,
     COMP_GRTR = 356,
     COMP_LESS = 357,
     COMP_ARITH = 358,
     R_SHIFT = 359,
     L_SHIFT = 360,
     MINUS = 361,
     PLUS = 362,
     MOD = 363,
     DIV = 364,
     STAR = 365,
     CAST = 366,
     DECR = 367,
     INCR = 368,
     INDEXOF = 369,
     SIZEOF = 370,
     B_NOT = 371,
     NOT = 372,
     UNARY = 373,
     HYPERUNARY = 374,
     LBRCKT = 375,
     LPAREN = 376,
     DOT = 377,
     ARROW = 378
   };
#endif
#define IDENT 258
#define TAG_NAME 259
#define LABEL_NAME 260
#define TYPEDEF_NAME 261
#define STRING 262
#define LSTRING 263
#define CHAR_CONST 264
#define LCHAR_CONST 265
#define INUM 266
#define RNUM 267
#define PP_DIR 268
#define PP_LINE 269
#define INVALID 270
#define CONST 271
#define VOLATILE 272
#define OUT 273
#define REDUCE 274
#define VOUT 275
#define ITER 276
#define KERNEL 277
#define AUTO 278
#define EXTRN 279
#define REGISTR 280
#define STATIC 281
#define TYPEDEF 282
#define VOID 283
#define CHAR 284
#define SHORT 285
#define INT 286
#define LONG 287
#define DOUBLE 288
#define SGNED 289
#define UNSGNED 290
#define FLOAT 291
#define FLOAT2 292
#define FLOAT3 293
#define FLOAT4 294
#define ENUM 295
#define STRUCT 296
#define UNION 297
#define BREAK 298
#define CASE 299
#define CONT 300
#define DEFLT 301
#define DO 302
#define ELSE 303
#define IF 304
#define FOR 305
#define GOTO 306
#define RETURN 307
#define SWITCH 308
#define WHILE 309
#define PLUS_EQ 310
#define MINUS_EQ 311
#define STAR_EQ 312
#define DIV_EQ 313
#define MOD_EQ 314
#define B_AND_EQ 315
#define B_OR_EQ 316
#define B_XOR_EQ 317
#define L_SHIFT_EQ 318
#define R_SHIFT_EQ 319
#define EQUAL 320
#define LESS_EQ 321
#define GRTR_EQ 322
#define NOT_EQ 323
#define RPAREN 324
#define RBRCKT 325
#define LBRACE 326
#define RBRACE 327
#define SEMICOLON 328
#define COMMA 329
#define ELLIPSIS 330
#define LB_SIGN 331
#define DOUB_LB_SIGN 332
#define BACKQUOTE 333
#define AT 334
#define ATTRIBUTE 335
#define ALIGNED 336
#define PACKED 337
#define CDECL 338
#define MODE 339
#define FORMAT 340
#define NORETURN 341
#define COMMA_OP 342
#define ASSIGN 343
#define EQ 344
#define COMMA_SEP 345
#define COLON 346
#define QUESTMARK 347
#define OR 348
#define AND 349
#define B_OR 350
#define B_XOR 351
#define B_AND 352
#define COMP_EQ 353
#define GRTR 354
#define LESS 355
#define COMP_GRTR 356
#define COMP_LESS 357
#define COMP_ARITH 358
#define R_SHIFT 359
#define L_SHIFT 360
#define MINUS 361
#define PLUS 362
#define MOD 363
#define DIV 364
#define STAR 365
#define CAST 366
#define DECR 367
#define INCR 368
#define INDEXOF 369
#define SIZEOF 370
#define B_NOT 371
#define NOT 372
#define UNARY 373
#define HYPERUNARY 374
#define LBRCKT 375
#define LPAREN 376
#define DOT 377
#define ARROW 378




/* Copy the first part of user declarations.  */
#line 1 "gram.y"

 /*
 ======================================================================

    CTool Library
    Copyright (C) 1995-2001	Shaun Flisakowski

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 ======================================================================
 */

/* grammar File for C - Shaun Flisakowski and Patrick Baudin */
/* Grammar was constructed with the assistance of:
    "C - A Reference Manual" (Fourth Edition),
    by Samuel P Harbison, and Guy L Steele Jr. */

#ifdef    NO_ALLOCA
#define   alloca  __builtin_alloca
#endif

#ifdef _WIN32
/* Don't complain about switch() statements that only have a 'default' */
#pragma warning( disable : 4065 )
#endif

#include <stdio.h>
#include <errno.h>
#include <setjmp.h>

#include "lexer.h"
#include "symbol.h"
#include "token.h"
#include "stemnt.h"
#include "location.h"
#include "project.h"
#include "brtexpress.h"
extern int err_cnt;
int yylex(YYSTYPE *lvalp);

extern int err_top_level;
/* Cause the `yydebug' variable to be defined.  */
#define YYDEBUG 1
void baseTypeFixup(BaseType * bt,Decl * decl) {
  BaseType * b = decl->form->getBase();
  while ((decl=decl->next)) {
    BaseType *nb = decl->form->getBase();
    if (memcmp(nb,b,sizeof(BaseType))!=0) {
      decl->form = decl->form->dup();
      *decl->form->getBase()=*b;
    }
  }

}
/*  int  yydebug = 1;  */

/* ###################################################### */
#line 232 "gram.y"

/* 1 if we explained undeclared var errors.  */
/*  static int undeclared_variable_notice = 0;  */


/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 408 "gram.tab.c"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

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
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAXIMUM)

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
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  71
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1527

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  124
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  202
/* YYNRULES -- Number of rules. */
#define YYNRULES  394
/* YYNRULES -- Number of states. */
#define YYNSTATES  589

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   378

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
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
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     4,     6,     8,    11,    15,    16,    18,
      20,    22,    24,    27,    31,    34,    38,    42,    43,    49,
      52,    53,    55,    58,    59,    62,    65,    67,    72,    75,
      76,    78,    81,    82,    85,    88,    90,    92,    94,    96,
      98,   100,   102,   104,   106,   108,   110,   112,   114,   117,
     120,   123,   127,   129,   131,   133,   135,   137,   143,   146,
     149,   153,   157,   159,   165,   173,   181,   187,   197,   199,
     201,   203,   205,   208,   213,   215,   217,   223,   225,   229,
     233,   234,   236,   238,   239,   241,   243,   245,   249,   251,
     255,   258,   260,   264,   266,   270,   272,   276,   279,   281,
     286,   288,   292,   294,   298,   300,   304,   306,   310,   312,
     316,   323,   332,   343,   348,   353,   358,   360,   362,   369,
     374,   376,   378,   380,   382,   384,   386,   388,   390,   392,
     394,   399,   402,   405,   410,   413,   416,   419,   422,   425,
     428,   430,   434,   436,   438,   440,   444,   448,   450,   452,
     454,   456,   458,   460,   462,   467,   469,   471,   474,   477,
     479,   483,   487,   492,   493,   495,   497,   499,   503,   507,
     509,   511,   513,   515,   517,   519,   521,   523,   525,   527,
     529,   531,   533,   535,   537,   539,   541,   543,   545,   546,
     548,   549,   550,   554,   555,   557,   558,   559,   563,   566,
     570,   573,   576,   579,   582,   583,   585,   587,   588,   591,
     593,   596,   598,   600,   602,   604,   606,   608,   610,   612,
     614,   615,   617,   620,   621,   625,   628,   629,   631,   632,
     636,   639,   641,   642,   646,   648,   652,   653,   655,   657,
     659,   663,   665,   667,   671,   673,   675,   677,   682,   683,
     685,   687,   689,   691,   693,   695,   697,   699,   704,   706,
     709,   710,   712,   714,   716,   718,   720,   722,   724,   726,
     728,   730,   732,   734,   736,   738,   740,   742,   744,   746,
     748,   750,   752,   754,   757,   760,   763,   766,   771,   776,
     779,   784,   789,   792,   797,   802,   803,   805,   806,   809,
     810,   812,   814,   816,   820,   822,   826,   828,   829,   830,
     834,   837,   840,   844,   847,   849,   851,   852,   856,   857,
     863,   865,   867,   869,   870,   875,   877,   878,   880,   882,
     884,   887,   889,   891,   893,   897,   899,   901,   906,   911,
     915,   920,   925,   927,   929,   933,   935,   936,   938,   942,
     945,   947,   950,   952,   953,   956,   958,   962,   964,   966,
     968,   970,   971,   972,   975,   976,   979,   981,   985,   987,
     991,   992,   995,   998,  1001,  1003,  1005,  1007,  1010,  1012,
    1016,  1020,  1025,  1029,  1034,  1035,  1037,  1044,  1045,  1047,
    1049,  1051,  1053,  1058,  1063
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const short yyrhs[] =
{
     125,     0,    -1,    -1,   126,    -1,     1,    -1,   128,   127,
      -1,   126,   128,   127,    -1,    -1,   224,    -1,   129,    -1,
      13,    -1,    14,    -1,     1,    73,    -1,     1,    72,   127,
      -1,   130,   131,    -1,   228,   294,   215,    -1,   227,   293,
     215,    -1,    -1,    71,   132,   219,   133,    72,    -1,     1,
      72,    -1,    -1,   134,    -1,   142,   135,    -1,    -1,   135,
     136,    -1,   135,    14,    -1,   141,    -1,    71,   219,   138,
      72,    -1,     1,    72,    -1,    -1,   139,    -1,   142,   140,
      -1,    -1,   140,   141,    -1,   140,    14,    -1,   142,    -1,
     143,    -1,   144,    -1,   145,    -1,   137,    -1,   146,    -1,
     147,    -1,   148,    -1,   149,    -1,   150,    -1,   151,    -1,
     152,    -1,   153,    -1,     1,    73,    -1,   182,    73,    -1,
     168,    73,    -1,   159,    91,   141,    -1,   154,    -1,   155,
      -1,   156,    -1,   157,    -1,   158,    -1,    53,   121,   168,
      69,   141,    -1,    43,    73,    -1,    45,    73,    -1,    52,
     167,    73,    -1,    51,     5,    73,    -1,    73,    -1,    49,
     121,   168,    69,   141,    -1,    49,   121,   168,    69,   141,
      48,   141,    -1,    47,   141,    54,   121,   168,    69,    73,
      -1,    54,   121,   168,    69,   141,    -1,    50,   121,   167,
      73,   167,    73,   167,    69,   141,    -1,   160,    -1,   161,
      -1,   162,    -1,   308,    -1,    44,   166,    -1,    44,   166,
      75,   166,    -1,    46,    -1,   169,    -1,   169,    92,   168,
      91,   163,    -1,   163,    -1,   185,   213,   164,    -1,   185,
     213,   182,    -1,    -1,   166,    -1,   168,    -1,    -1,   168,
      -1,   194,    -1,   170,    -1,   169,    93,   170,    -1,   172,
      -1,   170,    94,   172,    -1,   117,   176,    -1,   173,    -1,
     172,    95,   173,    -1,   174,    -1,   173,    96,   174,    -1,
     177,    -1,   174,    97,   177,    -1,   116,   176,    -1,   185,
      -1,   121,   232,    69,   176,    -1,   178,    -1,   177,   210,
     178,    -1,   179,    -1,   178,   211,   179,    -1,   180,    -1,
     179,   212,   180,    -1,   181,    -1,   180,   208,   181,    -1,
     176,    -1,   181,   209,   176,    -1,    37,   121,   164,    74,
     164,    69,    -1,    38,   121,   164,    74,   164,    74,   164,
      69,    -1,    39,   121,   164,    74,   164,    74,   164,    74,
     164,    69,    -1,    37,   121,     1,    69,    -1,    38,   121,
       1,    69,    -1,    39,   121,     1,    69,    -1,   164,    -1,
     182,    -1,    21,   121,   183,    74,   183,    69,    -1,    21,
     121,     1,    69,    -1,   197,    -1,   186,    -1,   188,    -1,
     189,    -1,   171,    -1,   175,    -1,   190,    -1,   191,    -1,
     192,    -1,   193,    -1,   115,   121,   230,    69,    -1,   115,
     185,    -1,   114,   308,    -1,   114,   121,   308,    69,    -1,
     106,   176,    -1,   107,   176,    -1,    97,   176,    -1,   110,
     176,    -1,   113,   185,    -1,   112,   185,    -1,   164,    -1,
     194,    74,   164,    -1,   308,    -1,   196,    -1,   214,    -1,
     121,   168,    69,    -1,   121,     1,    69,    -1,   195,    -1,
     198,    -1,   199,    -1,   205,    -1,   200,    -1,   201,    -1,
     187,    -1,   197,   120,   168,    70,    -1,   203,    -1,   204,
      -1,   197,   113,    -1,   197,   112,    -1,   310,    -1,   197,
     122,   202,    -1,   197,   123,   202,    -1,   197,   121,   206,
      69,    -1,    -1,   207,    -1,   164,    -1,   182,    -1,   207,
      74,   164,    -1,   207,    74,   182,    -1,   107,    -1,   106,
      -1,   110,    -1,   109,    -1,   108,    -1,    98,    -1,   103,
      -1,   102,    -1,   101,    -1,   105,    -1,   104,    -1,    89,
      -1,    88,    -1,    11,    -1,    12,    -1,     9,    -1,    10,
      -1,     7,    -1,     8,    -1,    -1,   216,    -1,    -1,    -1,
     217,   218,   223,    -1,    -1,   220,    -1,    -1,    -1,   221,
     222,   223,    -1,   226,    73,    -1,   226,    73,   223,    -1,
     225,    73,    -1,   226,    73,    -1,   227,   248,    -1,   228,
     248,    -1,    -1,   233,    -1,   320,    -1,    -1,   231,   232,
      -1,   233,    -1,   233,   229,    -1,   239,    -1,    24,    -1,
      26,    -1,    27,    -1,    23,    -1,    25,    -1,   234,    -1,
     235,    -1,   259,    -1,    -1,   239,    -1,   236,   238,    -1,
      -1,   237,   240,   238,    -1,   255,   238,    -1,    -1,   242,
      -1,    -1,   259,   243,   241,    -1,   255,   241,    -1,   242,
      -1,    -1,   293,   246,   323,    -1,   245,    -1,   245,    89,
     251,    -1,    -1,   249,    -1,   250,    -1,   247,    -1,   250,
      74,   247,    -1,   253,    -1,   253,    -1,   252,    74,   253,
      -1,   164,    -1,   182,    -1,   184,    -1,    71,   252,   254,
      72,    -1,    -1,    74,    -1,   256,    -1,    16,    -1,    17,
      -1,    18,    -1,    19,    -1,    21,    -1,    22,    -1,    20,
     120,   165,    70,    -1,   256,    -1,   257,   256,    -1,    -1,
     257,    -1,   270,    -1,   266,    -1,   268,    -1,   264,    -1,
     262,    -1,   263,    -1,   260,    -1,    28,    -1,    29,    -1,
      30,    -1,    31,    -1,    32,    -1,    36,    -1,    37,    -1,
      38,    -1,    39,    -1,    33,    -1,    34,    -1,    35,    -1,
       6,    -1,     4,    -1,    41,   261,    -1,    42,   261,    -1,
      40,   261,    -1,    41,   261,    -1,    41,    71,   271,    72,
      -1,   265,    71,   271,    72,    -1,    42,   261,    -1,    42,
      71,   271,    72,    -1,   267,    71,   271,    72,    -1,    40,
     261,    -1,    40,    71,   272,    72,    -1,   269,    71,   272,
      72,    -1,    -1,   278,    -1,    -1,   274,   273,    -1,    -1,
      74,    -1,   275,    -1,   276,    -1,   274,    74,   276,    -1,
     277,    -1,   277,    89,   164,    -1,   310,    -1,    -1,    -1,
     279,   280,   281,    -1,   282,    73,    -1,   281,    73,    -1,
     281,   282,    73,    -1,   244,   283,    -1,   244,    -1,   284,
      -1,    -1,   285,   287,   323,    -1,    -1,   284,    74,   286,
     287,   323,    -1,   288,    -1,   289,    -1,   293,    -1,    -1,
     292,    91,   290,   291,    -1,   163,    -1,    -1,   293,    -1,
     295,    -1,   295,    -1,   304,   296,    -1,   296,    -1,   297,
      -1,   308,    -1,   121,   295,    69,    -1,   298,    -1,   299,
      -1,   297,   121,   313,    69,    -1,   297,   121,   305,    69,
      -1,   297,   121,    69,    -1,   297,   120,   165,    70,    -1,
     297,   102,   301,   101,    -1,   214,    -1,   205,    -1,   121,
     168,    69,    -1,   308,    -1,    -1,   300,    -1,   301,    74,
     300,    -1,   110,   258,    -1,   302,    -1,   303,   302,    -1,
     303,    -1,    -1,   306,   307,    -1,   308,    -1,   307,    74,
     308,    -1,     3,    -1,     6,    -1,   308,    -1,   309,    -1,
      -1,    -1,   312,   315,    -1,    -1,   314,   315,    -1,   316,
      -1,   316,    74,    75,    -1,   317,    -1,   316,    74,   317,
      -1,    -1,   318,   319,    -1,   233,   293,    -1,   233,   320,
      -1,   233,    -1,   304,    -1,   321,    -1,   304,   321,    -1,
     322,    -1,   121,   320,    69,    -1,   120,   165,    70,    -1,
     322,   120,   165,    70,    -1,   121,   311,    69,    -1,   322,
     121,   311,    69,    -1,    -1,   324,    -1,    80,   121,   121,
     325,    69,    69,    -1,    -1,    82,    -1,    83,    -1,    16,
      -1,    86,    -1,    81,   121,    11,    69,    -1,    84,   121,
     308,    69,    -1,    85,   121,   308,    74,    11,    74,    11,
      69,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   244,   244,   251,   262,   270,   275,   282,   290,   294,
     298,   302,   306,   310,   316,   344,   373,   407,   406,   424,
     431,   434,   437,   456,   459,   468,   484,   487,   497,   504,
     507,   510,   529,   532,   541,   552,   556,   562,   563,   564,
     565,   566,   567,   568,   569,   570,   571,   572,   573,   596,
     603,   610,   624,   625,   628,   629,   630,   633,   642,   650,
     658,   666,   674,   681,   690,   700,   711,   720,   732,   733,
     734,   737,   745,   750,   758,   774,   775,   783,   784,   788,
     795,   798,   801,   805,   808,   811,   814,   815,   822,   823,
     830,   837,   838,   845,   846,   853,   854,   861,   868,   869,
     877,   878,   884,   885,   891,   892,   898,   899,   905,   906,
     912,   920,   930,   941,   945,   949,   955,   956,   959,   976,
     982,   983,   984,   985,   986,   987,   988,   989,   990,   991,
     994,  1001,  1008,  1012,  1032,  1038,  1045,  1052,  1058,  1065,
    1072,  1073,  1080,  1084,  1085,  1091,  1097,  1105,  1106,  1107,
    1108,  1109,  1110,  1111,  1115,  1123,  1124,  1127,  1134,  1141,
    1145,  1168,  1191,  1205,  1208,  1211,  1212,  1213,  1220,  1229,
    1230,  1233,  1234,  1235,  1238,  1241,  1242,  1243,  1246,  1247,
    1250,  1251,  1254,  1255,  1256,  1257,  1258,  1259,  1268,  1268,
    1279,  1284,  1279,  1300,  1300,  1310,  1315,  1310,  1323,  1328,
    1341,  1348,  1356,  1374,  1400,  1406,  1415,  1419,  1419,  1430,
    1439,  1462,  1468,  1469,  1470,  1473,  1474,  1477,  1478,  1488,
    1492,  1495,  1498,  1514,  1514,  1546,  1567,  1570,  1573,  1573,
    1583,  1598,  1608,  1607,  1618,  1619,  1627,  1630,  1633,  1636,
    1640,  1656,  1658,  1663,  1671,  1672,  1673,  1674,  1683,  1686,
    1698,  1701,  1702,  1703,  1704,  1705,  1706,  1707,  1715,  1716,
    1725,  1728,  1735,  1736,  1737,  1738,  1739,  1740,  1741,  1742,
    1743,  1744,  1745,  1746,  1747,  1748,  1749,  1750,  1751,  1752,
    1753,  1756,  1763,  1771,  1781,  1791,  1801,  1811,  1818,  1842,
    1852,  1860,  1885,  1895,  1902,  1926,  1929,  1933,  1936,  1942,
    1945,  1955,  1958,  1963,  1971,  1983,  1997,  2004,  2009,  2004,
    2023,  2029,  2037,  2045,  2050,  2061,  2065,  2065,  2073,  2072,
    2084,  2102,  2113,  2117,  2116,  2136,  2140,  2143,  2150,  2156,
    2162,  2167,  2170,  2173,  2178,  2184,  2185,  2186,  2202,  2218,
    2242,  2256,  2267,  2271,  2275,  2279,  2286,  2289,  2293,  2303,
    2309,  2310,  2317,  2324,  2324,  2339,  2344,  2356,  2360,  2372,
    2373,  2382,  2386,  2386,  2395,  2395,  2408,  2409,  2418,  2419,
    2427,  2427,  2437,  2453,  2470,  2483,  2487,  2491,  2498,  2501,
    2505,  2509,  2519,  2523,  2540,  2543,  2546,  2557,  2561,  2565,
    2569,  2573,  2577,  2591,  2600
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "IDENT", "TAG_NAME", "LABEL_NAME", 
  "TYPEDEF_NAME", "STRING", "LSTRING", "CHAR_CONST", "LCHAR_CONST", 
  "INUM", "RNUM", "PP_DIR", "PP_LINE", "INVALID", "CONST", "VOLATILE", 
  "OUT", "REDUCE", "VOUT", "ITER", "KERNEL", "AUTO", "EXTRN", "REGISTR", 
  "STATIC", "TYPEDEF", "VOID", "CHAR", "SHORT", "INT", "LONG", "DOUBLE", 
  "SGNED", "UNSGNED", "FLOAT", "FLOAT2", "FLOAT3", "FLOAT4", "ENUM", 
  "STRUCT", "UNION", "BREAK", "CASE", "CONT", "DEFLT", "DO", "ELSE", "IF", 
  "FOR", "GOTO", "RETURN", "SWITCH", "WHILE", "PLUS_EQ", "MINUS_EQ", 
  "STAR_EQ", "DIV_EQ", "MOD_EQ", "B_AND_EQ", "B_OR_EQ", "B_XOR_EQ", 
  "L_SHIFT_EQ", "R_SHIFT_EQ", "EQUAL", "LESS_EQ", "GRTR_EQ", "NOT_EQ", 
  "RPAREN", "RBRCKT", "LBRACE", "RBRACE", "SEMICOLON", "COMMA", 
  "ELLIPSIS", "LB_SIGN", "DOUB_LB_SIGN", "BACKQUOTE", "AT", "ATTRIBUTE", 
  "ALIGNED", "PACKED", "CDECL", "MODE", "FORMAT", "NORETURN", "COMMA_OP", 
  "ASSIGN", "EQ", "COMMA_SEP", "COLON", "QUESTMARK", "OR", "AND", "B_OR", 
  "B_XOR", "B_AND", "COMP_EQ", "GRTR", "LESS", "COMP_GRTR", "COMP_LESS", 
  "COMP_ARITH", "R_SHIFT", "L_SHIFT", "MINUS", "PLUS", "MOD", "DIV", 
  "STAR", "CAST", "DECR", "INCR", "INDEXOF", "SIZEOF", "B_NOT", "NOT", 
  "UNARY", "HYPERUNARY", "LBRCKT", "LPAREN", "DOT", "ARROW", "$accept", 
  "program", "trans_unit", "top_level_exit", "top_level_decl", "func_def", 
  "func_spec", "cmpnd_stemnt", "@1", "opt_stemnt_list", "stemnt_list", 
  "stemnt_list2", "stemnt", "cmpnd_stemnt_reentrance", 
  "opt_stemnt_list_reentrance", "stemnt_list_reentrance", 
  "stemnt_list_reentrance2", "stemnt_reentrance", 
  "non_constructor_stemnt", "constructor_stemnt", "expr_stemnt", 
  "labeled_stemnt", "cond_stemnt", "iter_stemnt", "switch_stemnt", 
  "break_stemnt", "continue_stemnt", "return_stemnt", "goto_stemnt", 
  "null_stemnt", "if_stemnt", "if_else_stemnt", "do_stemnt", 
  "while_stemnt", "for_stemnt", "label", "named_label", "case_label", 
  "deflt_label", "cond_expr", "assign_expr", "opt_const_expr", 
  "const_expr", "opt_expr", "expr", "log_or_expr", "log_and_expr", 
  "log_neg_expr", "bitwise_or_expr", "bitwise_xor_expr", 
  "bitwise_and_expr", "bitwise_neg_expr", "cast_expr", "equality_expr", 
  "relational_expr", "shift_expr", "additive_expr", "mult_expr", 
  "constructor_expr", "iter_constructor_arg", "iter_constructor_expr", 
  "unary_expr", "sizeof_expr", "indexof_expr", "unary_minus_expr", 
  "unary_plus_expr", "addr_expr", "indirection_expr", "preinc_expr", 
  "predec_expr", "comma_expr", "prim_expr", "paren_expr", "postfix_expr", 
  "subscript_expr", "comp_select_expr", "postinc_expr", "postdec_expr", 
  "field_ident", "direct_comp_select", "indirect_comp_select", 
  "func_call", "opt_expr_list", "expr_list", "add_op", "mult_op", 
  "equality_op", "relation_op", "shift_op", "assign_op", "constant", 
  "opt_KnR_declaration_list", "@2", "@3", "@4", "opt_declaration_list", 
  "@5", "@6", "@7", "declaration_list", "decl_stemnt", 
  "old_style_declaration", "declaration", "no_decl_specs", "decl_specs", 
  "abs_decl", "type_name", "@8", "type_name_bis", 
  "decl_specs_reentrance_bis", "local_or_global_storage_class", 
  "local_storage_class", "storage_class", "type_spec", 
  "opt_decl_specs_reentrance", "decl_specs_reentrance", "@9", 
  "opt_comp_decl_specs", "comp_decl_specs_reentrance", "@10", 
  "comp_decl_specs", "decl", "@11", "init_decl", "opt_init_decl_list", 
  "init_decl_list", "init_decl_list_reentrance", "initializer", 
  "initializer_list", "initializer_reentrance", "opt_comma", "type_qual", 
  "type_qual_token", "type_qual_list", "opt_type_qual_list", 
  "type_spec_reentrance", "typedef_name", "tag_ref", "struct_tag_ref", 
  "union_tag_ref", "enum_tag_ref", "struct_tag_def", "struct_type_define", 
  "union_tag_def", "union_type_define", "enum_tag_def", 
  "enum_type_define", "struct_or_union_definition", "enum_definition", 
  "opt_trailing_comma", "enum_def_list", "enum_def_list_reentrance", 
  "enum_const_def", "enum_constant", "field_list", "@12", "@13", 
  "field_list_reentrance", "comp_decl", "comp_decl_list", 
  "comp_decl_list_reentrance", "@14", "@15", "comp_declarator", 
  "simple_comp", "bit_field", "@16", "width", "opt_declarator", 
  "declarator", "func_declarator", "declarator_reentrance_bis", 
  "direct_declarator_reentrance_bis", "direct_declarator_reentrance", 
  "array_decl", "stream_decl", "dimension_constraint", "comma_constants", 
  "pointer_start", "pointer_reentrance", "pointer", "ident_list", "@17", 
  "ident_list_reentrance", "ident", "typename_as_ident", "any_ident", 
  "opt_param_type_list", "@18", "param_type_list", "@19", 
  "param_type_list_bis", "param_list", "param_decl", "@20", 
  "param_decl_bis", "abs_decl_reentrance", 
  "direct_abs_decl_reentrance_bis", "direct_abs_decl_reentrance", 
  "opt_gcc_attrib", "gcc_attrib", "gcc_inner", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned short yyr1[] =
{
       0,   124,   125,   125,   125,   126,   126,   127,   128,   128,
     128,   128,   128,   128,   129,   130,   130,   132,   131,   131,
     133,   133,   134,   135,   135,   135,   136,   137,   137,   138,
     138,   139,   140,   140,   140,   141,   141,   142,   142,   142,
     142,   142,   142,   142,   142,   142,   142,   142,   142,   143,
     144,   145,   146,   146,   147,   147,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   156,   157,   158,   159,   159,
     159,   160,   161,   161,   162,   163,   163,   164,   164,   164,
     165,   165,   166,   167,   167,   168,   169,   169,   170,   170,
     171,   172,   172,   173,   173,   174,   174,   175,   176,   176,
     177,   177,   178,   178,   179,   179,   180,   180,   181,   181,
     182,   182,   182,   182,   182,   182,   183,   183,   184,   184,
     185,   185,   185,   185,   185,   185,   185,   185,   185,   185,
     186,   186,   187,   187,   188,   189,   190,   191,   192,   193,
     194,   194,   195,   195,   195,   196,   196,   197,   197,   197,
     197,   197,   197,   197,   198,   199,   199,   200,   201,   202,
     203,   204,   205,   206,   206,   207,   207,   207,   207,   208,
     208,   209,   209,   209,   210,   211,   211,   211,   212,   212,
     213,   213,   214,   214,   214,   214,   214,   214,   216,   215,
     217,   218,   215,   220,   219,   221,   222,   219,   223,   223,
     224,   224,   225,   226,   227,   228,   229,   231,   230,   232,
     232,   233,   234,   234,   234,   235,   235,   236,   236,   237,
     238,   238,   239,   240,   239,   239,   241,   241,   243,   242,
     242,   244,   246,   245,   247,   247,   248,   248,   249,   250,
     250,   251,   252,   252,   253,   253,   253,   253,   254,   254,
     255,   256,   256,   256,   256,   256,   256,   256,   257,   257,
     258,   258,   259,   259,   259,   259,   259,   259,   259,   259,
     259,   259,   259,   259,   259,   259,   259,   259,   259,   259,
     259,   260,   261,   262,   263,   264,   265,   266,   266,   267,
     268,   268,   269,   270,   270,   271,   271,   272,   272,   273,
     273,   274,   275,   275,   276,   276,   277,   279,   280,   278,
     281,   281,   281,   282,   282,   283,   285,   284,   286,   284,
     287,   287,   288,   290,   289,   291,   292,   292,   293,   294,
     295,   295,   296,   297,   297,   297,   297,   297,   297,   297,
     298,   299,   300,   300,   300,   300,   301,   301,   301,   302,
     303,   303,   304,   306,   305,   307,   307,   308,   309,   310,
     310,   311,   312,   311,   314,   313,   315,   315,   316,   316,
     318,   317,   319,   319,   319,   320,   320,   320,   321,   322,
     322,   322,   322,   322,   323,   323,   324,   325,   325,   325,
     325,   325,   325,   325,   325
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     0,     1,     1,     2,     3,     0,     1,     1,
       1,     1,     2,     3,     2,     3,     3,     0,     5,     2,
       0,     1,     2,     0,     2,     2,     1,     4,     2,     0,
       1,     2,     0,     2,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     2,
       2,     3,     1,     1,     1,     1,     1,     5,     2,     2,
       3,     3,     1,     5,     7,     7,     5,     9,     1,     1,
       1,     1,     2,     4,     1,     1,     5,     1,     3,     3,
       0,     1,     1,     0,     1,     1,     1,     3,     1,     3,
       2,     1,     3,     1,     3,     1,     3,     2,     1,     4,
       1,     3,     1,     3,     1,     3,     1,     3,     1,     3,
       6,     8,    10,     4,     4,     4,     1,     1,     6,     4,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       4,     2,     2,     4,     2,     2,     2,     2,     2,     2,
       1,     3,     1,     1,     1,     3,     3,     1,     1,     1,
       1,     1,     1,     1,     4,     1,     1,     2,     2,     1,
       3,     3,     4,     0,     1,     1,     1,     3,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     0,     1,
       0,     0,     3,     0,     1,     0,     0,     3,     2,     3,
       2,     2,     2,     2,     0,     1,     1,     0,     2,     1,
       2,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       0,     1,     2,     0,     3,     2,     0,     1,     0,     3,
       2,     1,     0,     3,     1,     3,     0,     1,     1,     1,
       3,     1,     1,     3,     1,     1,     1,     4,     0,     1,
       1,     1,     1,     1,     1,     1,     1,     4,     1,     2,
       0,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     2,     2,     2,     4,     4,     2,
       4,     4,     2,     4,     4,     0,     1,     0,     2,     0,
       1,     1,     1,     3,     1,     3,     1,     0,     0,     3,
       2,     2,     3,     2,     1,     1,     0,     3,     0,     5,
       1,     1,     1,     0,     4,     1,     0,     1,     1,     1,
       2,     1,     1,     1,     3,     1,     1,     4,     4,     3,
       4,     4,     1,     1,     3,     1,     0,     1,     3,     2,
       1,     2,     1,     0,     2,     1,     3,     1,     1,     1,
       1,     0,     0,     2,     0,     2,     1,     3,     1,     3,
       0,     2,     2,     2,     1,     1,     1,     2,     1,     3,
       3,     4,     3,     4,     0,     1,     6,     0,     1,     1,
       1,     1,     4,     4,     8
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned short yydefact[] =
{
       0,     4,   281,    10,    11,   251,   252,   253,   254,     0,
     255,   256,   215,   212,   216,   213,   214,   269,   270,   271,
     272,   273,   278,   279,   280,   274,   275,   276,   277,     0,
       0,     0,     0,     0,     7,     9,     0,     8,     0,     0,
     236,   236,   205,   217,   218,   220,   223,   211,   220,   250,
     219,   268,   266,   267,   265,     0,   263,     0,   264,     0,
     262,     7,    12,    80,   282,   297,   285,   307,   283,   307,
     284,     1,     0,     7,     5,     0,    17,    14,   200,   201,
     357,   260,     0,   234,   239,   202,   237,   238,   190,   328,
     331,   332,   335,   336,   350,   352,     0,   333,   203,   232,
     190,   329,   222,   221,   220,   225,   307,   307,   297,    13,
     186,   187,   184,   185,   182,   183,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    77,   140,     0,
      81,    82,    75,    86,   124,    88,    91,    93,   125,   108,
      95,   100,   102,   104,   106,    98,   121,   153,   122,   123,
     126,   127,   128,   129,    85,   147,   143,   120,   148,   149,
     151,   152,   155,   156,   150,   144,   142,   358,     0,   299,
     301,   302,   304,   359,   360,   306,     0,   296,   308,     0,
       6,    19,   193,   258,   261,   349,     0,     0,     0,    16,
     189,   191,   384,   346,    80,   364,   351,   330,    15,   224,
       0,     0,     0,   136,    98,   134,   135,   137,     0,   139,
     138,     0,   132,     0,   131,    97,    90,     0,     0,     0,
     209,   257,     0,     0,     0,     0,     0,     0,   174,     0,
     177,   176,   175,     0,   179,   178,     0,   170,   169,     0,
     173,   172,   171,     0,   181,   180,     0,     0,   158,   157,
       0,   163,     0,     0,   293,   300,   298,     0,   287,     0,
     290,     0,   194,   196,   259,   334,     0,     0,     0,     0,
       0,   244,   245,   246,   235,   241,   240,     0,     0,   233,
     385,     0,     0,   150,   144,   347,     0,   142,     0,   339,
       0,     0,     0,   370,   288,   291,   294,     0,     0,     0,
     146,   145,     0,    80,   362,   210,   375,   206,   376,   378,
       0,    87,    89,    92,    94,    96,   101,   103,   105,   107,
     109,    78,    79,   141,     0,   165,   166,     0,   164,   160,
     159,   161,   303,   305,   231,   316,   226,   228,   309,     0,
       0,     0,     0,     0,    74,     0,     0,     0,     0,    83,
       0,     0,   193,    62,     0,    21,    39,    23,    37,    38,
      40,    41,    42,    43,    44,    45,    46,    47,    52,    53,
      54,    55,    56,     0,    68,    69,    70,     0,   142,     0,
       0,     0,     0,     0,   248,   242,   192,     0,   236,     0,
       0,     0,   341,   340,   338,   354,   355,   337,   365,   366,
     368,     0,   133,   130,   208,    99,     0,     0,   370,     0,
     377,    80,   362,     0,   154,   162,     0,   313,   315,   326,
     230,   227,   226,   311,     0,   310,    28,    48,    58,    72,
      59,     0,    35,    36,     0,     0,    83,     0,     0,    84,
       0,     0,     0,    18,     0,     0,    50,   197,     0,   116,
     117,     0,     0,     0,     0,     0,     0,     0,   249,     0,
     198,   387,   145,   348,     0,   370,   374,   371,   380,   382,
     363,   379,     0,     0,    76,   167,   168,   318,   384,   320,
     321,     0,   322,   229,   312,     0,     0,    49,     0,     0,
      61,    60,     0,     0,     0,    30,    32,    25,    24,    26,
      51,   119,     0,   113,     0,   114,     0,   115,     0,   243,
     247,   199,   390,     0,   388,   389,     0,     0,   391,     0,
     356,   367,   369,   362,   372,   375,   373,   381,   383,   326,
     317,   323,    73,     0,     0,    83,     0,     0,    27,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   384,     0,
       0,    63,     0,    57,    66,    34,    33,   118,   110,     0,
       0,     0,     0,     0,   386,   319,   325,   324,     0,     0,
      83,     0,     0,   392,   393,     0,    65,    64,     0,   111,
       0,     0,     0,     0,     0,    67,   112,     0,   394
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    32,    33,    74,    34,    35,    36,    77,   182,   354,
     355,   444,   498,   356,   494,   495,   539,   431,   432,   433,
     358,   359,   360,   361,   362,   363,   364,   365,   366,   367,
     368,   369,   370,   371,   372,   373,   374,   375,   376,   127,
     128,   129,   130,   438,   377,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   143,   144,   434,   451,
     273,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,   158,   159,   160,   161,   329,   162,
     163,   164,   327,   328,   239,   243,   229,   233,   236,   246,
     165,   189,   190,   191,   277,   261,   262,   263,   379,   386,
      37,    38,   387,    40,   388,   305,   298,   299,   219,    42,
      43,    44,    45,    46,   102,    47,   104,   420,   334,   422,
     335,    83,   192,    84,    98,    86,    87,   274,   384,   275,
     459,    48,    49,   184,   185,    50,    51,    66,    52,    53,
      54,    55,    56,    57,    58,    59,    60,   176,   168,   256,
     169,   170,   171,   172,   177,   178,   259,   338,   339,   417,
     418,   419,   529,   478,   479,   480,   549,   567,   481,    99,
     100,    89,    90,    91,    92,    93,   285,   286,    94,    95,
      96,   290,   291,   395,   166,   174,   175,   407,   408,   292,
     293,   398,   399,   400,   401,   467,   409,   308,   309,   279,
     280,   519
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -400
static const short yypact[] =
{
     434,    14,  -400,  -400,  -400,  -400,  -400,  -400,  -400,   -61,
    -400,  -400,  -400,  -400,  -400,  -400,  -400,  -400,  -400,  -400,
    -400,  -400,  -400,  -400,  -400,  -400,  -400,  -400,  -400,    79,
      80,    85,    67,   540,  -400,  -400,    36,  -400,     0,     8,
      37,    37,  -400,  -400,  -400,  1411,  -400,  -400,  1411,  -400,
    -400,  -400,  -400,  -400,  -400,     6,  -400,    39,  -400,    46,
    -400,  -400,  -400,  1170,  -400,    60,    64,    40,    89,    40,
     111,  -400,    14,  -400,  -400,    43,  -400,  -400,  -400,  -400,
    -400,   253,    37,   102,  -400,  -400,  -400,   119,    56,  -400,
    -400,   -50,  -400,  -400,  -400,   113,    48,  -400,  -400,  -400,
      75,   159,  -400,  -400,  1411,  -400,    40,    40,    60,  -400,
    -400,  -400,  -400,  -400,  -400,  -400,  1170,  1170,  1170,  1170,
    1218,  1218,    52,  1241,  1170,  1170,   598,  -400,  -400,   125,
    -400,  -400,     3,   131,  -400,   133,   123,   141,  -400,  -400,
     143,    83,    84,   124,   127,   189,  -400,  -400,  -400,  -400,
    -400,  -400,  -400,  -400,   182,  -400,  -400,   145,  -400,  -400,
    -400,  -400,  -400,  -400,  -400,  -400,  -400,  -400,   207,   224,
    -400,  -400,   198,  -400,  -400,  -400,   227,  -400,  -400,   228,
    -400,  -400,  1448,  -400,   253,  -400,   232,  1149,    37,  -400,
    -400,  -400,   222,    90,  1170,    57,  -400,  -400,  -400,  -400,
     231,   234,   235,  -400,  -400,  -400,  -400,  -400,   405,  -400,
    -400,   301,  -400,   646,  -400,  -400,  -400,   241,   242,   243,
      97,  -400,  1170,  1170,  1170,  1170,  1170,  1170,  -400,  1170,
    -400,  -400,  -400,  1170,  -400,  -400,  1170,  -400,  -400,  1170,
    -400,  -400,  -400,  1170,  -400,  -400,  1126,  1170,  -400,  -400,
    1170,  1126,    60,    60,  -400,    60,  -400,  1170,  -400,  1485,
    -400,   901,  -400,  -400,  -400,  -400,   192,   193,   194,   195,
    1149,  -400,  -400,  -400,  -400,  -400,  -400,  1411,   196,  -400,
    -400,   405,   145,     4,    93,  -400,   105,   107,   248,  -400,
     250,   301,   254,  -400,  -400,  -400,  -400,   256,   258,  1411,
    -400,  -400,  1170,  1170,    -7,  -400,   160,  -400,  -400,   163,
     230,   131,   133,   123,   141,   143,    83,    84,   124,   127,
    -400,  -400,  -400,  -400,   259,  -400,  -400,   261,   257,  -400,
    -400,  -400,  -400,  -400,  -400,   255,  1485,  -400,  1353,   260,
     213,   262,  1170,   264,  -400,   889,   211,   217,   329,  1170,
     218,   219,  1448,  -400,   269,  -400,  -400,  -400,  -400,  -400,
    -400,  -400,  -400,  -400,  -400,  -400,  -400,  -400,  -400,  -400,
    -400,  -400,  -400,   251,  -400,  -400,  -400,   270,   265,  1411,
     252,   958,  1034,  1097,   271,  -400,  -400,   273,    37,   223,
     278,    90,  -400,  -400,  -400,   276,  -400,  -400,  -400,   279,
    -400,  1411,  -400,  -400,  -400,  -400,   282,   285,  -400,   286,
    -400,  1170,   291,  1170,  -400,  -400,  1126,  -400,   287,    37,
    -400,  -400,  1485,  -400,   290,  -400,  -400,  -400,  -400,   295,
    -400,   317,  -400,  -400,   299,  1170,  1170,   302,   303,  -400,
    1170,  1170,   980,  -400,   727,   889,  -400,  -400,   305,  -400,
    -400,   306,   315,   311,   319,   316,   320,   318,  1149,   321,
    1411,   161,   109,  -400,   301,   322,    45,  -400,  -400,  -400,
    -400,  -400,   324,   326,  -400,  -400,  -400,  -400,   222,  -400,
    -400,   300,   308,  -400,  -400,  1170,   275,  -400,   331,   328,
    -400,  -400,   334,   338,   337,  -400,  -400,  -400,  -400,  -400,
    -400,  -400,  1126,  -400,  1170,  -400,  1170,  -400,  1170,  -400,
    -400,  -400,  -400,   289,  -400,  -400,   297,   298,  -400,   342,
    -400,  -400,  -400,    42,  -400,    55,  -400,  -400,  -400,    37,
    -400,  -400,  -400,  1170,   889,  1170,   889,   889,  -400,   808,
     351,   352,   348,   349,   414,   301,   301,   358,   222,  1170,
     359,   381,   360,  -400,  -400,  -400,  -400,  -400,  -400,  1170,
    1170,   367,   369,   365,  -400,  -400,  -400,  -400,   368,   889,
    1170,   373,   371,  -400,  -400,   432,  -400,  -400,   377,  -400,
    1170,   375,   889,   408,   467,  -400,  -400,   411,  -400
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
    -400,  -400,  -400,   129,   449,  -400,  -400,  -400,  -400,  -400,
    -400,  -400,  -400,  -400,  -400,  -400,  -400,  -395,  -239,  -400,
    -400,  -400,  -400,  -400,  -400,  -400,  -400,  -400,  -400,  -400,
    -400,  -400,  -400,  -400,  -400,  -400,  -400,  -400,  -400,  -377,
    -182,  -171,  -315,  -392,   -54,  -400,   267,  -400,   268,   266,
     272,  -400,   -86,   274,   280,   277,   288,   247,  -166,   -14,
    -400,  -105,  -400,  -400,  -400,  -400,  -400,  -400,  -400,  -400,
    -400,  -400,  -400,  -169,  -400,  -400,  -400,  -400,   281,  -400,
    -400,  -167,  -400,  -400,  -400,  -400,  -400,  -400,  -400,  -400,
    -165,   387,  -400,  -400,  -400,   148,  -400,  -400,  -400,  -344,
    -400,  -400,    41,  -400,    61,  -400,  -400,  -400,   204,  -119,
    -400,  -400,  -400,  -400,     5,   116,  -400,    73,  -289,  -400,
    -400,  -400,  -400,   325,   464,  -400,  -400,  -400,  -400,  -253,
    -400,  -230,   -47,  -400,  -400,  -213,  -400,   263,  -400,  -400,
    -400,  -400,  -400,  -400,  -400,  -400,  -400,   -15,   400,  -400,
    -400,  -400,   283,  -400,  -400,  -400,  -400,  -400,   176,  -400,
    -400,  -400,  -400,    -6,  -400,  -400,  -400,  -400,  -400,   -36,
    -400,   -39,   -93,  -400,  -400,  -400,   134,  -400,   433,  -400,
    -214,  -400,  -400,  -400,   -40,  -400,    44,   115,  -400,  -400,
    -400,   122,  -400,    66,  -400,  -400,  -212,  -296,  -400,  -399,
    -400,  -400
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -362
static const short yytable[] =
{
      97,    97,   101,   197,    88,   271,   306,   220,   307,   131,
     410,   204,   204,   204,   204,   209,   210,   385,   214,   204,
     204,   272,   357,   288,   282,   173,   283,   429,   284,   336,
     203,   205,   206,   207,   183,   447,   474,    75,   215,   216,
      80,    39,    97,   186,   489,    80,   337,   421,    80,   499,
     500,    80,   193,   105,   179,    80,    97,  -188,    80,    63,
    -353,    41,  -361,    80,   321,   323,   167,    71,   173,   325,
     194,   195,   218,    78,    39,   333,  -188,   106,  -343,   530,
     322,    79,   212,    64,    64,   326,    61,    62,   271,    64,
     306,   200,   201,    80,    41,   222,   223,   110,   111,   112,
     113,   114,   115,    81,   272,  -343,   336,    76,   336,   199,
     107,  -361,  -295,   303,   304,   181,   511,   108,   204,   204,
     204,   204,   204,   337,   204,   337,   289,  -188,   204,  -232,
    -232,   204,   406,   421,   204,  -292,  -232,   264,   204,   551,
     131,   553,   554,   552,   556,  -232,  -188,    81,    97,   565,
      65,    67,    81,   287,   218,    81,    69,   320,    82,   218,
    -286,   103,   303,   523,   103,   303,   523,  -342,   310,    82,
     532,   297,   566,   211,   577,   303,   523,   512,   578,   391,
     220,  -345,  -289,  -344,   230,   231,   232,   585,   234,   235,
     109,   187,   336,   188,  -342,   221,   324,   204,   449,   453,
     455,   457,   180,   496,   122,   509,   392,    81,  -345,   337,
    -344,   281,   173,   173,   450,   173,   405,   303,   304,   226,
     103,   378,   282,    81,   283,   224,   284,   390,   225,   410,
     237,   238,  -328,  -328,   475,   240,   241,   242,   227,  -328,
     472,   228,   513,   514,   515,   516,   517,   518,  -328,   131,
     476,   396,   525,   448,   526,    80,   247,   248,   249,   110,
     111,   112,   113,   114,   115,   250,   251,   252,   253,     5,
       6,     7,     8,     9,    10,    11,   271,   244,   245,   254,
     303,   304,   466,   411,   412,   426,   427,   257,   131,   267,
     268,   269,   272,    68,    70,   439,   330,   330,   255,   258,
     260,   265,   278,   294,    80,   378,   295,   296,   204,   525,
     300,   301,   302,   380,   381,   382,   383,   389,   393,   394,
     449,   413,   541,   397,   542,   402,   543,   403,  -314,   414,
     415,   416,   435,   425,   437,   428,   450,   430,   436,   440,
     441,   443,   445,   446,   461,   458,   460,   462,    97,   116,
     464,   287,   468,   465,   469,   471,   -71,   131,   117,   118,
    -361,   477,   119,   484,   120,   121,   122,   123,   124,   125,
     485,   486,   487,   126,   501,   490,   491,   571,   572,    97,
     502,   488,   439,   482,   503,   504,   492,   493,   505,   507,
     506,   531,   508,   510,   527,   528,   533,   521,   583,  -327,
     534,   535,   378,   536,   378,   378,   217,   537,    80,   538,
     544,   547,   110,   111,   112,   113,   114,   115,   545,   546,
     557,   558,   559,   560,   520,   561,    97,   564,   568,   569,
     524,   131,   197,   570,    -2,     1,   573,  -204,   574,   575,
       2,   576,   579,   581,   204,   580,   582,     3,     4,   584,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,   586,   587,   550,
     588,   439,    73,    97,   186,    97,   319,   198,   540,    97,
     311,   313,   312,   482,   378,   483,   378,   378,   314,   378,
     442,   315,   116,   404,    85,   562,   563,  -204,   202,   316,
     317,   117,   118,   276,   424,   119,   439,   120,   121,   122,
     123,   124,   125,   548,   318,   463,   126,   473,   196,   378,
     470,   522,     0,     0,   331,     0,     0,     0,   332,     0,
      -3,    72,   378,  -204,  -204,     0,     2,     0,     0,     0,
       0,     0,     0,     3,     4,  -204,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   217,
       0,    80,     0,     0,     2,   110,   111,   112,   113,   114,
     115,     0,     0,  -204,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,     0,     0,     0,     0,     0,     0,   217,     0,    80,
    -204,     0,  -207,   110,   111,   112,   113,   114,   115,     0,
       0,  -204,  -207,  -207,  -207,  -207,  -207,  -207,  -207,  -207,
    -207,  -207,  -207,  -207,  -207,  -207,  -207,  -207,  -207,  -207,
    -207,  -207,  -207,  -207,  -207,  -207,  -207,  -207,  -207,     0,
       0,     0,     0,     0,     0,   116,     0,     0,     0,     0,
       0,     0,     0,     0,   117,   118,     0,     0,   119,     0,
     120,   121,   122,   123,   124,   125,     0,     0,     0,   126,
       0,     0,     0,     0,     0,     0,     0,     0,   340,     0,
      80,     0,     0,     0,   110,   111,   112,   113,   114,   115,
       0,   497,     0,   116,     0,     0,     0,     0,     0,     0,
       0,     0,   117,   118,     0,     0,   119,     0,   120,   121,
     122,   123,   124,   125,   267,   268,   269,   126,     0,     0,
     341,   342,   343,   344,   345,     0,   346,   347,   348,   349,
     350,   351,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   352,   -22,
     353,     0,     0,     0,     0,     0,     0,     0,     0,   340,
       0,    80,     0,     0,     0,   110,   111,   112,   113,   114,
     115,     0,   555,     0,   116,     0,     0,     0,     0,     0,
       0,     0,     0,   117,   118,     0,     0,   119,     0,   120,
     121,   122,   123,   124,   125,   267,   268,   269,   126,     0,
       0,   341,   342,   343,   344,   345,     0,   346,   347,   348,
     349,   350,   351,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   352,
     -31,   353,     0,     0,     0,     0,     0,     0,     0,     0,
     340,     0,    80,     0,     0,     0,   110,   111,   112,   113,
     114,   115,   340,     0,    80,   116,     0,     0,   110,   111,
     112,   113,   114,   115,   117,   118,     0,     0,   119,     0,
     120,   121,   122,   123,   124,   125,   267,   268,   269,   126,
       0,     0,   341,   342,   343,   344,   345,     0,   346,   347,
     348,   349,   350,   351,   341,   342,   343,   344,   345,     0,
     346,   347,   348,   349,   350,   351,     0,     0,     0,   452,
     352,    80,   353,     0,     0,   110,   111,   112,   113,   114,
     115,     0,   352,   -20,   353,     0,     0,     0,     0,     0,
       0,   340,     0,    80,     0,     0,   116,   110,   111,   112,
     113,   114,   115,     0,     0,   117,   118,     0,   116,   119,
       0,   120,   121,   122,   123,   124,   125,   117,   118,     0,
     126,   119,     0,   120,   121,   122,   123,   124,   125,     0,
       0,     0,   126,   341,   342,   343,   344,   345,     0,   346,
     347,   348,   349,   350,   351,   454,     0,    80,     0,     0,
       0,   110,   111,   112,   113,   114,   115,     0,     0,     0,
       0,   352,   -29,   353,     0,   116,     0,     0,     0,     0,
       0,     0,     0,     0,   117,   118,     0,     0,   119,     0,
     120,   121,   122,   123,   124,   125,     0,   116,     0,   126,
       0,     0,     0,     0,     0,     0,   117,   118,     0,     0,
     119,     0,   120,   121,   122,   123,   124,   125,   456,     0,
      80,   126,     0,     0,   110,   111,   112,   113,   114,   115,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    80,
       0,   116,     0,   110,   111,   112,   113,   114,   115,     0,
     117,   118,     0,     0,   119,     0,   120,   121,   122,   123,
     124,   125,    80,     0,     0,   126,   110,   111,   112,   113,
     114,   115,     0,   267,   268,   269,     0,     0,     0,     0,
     266,     0,     0,    80,     0,     0,     0,   110,   111,   112,
     113,   114,   115,     0,     0,     0,   267,   268,   269,     0,
       0,     0,     0,     0,   116,     0,     0,     0,     0,     0,
       0,     0,     0,   117,   118,     0,     0,   119,     0,   120,
     121,   122,   123,   124,   125,     0,     0,     0,   126,     0,
     270,    80,     0,   116,     0,   110,   111,   112,   113,   114,
     115,     0,   117,   118,     0,     0,   119,     0,   120,   121,
     122,   123,   124,   125,    80,     0,   116,   126,   110,   111,
     112,   113,   114,   115,     0,   117,   118,     0,     0,   119,
       0,   120,   121,   122,   123,   124,   125,   116,     0,     0,
     126,     0,     0,     0,     0,     0,   117,   118,     0,     0,
     119,     0,   120,   121,   122,   123,   124,   125,     0,     0,
       0,   126,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   116,     0,     0,     0,     0,
       0,     0,     0,     0,   117,   118,     0,     0,   119,     0,
     120,   121,   122,   123,   124,   125,     0,     0,   116,   208,
       0,     0,     0,     0,     0,     0,     0,   117,   118,     0,
       0,   119,     0,   120,   121,   122,   123,   124,   125,     2,
       0,     0,   213,     0,     0,     0,     0,     0,     0,     5,
       6,     7,     8,     9,    10,    11,     0,     0,     0,     0,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     2,     0,     0,
       0,     0,     0,     0,     0,     0,   423,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,  -195,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  -195,  -195,  -195,  -195,  -195,  -195,
    -195,  -195,  -195,  -195,  -195,  -195,  -195,  -195,  -195,  -195,
    -195,  -195,  -195,  -195,  -195,  -195,  -195,  -195,  -195,  -195,
    -195,     2,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     5,     6,     7,     8,     9,    10,    11,     0,     0,
       0,     0,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31
};

static const short yycheck[] =
{
      40,    41,    41,    96,    40,   187,   220,   126,   220,    63,
     306,   116,   117,   118,   119,   120,   121,   270,   123,   124,
     125,   187,   261,   194,   193,    65,   193,   342,   193,   259,
     116,   117,   118,   119,    81,   379,   413,     1,   124,   125,
       3,     0,    82,    82,   436,     3,   259,   336,     3,   444,
     445,     3,   102,    48,    69,     3,    96,     1,     3,   120,
       3,     0,    69,     3,   246,   247,     6,     0,   108,   251,
     120,   121,   126,    73,    33,   257,     1,    71,    74,   478,
     246,    73,   122,     4,     4,   251,    72,    73,   270,     4,
     304,   106,   107,     3,    33,    92,    93,     7,     8,     9,
      10,    11,    12,   110,   270,   101,   336,    71,   338,   104,
      71,    69,    72,   120,   121,    72,   460,    71,   223,   224,
     225,   226,   227,   336,   229,   338,    69,    71,   233,    73,
      74,   236,   303,   422,   239,    71,    80,   184,   243,   534,
     194,   536,   537,   535,   539,    89,    71,   110,   188,   548,
      71,    71,   110,   193,   208,   110,    71,   243,   121,   213,
      71,    45,   120,   121,    48,   120,   121,    74,   222,   121,
     485,   211,   549,   121,   569,   120,   121,    16,   570,    74,
     299,    74,    71,    74,   101,   102,   103,   582,   104,   105,
      61,    89,   422,    74,   101,    70,   250,   302,   380,   381,
     382,   383,    73,   442,   114,   458,   101,   110,   101,   422,
     101,   121,   252,   253,   380,   255,   302,   120,   121,    96,
     104,   261,   391,   110,   391,    94,   391,   281,    95,   525,
     106,   107,    73,    74,   416,   108,   109,   110,    97,    80,
     411,    98,    81,    82,    83,    84,    85,    86,    89,   303,
     416,   291,   466,     1,   466,     3,    74,   112,   113,     7,
       8,     9,    10,    11,    12,   120,   121,   122,   123,    16,
      17,    18,    19,    20,    21,    22,   458,    88,    89,    72,
     120,   121,   401,   120,   121,    72,    73,    89,   342,    37,
      38,    39,   458,    30,    31,   349,   252,   253,    74,    72,
      72,    69,    80,    72,     3,   345,    72,    72,   413,   523,
      69,    69,    69,   121,   121,   121,   121,   121,    70,    69,
     502,    91,   504,    69,   506,    69,   508,    69,    73,    70,
      69,    74,   121,    73,     5,    73,   502,    73,   121,   121,
     121,    72,    91,    73,   121,    74,    73,    69,   388,    97,
      74,   391,    70,    74,    69,    69,    91,   411,   106,   107,
      69,    74,   110,    73,   112,   113,   114,   115,   116,   117,
      75,    54,    73,   121,    69,    73,    73,   559,   560,   419,
      74,   435,   436,   419,    69,    74,   440,   441,    69,    69,
      74,    91,    74,    72,    70,    69,   121,    75,   580,    91,
      69,    73,   442,    69,   444,   445,     1,    69,     3,    72,
     121,    69,     7,     8,     9,    10,    11,    12,   121,   121,
      69,    69,    74,    74,   464,    11,   466,    69,    69,    48,
     466,   485,   525,    73,     0,     1,    69,     3,    69,    74,
       6,    73,    69,    11,   549,    74,    69,    13,    14,    74,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    69,    11,   533,
      69,   535,    33,   523,   523,   525,   239,   100,   502,   529,
     223,   225,   224,   529,   534,   422,   536,   537,   226,   539,
     352,   227,    97,   299,    40,   545,   546,    73,   108,   229,
     233,   106,   107,   188,   338,   110,   570,   112,   113,   114,
     115,   116,   117,   529,   236,   391,   121,   412,    95,   569,
     408,   465,    -1,    -1,   253,    -1,    -1,    -1,   255,    -1,
       0,     1,   582,     3,   110,    -1,     6,    -1,    -1,    -1,
      -1,    -1,    -1,    13,    14,   121,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     1,
      -1,     3,    -1,    -1,     6,     7,     8,     9,    10,    11,
      12,    -1,    -1,    73,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,     3,
     110,    -1,     6,     7,     8,     9,    10,    11,    12,    -1,
      -1,   121,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    -1,
      -1,    -1,    -1,    -1,    -1,    97,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   106,   107,    -1,    -1,   110,    -1,
     112,   113,   114,   115,   116,   117,    -1,    -1,    -1,   121,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     1,    -1,
       3,    -1,    -1,    -1,     7,     8,     9,    10,    11,    12,
      -1,    14,    -1,    97,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   106,   107,    -1,    -1,   110,    -1,   112,   113,
     114,   115,   116,   117,    37,    38,    39,   121,    -1,    -1,
      43,    44,    45,    46,    47,    -1,    49,    50,    51,    52,
      53,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,    72,
      73,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     1,
      -1,     3,    -1,    -1,    -1,     7,     8,     9,    10,    11,
      12,    -1,    14,    -1,    97,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   106,   107,    -1,    -1,   110,    -1,   112,
     113,   114,   115,   116,   117,    37,    38,    39,   121,    -1,
      -1,    43,    44,    45,    46,    47,    -1,    49,    50,    51,
      52,    53,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    71,
      72,    73,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
       1,    -1,     3,    -1,    -1,    -1,     7,     8,     9,    10,
      11,    12,     1,    -1,     3,    97,    -1,    -1,     7,     8,
       9,    10,    11,    12,   106,   107,    -1,    -1,   110,    -1,
     112,   113,   114,   115,   116,   117,    37,    38,    39,   121,
      -1,    -1,    43,    44,    45,    46,    47,    -1,    49,    50,
      51,    52,    53,    54,    43,    44,    45,    46,    47,    -1,
      49,    50,    51,    52,    53,    54,    -1,    -1,    -1,     1,
      71,     3,    73,    -1,    -1,     7,     8,     9,    10,    11,
      12,    -1,    71,    72,    73,    -1,    -1,    -1,    -1,    -1,
      -1,     1,    -1,     3,    -1,    -1,    97,     7,     8,     9,
      10,    11,    12,    -1,    -1,   106,   107,    -1,    97,   110,
      -1,   112,   113,   114,   115,   116,   117,   106,   107,    -1,
     121,   110,    -1,   112,   113,   114,   115,   116,   117,    -1,
      -1,    -1,   121,    43,    44,    45,    46,    47,    -1,    49,
      50,    51,    52,    53,    54,     1,    -1,     3,    -1,    -1,
      -1,     7,     8,     9,    10,    11,    12,    -1,    -1,    -1,
      -1,    71,    72,    73,    -1,    97,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   106,   107,    -1,    -1,   110,    -1,
     112,   113,   114,   115,   116,   117,    -1,    97,    -1,   121,
      -1,    -1,    -1,    -1,    -1,    -1,   106,   107,    -1,    -1,
     110,    -1,   112,   113,   114,   115,   116,   117,     1,    -1,
       3,   121,    -1,    -1,     7,     8,     9,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,
      -1,    97,    -1,     7,     8,     9,    10,    11,    12,    -1,
     106,   107,    -1,    -1,   110,    -1,   112,   113,   114,   115,
     116,   117,     3,    -1,    -1,   121,     7,     8,     9,    10,
      11,    12,    -1,    37,    38,    39,    -1,    -1,    -1,    -1,
      21,    -1,    -1,     3,    -1,    -1,    -1,     7,     8,     9,
      10,    11,    12,    -1,    -1,    -1,    37,    38,    39,    -1,
      -1,    -1,    -1,    -1,    97,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   106,   107,    -1,    -1,   110,    -1,   112,
     113,   114,   115,   116,   117,    -1,    -1,    -1,   121,    -1,
      71,     3,    -1,    97,    -1,     7,     8,     9,    10,    11,
      12,    -1,   106,   107,    -1,    -1,   110,    -1,   112,   113,
     114,   115,   116,   117,     3,    -1,    97,   121,     7,     8,
       9,    10,    11,    12,    -1,   106,   107,    -1,    -1,   110,
      -1,   112,   113,   114,   115,   116,   117,    97,    -1,    -1,
     121,    -1,    -1,    -1,    -1,    -1,   106,   107,    -1,    -1,
     110,    -1,   112,   113,   114,   115,   116,   117,    -1,    -1,
      -1,   121,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    97,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   106,   107,    -1,    -1,   110,    -1,
     112,   113,   114,   115,   116,   117,    -1,    -1,    97,   121,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   106,   107,    -1,
      -1,   110,    -1,   112,   113,   114,   115,   116,   117,     6,
      -1,    -1,   121,    -1,    -1,    -1,    -1,    -1,    -1,    16,
      17,    18,    19,    20,    21,    22,    -1,    -1,    -1,    -1,
      -1,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     6,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    73,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,     6,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,     6,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    16,    17,    18,    19,    20,    21,    22,    -1,    -1,
      -1,    -1,    -1,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned short yystos[] =
{
       0,     1,     6,    13,    14,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,   125,   126,   128,   129,   130,   224,   225,   226,
     227,   228,   233,   234,   235,   236,   237,   239,   255,   256,
     259,   260,   262,   263,   264,   265,   266,   267,   268,   269,
     270,    72,    73,   120,     4,    71,   261,    71,   261,    71,
     261,     0,     1,   128,   127,     1,    71,   131,    73,    73,
       3,   110,   121,   245,   247,   248,   249,   250,   293,   295,
     296,   297,   298,   299,   302,   303,   304,   308,   248,   293,
     294,   295,   238,   239,   240,   238,    71,    71,    71,   127,
       7,     8,     9,    10,    11,    12,    97,   106,   107,   110,
     112,   113,   114,   115,   116,   117,   121,   163,   164,   165,
     166,   168,   169,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,   185,   186,   187,   188,   189,
     190,   191,   192,   193,   194,   195,   196,   197,   198,   199,
     200,   201,   203,   204,   205,   214,   308,     6,   272,   274,
     275,   276,   277,   308,   309,   310,   271,   278,   279,   271,
     127,    72,   132,   256,   257,   258,   295,    89,    74,   215,
     216,   217,   246,   102,   120,   121,   302,   296,   215,   238,
     271,   271,   272,   176,   185,   176,   176,   176,   121,   185,
     185,   121,   308,   121,   185,   176,   176,     1,   168,   232,
     233,    70,    92,    93,    94,    95,    96,    97,    98,   210,
     101,   102,   103,   211,   104,   105,   212,   106,   107,   208,
     108,   109,   110,   209,    88,    89,   213,    74,   112,   113,
     120,   121,   122,   123,    72,    74,   273,    89,    72,   280,
      72,   219,   220,   221,   256,    69,    21,    37,    38,    39,
      71,   164,   182,   184,   251,   253,   247,   218,    80,   323,
     324,   121,   197,   205,   214,   300,   301,   308,   165,    69,
     305,   306,   313,   314,    72,    72,    72,   308,   230,   231,
      69,    69,    69,   120,   121,   229,   304,   320,   321,   322,
     168,   170,   172,   173,   174,   177,   178,   179,   180,   181,
     176,   164,   182,   164,   168,   164,   182,   206,   207,   202,
     310,   202,   276,   164,   242,   244,   255,   259,   281,   282,
       1,    43,    44,    45,    46,    47,    49,    50,    51,    52,
      53,    54,    71,    73,   133,   134,   137,   142,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   158,   159,   160,   161,   162,   168,   308,   222,
     121,   121,   121,   121,   252,   253,   223,   226,   228,   121,
     168,    74,   101,    70,    69,   307,   308,    69,   315,   316,
     317,   318,    69,    69,   232,   176,   165,   311,   312,   320,
     321,   120,   121,    91,    70,    69,    74,   283,   284,   285,
     241,   242,   243,    73,   282,    73,    72,    73,    73,   166,
      73,   141,   142,   143,   182,   121,   121,     5,   167,   168,
     121,   121,   219,    72,   135,    91,    73,   223,     1,   164,
     182,   183,     1,   164,     1,   164,     1,   164,    74,   254,
      73,   121,    69,   300,    74,    74,   233,   319,    70,    69,
     315,    69,   165,   311,   163,   164,   182,    74,   287,   288,
     289,   292,   293,   241,    73,    75,    54,    73,   168,   167,
      73,    73,   168,   168,   138,   139,   142,    14,   136,   141,
     141,    69,    74,    69,    74,    69,    74,    69,    74,   253,
      72,   223,    16,    81,    82,    83,    84,    85,    86,   325,
     308,    75,   317,   121,   293,   304,   320,    70,    69,   286,
     323,    91,   166,   121,    69,    73,    69,    69,    72,   140,
     183,   164,   164,   164,   121,   121,   121,    69,   287,   290,
     168,   141,   167,   141,   141,    14,   141,    69,    69,    74,
      74,    11,   308,   308,    69,   323,   163,   291,    69,    48,
      73,   164,   164,    69,    69,    74,    73,   141,   167,    69,
      74,    11,    69,   164,    74,   141,    69,    11,    69
};

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
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
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
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)         \
  Current.first_line   = Rhs[1].first_line;      \
  Current.first_column = Rhs[1].first_column;    \
  Current.last_line    = Rhs[N].last_line;       \
  Current.last_column  = Rhs[N].last_column;
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval)
#endif

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

# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Token, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (cinluded).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short *bottom, short *top)
#else
static void
yy_stack_print (bottom, top)
    short *bottom;
    short *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
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



#if YYERROR_VERBOSE

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

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */






/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  /* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;

  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
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

  if (yyss + yystacksize - 1 <= yyssp)
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
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
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
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


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

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 244 "gram.y"
    {
            if (err_cnt == 0)
              *gProject->Parse_TOS->yyerrstream
              << "Warning: ANSI/ISO C forbids an empty source file.\n";
            gProject->Parse_TOS->transUnit = (TransUnit*) NULL;
            yyval.transunit = (TransUnit*) NULL;
        ;}
    break;

  case 3:
#line 252 "gram.y"
    {
            if (err_cnt)
            {
                *gProject->Parse_TOS->yyerrstream
                << err_cnt << " errors found.\n";
                gProject->Parse_TOS->transUnit = (TransUnit*) NULL;
            } else {
                gProject->Parse_TOS->transUnit = yyval.transunit;
            }
        ;}
    break;

  case 4:
#line 263 "gram.y"
    {
            *gProject->Parse_TOS->yyerrstream << "Errors - Aborting parse.\n";
            gProject->Parse_TOS->transUnit = (TransUnit*) NULL;
            YYACCEPT;
        ;}
    break;

  case 5:
#line 271 "gram.y"
    {
            yyval.transunit = gProject->Parse_TOS->transUnit;
            yyval.transunit->add(yyvsp[-1].stemnt);
        ;}
    break;

  case 6:
#line 276 "gram.y"
    {
            yyval.transunit->add(yyvsp[-1].stemnt);
        ;}
    break;

  case 7:
#line 282 "gram.y"
    {
            gProject->Parse_TOS->parseCtxt->ReinitializeCtxt();
            if (gProject->Parse_TOS->transUnit)
                gProject->Parse_TOS->transUnit->contxt.ExitScopes(FILE_SCOPE);
            err_top_level = 0;            
        ;}
    break;

  case 8:
#line 291 "gram.y"
    {
            yyval.stemnt = yyvsp[0].declStemnt;
        ;}
    break;

  case 9:
#line 295 "gram.y"
    {
            yyval.stemnt = yyvsp[0].functionDef;
        ;}
    break;

  case 10:
#line 299 "gram.y"
    {
            yyval.stemnt = yyvsp[0].stemnt;
        ;}
    break;

  case 11:
#line 303 "gram.y"
    {
            yyval.stemnt = yyvsp[0].stemnt;
        ;}
    break;

  case 12:
#line 307 "gram.y"
    {
            yyval.stemnt = (Statement*) NULL;
        ;}
    break;

  case 13:
#line 311 "gram.y"
    {
            yyval.stemnt = (Statement*) NULL;
        ;}
    break;

  case 14:
#line 317 "gram.y"
    {
            if (yyvsp[0].stemnt != NULL)
            {
                yyval.functionDef = new FunctionDef(yyvsp[0].stemnt->location);
                Block *blk = (Block*) yyvsp[0].stemnt;
    
                yyval.functionDef->decl = yyvsp[-1].decl;
                
                if (yyvsp[-1].decl->name &&
                    yyvsp[-1].decl->name->entry)
                    yyvsp[-1].decl->name->entry->u2FunctionDef = yyval.functionDef;
                
                // Steal internals of the compound statement
                yyval.functionDef->head = blk->head;
                yyval.functionDef->tail = blk->tail;
    
                blk->head = blk->tail = (Statement*) NULL;
                delete yyvsp[0].stemnt;    
            }
			else
			{
				delete yyvsp[-1].decl;
				yyval.functionDef = (FunctionDef*) NULL;
			}
        ;}
    break;

  case 15:
#line 345 "gram.y"
    {
            gProject->Parse_TOS->parseCtxt->ResetDeclCtxt();
            
            possibleType = true;
            yyval.decl = yyvsp[-1].decl;

            if (yyval.decl->form != NULL)
            {
                assert(err_top_level ||
                       yyval.decl->form->type == TT_Function );
    
                yyval.decl->extend(yyvsp[-2].base);
    
                /* This is adding K&R-style declarations if $3 exists */
                if (yyvsp[0].decl != NULL)
                {
                    FunctionType *fnc = (FunctionType*) (yyval.decl->form);
                    fnc->KnR_decl = true;
                    Decl *param = yyvsp[0].decl;
                    while (param != NULL)
                    {
                        Decl *next= param->next;
                            delete param ;
                        param = next;
                    }
                }
            }
        ;}
    break;

  case 16:
#line 374 "gram.y"
    {

            gProject->Parse_TOS->parseCtxt->ResetDeclCtxt();
            
            yyval.decl = yyvsp[-1].decl;

            if (yyval.decl->form != NULL)
            {
                assert(err_top_level ||
                       yyval.decl->form->type == TT_Function );
                yyval.decl->extend(yyvsp[-2].base);
    
                /* This is adding K&R-style declarations if $3 exists */
                if (yyvsp[0].decl != NULL)
                {
                    FunctionType *fnc = (FunctionType*) (yyval.decl->form);
                    fnc->KnR_decl = true;
                    Decl *param = yyvsp[0].decl;
                    while (param != NULL)
                    {
                        Decl *next= param->next;
                            delete param ;
                        param = next;
                    }
                }
            }
        ;}
    break;

  case 17:
#line 407 "gram.y"
    {  
            if (gProject->Parse_TOS->transUnit)
                gProject->Parse_TOS->transUnit->contxt.ReEnterScope();
        ;}
    break;

  case 18:
#line 412 "gram.y"
    {
            Block*    block = new Block(*yyvsp[-4].loc);
            yyval.stemnt = block;
            block->addDecls(yyvsp[-2].decl);
            block->addStatements(ReverseList(yyvsp[-1].stemnt));
            if (gProject->Parse_TOS->transUnit)
            {
                yyCheckLabelsDefinition(gProject->Parse_TOS->transUnit->contxt.labels);
                gProject->Parse_TOS->transUnit->contxt.ExitScope();
                gProject->Parse_TOS->transUnit->contxt.ExitScope();
            }
        ;}
    break;

  case 19:
#line 425 "gram.y"
    {
            yyval.stemnt = (Statement*) NULL;
        ;}
    break;

  case 20:
#line 431 "gram.y"
    {
            yyval.stemnt = (Statement*) NULL;
        ;}
    break;

  case 22:
#line 438 "gram.y"
    {
	    /*
	     * All the statements are expected in a reversed list (because
	     * of how we parse stemnt_list2) so we need to take the
	     * non_constructor statement at the end.
	     */
            if (yyvsp[0].stemnt)
            {
	        Statement *s;

		for (s = yyvsp[0].stemnt; s->next; s = s->next) /* Traverse to the end */;
		s->next = yyvsp[-1].stemnt;
                yyval.stemnt = yyvsp[0].stemnt;
            }
        ;}
    break;

  case 23:
#line 456 "gram.y"
    {
	   yyval.stemnt = (Statement *) NULL;
	;}
    break;

  case 24:
#line 460 "gram.y"
    {
            /* Hook them up backwards, we'll reverse them later. */
            if (yyvsp[0].stemnt)
            {
                yyvsp[0].stemnt->next = yyvsp[-1].stemnt;
                yyval.stemnt = yyvsp[0].stemnt;
            }
        ;}
    break;

  case 25:
#line 469 "gram.y"
    {    /* preprocessor #line directive */
            /* Hook them up backwards, we'll reverse them later. */
            if (yyvsp[0].stemnt)
            {
                yyvsp[0].stemnt->next = yyvsp[-1].stemnt;
                yyval.stemnt = yyvsp[0].stemnt;
            }
        ;}
    break;

  case 27:
#line 488 "gram.y"
    {
            Block*    block = new Block(*yyvsp[-3].loc);
            yyval.stemnt = block;
            block->addDecls(yyvsp[-2].decl);
            block->addStatements(ReverseList(yyvsp[-1].stemnt));
            
            if (gProject->Parse_TOS->transUnit)
                gProject->Parse_TOS->transUnit->contxt.ExitScope();
        ;}
    break;

  case 28:
#line 498 "gram.y"
    {
            yyval.stemnt = (Statement*) NULL;
        ;}
    break;

  case 29:
#line 504 "gram.y"
    {
            yyval.stemnt = (Statement*) NULL;
        ;}
    break;

  case 31:
#line 511 "gram.y"
    {
	    /*
	     * All the statements are expected in a reversed list (because
	     * of how we parse stemnt_list_reentrance2) so we need to take
	     * the non_constructor statement at the end.
	     */
            if (yyvsp[0].stemnt)
            {
	        Statement *s;

		for (s = yyvsp[0].stemnt; s->next; s = s->next) /* Traverse to the end */;
		s->next = yyvsp[-1].stemnt;
                yyval.stemnt = yyvsp[0].stemnt;
            }
        ;}
    break;

  case 32:
#line 529 "gram.y"
    {
	   yyval.stemnt = (Statement *) NULL;
	;}
    break;

  case 33:
#line 533 "gram.y"
    {
            /* Hook them up backwards, we'll reverse them later. */
            if (yyvsp[0].stemnt)
            {
                yyvsp[0].stemnt->next = yyvsp[-1].stemnt;
                yyval.stemnt = yyvsp[0].stemnt;
            }
        ;}
    break;

  case 34:
#line 542 "gram.y"
    {    /* preprocessor #line directive */
            /* Hook them up backwards, we'll reverse them later. */
            if (yyvsp[0].stemnt)
            {
                yyvsp[0].stemnt->next = yyvsp[-1].stemnt;
                yyval.stemnt = yyvsp[0].stemnt;
            }
        ;}
    break;

  case 35:
#line 553 "gram.y"
    {
	    yyval.stemnt = yyvsp[0].stemnt;
	 ;}
    break;

  case 36:
#line 557 "gram.y"
    {
	    yyval.stemnt = yyvsp[0].stemnt;
	 ;}
    break;

  case 48:
#line 574 "gram.y"
    {
            delete yyvsp[0].loc;
            yyval.stemnt = (Statement*) NULL;
        ;}
    break;

  case 49:
#line 597 "gram.y"
    {
            yyval.stemnt = new ExpressionStemnt(yyvsp[-1].value,*yyvsp[0].loc);
            delete yyvsp[0].loc;
	;}
    break;

  case 50:
#line 604 "gram.y"
    {
            yyval.stemnt = new ExpressionStemnt(yyvsp[-1].value,*yyvsp[0].loc);
            delete yyvsp[0].loc;
        ;}
    break;

  case 51:
#line 611 "gram.y"
    {
            yyval.stemnt = yyvsp[0].stemnt;
            if (yyval.stemnt == NULL)
            {
              /* Sorry, we must have a statement here. */
              yyerr("Can't have a label at the end of a block! ");
              yyval.stemnt = new Statement(ST_NullStemnt,*yyvsp[-1].loc);
            }
            yyval.stemnt->addHeadLabel(yyvsp[-2].label);
            delete yyvsp[-1].loc;
        ;}
    break;

  case 57:
#line 634 "gram.y"
    {
            yyval.stemnt = new SwitchStemnt(yyvsp[-2].value,yyvsp[0].stemnt,*yyvsp[-4].loc);
            delete yyvsp[-4].loc;
            delete yyvsp[-3].loc;
            delete yyvsp[-1].loc;
        ;}
    break;

  case 58:
#line 643 "gram.y"
    {
            yyval.stemnt = new Statement(ST_BreakStemnt,*yyvsp[-1].loc);
            delete yyvsp[-1].loc;
            delete yyvsp[0].loc;
        ;}
    break;

  case 59:
#line 651 "gram.y"
    {
            yyval.stemnt = new Statement(ST_ContinueStemnt,*yyvsp[-1].loc);
            delete yyvsp[-1].loc;
            delete yyvsp[0].loc;
        ;}
    break;

  case 60:
#line 659 "gram.y"
    {
            yyval.stemnt = new ReturnStemnt(yyvsp[-1].value,*yyvsp[-2].loc);
            delete yyvsp[-2].loc;
            delete yyvsp[0].loc;
        ;}
    break;

  case 61:
#line 667 "gram.y"
    {
            yyval.stemnt = new GotoStemnt(yyvsp[-1].symbol,*yyvsp[-2].loc);
            delete yyvsp[-2].loc;
            delete yyvsp[0].loc;
        ;}
    break;

  case 62:
#line 675 "gram.y"
    {
            yyval.stemnt = new Statement(ST_NullStemnt,*yyvsp[0].loc);
            delete yyvsp[0].loc;
        ;}
    break;

  case 63:
#line 682 "gram.y"
    {
            yyval.stemnt = new IfStemnt(yyvsp[-2].value,yyvsp[0].stemnt,*yyvsp[-4].loc);
            delete yyvsp[-4].loc;
            delete yyvsp[-3].loc;
            delete yyvsp[-1].loc;
        ;}
    break;

  case 64:
#line 691 "gram.y"
    {
            yyval.stemnt = new IfStemnt(yyvsp[-4].value,yyvsp[-2].stemnt,*yyvsp[-6].loc,yyvsp[0].stemnt);
            delete yyvsp[-6].loc;
            delete yyvsp[-5].loc;
            delete yyvsp[-3].loc;
            delete yyvsp[-1].loc;
        ;}
    break;

  case 65:
#line 701 "gram.y"
    {
            yyval.stemnt = new DoWhileStemnt(yyvsp[-2].value,yyvsp[-5].stemnt,*yyvsp[-6].loc);
            delete yyvsp[-6].loc;
            delete yyvsp[-4].loc;
            delete yyvsp[-3].loc;
            delete yyvsp[-1].loc;
            delete yyvsp[0].loc;
        ;}
    break;

  case 66:
#line 712 "gram.y"
    {
            yyval.stemnt = new WhileStemnt(yyvsp[-2].value,yyvsp[0].stemnt,*yyvsp[-4].loc);
            delete yyvsp[-4].loc;
            delete yyvsp[-3].loc;
            delete yyvsp[-1].loc;
        ;}
    break;

  case 67:
#line 722 "gram.y"
    {
            yyval.stemnt = new ForStemnt(yyvsp[-6].value,yyvsp[-4].value,yyvsp[-2].value,*yyvsp[-8].loc,yyvsp[0].stemnt);
            delete yyvsp[-8].loc;
            delete yyvsp[-7].loc;
            delete yyvsp[-5].loc;
            delete yyvsp[-3].loc;
            delete yyvsp[-1].loc;
        ;}
    break;

  case 71:
#line 738 "gram.y"
    {
            if (gProject->Parse_TOS->transUnit)
                yyval.label = gProject->Parse_TOS->parseCtxt->Mk_named_label(yyvsp[0].symbol,
                                gProject->Parse_TOS->transUnit->contxt.labels);
        ;}
    break;

  case 72:
#line 746 "gram.y"
    {
            yyval.label = new Label(yyvsp[0].value);
            delete yyvsp[-1].loc;
        ;}
    break;

  case 73:
#line 751 "gram.y"
    {
            yyval.label = new Label(yyvsp[-2].value,yyvsp[0].value);
            delete yyvsp[-3].loc;
            delete yyvsp[-1].loc;
        ;}
    break;

  case 74:
#line 759 "gram.y"
    {
            yyval.label = new Label(LT_Default);
            delete yyvsp[0].loc;
        ;}
    break;

  case 76:
#line 776 "gram.y"
    {
            yyval.value = new TrinaryExpr(yyvsp[-4].value,yyvsp[-2].value,yyvsp[0].value,*yyvsp[-3].loc);
            delete yyvsp[-3].loc;
            delete yyvsp[-1].loc;
        ;}
    break;

  case 78:
#line 785 "gram.y"
    {
            yyval.value = new AssignExpr(yyvsp[-1].assignOp,yyvsp[-2].value,yyvsp[0].value,NoLocation);
        ;}
    break;

  case 79:
#line 789 "gram.y"
    {
            yyval.value = new AssignExpr(yyvsp[-1].assignOp,yyvsp[-2].value,yyvsp[0].value,NoLocation);
        ;}
    break;

  case 80:
#line 795 "gram.y"
    {
            yyval.value = (Expression*) NULL;
        ;}
    break;

  case 83:
#line 805 "gram.y"
    {
           yyval.value = (Expression*) NULL;
        ;}
    break;

  case 87:
#line 816 "gram.y"
    {
            yyval.value = new BinaryExpr(BO_Or,yyvsp[-2].value,yyvsp[0].value,*yyvsp[-1].loc);
            delete yyvsp[-1].loc;
        ;}
    break;

  case 89:
#line 824 "gram.y"
    {
            yyval.value = new BinaryExpr(BO_And,yyvsp[-2].value,yyvsp[0].value,*yyvsp[-1].loc);
            delete yyvsp[-1].loc;
        ;}
    break;

  case 90:
#line 831 "gram.y"
    {
            yyval.value = new UnaryExpr(UO_Not,yyvsp[0].value,*yyvsp[-1].loc);
            delete yyvsp[-1].loc;
        ;}
    break;

  case 92:
#line 839 "gram.y"
    {
            yyval.value = new BinaryExpr(BO_BitOr,yyvsp[-2].value,yyvsp[0].value,*yyvsp[-1].loc);
            delete yyvsp[-1].loc;
        ;}
    break;

  case 94:
#line 847 "gram.y"
    {
            yyval.value = new BinaryExpr(BO_BitXor,yyvsp[-2].value,yyvsp[0].value,*yyvsp[-1].loc);
            delete yyvsp[-1].loc;
        ;}
    break;

  case 96:
#line 855 "gram.y"
    {
            yyval.value = new BinaryExpr(BO_BitAnd,yyvsp[-2].value,yyvsp[0].value,*yyvsp[-1].loc);
            delete yyvsp[-1].loc;
        ;}
    break;

  case 97:
#line 862 "gram.y"
    {
            yyval.value = new UnaryExpr(UO_BitNot,yyvsp[0].value,*yyvsp[-1].loc);
            delete yyvsp[-1].loc;
        ;}
    break;

  case 99:
#line 870 "gram.y"
    {
            yyval.value = new CastExpr(yyvsp[-2].type,yyvsp[0].value,*yyvsp[-3].loc);
            delete yyvsp[-3].loc;
            delete yyvsp[-1].loc;
        ;}
    break;

  case 101:
#line 879 "gram.y"
    {
            yyval.value = new RelExpr(yyvsp[-1].relOp,yyvsp[-2].value,yyvsp[0].value,NoLocation);
        ;}
    break;

  case 103:
#line 886 "gram.y"
    {
            yyval.value = new RelExpr(yyvsp[-1].relOp,yyvsp[-2].value,yyvsp[0].value,NoLocation);
        ;}
    break;

  case 105:
#line 893 "gram.y"
    {
            yyval.value = new BinaryExpr(yyvsp[-1].binOp,yyvsp[-2].value,yyvsp[0].value,NoLocation);
        ;}
    break;

  case 107:
#line 900 "gram.y"
    {
            yyval.value = new BinaryExpr(yyvsp[-1].binOp,yyvsp[-2].value,yyvsp[0].value,NoLocation);
        ;}
    break;

  case 109:
#line 907 "gram.y"
    {
            yyval.value = new BinaryExpr(yyvsp[-1].binOp,yyvsp[-2].value,yyvsp[0].value,NoLocation);
        ;}
    break;

  case 110:
#line 913 "gram.y"
    {
	    Expression *exprs[] = { yyvsp[-3].value, yyvsp[-1].value };
            yyval.value = new ConstructorExpr(yyvsp[-5].base, exprs, NoLocation);
            delete yyvsp[-4].loc;
            delete yyvsp[-2].loc;
            delete yyvsp[0].loc;
        ;}
    break;

  case 111:
#line 922 "gram.y"
    {
	    Expression *exprs[] = { yyvsp[-5].value, yyvsp[-3].value, yyvsp[-1].value };
            yyval.value = new ConstructorExpr(yyvsp[-7].base, exprs, NoLocation);
            delete yyvsp[-6].loc;
            delete yyvsp[-4].loc;
            delete yyvsp[-2].loc;
            delete yyvsp[0].loc;
        ;}
    break;

  case 112:
#line 932 "gram.y"
    {
	    Expression *exprs[] = { yyvsp[-7].value, yyvsp[-5].value, yyvsp[-3].value, yyvsp[-1].value };
            yyval.value = new ConstructorExpr(yyvsp[-9].base, exprs, NoLocation);
            delete yyvsp[-8].loc;
            delete yyvsp[-6].loc;
            delete yyvsp[-4].loc;
            delete yyvsp[-2].loc;
            delete yyvsp[0].loc;
        ;}
    break;

  case 113:
#line 942 "gram.y"
    {
	   yyval.value = (Expression *) NULL;
	;}
    break;

  case 114:
#line 946 "gram.y"
    {
	   yyval.value = (Expression *) NULL;
	;}
    break;

  case 115:
#line 950 "gram.y"
    {
	   yyval.value = (Expression *) NULL;
	;}
    break;

  case 118:
#line 961 "gram.y"
    {
	   Symbol *sym = new Symbol();
	   Variable *var;

	   sym->name = strdup("iter");
	   var = new Variable(sym, *yyvsp[-4].loc);
	   yyval.value = new FunctionCall(var, *yyvsp[-4].loc);

	   ((FunctionCall *) yyval.value)->addArg(yyvsp[-3].value);
	   ((FunctionCall *) yyval.value)->addArg(yyvsp[-1].value);

           delete yyvsp[-4].loc;
           delete yyvsp[-2].loc;
           delete yyvsp[0].loc;
	;}
    break;

  case 119:
#line 977 "gram.y"
    {
	   yyval.value = (Expression *) NULL;
	;}
    break;

  case 130:
#line 995 "gram.y"
    {
            yyval.value = new SizeofExpr(yyvsp[-1].type,*yyvsp[-3].loc);
            delete yyvsp[-3].loc;
            delete yyvsp[-2].loc;
            delete yyvsp[0].loc;
        ;}
    break;

  case 131:
#line 1002 "gram.y"
    {
            yyval.value = new SizeofExpr(yyvsp[0].value,*yyvsp[-1].loc);
            delete yyvsp[-1].loc;
        ;}
    break;

  case 132:
#line 1009 "gram.y"
    {
	  yyval.value = new BrtIndexofExpr(new Variable(yyvsp[0].symbol,*yyvsp[-1].loc),*yyvsp[-1].loc);
	;}
    break;

  case 133:
#line 1013 "gram.y"
    {
	  yyval.value = new BrtIndexofExpr(new Variable(yyvsp[-1].symbol,*yyvsp[-3].loc),*yyvsp[-3].loc);
	;}
    break;

  case 134:
#line 1033 "gram.y"
    {
            yyval.value = new UnaryExpr(UO_Minus,yyvsp[0].value,NoLocation);
        ;}
    break;

  case 135:
#line 1039 "gram.y"
    {
            /* Unary plus is an ISO addition (for symmetry) - ignore it */
            yyval.value = yyvsp[0].value;
        ;}
    break;

  case 136:
#line 1046 "gram.y"
    {
            yyval.value = new UnaryExpr(UO_AddrOf,yyvsp[0].value,*yyvsp[-1].loc);
            delete yyvsp[-1].loc;
        ;}
    break;

  case 137:
#line 1053 "gram.y"
    {
            yyval.value = new UnaryExpr(UO_Deref,yyvsp[0].value,NoLocation);
        ;}
    break;

  case 138:
#line 1059 "gram.y"
    {
            yyval.value = new UnaryExpr(UO_PreInc,yyvsp[0].value,*yyvsp[-1].loc);
            delete yyvsp[-1].loc;
        ;}
    break;

  case 139:
#line 1066 "gram.y"
    {
            yyval.value = new UnaryExpr(UO_PreDec,yyvsp[0].value,*yyvsp[-1].loc);
            delete yyvsp[-1].loc;
        ;}
    break;

  case 141:
#line 1074 "gram.y"
    {
            yyval.value = new BinaryExpr(BO_Comma,yyvsp[-2].value,yyvsp[0].value,*yyvsp[-1].loc);
            delete yyvsp[-1].loc;
        ;}
    break;

  case 142:
#line 1081 "gram.y"
    {
            yyval.value = new Variable(yyvsp[0].symbol,NoLocation);
        ;}
    break;

  case 144:
#line 1086 "gram.y"
    {
            yyval.value = yyvsp[0].consValue;
        ;}
    break;

  case 145:
#line 1092 "gram.y"
    {
            yyval.value = yyvsp[-1].value;
            delete yyvsp[-2].loc;
            delete yyvsp[0].loc;
        ;}
    break;

  case 146:
#line 1098 "gram.y"
    {
            yyval.value = (Expression*) NULL;
            delete yyvsp[-2].loc;
            delete yyvsp[0].loc;
        ;}
    break;

  case 154:
#line 1116 "gram.y"
    {
            yyval.value = new IndexExpr(yyvsp[-3].value,yyvsp[-1].value,*yyvsp[-2].loc);
            delete yyvsp[-2].loc;
            delete yyvsp[0].loc;
        ;}
    break;

  case 157:
#line 1128 "gram.y"
    {
            yyval.value = new UnaryExpr(UO_PostInc,yyvsp[-1].value,*yyvsp[0].loc);
            delete yyvsp[0].loc;
        ;}
    break;

  case 158:
#line 1135 "gram.y"
    {
            yyval.value = new UnaryExpr(UO_PostDec,yyvsp[-1].value,*yyvsp[0].loc);
            delete yyvsp[0].loc;
        ;}
    break;

  case 160:
#line 1146 "gram.y"
    {
            Variable *var = new Variable(yyvsp[0].symbol,*yyvsp[-1].loc);
            BinaryExpr *be = new BinaryExpr(BO_Member,yyvsp[-2].value,var,*yyvsp[-1].loc);
            delete yyvsp[-1].loc;
            yyval.value = be;

            // Lookup the component in its struct
            // if possible.
            if (yyvsp[-2].value->etype == ET_Variable)
            {
                Variable  *var = (Variable*) yyvsp[-2].value;
                Symbol    *varName = var->name;
                SymEntry  *entry = varName->entry;

                if (entry && entry->uVarDecl)
                {
                    entry->uVarDecl->lookup(yyvsp[0].symbol);
                }
            }
        ;}
    break;

  case 161:
#line 1169 "gram.y"
    {
            Variable *var = new Variable(yyvsp[0].symbol,*yyvsp[-1].loc);
            BinaryExpr *be = new BinaryExpr(BO_PtrMember,yyvsp[-2].value,var,*yyvsp[-1].loc);
            delete yyvsp[-1].loc;
            yyval.value = be;

            // Lookup the component in its struct
            // if possible.
            if (yyvsp[-2].value->etype == ET_Variable)
            {
                Variable  *var = (Variable*) yyvsp[-2].value;
                Symbol    *varName = var->name;
                SymEntry  *entry = varName->entry;

                if (entry && entry->uVarDecl)
                {
                    entry->uVarDecl->lookup(yyvsp[0].symbol);
                }
            }
        ;}
    break;

  case 162:
#line 1192 "gram.y"
    {
            FunctionCall* fc = new FunctionCall(yyvsp[-3].value,*yyvsp[-2].loc);

            /* add function args */
            fc->addArgs(ReverseList(yyvsp[-1].value));

            delete yyvsp[-2].loc;
            delete yyvsp[0].loc;
            yyval.value = fc;
        ;}
    break;

  case 163:
#line 1205 "gram.y"
    {
            yyval.value = (Expression*) NULL;
        ;}
    break;

  case 167:
#line 1214 "gram.y"
    {
            yyval.value = yyvsp[0].value;
            yyval.value->next = yyvsp[-2].value;

            delete yyvsp[-1].loc;
        ;}
    break;

  case 168:
#line 1221 "gram.y"
    {
            yyval.value = yyvsp[0].value;
            yyval.value->next = yyvsp[-2].value;

            delete yyvsp[-1].loc;
        ;}
    break;

  case 188:
#line 1268 "gram.y"
    {
            if (gProject->Parse_TOS->transUnit)
                gProject->Parse_TOS->transUnit->contxt.ReEnterScope();
        ;}
    break;

  case 189:
#line 1273 "gram.y"
    {
            yyval.decl = (Decl*) NULL;
            if (gProject->Parse_TOS->transUnit)
                gProject->Parse_TOS->transUnit->contxt.ExitScope();
        ;}
    break;

  case 190:
#line 1279 "gram.y"
    {
            if (gProject->Parse_TOS->transUnit)
                gProject->Parse_TOS->transUnit->contxt.ReEnterScope();
            gProject->Parse_TOS->parseCtxt->ResetDeclCtxt();
        ;}
    break;

  case 191:
#line 1284 "gram.y"
    {   gProject->Parse_TOS->parseCtxt->SetVarParam(1, !err_top_level, 0); 
            gProject->Parse_TOS->parseCtxt->SetIsKnR(true); 
        ;}
    break;

  case 192:
#line 1288 "gram.y"
    {   yyval.decl = yyvsp[0].decl;
            gProject->Parse_TOS->parseCtxt->SetIsKnR(false); 
            gProject->Parse_TOS->parseCtxt->SetVarParam(0, !err_top_level, 1); 
            
            // Exit, but will allow re-enter for a function.
            // Hack, to handle parameters being in the function's scope.
            if (gProject->Parse_TOS->transUnit)
                gProject->Parse_TOS->transUnit->contxt.ExitScope(true);
        ;}
    break;

  case 193:
#line 1300 "gram.y"
    {
            if (gProject->Parse_TOS->transUnit)
                gProject->Parse_TOS->transUnit->contxt.EnterScope();
            gProject->Parse_TOS->parseCtxt->ResetDeclCtxt();
        ;}
    break;

  case 194:
#line 1306 "gram.y"
    {
            yyval.decl = (Decl*) NULL;
        ;}
    break;

  case 195:
#line 1310 "gram.y"
    {
            if (gProject->Parse_TOS->transUnit)
                gProject->Parse_TOS->transUnit->contxt.EnterScope();
            gProject->Parse_TOS->parseCtxt->ResetDeclCtxt();
        ;}
    break;

  case 196:
#line 1315 "gram.y"
    {   gProject->Parse_TOS->parseCtxt->SetVarParam(0, !err_top_level, 0); 
        ;}
    break;

  case 197:
#line 1318 "gram.y"
    {   yyval.decl = yyvsp[0].decl;
            gProject->Parse_TOS->parseCtxt->SetVarParam(0, !err_top_level, 0);
        ;}
    break;

  case 198:
#line 1324 "gram.y"
    {
            yyval.decl = yyvsp[-1].decl;
            delete yyvsp[0].loc;
        ;}
    break;

  case 199:
#line 1329 "gram.y"
    {
            yyval.decl = yyvsp[-2].decl;

			Decl*	appendDecl = yyvsp[-2].decl;
			while (appendDecl->next != NULL)
				appendDecl = appendDecl->next;

            appendDecl->next = yyvsp[0].decl;
            delete yyvsp[-1].loc;
        ;}
    break;

  case 200:
#line 1342 "gram.y"
    {
            yyval.declStemnt = new DeclStemnt(*yyvsp[0].loc);
            yyval.declStemnt->addDecls(ReverseList(yyvsp[-1].decl));
            delete yyvsp[0].loc;
        ;}
    break;

  case 201:
#line 1349 "gram.y"
    {
            yyval.declStemnt = new DeclStemnt(*yyvsp[0].loc);
            yyval.declStemnt->addDecls(ReverseList(yyvsp[-1].decl));
            delete yyvsp[0].loc;
        ;}
    break;

  case 202:
#line 1357 "gram.y"
    {
            assert (err_top_level ||
                    yyvsp[-1].base == gProject->Parse_TOS->parseCtxt->curCtxt->decl_specs);
            gProject->Parse_TOS->parseCtxt->ResetDeclCtxt();
            
            yywarn("old-style declaration or incorrect type");

            possibleType = true;
            yyval.decl = yyvsp[0].decl;

            if (yyval.decl == NULL)
            {
                yyval.decl = new Decl(yyvsp[-1].base);
            }
        ;}
    break;

  case 203:
#line 1375 "gram.y"
    {
            assert (1||err_top_level ||
                    yyvsp[-1].base == gProject->Parse_TOS->parseCtxt->curCtxt->decl_specs);
            if (yyvsp[-1].base!=gProject->Parse_TOS->parseCtxt->curCtxt->decl_specs) {
              if (!err_top_level) {
                baseTypeFixup(yyvsp[-1].base,yyvsp[0].decl);
              }
            }
            gProject->Parse_TOS->parseCtxt->ResetDeclCtxt();            
            
            possibleType = true;
            yyval.decl = yyvsp[0].decl;
            
            if (yyval.decl == NULL)
            {
                yyval.decl = new Decl(yyvsp[-1].base);
            }
        ;}
    break;

  case 204:
#line 1400 "gram.y"
    {
            yyval.base = new BaseType(BT_Int);
            gProject->Parse_TOS->parseCtxt->SetDeclCtxt(yyval.base);
        ;}
    break;

  case 207:
#line 1419 "gram.y"
    {   
            gProject->Parse_TOS->parseCtxt->PushCtxt();
            gProject->Parse_TOS->parseCtxt->ResetVarParam();
        ;}
    break;

  case 208:
#line 1424 "gram.y"
    {
            yyval.type = yyvsp[0].type;
            gProject->Parse_TOS->parseCtxt->PopCtxt(false);
        ;}
    break;

  case 209:
#line 1431 "gram.y"
    {
            assert (yyvsp[0].base == gProject->Parse_TOS->parseCtxt->curCtxt->decl_specs);
            
            possibleType = true;
            yyval.type = yyvsp[0].base;
            if (yyval.type->isFunction())
                yyerr ("Function type not allowed as type name");
        ;}
    break;

  case 210:
#line 1440 "gram.y"
    {
            assert (yyvsp[-1].base == gProject->Parse_TOS->parseCtxt->curCtxt->decl_specs);
            
            possibleType = true;
            yyval.type = yyvsp[0].type;
            
            Type * extended = yyval.type->extend(yyvsp[-1].base);
            if (yyval.type->isFunction())
                yyerr ("Function type not allowed as type name");
            else if (extended && 
                yyvsp[-1].base && yyvsp[-1].base->isFunction() && 
                ! extended->isPointer())
                yyerr ("Wrong type combination") ;
                
        ;}
    break;

  case 211:
#line 1463 "gram.y"
    {
            gProject->Parse_TOS->parseCtxt->SetDeclCtxt(yyval.base);
        ;}
    break;

  case 218:
#line 1479 "gram.y"
    {
            if (! gProject->Parse_TOS->transUnit ||
                gProject->Parse_TOS->transUnit->contxt.syms->current->level >= FUNCTION_SCOPE)
                 yyval.storage = yyvsp[0].storage ;             
             else
                 yyval.storage = ST_None ;              
        ;}
    break;

  case 220:
#line 1492 "gram.y"
    {
            yyval.base = (BaseType*) NULL;
        ;}
    break;

  case 222:
#line 1499 "gram.y"
    {
            yyval.base = yyvsp[0].base;

            if (!yyval.base)
            {
                yyval.base = new BaseType();
            }

            if (yyvsp[-1].storage == ST_None)
                 yyerr("Invalid use of local storage type");
            else if (yyval.base->storage != ST_None)             
                 yywarn("Overloading previous storage type specification");
            else
                 yyval.base->storage = yyvsp[-1].storage;
        ;}
    break;

  case 223:
#line 1514 "gram.y"
    { possibleType = false; ;}
    break;

  case 224:
#line 1515 "gram.y"
    {
            yyval.base = yyvsp[-2].base;

            if (yyvsp[0].base)
            {
                if ((yyvsp[0].base->typemask & BT_Long)
                    && (yyval.base->typemask & BT_Long))
                {
                   // long long : A likely C9X addition 
                   yyval.base->typemask &= ~BT_Long;
                   yyvsp[0].base->typemask &= ~BT_Long;
                   yyval.base->typemask |= yyvsp[0].base->typemask;
                   yyval.base->typemask |=  BT_LongLong;
                }
                else
                    yyval.base->typemask |= yyvsp[0].base->typemask;

                if (yyvsp[0].base->storage != ST_None)
                    yyval.base->storage = yyvsp[0].base->storage;

                // delete $3;
            }

            /*
            std::cout << "In decl_spec: ";
            $$->printBase(std::cout,0);
            if ($$->storage == ST_Typedef)
                std::cout << "(is a typedef)";
            std::cout << std::endl;
            */
        ;}
    break;

  case 225:
#line 1547 "gram.y"
    {
            yyval.base = yyvsp[0].base;

            if (!yyval.base)
            {
                yyval.base = new BaseType();
            }

            if (TQ_None != (yyval.base->qualifier & yyvsp[-1].typeQual))
                yywarn("qualifier already specified");  
                              
            yyval.base->qualifier |= yyvsp[-1].typeQual;

        ;}
    break;

  case 226:
#line 1567 "gram.y"
    {
           yyval.base = (BaseType*) NULL;
        ;}
    break;

  case 228:
#line 1573 "gram.y"
    { possibleType = false; ;}
    break;

  case 229:
#line 1574 "gram.y"
    {
            yyval.base = yyvsp[-2].base;

            if (yyvsp[0].base)
            {
                yyval.base->typemask |= yyvsp[0].base->typemask;
                // delete $3;
            }
        ;}
    break;

  case 230:
#line 1584 "gram.y"
    {
            yyval.base = yyvsp[0].base;

            if (!yyval.base)
            {
                yyval.base = new BaseType();
            }

            if (TQ_None != (yyval.base->qualifier & yyvsp[-1].typeQual))
                yywarn("qualifier already specified");
            yyval.base->qualifier |= yyvsp[-1].typeQual;
        ;}
    break;

  case 231:
#line 1599 "gram.y"
    {
            gProject->Parse_TOS->parseCtxt->SetDeclCtxt(yyval.base);
        ;}
    break;

  case 232:
#line 1608 "gram.y"
    {
           yyvsp[0].decl->extend(gProject->Parse_TOS->parseCtxt->UseDeclCtxt());
        ;}
    break;

  case 233:
#line 1612 "gram.y"
    {
           yyvsp[-2].decl->attrib = yyvsp[0].gccAttrib;
           yyval.decl = yyvsp[-2].decl;
        ;}
    break;

  case 235:
#line 1620 "gram.y"
    {
           yyvsp[-2].decl->initializer = yyvsp[0].value;
           yyval.decl = yyvsp[-2].decl;
        ;}
    break;

  case 236:
#line 1627 "gram.y"
    {
          yyval.decl = (Decl*) NULL;
        ;}
    break;

  case 239:
#line 1637 "gram.y"
    {
            yyval.decl = yyvsp[0].decl;
        ;}
    break;

  case 240:
#line 1641 "gram.y"
    {
            yyval.decl = yyvsp[-2].decl;

			Decl*	appendDecl = yyvsp[-2].decl;
			while (appendDecl->next != NULL)
				appendDecl = appendDecl->next;

            appendDecl->next = yyvsp[0].decl;
            delete yyvsp[-1].loc;
        ;}
    break;

  case 242:
#line 1659 "gram.y"
    {
            yyval.arrayConst = new ArrayConstant(NoLocation);
            yyval.arrayConst->addElement(yyvsp[0].value);
        ;}
    break;

  case 243:
#line 1664 "gram.y"
    {
            yyval.arrayConst = yyvsp[-2].arrayConst;
            yyval.arrayConst->addElement(yyvsp[0].value);
            delete yyvsp[-1].loc;
        ;}
    break;

  case 247:
#line 1675 "gram.y"
    {
            yyval.value = yyvsp[-2].arrayConst;
            delete yyvsp[-3].loc;
            delete yyvsp[0].loc;
        ;}
    break;

  case 248:
#line 1683 "gram.y"
    {
            yyval.loc = (Location*) NULL;
        ;}
    break;

  case 249:
#line 1687 "gram.y"
    {
            delete yyvsp[0].loc;
            yyval.loc = (Location*) NULL;
        ;}
    break;

  case 257:
#line 1708 "gram.y"
    {
           TypeQual r(yyvsp[-3].typeQual);
           r.vout=yyvsp[-1].value;
           yyval.typeQual = r;
        ;}
    break;

  case 259:
#line 1717 "gram.y"
    {
            yyval.typeQual = yyvsp[-1].typeQual | yyvsp[0].typeQual;
            if (TQ_None != (yyvsp[0].typeQual & yyvsp[-1].typeQual))
                yywarn("qualifier already specified");                               
        ;}
    break;

  case 260:
#line 1725 "gram.y"
    {
            yyval.typeQual = TQ_None;
        ;}
    break;

  case 281:
#line 1757 "gram.y"
    {
            yyval.base = new BaseType(BT_UserType);
            yyval.base->typeName = yyvsp[0].symbol;
        ;}
    break;

  case 282:
#line 1764 "gram.y"
    {
            assert ((! yyval.symbol->entry) || 
                    yyval.symbol->entry->IsTagDecl()) ;
        ;}
    break;

  case 283:
#line 1772 "gram.y"
    {
            if (gProject->Parse_TOS->transUnit)
                yyval.base = gProject->Parse_TOS->parseCtxt->Mk_tag_ref(yyvsp[-1].typeSpec, yyvsp[0].symbol,
                                                                gProject->Parse_TOS->transUnit->contxt.tags);
            else
                yyval.base = NULL;                                         
        ;}
    break;

  case 284:
#line 1782 "gram.y"
    {
            if (gProject->Parse_TOS->transUnit)
                yyval.base = gProject->Parse_TOS->parseCtxt->Mk_tag_ref(yyvsp[-1].typeSpec, yyvsp[0].symbol,
                                                                gProject->Parse_TOS->transUnit->contxt.tags);
            else
                yyval.base = NULL;                                         
        ;}
    break;

  case 285:
#line 1792 "gram.y"
    {
            if (gProject->Parse_TOS->transUnit)
                yyval.base = gProject->Parse_TOS->parseCtxt->Mk_tag_ref(yyvsp[-1].typeSpec, yyvsp[0].symbol,
                                                                gProject->Parse_TOS->transUnit->contxt.tags);
            else
                yyval.base = NULL;                                         
        ;}
    break;

  case 286:
#line 1802 "gram.y"
    {
            if (gProject->Parse_TOS->transUnit)
                yyval.base = gProject->Parse_TOS->parseCtxt->Mk_tag_def(yyvsp[-1].typeSpec, yyvsp[0].symbol,
                                                            gProject->Parse_TOS->transUnit->contxt.tags);
            else
                yyval.base = NULL;                                         
        ;}
    break;

  case 287:
#line 1812 "gram.y"
    {
            yyval.base = new BaseType(yyvsp[-1].strDef);
            yyvsp[-1].strDef->_isUnion = false;
            delete yyvsp[-2].loc;
            delete yyvsp[0].loc;
        ;}
    break;

  case 288:
#line 1819 "gram.y"
    {
            yyval.base = yyvsp[-3].base;
            assert (! yyval.base->stDefn);
            yyval.base->stDefn = yyvsp[-1].strDef;
            yyvsp[-1].strDef->tag = yyvsp[-3].base->tag->dup();
            yyvsp[-1].strDef->_isUnion = false;

            // Overload the incomplete definition
            yyval.base->tag->entry->uStructDef = yyval.base ;
            
//             std::cout << "struct/union/enum_type_define:"
//                          "The definition of:"
//                       << "(uStructDef:" << $1->tag->entry->uStructDef << ")"
//                       << "(uStructDef->stDefn:" << $1->tag->entry->uStructDef->stDefn << ")"
//                       << "(" << $1->tag->entry << ")" << $1->tag->name  << "$" ;
//             $1->tag->entry->scope->ShowScopeId(std::cout);
//             std::cout << " has been completed" << endl; 
            
            delete yyvsp[-2].loc;
            delete yyvsp[0].loc;
        ;}
    break;

  case 289:
#line 1843 "gram.y"
    {
            if (gProject->Parse_TOS->transUnit)
                yyval.base = gProject->Parse_TOS->parseCtxt->Mk_tag_def(yyvsp[-1].typeSpec, yyvsp[0].symbol,
                                                            gProject->Parse_TOS->transUnit->contxt.tags);
            else
              yyval.base = NULL ;
        ;}
    break;

  case 290:
#line 1853 "gram.y"
    {
            yyval.base = new BaseType(yyvsp[-1].strDef);
            yyvsp[-1].strDef->_isUnion = true;

            delete yyvsp[-2].loc;
            delete yyvsp[0].loc;
        ;}
    break;

  case 291:
#line 1861 "gram.y"
    {
            yyval.base = yyvsp[-3].base;
            assert (! yyval.base->stDefn);
            yyval.base->stDefn = yyvsp[-1].strDef;
            yyvsp[-1].strDef->tag = yyvsp[-3].base->tag->dup();
            yyvsp[-1].strDef->_isUnion = true;

            // Overload the incomplete definition
            yyval.base->tag->entry->uStructDef = yyval.base ;
            
//             std::cout << "struct/union/enum_type_define:"
//                          "The definition of:"
//                       << "(uStructDef:" << $1->tag->entry->uStructDef << ")"
//                       << "(uStructDef->stDefn:" << $1->tag->entry->uStructDef->stDefn << ")"
//                       << "(" << $1->tag->entry << ")" << $1->tag->name  << "$" ;
//             $1->tag->entry->scope->ShowScopeId(std::cout);
//             std::cout << " has been completed" << endl; 
            
            delete yyvsp[-2].loc;
            delete yyvsp[0].loc;
 
        ;}
    break;

  case 292:
#line 1886 "gram.y"
    {
            if (gProject->Parse_TOS->transUnit)
                yyval.base = gProject->Parse_TOS->parseCtxt->Mk_tag_def(yyvsp[-1].typeSpec,yyvsp[0].symbol,
                                                            gProject->Parse_TOS->transUnit->contxt.tags);
            else
              yyval.base = NULL;
        ;}
    break;

  case 293:
#line 1896 "gram.y"
    {
            yyval.base = new BaseType(yyvsp[-1].enDef);

            delete yyvsp[-2].loc;
            delete yyvsp[0].loc;
        ;}
    break;

  case 294:
#line 1903 "gram.y"
    {
            yyval.base = yyvsp[-3].base;
            assert (! yyval.base->enDefn);
            yyval.base->enDefn = yyvsp[-1].enDef;
            yyvsp[-1].enDef->tag = yyvsp[-3].base->tag->dup();

            // Overload the incomplete definition
            yyval.base->tag->entry->uStructDef = yyval.base ;
            
//             std::cout << "struct/union/enum_type_define:"
//                          "The definition of:"
//                       << "(uStructDef:" << $1->tag->entry->uStructDef << ")"
//                       << "(uStructDef->stDefn:" << $1->tag->entry->uStructDef->stDefn << ")"
//                       << "(" << $1->tag->entry << ")" << $1->tag->name  << "$" ;
//             $1->tag->entry->scope->ShowScopeId(std::cout);
//             std::cout << " has been completed" << endl; 
            
            delete yyvsp[-2].loc;
            delete yyvsp[0].loc;
        ;}
    break;

  case 295:
#line 1926 "gram.y"
    {  yyval.strDef = new StructDef();
           yywarn("ANSI/ISO C prohibits empty struct/union");
        ;}
    break;

  case 297:
#line 1933 "gram.y"
    {  yyval.enDef = new EnumDef();
           yywarn("ANSI/ISO C prohibits empty enum");
        ;}
    break;

  case 298:
#line 1937 "gram.y"
    {  yyval.enDef = yyvsp[-1].enDef;
        ;}
    break;

  case 299:
#line 1942 "gram.y"
    {
            yyval.loc = NULL;
        ;}
    break;

  case 300:
#line 1946 "gram.y"
    {
          yywarn("Trailing comma in enum type definition");
        ;}
    break;

  case 302:
#line 1959 "gram.y"
    {
            yyval.enDef = new EnumDef();
            yyval.enDef->addElement(yyvsp[0].enConst);
        ;}
    break;

  case 303:
#line 1964 "gram.y"
    {
            yyval.enDef = yyvsp[-2].enDef;
            yyval.enDef->addElement(yyvsp[0].enConst);
            delete yyvsp[-1].loc;
        ;}
    break;

  case 304:
#line 1972 "gram.y"
    {
            yyval.enConst = new EnumConstant(yyvsp[0].symbol,NULL,NoLocation);
            if (gProject->Parse_TOS->transUnit)
            {
              if (gProject->Parse_TOS->transUnit->contxt.syms->IsDefined(yyvsp[0].symbol->name))
                 yyerr("Duplicate enumeration constant");
                 
              yyvsp[0].symbol->entry = gProject->Parse_TOS->transUnit->contxt.syms->Insert(
                                  mk_enum_const(yyvsp[0].symbol->name, yyval.enConst));
            }
        ;}
    break;

  case 305:
#line 1984 "gram.y"
    {
            yyval.enConst = new EnumConstant(yyvsp[-2].symbol,yyvsp[0].value,NoLocation);
            if (gProject->Parse_TOS->transUnit)
            {
              if (gProject->Parse_TOS->transUnit->contxt.syms->IsDefined(yyvsp[-2].symbol->name))
                 yyerr("Duplicate enumeration constant");
                 
              yyvsp[-2].symbol->entry = gProject->Parse_TOS->transUnit->contxt.syms->Insert(
                                  mk_enum_const(yyvsp[-2].symbol->name, yyval.enConst));
            }
        ;}
    break;

  case 307:
#line 2004 "gram.y"
    {
            if (gProject->Parse_TOS->transUnit)
                gProject->Parse_TOS->transUnit->contxt.EnterScope();
            gProject->Parse_TOS->parseCtxt->PushCtxt();
        ;}
    break;

  case 308:
#line 2009 "gram.y"
    {
            assert (!err_top_level || possibleType);
             /* Safety precaution! */
             possibleType=true;
        ;}
    break;

  case 309:
#line 2015 "gram.y"
    {
            gProject->Parse_TOS->parseCtxt->PopCtxt(false);
            if (gProject->Parse_TOS->transUnit)
                gProject->Parse_TOS->transUnit->contxt.ExitScope();
            yyval.strDef = yyvsp[0].strDef ;
        ;}
    break;

  case 310:
#line 2024 "gram.y"
    {
            yyval.strDef = new StructDef();
            yyval.strDef->addComponent(ReverseList(yyvsp[-1].decl));
            delete yyvsp[0].loc;
        ;}
    break;

  case 311:
#line 2030 "gram.y"
    {
            // A useful gcc extension:
            //   naked semicolons in struct/union definitions. 
            yyval.strDef = yyvsp[-1].strDef;
            yywarn ("Empty declaration");
            delete yyvsp[0].loc;
        ;}
    break;

  case 312:
#line 2038 "gram.y"
    {
            yyval.strDef = yyvsp[-2].strDef;
            yyval.strDef->addComponent(ReverseList(yyvsp[-1].decl));
            delete yyvsp[0].loc;
        ;}
    break;

  case 313:
#line 2046 "gram.y"
    {
            possibleType = true;
            yyval.decl = yyvsp[0].decl;
        ;}
    break;

  case 314:
#line 2051 "gram.y"
    {
            possibleType = true;
            yyval.decl = new Decl (yyvsp[0].base);
            yywarn ("No field declarator");
        ;}
    break;

  case 316:
#line 2065 "gram.y"
    {   gProject->Parse_TOS->parseCtxt->SetIsFieldId(true); 
        ;}
    break;

  case 317:
#line 2068 "gram.y"
    {
            yyval.decl = yyvsp[-1].decl;
            yyval.decl->attrib = yyvsp[0].gccAttrib;
        ;}
    break;

  case 318:
#line 2073 "gram.y"
    {   gProject->Parse_TOS->parseCtxt->SetIsFieldId(true); 
        ;}
    break;

  case 319:
#line 2076 "gram.y"
    {
            yyval.decl = yyvsp[-1].decl;
            yyval.decl->attrib = yyvsp[0].gccAttrib;
            yyval.decl->next = yyvsp[-4].decl;
            delete yyvsp[-3].loc;
        ;}
    break;

  case 320:
#line 2085 "gram.y"
    {
           gProject->Parse_TOS->parseCtxt->SetIsFieldId(false); 
           Type * decl = gProject->Parse_TOS->parseCtxt->UseDeclCtxt();
           Type * extended = yyval.decl->extend(decl);
           if (yyval.decl->form &&
               yyval.decl->form->isFunction())
               yyerr ("Function type not allowed as field");
           else if (yyval.decl->form &&
                    yyval.decl->form->isArray() &&
                    ! ((ArrayType *) yyval.decl->form)->size)
               yyerr ("Unsized array not allowed as field");
           else if (extended && 
               decl && decl->isFunction() && 
               ! extended->isPointer())
               yyerr ("Wrong type combination") ;
                
        ;}
    break;

  case 321:
#line 2103 "gram.y"
    {
           Type * decl = gProject->Parse_TOS->parseCtxt->UseDeclCtxt();
           yyval.decl->extend(decl);
           if (! decl)
               yyerr ("No type specifier for bit field") ;
           else if (!yyval.decl->form)
               yyerr ("Wrong type combination") ;
        ;}
    break;

  case 323:
#line 2117 "gram.y"
    {   gProject->Parse_TOS->parseCtxt->SetIsFieldId(false); 
        ;}
    break;

  case 324:
#line 2120 "gram.y"
    {
            BitFieldType  *bf = new BitFieldType(yyvsp[0].value);
            yyval.decl = yyvsp[-3].decl;

            if (yyval.decl == NULL)
            {
                yyval.decl = new Decl(bf);
            }
            else
            {
                bf->subType = yyval.decl->form;
                yyval.decl->form = bf;
            }
        ;}
    break;

  case 326:
#line 2140 "gram.y"
    {
           yyval.decl = (Decl*) NULL;
        ;}
    break;

  case 328:
#line 2151 "gram.y"
    {
            gProject->Parse_TOS->parseCtxt->Mk_declarator (yyval.decl);
        ;}
    break;

  case 329:
#line 2157 "gram.y"
    {
            gProject->Parse_TOS->parseCtxt->Mk_func_declarator (yyval.decl);
        ;}
    break;

  case 330:
#line 2163 "gram.y"
    {
            yyval.decl = yyvsp[0].decl;
            yyval.decl->extend(yyvsp[-1].ptr);
        ;}
    break;

  case 333:
#line 2174 "gram.y"
    {  if (gProject->Parse_TOS->transUnit)
                yyval.decl = gProject->Parse_TOS->parseCtxt->Mk_direct_declarator_reentrance (yyvsp[0].symbol,
                gProject->Parse_TOS->transUnit->contxt.syms);
        ;}
    break;

  case 334:
#line 2179 "gram.y"
    {
            yyval.decl = yyvsp[-1].decl;
            delete yyvsp[-2].loc ;
            delete yyvsp[0].loc ;
        ;}
    break;

  case 337:
#line 2187 "gram.y"
    {
            yyval.decl = yyvsp[-3].decl;
            FunctionType * ft = new FunctionType(ReverseList(yyvsp[-1].decl));
            Type * extended = yyval.decl->extend(ft);
            if (extended && ! extended->isPointer())
                yyerr ("Wrong type combination") ;
                
            delete yyvsp[-2].loc ;
            delete yyvsp[0].loc ;
            // Exit, but will allow re-enter for a function.
            // Hack, to handle parameters being in the function's scope.
            if (gProject->Parse_TOS->transUnit)
                gProject->Parse_TOS->transUnit->contxt.ExitScope(true);

        ;}
    break;

  case 338:
#line 2203 "gram.y"
    {
            yyval.decl = yyvsp[-3].decl;
            FunctionType * ft = new FunctionType(ReverseList(yyvsp[-1].decl));
            Type * extended = yyval.decl->extend(ft);
            if (extended && ! extended->isPointer())
                yyerr ("Wrong type combination") ;

            delete yyvsp[-2].loc ;
            delete yyvsp[0].loc ;
            // Exit, but will allow re-enter for a function.
            // Hack, to handle parameters being in the function's scope.
            if (gProject->Parse_TOS->transUnit)
                gProject->Parse_TOS->transUnit->contxt.ExitScope(true);

        ;}
    break;

  case 339:
#line 2219 "gram.y"
    {
            yyval.decl = yyvsp[-2].decl;

			if (yyval.decl != NULL)
			{
				FunctionType* ft = new FunctionType();
				Type* extended = yyval.decl->extend(ft);
				if (extended && ! extended->isPointer())
           	 	    yyerr ("Wrong type combination") ;
			}
            
            delete yyvsp[-1].loc ;
            delete yyvsp[0].loc ;
            if (gProject->Parse_TOS->transUnit)
            {
                gProject->Parse_TOS->transUnit->contxt.EnterScope();
                // Exit, but will allow re-enter for a function.
                // Hack, to handle parameters being in the function's scope.
                gProject->Parse_TOS->transUnit->contxt.ExitScope(true);
            }
        ;}
    break;

  case 340:
#line 2243 "gram.y"
    {
            yyval.decl = yyvsp[-3].decl;
            ArrayType * at = new ArrayType(TT_Array, yyvsp[-1].value);
            Type * extended = yyval.decl->extend(at);
            if (extended && 
                extended->isFunction())
                yyerr ("Wrong type combination") ;
              
            delete yyvsp[-2].loc ;
            delete yyvsp[0].loc ;
        ;}
    break;

  case 341:
#line 2257 "gram.y"
    {
            yyval.decl = yyvsp[-3].decl;
            ArrayType * at = new ArrayType(TT_Stream, yyvsp[-1].value);
            Type * extended = yyval.decl->extend(at);

            if (extended &&
                extended->isFunction())
                yyerr ("Wrong type combination") ;
        ;}
    break;

  case 342:
#line 2268 "gram.y"
    {
            yyval.value = yyvsp[0].consValue;
        ;}
    break;

  case 343:
#line 2272 "gram.y"
    {
            yyval.value = yyvsp[0].value;
        ;}
    break;

  case 344:
#line 2276 "gram.y"
    {
           yyval.value = yyvsp[-1].value;
        ;}
    break;

  case 345:
#line 2280 "gram.y"
    { 
            yyval.value = new Variable (yyvsp[0].symbol,NoLocation);
        ;}
    break;

  case 346:
#line 2286 "gram.y"
    {
	   yyval.value = NULL;
	;}
    break;

  case 347:
#line 2290 "gram.y"
    {
            yyval.value = yyvsp[0].value;
        ;}
    break;

  case 348:
#line 2294 "gram.y"
    {
            yyval.value = new BinaryExpr(BO_Comma,yyvsp[-2].value,yyvsp[0].value,*yyvsp[-1].loc);
            delete yyvsp[-1].loc;
        ;}
    break;

  case 349:
#line 2304 "gram.y"
    {
            yyval.ptr = new PtrType(yyvsp[0].typeQual);    
        ;}
    break;

  case 351:
#line 2311 "gram.y"
    {
            yyval.ptr = yyvsp[0].ptr;
            yyval.ptr->subType = yyvsp[-1].ptr;
        ;}
    break;

  case 353:
#line 2324 "gram.y"
    {  gProject->Parse_TOS->parseCtxt->IncrVarParam(1);
          if (gProject->Parse_TOS->transUnit)
              gProject->Parse_TOS->transUnit->contxt.EnterScope();
          gProject->Parse_TOS->parseCtxt->PushCtxt();
        ;}
    break;

  case 354:
#line 2330 "gram.y"
    {
          // Exit, but will allow re-enter for a function.
          // Hack, to handle parameters being in the function's scope.
          gProject->Parse_TOS->parseCtxt->PopCtxt(true);
          gProject->Parse_TOS->parseCtxt->IncrVarParam(-1);
          yyval.decl = yyvsp[0].decl;
       ;}
    break;

  case 355:
#line 2340 "gram.y"
    {  if (gProject->Parse_TOS->transUnit)
               yyval.decl = gProject->Parse_TOS->parseCtxt->Mk_direct_declarator_reentrance (yyvsp[0].symbol,
                gProject->Parse_TOS->transUnit->contxt.syms);
        ;}
    break;

  case 356:
#line 2345 "gram.y"
    {  yyval.decl = yyvsp[-2].decl;
           if (gProject->Parse_TOS->transUnit)
           {
              yyval.decl = gProject->Parse_TOS->parseCtxt->Mk_direct_declarator_reentrance (yyvsp[0].symbol,
                gProject->Parse_TOS->transUnit->contxt.syms);
              yyval.decl->next = yyvsp[-2].decl;
           }
        ;}
    break;

  case 358:
#line 2361 "gram.y"
    {
            /* Convert a TYPEDEF_NAME back into a normal IDENT */
            yyval.symbol = yyvsp[0].symbol;
            yyval.symbol->entry = (SymEntry*) NULL;
        ;}
    break;

  case 361:
#line 2382 "gram.y"
    {
           yyval.decl = (Decl*) NULL;
        ;}
    break;

  case 362:
#line 2386 "gram.y"
    { gProject->Parse_TOS->parseCtxt->IncrVarParam(1); 
        ;}
    break;

  case 363:
#line 2389 "gram.y"
    { gProject->Parse_TOS->parseCtxt->IncrVarParam(-1); 
           yyval.decl = yyvsp[0].decl;
        ;}
    break;

  case 364:
#line 2395 "gram.y"
    {   gProject->Parse_TOS->parseCtxt->IncrVarParam(1);
            if (gProject->Parse_TOS->transUnit)
                gProject->Parse_TOS->transUnit->contxt.EnterScope();
            gProject->Parse_TOS->parseCtxt->PushCtxt();
        ;}
    break;

  case 365:
#line 2401 "gram.y"
    {
            gProject->Parse_TOS->parseCtxt->PopCtxt(true);
            gProject->Parse_TOS->parseCtxt->IncrVarParam(-1);
            yyval.decl = yyvsp[0].decl ;
        ;}
    break;

  case 367:
#line 2410 "gram.y"
    {
            BaseType *bt = new BaseType(BT_Ellipsis);

            yyval.decl = new Decl(bt);
            yyval.decl->next = yyvsp[-2].decl;
        ;}
    break;

  case 369:
#line 2420 "gram.y"
    {
            yyval.decl = yyvsp[0].decl;
            yyval.decl->next = yyvsp[-2].decl;
        ;}
    break;

  case 370:
#line 2427 "gram.y"
    {   
            gProject->Parse_TOS->parseCtxt->PushCtxt();
        ;}
    break;

  case 371:
#line 2431 "gram.y"
    {
            gProject->Parse_TOS->parseCtxt->PopCtxt(true);
            yyval.decl = yyvsp[0].decl;
        ;}
    break;

  case 372:
#line 2438 "gram.y"
    {
            assert (err_top_level ||
                    yyvsp[-1].base == gProject->Parse_TOS->parseCtxt->curCtxt->decl_specs);
            possibleType = true;
            yyval.decl = yyvsp[0].decl;
            Type * decl = gProject->Parse_TOS->parseCtxt->UseDeclCtxt();
            Type * extended = yyval.decl->extend(decl);             
            if (yyval.decl->form &&
                yyval.decl->form->isFunction())
                yyerr ("Function type not allowed");
            else if (extended && 
                decl && decl->isFunction() && 
                ! extended->isPointer())
                yyerr ("Wrong type combination") ;
        ;}
    break;

  case 373:
#line 2454 "gram.y"
    {
            assert (err_top_level ||
                    yyvsp[-1].base == gProject->Parse_TOS->parseCtxt->curCtxt->decl_specs);
            possibleType = true;
            yyval.decl = new Decl(yyvsp[0].type);
            
            Type * decl = gProject->Parse_TOS->parseCtxt->UseDeclCtxt();
            Type * extended = yyval.decl->extend(decl);
            if (yyval.decl->form &&
                yyval.decl->form->isFunction())
                yyerr ("Function type not allowed for parameter");
            else if (extended && 
                decl && decl->isFunction() && 
                ! extended->isPointer())
                yyerr ("Wrong type combination") ;
        ;}
    break;

  case 374:
#line 2471 "gram.y"
    {
            possibleType = true;
            yyval.decl = new Decl(yyvsp[0].base);
            if (yyval.decl->form &&
                yyval.decl->form->isFunction())
                yyerr ("Function type not allowed for parameter");
        ;}
    break;

  case 375:
#line 2484 "gram.y"
    {
            yyval.type = yyvsp[0].ptr;
        ;}
    break;

  case 376:
#line 2488 "gram.y"
    {
            yyval.type = yyvsp[0].type;
        ;}
    break;

  case 377:
#line 2492 "gram.y"
    {
            yyval.type = yyvsp[0].type;
            yyval.type->extend(yyvsp[-1].ptr);
        ;}
    break;

  case 379:
#line 2502 "gram.y"
    {
            yyval.type = yyvsp[-1].type;
        ;}
    break;

  case 380:
#line 2506 "gram.y"
    {
            yyval.type = new ArrayType(TT_Array, yyvsp[-1].value);
        ;}
    break;

  case 381:
#line 2510 "gram.y"
    {
            ArrayType *at = new ArrayType(TT_Array, yyvsp[-1].value);
            yyval.type = yyvsp[-3].type;
            yyval.type->extend(at);
            Type * extended = yyval.type->extend(at) ;
            if (extended && 
                extended->isFunction())
                yyerr ("Wrong type combination") ;
        ;}
    break;

  case 382:
#line 2520 "gram.y"
    {
            yyval.type = new FunctionType(ReverseList(yyvsp[-1].decl));
        ;}
    break;

  case 383:
#line 2524 "gram.y"
    {
            FunctionType * ft = new FunctionType(ReverseList(yyvsp[-1].decl));
            yyval.type = yyvsp[-3].type;
            Type * extended = yyval.type->extend(ft) ;
            if (extended && 
                ! extended->isPointer())
                yyerr ("Wrong type combination") ;
                
        ;}
    break;

  case 384:
#line 2540 "gram.y"
    {
            yyval.gccAttrib = (GccAttrib*) NULL;
        ;}
    break;

  case 386:
#line 2547 "gram.y"
    {
                yyval.gccAttrib = yyvsp[-2].gccAttrib;
                delete yyvsp[-4].loc;
                delete yyvsp[-3].loc;
                delete yyvsp[-1].loc;
                delete yyvsp[0].loc;
            ;}
    break;

  case 387:
#line 2557 "gram.y"
    {
                /* The lexer ate some unsupported option. */
                yyval.gccAttrib = new GccAttrib( GCC_Unsupported);
            ;}
    break;

  case 388:
#line 2562 "gram.y"
    {
                yyval.gccAttrib = new GccAttrib( GCC_Packed );
            ;}
    break;

  case 389:
#line 2566 "gram.y"
    {
                yyval.gccAttrib = new GccAttrib( GCC_CDecl );
            ;}
    break;

  case 390:
#line 2570 "gram.y"
    {
                yyval.gccAttrib = new GccAttrib( GCC_Const );
            ;}
    break;

  case 391:
#line 2574 "gram.y"
    {
                yyval.gccAttrib = new GccAttrib( GCC_NoReturn );
            ;}
    break;

  case 392:
#line 2578 "gram.y"
    {
                yyval.gccAttrib = new GccAttrib( GCC_Aligned );

                if (yyvsp[-1].consValue->ctype == CT_Int)
                {
                    IntConstant    *iCons = (IntConstant*) yyvsp[-1].consValue;

                    yyval.gccAttrib->value = iCons->lng;
                }

                delete yyvsp[-2].loc;
                delete yyvsp[0].loc;
            ;}
    break;

  case 393:
#line 2592 "gram.y"
    {
                yyval.gccAttrib = new GccAttrib( GCC_Mode );

                yyval.gccAttrib->mode = yyvsp[-1].symbol;

                delete yyvsp[-2].loc;
                delete yyvsp[0].loc;
            ;}
    break;

  case 394:
#line 2601 "gram.y"
    {
                yyval.gccAttrib = new GccAttrib( GCC_Format );
    
                yyval.gccAttrib->mode = yyvsp[-5].symbol;

                if (yyvsp[-3].consValue->ctype == CT_Int)
                {
                    IntConstant    *iCons = (IntConstant*) yyvsp[-3].consValue;

                    yyval.gccAttrib->strIdx = iCons->lng;
                }

                if (yyvsp[-1].consValue->ctype == CT_Int)
                {
                    IntConstant    *iCons = (IntConstant*) yyvsp[-1].consValue;

                    yyval.gccAttrib->first = iCons->lng;
                }

                delete yyvsp[-6].loc;
                delete yyvsp[0].loc;
            ;}
    break;


    }

/* Line 999 of yacc.c.  */
#line 4617 "gram.tab.c"

  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  const char* yyprefix;
	  char *yymsg;
	  int yyx;

	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  int yyxbegin = yyn < 0 ? -yyn : 0;

	  /* Stay within bounds of both yycheck and yytname.  */
	  int yychecklim = YYLAST - yyn;
	  int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
	  int yycount = 0;

	  yyprefix = ", expecting ";
	  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      {
		yysize += yystrlen (yyprefix) + yystrlen (yytname [yyx]);
		yycount += 1;
		if (yycount == 5)
		  {
		    yysize = 0;
		    break;
		  }
	      }
	  yysize += (sizeof ("syntax error, unexpected ")
		     + yystrlen (yytname[yytype]));
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yyprefix = ", expecting ";
		  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			yyp = yystpcpy (yyp, yyprefix);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yyprefix = " or ";
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* Return failure if at end of input.  */
      if (yychar == YYEOF)
        {
	  /* Pop the error token.  */
          YYPOPSTACK;
	  /* Pop the rest of the stack.  */
	  while (yyss < yyssp)
	    {
	      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
	      yydestruct (yystos[*yyssp], yyvsp);
	      YYPOPSTACK;
	    }
	  YYABORT;
        }

      YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
      yydestruct (yytoken, &yylval);
      yychar = YYEMPTY;

    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*----------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action.  |
`----------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      yyvsp--;
      yystate = *--yyssp;

      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


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

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 2625 "gram.y"


/*******************************************************/

