#
# @(#)Makefile	1.81 03/05/16
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Copy the redistributable files into their proper locations.
#

BUILDDIR = ../..
include $(BUILDDIR)/common/Defs.gmk
PRODUCT  = java

SERVER_LOCATION = server
CLIENT_LOCATION = client

ifeq ($(PLATFORM), windows)
LIB_LOCATION = $(BINDIR)
else
LIB_LOCATION = $(LIBDIR)/$(LIBARCH)
endif


OPTIMIZED_JVM_NAME     = $(LIB_PREFIX)jvm$(OPT_SUFFIX).$(LIBRARY_SUFFIX)
DEBUG_JVM_NAME         = $(LIB_PREFIX)jvm$(DBG_SUFFIX).$(LIBRARY_SUFFIX)
JVM_NAME               = $(LIB_PREFIX)jvm$(SUFFIX).$(LIBRARY_SUFFIX)

OPTIMIZED_JVMLIB_NAME  = $(LIB_PREFIX)jvm$(OPT_SUFFIX).$(LIB_SUFFIX)
DEBUG_JVMLIB_NAME      = $(LIB_PREFIX)jvm$(DBG_SUFFIX).$(LIB_SUFFIX)
JVMLIB_NAME            = $(LIB_PREFIX)jvm$(SUFFIX).$(LIB_SUFFIX)

OPTIMIZED_JVMMAP_NAME  = $(LIB_PREFIX)jvm$(OPT_SUFFIX).map
DEBUG_JVMMAP_NAME      = $(LIB_PREFIX)jvm$(DBG_SUFFIX).map
JVMMAP_NAME            = $(LIB_PREFIX)jvm$(SUFFIX).map

OPTIMIZED_JVMPDB_NAME  = $(LIB_PREFIX)jvm$(OPT_SUFFIX).pdb
DEBUG_JVMPDB_NAME      = $(LIB_PREFIX)jvm$(DBG_SUFFIX).pdb
JVMPDB_NAME            = $(LIB_PREFIX)jvm$(SUFFIX).pdb

OPTIMIZED_LIBJSIG_NAME = $(LIB_PREFIX)jsig$(OPT_SUFFIX).$(LIBRARY_SUFFIX)
DEBUG_LIBJSIG_NAME     = $(LIB_PREFIX)jsig$(DBG_SUFFIX).$(LIBRARY_SUFFIX)
LIBJSIG_NAME           = $(LIB_PREFIX)jsig$(SUFFIX).$(LIBRARY_SUFFIX)

include $(BUILDDIR)/common/Defs.gmk

INSTALLED_FILES_FILE = $(TMPDIR)/.installed.files

all: optimized debug

OPTIMIZED = $(LIBDIR)/sunrsasign.jar \
            $(LIB_LOCATION)/$(SERVER_LOCATION)/$(OPTIMIZED_JVM_NAME) \
            $(LIB_LOCATION)/$(SERVER_LOCATION)/Xusage.txt \
            $(LIBDIR)/jce.jar \
            $(EXTDIR)/sunjce_provider.jar \
            $(LIBDIR)/security/US_export_policy.jar \
            $(LIBDIR)/security/local_policy.jar \
            $(LIBDIR)/jsse.jar \
            $(CLASSBINDIR)/sun/security/jgss/spi \
            $(CLASSBINDIR)/sun/security/krb5 \

DEBUG     = $(LIBDIR)/sunrsasign.jar \
            $(LIB_LOCATION)/$(SERVER_LOCATION)/$(DEBUG_JVM_NAME) \
            $(LIB_LOCATION)/$(SERVER_LOCATION)/Xusage.txt \
            $(LIBDIR)/jce.jar \
            $(EXTDIR)/sunjce_provider.jar \
            $(LIBDIR)/security/US_export_policy.jar \
            $(LIBDIR)/security/local_policy.jar \
            $(LIBDIR)/jsse.jar \
            $(CLASSBINDIR)/sun/security/jgss/spi \
            $(CLASSBINDIR)/sun/security/krb5 \


# Hotspot client is only available on 32-bit builds
ifeq ($(ARCH_DATA_MODEL), 32)
  OPTIMIZED += $(LIB_LOCATION)/$(CLIENT_LOCATION)/$(OPTIMIZED_JVM_NAME) \
               $(LIB_LOCATION)/$(CLIENT_LOCATION)/Xusage.txt

  DEBUG +=     $(LIB_LOCATION)/$(CLIENT_LOCATION)/$(DEBUG_JVM_NAME) \
               $(LIB_LOCATION)/$(CLIENT_LOCATION)/Xusage.txt
