#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.43 03/01/23 
#
# Makefile for java.nio
#

BUILDDIR = ../..
PACKAGE = java.nio
LIBRARY = nio
PRODUCT = java
include $(BUILDDIR)/common/Defs.gmk

NIO_SRC = $(SHARE_SRC)/classes/java/nio
NIO_GEN = $(GENSRCDIR)/java/nio

SNIO_SRC = $(SHARE_SRC)/classes/sun/nio
SNIO_GEN = $(GENSRCDIR)/sun/nio

#
# Files to compile
#

include FILES_java.gmk
include FILES_c.gmk
include Exportedfiles.gmk

ifeq ($(PLATFORM), solaris)
FILES_java += \
        sun/nio/ch/AbstractPollSelectorImpl.java \
	sun/nio/ch/DevPollArrayWrapper.java \
	sun/nio/ch/DevPollSelectorImpl.java \
        sun/nio/ch/DevPollSelectorProvider.java \
        sun/nio/ch/PollSelectorProvider.java \
        sun/nio/ch/PollSelectorImpl.java

FILES_c += \
        DevPollArrayWrapper.c \
        PollArrayWrapper.c \
	NativeThread.c

FILES_export += \
	sun/nio/ch/DevPollArrayWrapper.java \
	sun/nio/ch/NativeThread.java
endif # PLATFORM = solaris

ifeq ($(PLATFORM), windows)
FILES_java += \
        sun/nio/ch/WindowsSelectorImpl.java \
        sun/nio/ch/WindowsSelectorProvider.java

FILES_c += \
        WindowsSelectorImpl.c

FILES_export += \
	sun/nio/ch/WindowsSelectorImpl.java
endif # PLATFORM = windows

ifeq ($(PLATFORM), linux)
FILES_java += \
        sun/nio/ch/AbstractPollSelectorImpl.java \
        sun/nio/ch/PollSelectorProvider.java \
        sun/nio/ch/PollSelectorImpl.java

FILES_c += \
        PollArrayWrapper.c \
	NativeThread.c

FILES_export += \
	sun/nio/ch/NativeThread.java
endif # PLATFORM = linux

# Find platform-specific C source files
#
vpath %.c $(PLATFORM_SRC)/native/sun/nio/ch
vpath %.c $(SHARE_SRC)/native/sun/nio/ch

#
# Various variables
#

JAVALIB = 		# Don't self-link
JAVAHFLAGS += -jni	# Generate JNI-style header files

# We use the bootstrap compiler because clobbering NIO
# clobbers the charset classes used by the built compiler
#
JAVAC_CMD = $(JAVAC_BOOT_CMD) -source 1.4
JAVAH_CMD = $(JAVAH_BOOT_CMD)

#
# Access to io_util.c and net_util.c
#

OTHER_INCLUDES += \
  -I$(SHARE_SRC)/native/sun/nio/ch \
  -I$(SHARE_SRC)/native/java/io \
  -I$(SHARE_SRC)/native/java/net \
  -I$(PLATFORM_SRC)/native/java/net \
  -I$(CLASSHDRDIR)/../../../java.lang/java/CClassHeaders

ifeq ($(PLATFORM),windows)
  OTHER_LDLIBS += $(JVMLIB) ws2_32.lib \
     -libpath:$(LIBDIR) java$(SUFFIX).lib \
     $(OBJDIR)/../../../../sun/java.net/net/$(OBJDIRNAME)/net$(SUFFIX).lib \
     $(OBJDIR)/../../../java.lang/java/$(OBJDIRNAME)/io_util.obj \
     $(OBJDIR)/../../../java.lang/java/$(OBJDIRNAME)/FileDescriptor.obj
endif
ifeq ($(PLATFORM), linux)
OTHER_CFLAGS += -Werror
OTHER_LDLIBS += -L$(LIBDIR)/$(LIBARCH)$(SUFFIX) -ljava$(SUFFIX) -lnet$(SUFFIX) -lpthread
endif
ifeq ($(PLATFORM), solaris)
OTHER_LDLIBS += $(JVMLIB) -lsocket -lposix4 -ldl \
		-L$(LIBDIR)/$(LIBARCH)$(SUFFIX) -ljava$(SUFFIX) -lnet$(SUFFIX)
