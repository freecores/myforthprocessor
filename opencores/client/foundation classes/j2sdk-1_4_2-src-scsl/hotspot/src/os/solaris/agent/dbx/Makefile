# 
# @(#)Makefile	1.13 03/01/23 11:04:20
# 
# Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 


# Targets are:
#   32bit:  Build the 32 bit version in ./32bit
#   64bit:  Build the 64 bit version in ./64bit
#   helloWorld:  Build the helloWorld test program
#   all:    Build all of the above.  This is the default.
#
# NOTE: This makefile uses IOBuf.cpp, IOBuf.hpp, Buffer.cpp, and
#           Buffer.hpp from the src/os/win32/agent directory.

.PHONY: 32bit 64bit

ARCH_ORIG = $(shell uname -p)

C++    := /java/devtools/$(ARCH_ORIG)/SUNWspro/SC6.1/bin/CC
RM     := /usr/bin/rm
MKDIRS := /usr/bin/mkdir -p


ARCH     := $(subst i386,i486,$(ARCH_ORIG))
# INCLUDES := -I/net/sparcworks.eng/export/set/sparcworks5/dbx_62_intg/dev/src/dbx -I../../win32/agent
INCLUDES := -I. -I../../../win32/agent
CFLAGS_32bit := -xarch=v8
CFLAGS_64bit := -xarch=v9
CFLAGS   := -PIC -xO3 $(INCLUDES)
LIBS     := -lsocket -lnsl -lrtld_db
LDFLAGS  := -G

ifneq "$(ARCH)" "i486"
    CFLAGS += $(CFLAGS_$(VERSION))
    LDFLAGS += $(CFLAGS_$(VERSION))
endif

# We use IOBuf.hpp, IOBuf.cpp, Buffer.hpp, and Buffer.cpp from the win32 dir.
vpath %.cpp .:../../../win32/agent
vpath %.hpp .:../../../win32/agent

OBJS = $(VERSION)/svc_agent_dbx.o $(VERSION)/IOBuf.o $(VERSION)/Buffer.o



# The default is to make both 32 bit and 64 bit versions.
all:: 32bit 64bit

32bit 64bit:: 
	$(MKDIRS) $@
	$(MAKE) $@/libsvc_agent_dbx.so  helloWorld VERSION=$@

$(VERSION)/IOBuf.o: IOBuf.hpp
$(VERSION)/Buffer.o: Buffer.hpp
$(VERSION)/svc_agent_dbx.o: svc_agent_dbx.hpp

$(VERSION)/%.o: %.cpp
	$(C++) $(CFLAGS) -c $< -o $@

$(VERSION)/libsvc_agent_dbx.so:: $(OBJS)
	$(C++) $(LDFLAGS) -o $(VERSION)/libsvc_agent_dbx.so $(OBJS) $(LIBS)

# Would be nice to move this into a shared directory
helloWorld:: helloWorld.cpp
	$(C++) -g $< -o $@

clean::
	$(RM) -rf 32bit 64bit *.o helloWorld