endif

ifeq ($(PLATFORM), linux)
# linux
OPTIMIZED_LIBCHMOD_NAME = $(LIB_PREFIX)native_chmod$(OPT_SUFFIX).$(LIBRARY_SUFFIX)
DEBUG_LIBCHMOD_NAME  = $(LIB_PREFIX)native_chmod$(DBG_SUFFIX).$(LIBRARY_SUFFIX)

OPTIMIZED += $(LIBDIR)/$(LIBARCH)/$(OPTIMIZED_LIBCHMOD_NAME)
DEBUG += $(LIBDIR)/$(LIBARCH)/$(DEBUG_LIBCHMOD_NAME)

NATIVE_CHMOD_ZIP = $(BUILDDIR)/tools/crypto/jgss/$(ARCH)/native_chmod.tar.gz
$(LIBDIR)/$(LIBARCH)/$(OPTIMIZED_LIBCHMOD_NAME) \
$(LIBDIR)/$(LIBARCH)/$(DEBUG_LIBCHMOD_NAME): \
	$(NATIVE_CHMOD_ZIP) $(LIBDIR) $(TMPDIR)
	$(RM) $@
	($(GUNZIP) < $<) | ($(CD) $(LIBDIR)/$(LIBARCH); $(TAR) xf -)
	$(ECHO) $@ >> $(INSTALLED_FILES_FILE)
# linux
endif

ifeq ($(PLATFORM), windows)
#  Windows     vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv  Windows

OPTIMIZED += $(BINDIR)/w2k_lsa_auth.dll
DEBUG += $(BINDIR)/w2k_lsa_auth$(DBG_SUFFIX).dll

W2K_LSA_AUTH_ZIP = $(BUILDDIR)/tools/crypto/jgss/$(ARCH)/w2k_lsa_auth.zip
$(BINDIR)/w2k_lsa_auth$(SUFFIX).dll: $(W2K_LSA_AUTH_ZIP) $(BINDIR) $(TMPDIR)
	$(RM) $@
	$(UNZIP) $(W2K_LSA_AUTH_ZIP) w2k_lsa_auth$(SUFFIX).dll \
		-d $(BINDIR)
	$(ECHO) $@ >> $(INSTALLED_FILES_FILE)

  OPTIMIZED += $(BINDIR)/msvcrt.dll
  ifeq ($(ARCH_DATA_MODEL), 64)
    OPTIMIZED += $(BINDIR)/msvcrtd.dll
  endif

$(BINDIR)/msvcrt.dll: $(MSVCRT_DLL_PATH)/msvcrt.dll $(BINDIR) 
	$(RM) $@
	$(CP) $< $@
	@$(ECHO) $@ >> $(INSTALLED_FILES_FILE)

$(BINDIR)/msvcrtd.dll: $(MSVCRTD_DLL_PATH)/msvcrtd.dll $(BINDIR) 
	$(RM) $@
	$(CP) $< $@
	@$(ECHO) $@ >> $(INSTALLED_FILES_FILE)

# Get the hotspot .map and .pdb files for client and server
OPTIMIZED += \
	$(LIBDIR)/$(OPTIMIZED_JVMLIB_NAME) \
	$(LIB_LOCATION)/$(SERVER_LOCATION)/$(OPTIMIZED_JVMMAP_NAME) \
	$(LIB_LOCATION)/$(SERVER_LOCATION)/$(OPTIMIZED_JVMPDB_NAME)

DEBUG     += \
	$(LIBDIR)/$(DEBUG_JVMLIB_NAME) \
	$(LIB_LOCATION)/$(SERVER_LOCATION)/$(DEBUG_JVMMAP_NAME) \
	$(LIB_LOCATION)/$(SERVER_LOCATION)/$(DEBUG_JVMPDB_NAME)


# Hotspot client is only available on 32-bit builds
ifeq ($(ARCH_DATA_MODEL), 32)
  OPTIMIZED += $(LIB_LOCATION)/$(CLIENT_LOCATION)/$(OPTIMIZED_JVMMAP_NAME) \
               $(LIB_LOCATION)/$(CLIENT_LOCATION)/$(OPTIMIZED_JVMPDB_NAME)
  DEBUG     += $(LIB_LOCATION)/$(CLIENT_LOCATION)/$(DEBUG_JVMMAP_NAME) \
               $(LIB_LOCATION)/$(CLIENT_LOCATION)/$(DEBUG_JVMPDB_NAME)
