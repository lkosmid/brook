
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


../bin/brcc.exe: brook.tab.c lex.yy.c parser.c
	cl -nologo -Zi -o ../bin/brcc.exe brook.tab.c lex.yy.c parser.c

brook.tab.c: brook.y
	bison -d brook.y 
##	bison -d -v -t brook.y 

lex.yy.c: brook.l
	flex brook.l

clean:
	rm -rf lex.yy.c brook.tab.c brook.tab.h \
		*~ *.obj *.exe _kernel.pl *.ilk *.opt *.pdb

