
#################
## Brook2Cg 
##  Makefile
##  Written by Ian Buck, 2002
#################

## XXX: This really is only a placeholder Makefile until a 
## real cross platform one can be built.



include ../config/$(OS).mk

all: makedirs ../bin/brcc${BINSUFFIX}

makedirs:
	@if test ! -d ../bin; then mkdir ../bin; fi

fp30_assist.cg.bin: fp30_assist.cg
	cpp -P < fp30_assist.cg | perl makestring.pl > fp30_assist.cg.bin

../bin/brcc${BINSUFFIX}: brook.tab.c lex.yy.c brcc.c parser.c heapstring.c codegen.c \
		 fp30_assist.cg.bin
	${CC} ${CFLAGS} ${C_DEBUG_FLAG} ${C_OUTPUT_FLAG} \
		../bin/brcc${BINSUFFIX} brook.tab.c \
		lex.yy.c parser.c brcc.c codegen.c heapstring.c

brook.tab.c: brook.y
	bison -d brook.y 
##	bison -d -v -t brook.y 

lex.yy.c: brook.l
	flex brook.l

clean:
	rm -rf lex.yy.c brook.tab.c brook.tab.h fp30_assist.cg.bin \
		*~ *${OBJSUFFIX} brcc${BINSUFFIX} _kernel.cg *.ilk *.opt *.pdb