endif

$(LIBDIR)/$(OPTIMIZED_JVMLIB_NAME) \
$(LIBDIR)/$(DEBUG_JVMLIB_NAME): \
	$(HOTSPOT_LIB_PATH)/$(JVMLIB_NAME) $(LIBDIR) $(TMPDIR)
	$(RM) $@
	$(CP) $< $@
	@$(ECHO) $@ >> $(INSTALLED_FILES_FILE)

# it is OK for the .map and .pdb files to not exist, so do not force a 
# dependency on them from the bootstrap location, and allow the copy to fail.
$(LIB_LOCATION)/$(CLIENT_LOCATION)/$(OPTIMIZED_JVMMAP_NAME) \
$(LIB_LOCATION)/$(CLIENT_LOCATION)/$(DEBUG_JVMMAP_NAME): \
	$(LIB_LOCATION)/$(CLIENT_LOCATION) $(TMPDIR)
	$(RM) $@
	-$(CP) $(HOTSPOT_CLIENT_PATH)/$(JVMMAP_NAME)  $@
	@$(ECHO) $@ >> $(INSTALLED_FILES_FILE)

$(LIB_LOCATION)/$(SERVER_LOCATION)/$(OPTIMIZED_JVMMAP_NAME) \
$(LIB_LOCATION)/$(SERVER_LOCATION)/$(DEBUG_JVMMAP_NAME): \
	$(LIB_LOCATION)/$(SERVER_LOCATION) $(TMPDIR)
	$(RM) $@
	-$(CP) $(HOTSPOT_SERVER_PATH)/$(JVMMAP_NAME) $@
	@$(ECHO) $@ >> $(INSTALLED_FILES_FILE)

$(LIB_LOCATION)/$(CLIENT_LOCATION)/$(OPTIMIZED_JVMPDB_NAME) \
$(LIB_LOCATION)/$(CLIENT_LOCATION)/$(DEBUG_JVMPDB_NAME): \
	$(LIB_LOCATION)/$(CLIENT_LOCATION) $(TMPDIR)
	$(RM) $@
	-$(CP) $(HOTSPOT_CLIENT_PATH)/$(JVMPDB_NAME)  $@
	@$(ECHO) $@ >> $(INSTALLED_FILES_FILE)

$(LIB_LOCATION)/$(SERVER_LOCATION)/$(OPTIMIZED_JVMPDB_NAME) \
$(LIB_LOCATION)/$(SERVER_LOCATION)/$(DEBUG_JVMPDB_NAME): \
	$(LIB_LOCATION)/$(SERVER_LOCATION) $(TMPDIR)
	$(RM) $@
	-$(CP) $(HOTSPOT_SERVER_PATH)/$(JVMPDB_NAME) $@
	@$(ECHO) $@ >> $(INSTALLED_FILES_FILE)


#  Windows     ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^  Windows
else # PLATFORM
#  NOT Windows vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv  NOT Windows

OPTIMIZED += $(LIB_LOCATION)/$(OPTIMIZED_LIBJSIG_NAME) $(LIB_LOCATION)/$(SERVER_LOCATION)/$(OPTIMIZED_LIBJSIG_NAME)
DEBUG += $(LIB_LOCATION)/$(DEBUG_LIBJSIG_NAME) $(LIB_LOCATION)/$(SERVER_LOCATION)/$(DEBUG_LIBJSIG_NAME)

ifeq ($(ARCH_DATA_MODEL), 32)

OPTIMIZED += $(LIB_LOCATION)/$(CLIENT_LOCATION)/$(OPTIMIZED_LIBJSIG_NAME)
DEBUG += $(LIB_LOCATION)/$(CLIENT_LOCATION)/$(DEBUG_LIBJSIG_NAME)

ifeq ($(PLATFORM), solaris)
#  solaris   vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv  solaris

# For backwards compatability, make a link of the 32-bit client JVM to $(LIBDIR)
ifeq ($(ARCH_DATA_MODEL), 32)
OPTIMIZED += $(LIB_LOCATION)/$(OPTIMIZED_JVM_NAME)