endif # PLATFORM

#
# Rules
#

all: sources classes

clean::
	$(RM) -r $(NIO_GEN)
	$(RM) -r $(CLASSDESTDIR)/java/nio
	$(RM) -r $(CLASSDESTDIR)/sun/nio

include $(BUILDDIR)/common/Mapfile-vers.gmk
include $(BUILDDIR)/common/Library.gmk

#
# Generate source files
#

SPP = spp.sh
SPP_CMD = $(SH) $(SPP)

FILES_genout = $(FILES_gen:%.java=$(GENSRCDIR)/%.java)

BUF_SRC=$(NIO_SRC)
CH_SRC=$(NIO_SRC)/channels
CS_SRC=$(NIO_SRC)/charset
SCH_SRC=$(SNIO_SRC)/ch

BUF_GEN=$(NIO_GEN)
CH_GEN=$(NIO_GEN)/channels
CS_GEN=$(NIO_GEN)/charset
SCH_GEN=$(SNIO_GEN)/ch

$(BUF_GEN) $(CH_GEN) $(CS_GEN) $(SCH_GEN): ; $(MKDIR) -p $@

sources: $(SPP) $(BUF_GEN) $(CH_GEN) $(CS_GEN) $(SCH_GEN) $(FILES_genout)

.PHONY: sources 


#
# Generated buffer classes
#

GEN_BUFFER_SH = genBuffer.sh

GEN_BUFFER_CMD = SPP="$(SPP_CMD)" NAWK=$(NAWK) SED=$(SED) \
                 $(SH) $(GEN_BUFFER_SH)

# Public abstract buffer classes
#
$(BUF_GEN)/ByteBuffer.java: $(BUF_SRC)/X-Buffer.java \
			    $(BUF_SRC)/X-Buffer-bin.java \
			    $(GEN_BUFFER_SH)
	TYPE=byte BIN=1 SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/CharBuffer.java: $(BUF_SRC)/X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=char SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/ShortBuffer.java: $(BUF_SRC)/X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=short SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/IntBuffer.java: $(BUF_SRC)/X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=int SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/LongBuffer.java: $(BUF_SRC)/X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=long SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/FloatBuffer.java: $(BUF_SRC)/X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=float SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/DoubleBuffer.java: $(BUF_SRC)/X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=double SRC=$< DST=$@ $(GEN_BUFFER_CMD)

# Buffers whose contents are heap-allocated
# 
$(BUF_GEN)/HeapByteBuffer.java: $(BUF_SRC)/Heap-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=byte SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/HeapByteBuffer%.java: $(BUF_SRC)/Heap-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=byte RW=$* SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/HeapCharBuffer.java: $(BUF_SRC)/Heap-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=char SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/HeapCharBuffer%.java: $(BUF_SRC)/Heap-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=char RW=$* SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/HeapShortBuffer.java: $(BUF_SRC)/Heap-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=short SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/HeapShortBuffer%.java: $(BUF_SRC)/Heap-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=short RW=$* SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/HeapIntBuffer.java: $(BUF_SRC)/Heap-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=int SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/HeapIntBuffer%.java: $(BUF_SRC)/Heap-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=int RW=$* SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/HeapLongBuffer.java: $(BUF_SRC)/Heap-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=long SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/HeapLongBuffer%.java: $(BUF_SRC)/Heap-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=long RW=$* SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/HeapFloatBuffer.java: $(BUF_SRC)/Heap-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=float SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/HeapFloatBuffer%.java: $(BUF_SRC)/Heap-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=float RW=$* SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/HeapDoubleBuffer.java: $(BUF_SRC)/Heap-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=double SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/HeapDoubleBuffer%.java: $(BUF_SRC)/Heap-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=double RW=$* SRC=$< DST=$@ $(GEN_BUFFER_CMD)

