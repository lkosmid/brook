
ROOTDIR = ..
EXECUTABLE = brcc
FILES = context decl express parseenv project stemnt symbol token \
	location brtdecl lexer gram main getopt brtexpress codegen \
	subprocess brtstemnt

ifdef BROOKCPU
FILES += brook2cpp
CFLAGS += -DBROOK_CPU
endif

CPP_PATH = /usr/bin/cpp
SLOP = lexer.cpp gram.cpp gram.output gram.h

ADDITIONAL_DEPENDANCIES = gram.h

include ../common.mk

lexer.cpp: lexer.l gram.h
	flex $<
	rm -rf lexer.cpp
	mv lex.yy.c lexer.cpp

gram.cpp gram.h:  gram.y
	bison -d -b gram -o gram.cpp $<
	mv gram.hpp gram.h