# create a link from lib/libjvm.so to client/libjvm.so
$(LIB_LOCATION)/$(OPTIMIZED_JVM_NAME): $(LIB_LOCATION)/$(CLIENT_LOCATION)/$(OPTIMIZED_JVM_NAME) $(TMPDIR)
	$(LN) -s $(CLIENT_LOCATION)/$(OPTIMIZED_JVM_NAME) $@
	@$(ECHO) $@ >> $(INSTALLED_FILES_FILE)
endif
#  solaris   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ solaris
endif # PLATFORM

$(LIB_LOCATION)/hotspot: $(TMPDIR)
	$(LN) -s $(CLIENT_LOCATION) $@
	@$(ECHO) $@ >> $(INSTALLED_FILES_FILE)
endif

#  NOT Windows ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ NOT Windows

endif # PLATFORM

$(LIB_LOCATION)/$(SERVER_LOCATION) $(LIB_LOCATION)/$(CLIENT_LOCATION) $(LIBDIR) $(EXTDIR) $(LIBDIR)/security $(LIBDIR)/fonts $(BINDIR) $(CLASSBINDIR):
	$(MKDIR) -p $@

$(LIB_LOCATION)/$(CLIENT_LOCATION)/$(OPTIMIZED_JVM_NAME) \
$(LIB_LOCATION)/$(CLIENT_LOCATION)/$(DEBUG_JVM_NAME): \
	$(HOTSPOT_CLIENT_PATH)/$(JVM_NAME) $(LIB_LOCATION)/$(CLIENT_LOCATION) $(TMPDIR)
	$(RM) $@
	$(CP) $< $@
	@$(ECHO) $@ >> $(INSTALLED_FILES_FILE)

$(LIB_LOCATION)/$(DEBUG_LIBJSIG_NAME) \
$(LIB_LOCATION)/$(OPTIMIZED_LIBJSIG_NAME): \
	$(HOTSPOT_SERVER_PATH)/$(LIBJSIG_NAME) $(LIB_LOCATION) $(TMPDIR)
	$(RM) $@
	$(CP) $< $@
	@$(ECHO) $@ >> $(INSTALLED_FILES_FILE)

$(LIB_LOCATION)/$(CLIENT_LOCATION)/$(DEBUG_LIBJSIG_NAME) \
$(LIB_LOCATION)/$(SERVER_LOCATION)/$(DEBUG_LIBJSIG_NAME):
	$(LN) -s ../$(DEBUG_LIBJSIG_NAME) $@
	@$(ECHO) $@ >> $(INSTALLED_FILES_FILE)

$(LIB_LOCATION)/$(CLIENT_LOCATION)/$(OPTIMIZED_LIBJSIG_NAME) \
$(LIB_LOCATION)/$(SERVER_LOCATION)/$(OPTIMIZED_LIBJSIG_NAME):
	$(LN) -s ../$(OPTIMIZED_LIBJSIG_NAME) $@
	@$(ECHO) $@ >> $(INSTALLED_FILES_FILE)

$(LIB_LOCATION)/$(SERVER_LOCATION)/$(OPTIMIZED_JVM_NAME) \
$(LIB_LOCATION)/$(SERVER_LOCATION)/$(DEBUG_JVM_NAME) : \
	$(HOTSPOT_SERVER_PATH)/$(JVM_NAME) $(LIB_LOCATION)/$(SERVER_LOCATION) $(TMPDIR)
	$(RM) $@
	$(CP) $< $@
	@$(ECHO) $@ >> $(INSTALLED_FILES_FILE)

$(LIBDIR)/jce.jar: $(BUILDDIR)/tools/crypto/jce/jce.jar $(LIBDIR) $(TMPDIR)
	$(RM) $@
	$(CP) $< $@
	$(ECHO) $@ >> $(INSTALLED_FILES_FILE)

$(EXTDIR)/sunjce_provider.jar: $(BUILDDIR)/tools/crypto/jce/sunjce_provider.jar $(EXTDIR) $(TMPDIR)
	$(RM) $@
	$(CP) $< $@
	$(ECHO) $@ >> $(INSTALLED_FILES_FILE)

$(LIBDIR)/security/US_export_policy.jar: $(BUILDDIR)/tools/crypto/jce/US_export_policy.jar $(LIBDIR)/security $(TMPDIR)
	$(RM) $@
	$(CP) $< $@
	$(ECHO) $@ >> $(INSTALLED_FILES_FILE)

