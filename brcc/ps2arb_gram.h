#ifndef BISON_PS2ARB_GRAM_HPP
# define BISON_PS2ARB_GRAM_HPP

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


extern YYSTYPE ps2arb_lval;

#endif /* not BISON_PS2ARB_GRAM_HPP */