# Direct byte buffer
# 
$(BUF_GEN)/DirectByteBuffer.java: $(BUF_SRC)/Direct-X-Buffer.java \
				  $(BUF_SRC)/Direct-X-Buffer.java \
				  $(GEN_BUFFER_SH)
	TYPE=byte BIN=1 SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/DirectByteBuffer%.java: $(BUF_SRC)/Direct-X-Buffer.java \
				   $(BUF_SRC)/Direct-X-Buffer.java \
				   $(GEN_BUFFER_SH)
	TYPE=byte RW=$* BIN=1 SRC=$< DST=$@ $(GEN_BUFFER_CMD)

# Unswapped views of direct byte buffers
#
$(BUF_GEN)/DirectCharBufferU.java: $(BUF_SRC)/Direct-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=char BO=U SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/DirectCharBuffer%U.java: $(BUF_SRC)/Direct-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=char RW=$* BO=U SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/DirectShortBufferU.java: $(BUF_SRC)/Direct-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=short BO=U SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/DirectShortBuffer%U.java: $(BUF_SRC)/Direct-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=short RW=$* BO=U SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/DirectIntBufferU.java: $(BUF_SRC)/Direct-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=int BO=U SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/DirectIntBuffer%U.java: $(BUF_SRC)/Direct-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=int RW=$* BO=U SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/DirectLongBufferU.java: $(BUF_SRC)/Direct-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=long BO=U SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/DirectLongBuffer%U.java: $(BUF_SRC)/Direct-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=long RW=$* BO=U SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/DirectFloatBufferU.java: $(BUF_SRC)/Direct-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=float BO=U SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/DirectFloatBuffer%U.java: $(BUF_SRC)/Direct-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=float RW=$* BO=U SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/DirectDoubleBufferU.java: $(BUF_SRC)/Direct-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=double BO=U SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/DirectDoubleBuffer%U.java: $(BUF_SRC)/Direct-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=double RW=$* BO=U SRC=$< DST=$@ $(GEN_BUFFER_CMD)

# Swapped views of direct byte buffers
#
$(BUF_GEN)/DirectCharBufferS.java: $(BUF_SRC)/Direct-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=char BO=S SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/DirectCharBuffer%S.java: $(BUF_SRC)/Direct-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=char RW=$* BO=S SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/DirectShortBufferS.java: $(BUF_SRC)/Direct-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=short BO=S SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/DirectShortBuffer%S.java: $(BUF_SRC)/Direct-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=short RW=$* BO=S SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/DirectIntBufferS.java: $(BUF_SRC)/Direct-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=int BO=S SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/DirectIntBuffer%S.java: $(BUF_SRC)/Direct-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=int RW=$* BO=S SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/DirectLongBufferS.java: $(BUF_SRC)/Direct-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=long BO=S SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/DirectLongBuffer%S.java: $(BUF_SRC)/Direct-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=long RW=$* BO=S SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/DirectFloatBufferS.java: $(BUF_SRC)/Direct-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=float BO=S SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/DirectFloatBuffer%S.java: $(BUF_SRC)/Direct-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=float RW=$* BO=S SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/DirectDoubleBufferS.java: $(BUF_SRC)/Direct-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=double BO=S SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/DirectDoubleBuffer%S.java: $(BUF_SRC)/Direct-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=double RW=$* BO=S SRC=$< DST=$@ $(GEN_BUFFER_CMD)

# Big-endian views of byte buffers
#
$(BUF_GEN)/ByteBufferAsCharBufferB.java: $(BUF_SRC)/ByteBufferAs-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=char BO=B SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/ByteBufferAsCharBuffer%B.java: $(BUF_SRC)/ByteBufferAs-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=char RW=$* BO=B SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/ByteBufferAsShortBufferB.java: $(BUF_SRC)/ByteBufferAs-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=short BO=B SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/ByteBufferAsShortBuffer%B.java: $(BUF_SRC)/ByteBufferAs-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=short RW=$* BO=B SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/ByteBufferAsIntBufferB.java: $(BUF_SRC)/ByteBufferAs-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=int BO=B SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/ByteBufferAsIntBuffer%B.java: $(BUF_SRC)/ByteBufferAs-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=int RW=$* BO=B SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/ByteBufferAsLongBufferB.java: $(BUF_SRC)/ByteBufferAs-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=long BO=B SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/ByteBufferAsLongBuffer%B.java: $(BUF_SRC)/ByteBufferAs-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=long RW=$* BO=B SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/ByteBufferAsFloatBufferB.java: $(BUF_SRC)/ByteBufferAs-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=float BO=B SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/ByteBufferAsFloatBuffer%B.java: $(BUF_SRC)/ByteBufferAs-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=float RW=$* BO=B SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/ByteBufferAsDoubleBufferB.java: $(BUF_SRC)/ByteBufferAs-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=double BO=B SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/ByteBufferAsDoubleBuffer%B.java: $(BUF_SRC)/ByteBufferAs-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=double RW=$* BO=B SRC=$< DST=$@ $(GEN_BUFFER_CMD)

