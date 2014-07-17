#
# @(#)Makefile	1.16 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Build Poller class demo.
#

BUILDDIR = ../../..

include $(BUILDDIR)/common/Defs.gmk

DEMO_SRC_DIR = $(PLATFORM_SRC)/demo/jni/Poller
DEMO_DST_DIR = $(DEMODIR)/jni/Poller
DEMO_DST_LIB_DIR = $(DEMO_DST_DIR)/$(LIBARCH)
DEMO_DST_OBJ_DIR = $(OBJDIR)/Poller/$(LIBARCH)
DEMO_LIB = $(DEMO_DST_LIB_DIR)/libPoller.so
DEMO_HEADERS = $(DEMO_DST_DIR)/Poller.h
DEMO_OBJ = $(DEMO_DST_OBJ_DIR)/Poller.o

DEMO_CLASSES =					\
	$(DEMO_DST_DIR)/Client.class		\
	$(DEMO_DST_DIR)/LinkedQueue.class	\
	$(DEMO_DST_DIR)/Poller.class		\
	$(DEMO_DST_DIR)/PollingServer.class	\
	$(DEMO_DST_DIR)/SimpleServer.class	


# Java source files to be copied to demo.

DEMO_FILES_java = 				\
	$(DEMO_DST_DIR)/PollingServer.java	\
	$(DEMO_DST_DIR)/Poller.java		\
	$(DEMO_DST_DIR)/Client.java		\
	$(DEMO_DST_DIR)/LinkedQueue.java	\
	$(DEMO_DST_DIR)/SimpleServer.java

# C source files to be copied to demo.

DEMO_FILES_c = \
	$(DEMO_DST_DIR)/Poller.c

# Other files to be copied to demo

DEMO_FILES_OTHER = \
	$(DEMO_DST_DIR)/README.txt		\

# Copy all files to demo directory and build the remaining objects in place.

all:  $(DEMO_DST_DIR) $(DEMO_DST_LIB_DIR) $(DEMO_DST_OBJ_DIR) $(DEMO_FILES_c) $(DEMO_FILES_java) $(DEMO_FILES_OTHER) $(DEMO_CLASSES) $(DEMO_LIB) 

#	$(JAVAC) -d $(DEMO_DST_DIR) $(DEMO_FILES_java)

$(DEMO_LIB) : $(DEMO_OBJ)
	$(CC) $(CFLAGS) -G $(CC_OBJECT_OUTPUT_FLAG)$(DEMO_DST_LIB_DIR)/libpoller.so \
	$(DEMO_DST_OBJ_DIR)/Poller.o

$(DEMO_OBJ) : $(DEMO_FILES_c) $(DEMO_HEADERS)
	$(CC) $(CFLAGS) -c $(CC_OBJECT_OUTPUT_FLAG)$(DEMO_DST_OBJ_DIR)/Poller.o \
	-I$(INCLUDEDIR) -I$(PLATFORM_INCLUDE)		\
	$(DEMO_DST_DIR)/Poller.c

$(DEMO_HEADERS) : 
	$(JAVAH_CMD) -jni -d $(DEMO_DST_DIR) -classpath $(DEMO_DST_DIR) Poller

$(DEMO_CLASSES) :
	$(JAVAC) -d $(DEMO_DST_DIR) $(DEMO_FILES_java)

clobber clean:
	$(RM) -r $(DEMO_DST_DIR)

$(DEMO_DST_DIR):
	$(MKDIR) -p $@

$(DEMO_DST_LIB_DIR):
	$(MKDIR) -p $@

$(DEMO_DST_OBJ_DIR):
	$(MKDIR) -p $@

$(DEMO_DST_DIR)/%: $(DEMO_SRC_DIR)/%
	$(RM) $@
	$(CP) $< $@

.PHONY: all clobber clean
