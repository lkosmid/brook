
#################
## Brook2Cg 
##  Makefile
##  Written by Ian Buck, 2002
#################

## XXX: This really is only a placeholder Makefile until a 
## real cross platform one can be built.

all: makedirs ../bin/brcc.exe

makedirs:
	@if test ! -d ../bin; then mkdir ../bin; fi

fp30_assist.cg.bin: fp30_assist.cg
	cpp -P < fp30_assist.cg | perl makestring.pl > fp30_assist.cg.bin

../bin/brcc.exe: brook.tab.c lex.yy.c brcc.c parser.c codegen.c \
		 fp30_assist.cg.bin
	cl -nologo -W3 -Zi -MLd -o ../bin/brcc.exe brook.tab.c \
		lex.yy.c parser.c brcc.c codegen.c

brook.tab.c: brook.y
	bison -d brook.y 
##	bison -d -v -t brook.y 

lex.yy.c: brook.l
	flex brook.l

clean:
	rm -rf lex.yy.c brook.tab.c brook.tab.h \
		*~ *.obj *.exe _kernel.cg *.ilk *.opt *.pdb