# Little-endian views of byte buffers
#
$(BUF_GEN)/ByteBufferAsCharBufferL.java: $(BUF_SRC)/ByteBufferAs-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=char BO=L SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/ByteBufferAsCharBuffer%L.java: $(BUF_SRC)/ByteBufferAs-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=char RW=$* BO=L SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/ByteBufferAsShortBufferL.java: $(BUF_SRC)/ByteBufferAs-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=short BO=L SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/ByteBufferAsShortBuffer%L.java: $(BUF_SRC)/ByteBufferAs-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=short RW=$* BO=L SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/ByteBufferAsIntBufferL.java: $(BUF_SRC)/ByteBufferAs-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=int BO=L SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/ByteBufferAsIntBuffer%L.java: $(BUF_SRC)/ByteBufferAs-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=int RW=$* BO=L SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/ByteBufferAsLongBufferL.java: $(BUF_SRC)/ByteBufferAs-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=long BO=L SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/ByteBufferAsLongBuffer%L.java: $(BUF_SRC)/ByteBufferAs-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=long RW=$* BO=L SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/ByteBufferAsFloatBufferL.java: $(BUF_SRC)/ByteBufferAs-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=float BO=L SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/ByteBufferAsFloatBuffer%L.java: $(BUF_SRC)/ByteBufferAs-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=float RW=$* BO=L SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/ByteBufferAsDoubleBufferL.java: $(BUF_SRC)/ByteBufferAs-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=double BO=L SRC=$< DST=$@ $(GEN_BUFFER_CMD)
$(BUF_GEN)/ByteBufferAsDoubleBuffer%L.java: $(BUF_SRC)/ByteBufferAs-X-Buffer.java $(GEN_BUFFER_SH)
	TYPE=double RW=$* BO=L SRC=$< DST=$@ $(GEN_BUFFER_CMD)

#
# Generated coder classes
#

GEN_CODER_SH = genCoder.sh

GEN_CODER_CMD = SPP="$(SPP_CMD)" SED=$(SED) NAWK=$(NAWK) $(SH) $(GEN_CODER_SH)

$(CS_GEN)/CharsetDecoder.java: $(CS_SRC)/Charset-X-Coder.java $(GEN_CODER_SH)
	$(GEN_CODER_CMD) decoder $< $@

$(CS_GEN)/CharsetEncoder.java: $(CS_SRC)/Charset-X-Coder.java $(GEN_CODER_SH)
	$(GEN_CODER_CMD) encoder $< $@

#
# Generated exception classes
#

$(CH_GEN)/%Exception.java: genExceptions.sh $(CH_SRC)/exceptions
	$(SHELL) genExceptions.sh $(CH_SRC)/exceptions $(CH_GEN)

$(CS_GEN)/%Exception.java: genExceptions.sh $(CS_SRC)/exceptions
	$(SHELL) genExceptions.sh $(CS_SRC)/exceptions $(CS_GEN)

$(BUF_GEN)/%Exception.java: genExceptions.sh $(BUF_SRC)/exceptions
	$(SHELL) genExceptions.sh $(BUF_SRC)/exceptions $(BUF_GEN)

$(SCH_GEN)/%Exception.java: genExceptions.sh $(SCH_SRC)/exceptions
	$(SHELL) genExceptions.sh $(SCH_SRC)/exceptions $(SCH_GEN)
