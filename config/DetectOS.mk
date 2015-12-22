#set the following to use Open GL ES
MOBILEPROFILE = GLES

ARCH=$(shell uname | sed -e 's/-.*//g')

ifeq ($(ARCH), CYGWIN)
OS=Windows_NT
else
ifeq ($(ARCH), CYGWIN_NT)
OS=Windows_NT
else
ifeq ($(ARCH), Linux)
OS=Linux
else
OS=Default
endif
endif
endif
ifdef MOBILEPROFILE
OS:=$(join $(OS),$(MOBILEPROFILE))
endif

$(info $(OS))