$(LIBDIR)/security/local_policy.jar: $(BUILDDIR)/tools/crypto/jce/local_policy.jar $(LIBDIR)/security $(TMPDIR)
	$(RM) $@
	$(CP) $< $@
	$(ECHO) $@ >> $(INSTALLED_FILES_FILE)

$(LIBDIR)/jsse.jar: $(BUILDDIR)/tools/crypto/jsse/jsse.jar $(LIBDIR) $(TMPDIR)
	$(RM) $@
	$(CP) $< $@
	$(ECHO) $@ >> $(INSTALLED_FILES_FILE)

$(CLASSBINDIR)/sun/security/jgss/spi \
$(CLASSBINDIR)/sun/security/krb5: $(BUILDDIR)/tools/crypto/jgss/jgss_kerberos.jar $(CLASSBINDIR) $(TMPDIR)
	$(RM) -r $@
	$(CP) $< $(CLASSBINDIR)
	$(CD) $(CLASSBINDIR); $(BOOTDIR)/bin/jar xf $(<F) $(subst $(CLASSBINDIR)/, ,$@)
	$(CD) $(CLASSBINDIR); $(RM) $(<F)
	$(BOOTDIR)/bin/jar tf $< $(subst $(CLASSBINDIR)/, ,$@) \
	         >> $(INSTALLED_FILES_FILE)

$(LIB_LOCATION)/$(SERVER_LOCATION)/Xusage.txt : $(HOTSPOT_SERVER_PATH)/Xusage.txt $(LIB_LOCATION)/$(SERVER_LOCATION) $(TMPDIR)
	$(RM) $@
	$(CP) $< $@
	@$(ECHO) $@ >> $(INSTALLED_FILES_FILE)

$(LIB_LOCATION)/$(CLIENT_LOCATION)/Xusage.txt : $(HOTSPOT_CLIENT_PATH)/Xusage.txt $(LIB_LOCATION)/$(CLIENT_LOCATION) $(TMPDIR)
	$(RM) $@
	$(CP) $< $@
	@$(ECHO) $@ >> $(INSTALLED_FILES_FILE)

$(LIBDIR)/sunrsasign.jar: $(BUILDDIR)/tools/crypto/sunrsasign.jar $(LIBDIR) $(TMPDIR)
	$(RM) $@
	$(CP) $< $@
	@$(ECHO) $@ >> $(INSTALLED_FILES_FILE)


include FILES.gmk
FONTFILES = $(SHARED_FONTFILES)
FONTSDIR  = $(LIBDIR)/fonts
FONTSDIRFILE  = $(FONTSDIR)/fonts.dir

OPTIMIZED += $(FONTFILES) 
DEBUG     += $(FONTFILES) 

ifneq ($(PLATFORM), windows)
OPTIMIZED += $(FONTSDIRFILE)
DEBUG     += $(FONTSDIRFILE)
endif

$(LIBDIR)/fonts/%.ttf: $(SHARE_SRC)/lib/fonts/%.ttf $(LIBDIR)/fonts $(TMPDIR)
	$(RM) $@
	$(CP) $< $@
	$(ECHO) $@ >> $(INSTALLED_FILES_FILE)

$(FONTSDIRFILE): $(PLATFORM_SRC)/classes/sun/awt/motif/java.fonts.dir $(FONTSDIR) $(TMPDIR)
	$(RM) $@
	$(CP) $< $@
	$(ECHO) $@ >> $(INSTALLED_FILES_FILE)

$(TMPDIR):
	$(MKDIR) -p $@

clean clobber::
	@if [ -s $(INSTALLED_FILES_FILE) ]; then \
		$(ECHO) $(RM) $(shell if [ -s $(INSTALLED_FILES_FILE) ]; then $(CAT) $(INSTALLED_FILES_FILE); fi); \
		$(RM) $(shell if [ -s $(INSTALLED_FILES_FILE) ]; then $(CAT) $(INSTALLED_FILES_FILE); fi); \
	fi
	$(RM) $(INSTALLED_FILES_FILE)
	$(RM) -r $(LIBDIR)/fonts
	$(RMDIR) $(LIB_LOCATION)/$(CLIENT_LOCATION) $(LIB_LOCATION)/$(SERVER_LOCATION)

ifdef VARIANT
debug: $(DEBUG)
optimized: $(OPTIMIZED)
else
debug:
	$(MAKE) VARIANT=DBG $@
optimized:
	$(MAKE) VARIANT=OPT $@
endif

.PHONY: all debug optimized
