
ROOTDIR = ..
EXECUTABLE = brcc
FILES = context decl express parseenv project stemnt symbol token \
	location brtdecl lexer gram main getopt brtexpress codegen \
	subprocess brtstemnt b2ctransform brtkernel b2creduce brtscatter \
	b2cindex brtreduce brtgather brtvout

CPP_PATH = /usr/bin/cpp
SLOP = lexer.cpp gram.cpp gram.output gram.h

ADDITIONAL_DEPENDANCIES = gram.h

include ../common.mk

lexer.cpp: lexer.l gram.h
	flex $<
	rm -rf lexer.cpp
	mv lex.yy.c lexer.cpp

gram.cpp gram.h:  gram.y
	bison -d -b gram $<
	mv -f gram.tab.c gram.cpp
	mv -f gram.tab.h gram.h
# DO NOT DELETE
