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




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif





