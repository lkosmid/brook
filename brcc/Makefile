
ROOTDIR = ..
EXECUTABLE = brcc
FILES = context decl express parseenv project stemnt symbol token \
	location brtdecl lexer gram main getopt brtexpress codegen \
	subprocess brtstemnt brook2cpp brtkernel brookreduce brtscatter \
	brookindex

CPP_PATH = /usr/bin/cpp
SLOP = lexer.cpp gram.cpp gram.output gram.h fp30_assist.cg.bin

ADDITIONAL_DEPENDANCIES = gram.h fp30_assist.cg.bin

include ../common.mk

lexer.cpp: lexer.l gram.h
	flex $<
	rm -rf lexer.cpp
	mv lex.yy.c lexer.cpp

gram.cpp gram.h:  gram.y
	bison -d -b gram -o gram.cpp $<
	touch gram.cpp.h
	-mv -f gram.cpp.h gram.h
	-mv -f gram.hpp gram.h
#the gram.cpp.h line is for macs

fp30_assist.cg.bin: fp30_assist.cg
	cpp -P < fp30_assist.cg | perl makestring.pl > fp30_assist.cg.bin
