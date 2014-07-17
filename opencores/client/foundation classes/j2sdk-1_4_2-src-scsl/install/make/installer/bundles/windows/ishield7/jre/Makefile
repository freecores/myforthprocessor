#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

# @(#)Makefile	1.82 02/04/19
#
# This is the makefile for building the J2RE installer bundles after compiling
# the j2sdk sources. Listed below are some general notes in using this makefile.
#
# 1) Requires InstallShield Developer 7.04 defined as $(ALT_ISHIELD7DIR).
# 2) Requires InstallShield Developer 7.04 lang packs East and West
# 3) Requires MKS Toolkit (should be first thing in your PATH).
# 5) $(TMPDIR) is the temporary building directory.
# 6) $(BASE_IMAGE_JRE_DIR) is the base J2RE image.
# 7) $(NEW_IMAGE_JRE_DIR) is the new J2RE image.
# 8) $(BUILDDIR) is two directories above this directory.
# 9) $(CURRENTDIR) is this directory.
#
# Unique target : all
#

# get a standard build environment set...
BUILDDIR=../../../../..
PRODUCT = ishield7

include $(BUILDDIR)/common/Defs.gmk
include $(BUILDDIR)/common/Pack.gmk

#set path names
ifneq ($(BASE_IMAGE_FULLVERSION), $(NEW_IMAGE_FULLVERSION))
  TMPDIR = $(TMPISHIELD7_DIR)/patch
else
  TMPDIR = $(TMPISHIELD7_DIR)/regular
endif

ifdef JDK_UPDATE_VERSION
  CSPACE=94000000
  OSPACE=20000000
  ESPACE=5000000
else
  CSPACE=48000000
  OSPACE=14000000
  ESPACE=4000000
endif

CURRENTDIR = $(shell $(PWD))
TMPJREIMAGE = $(TMPDIR)/jre-image
TMPPACKED_JRE_JARDIR = $(TMPDIR)/packed-jre-jars

SED_CORE_RESERVE=$(SED) -e 's%CORESPACE%$(CSPACE)%g'
SED_OTHER_RESERVE=$(SED) -e 's%OTHERSPACE%$(OSPACE)%g'
SED_EXTRA_RESERVE=$(SED) -e 's%EXTRASPACE%$(ESPACE)%g'
SED_ALL_RESERVE=$(SED_CORE_RESERVE) | $(SED_OTHER_RESERVE) | $(SED_EXTRA_RESERVE)

SED_BINARY_LOCATION = $(SED) -e 's%BINARYPROJECT%$(subst /,\\,$(TMPDIR)/jre.ism)%g'
SED_TEXT_LOCATION = $(SED) -e 's%TEXTPROJECT%$(subst /,\\,$(TMPDIR)/jre.isv)%g'
SED_CAB_LOCATION = $(SED) -e 's%CABLOCATION%$(JRE_CAB_URL)%g'
SED_JDK_VERSION = $(SED) -e 's%JDKVERSION%$(JDK_VERSION)%g'

INSTALLERFILESINGLEEXE=$(JREISHIELDDIR)/jre/singleexe/DiskImages/Disk1/setup.exe
INSTALLERFILEBUNDLEWI=$(JREISHIELDDIR)/jre/bundlewi/DiskImages/Disk1/setup.exe
INSTALLERFILES=$(JREISHIELDDIR)/jre/iftw/DiskImages/Disk1/

###################### all ################################
# 
# THIS IS THE ONLY VALID TARGET (with clean). Copy the jre-image
# to the temp directory, setup the ishield 7 project, and build

all: $(TMPDIR) setup_image setup_installer_files create_bundle 

###################### clean #############################
#
# Remove all produced files.
#
clean: 
	$(RM) -r $(TMPDIR)

$(TMPDIR):
	$(MKDIR) -p $(TMPDIR)

$(TMPDIR)/setup.bmp:
	$(CP) $(BUILDDIR)/installer/bundles/$(PLATFORM)/ishield7/setup.bmp $@

ifeq ($(MILESTONE), fcs)
  LICENSEEND=fcs
else
  LICENSEEND=pre
endif

copy_licenses:
	$(CP) $(J2SE_TOPDIR)/src/share/doc/jre/LICENSE.rtf.$(LICENSEEND) $(TMPDIR)/LICENSE.rtf
	$(CP) $(J2SE_TOPDIR)/src/share/doc/jre/LICENSE_de.rtf.$(LICENSEEND) $(TMPDIR)/LICENSE_de.rtf
	$(CP) $(J2SE_TOPDIR)/src/share/doc/jre/LICENSE_es.rtf.$(LICENSEEND) $(TMPDIR)/LICENSE_es.rtf
	$(CP) $(J2SE_TOPDIR)/src/share/doc/jre/LICENSE_fr.rtf.$(LICENSEEND) $(TMPDIR)/LICENSE_fr.rtf
	$(CP) $(J2SE_TOPDIR)/src/share/doc/jre/LICENSE_it.rtf.$(LICENSEEND) $(TMPDIR)/LICENSE_it.rtf
	$(CP) $(J2SE_TOPDIR)/src/share/doc/jre/LICENSE_ja.rtf.$(LICENSEEND) $(TMPDIR)/LICENSE_ja.rtf
	$(CP) $(J2SE_TOPDIR)/src/share/doc/jre/LICENSE_ko.rtf.$(LICENSEEND) $(TMPDIR)/LICENSE_ko.rtf
	$(CP) $(J2SE_TOPDIR)/src/share/doc/jre/LICENSE_sv.rtf.$(LICENSEEND) $(TMPDIR)/LICENSE_sv.rtf
	$(CP) $(J2SE_TOPDIR)/src/share/doc/jre/LICENSE_zh_CN.rtf.$(LICENSEEND) $(TMPDIR)/LICENSE_zh_CN.rtf
	$(CP) $(J2SE_TOPDIR)/src/share/doc/jre/LICENSE_zh_TW.rtf.$(LICENSEEND) $(TMPDIR)/LICENSE_zh_TW.rtf


ifneq ($(BASE_IMAGE_FULLVERSION), $(NEW_IMAGE_FULLVERSION))
    JRE_BUNDLE_NAME = j2re-$(BIN_BUNDLE_NAMEPART)-patch-$(BUNDLE_DATE)
    JRE_IFTW_BUNDLE_NAME = $(JRE_SHORT_BUNDLE_NAME)-patch-iftw
else
    JRE_BUNDLE_NAME = j2re-$(BIN_BUNDLE_NAMEPART)-$(BUNDLE_DATE)
    JRE_IFTW_BUNDLE_NAME = $(JRE_SHORT_BUNDLE_NAME)-iftw
endif


JREISHIELDDIR = $(TMPDIR)/jre

$(JREISHIELDDIR):
	$(MKDIR) -p $@

ZIPPER = $(TMPDIR)/zipper$(EXE_SUFFIX)

$(ZIPPER):
	$(CP) $(ALT_OUTPUTDIR)/tmp/sun.tools.zipper/$(OBJDIRNAME)/zipper$(EXE_SUFFIX) $@

COPIER = $(TMPDIR)/copier$(EXE_SUFFIX)

$(COPIER):
	$(CP) $(ALT_OUTPUTDIR)/tmp/sun.tools.copier/$(OBJDIRNAME)/copier$(EXE_SUFFIX) $@

TOUCHFILE = $(TMPDIR)/touchfile$(EXE_SUFFIX)

$(TOUCHFILE):
	$(CP) $(ALT_OUTPUTDIR)/tmp/sun.tools.touchfile/$(OBJDIRNAME)/touchfile$(EXE_SUFFIX) $@

PATCHER = $(TMPDIR)/patchjre$(EXE_SUFFIX)

$(PATCHER):
# 
# Use stub patcher to reduce patcher size for download if no patch should be applied.
#
ifneq ($(BASE_IMAGE_FULLVERSION), $(NEW_IMAGE_FULLVERSION))
	$(CP) $(ALT_OUTPUTDIR)/tmp/patcher/jre/$(OBJDIRNAME)/j2re-patcher$(EXE_SUFFIX) $@
else
	$(CP) $(ALT_OUTPUTDIR)/tmp/patcher/stub/$(OBJDIRNAME)/stub-patcher$(EXE_SUFFIX) $@
endif


UNPACKER = $(TMPDIR)/unpack$(EXE_SUFFIX)

$(UNPACKER):
	$(CP) $(UNPACK_EXE) $@

###################### create_bundle ###################### 
# 
# Create the install project and build the release
#
create_bundle: copy_licenses $(TMPDIR)/setup.bmp 
	$(BUILDER) -p $(TMPDIR)/jre.ism -r "bundlewi" -c COMP
ifeq ($(SIGN_FLAG),1)
	$(SIGNER) /spc a:\sun.spc /v a:\sun.pvk -t http://timestamp.verisign.com/scripts/timstamp.dll $(INSTALLERFILEBUNDLEWI)
endif
	$(CP) $(INSTALLERFILEBUNDLEWI) $(BIN_BUNDLEDIR)/$(JRE_BUNDLE_NAME).exe
	$(CP) $(CURRENTDIR)/jre/publicjreISSetupFile.idt $(JREISHIELDDIR)/ISSetupFile.idt
	$(RM) $(TMPDIR)/jre.ism
	$(WSCRIPT) $(TMPDIR)/jrefile.vbs
	$(BUILDER) -p $(TMPDIR)/jre.ism -r "singleexe" -c COMP
	$(CP) -f $(INSTALLERFILESINGLEEXE) $(TMPDIR)/jre-full-setup.exe
	$(BUILDER) -p $(TMPDIR)/jre.ism -r "iftw" -c COMP
ifeq ($(SIGN_FLAG),1)
	$(SIGNER) /spc a:\sun.spc /v a:\sun.pvk -t http://timestamp.verisign.com/scripts/timstamp.dll $(INSTALLERFILES)/setup.exe
endif
	$(CP) -f $(INSTALLERFILES)/setup.exe $(TMPDIR)/jre-iftw-setup.exe
	$(MV) $(INSTALLERFILES)/setup.exe $(INSTALLERFILES)/$(JRE_IFTW_BUNDLE_NAME).exe
	$(RM) $(INSTALLERFILES)/setup.bmp
	$(CD) $(INSTALLERFILES) ; $(ZIPEXE) -r $(BIN_BUNDLEDIR)/$(JRE_BUNDLE_NAME).zip *

setup_installer_files: $(COPIER) $(PATCHER) $(UNPACKER) $(REGUTILS) $(JREISHIELDDIR)
	$(CP) -f $(NEW_IMAGE_JRE_DIR)/bin/RegUtils.dll $(TMPDIR)
	$(CP) $(CURRENTDIR)/jre.isv $(TMPDIR)
	$(CD) $(CURRENTDIR)/jre/; $(FIND) . -depth | $(GREP) -v "SCCS" | $(CPIO) -pdum $(JREISHIELDDIR)/
	$(CHMOD) 777 $(JREISHIELDDIR)/*
	$(SED_CAB_LOCATION) $(CURRENTDIR)/jre/ISReleaseExtended.idt > $(JREISHIELDDIR)/ISReleaseExtended.idt
	$(CAT) $(CURRENTDIR)/jre/Property.idt | $(SED_JDK_VERSION) | $(PROCESS_FILES_COMP) | $(SED_UPDATE_VERSION) | $(SED_FULL_VERSION) > $(JREISHIELDDIR)/Property.idt
	$(CAT) $(CURRENTDIR)/jre/_SummaryInformation.idt | $(PROCESS_FILES_COMP) | $(SED_UPDATE_VERSION) > $(JREISHIELDDIR)/_SummaryInformation.idt
	$(SED_FULL_VERSION) $(CURRENTDIR)/jre/Directory.idt | $(SED_BASE_IMAGE_FULL_VERSION) | $(SED_NEW_IMAGE_FULL_VERSION) > $(JREISHIELDDIR)/Directory.idt
	$(CAT) $(CURRENTDIR)/jre/File.idt | $(PROCESS_FILES_COMP) > $(JREISHIELDDIR)/File.idt
	$(CAT) $(CURRENTDIR)/jre/Component.idt | $(PROCESS_FILES_COMP) > $(JREISHIELDDIR)/Component.idt
	$(CAT) $(CURRENTDIR)/jre/Registry.idt | $(PROCESS_FILES_COMP) > $(JREISHIELDDIR)/Registry.idt
	$(CAT) $(CURRENTDIR)/jre/FeatureComponents.idt | $(PROCESS_FILES_COMP) > $(JREISHIELDDIR)/FeatureComponents.idt
	$(CAT) $(CURRENTDIR)/jre/ISComponentExtended.idt | $(PROCESS_FILES_COMP) > $(JREISHIELDDIR)/ISComponentExtended.idt
	$(CAT) $(CURRENTDIR)/jre/CustomAction.idt | $(PROCESS_FILES_COMP) > $(JREISHIELDDIR)/CustomAction.idt
	$(CAT) $(CURRENTDIR)/jre/ISString.idt | $(SED_FULL_VERSION) > $(JREISHIELDDIR)/ISString.idt
	$(CAT) $(CURRENTDIR)/jre/ReserveCost.idt | $(PROCESS_FILES_COMP) | $(SED_ALL_RESERVE) > $(JREISHIELDDIR)/ReserveCost.idt
	$(MAKE) create_removefile
	$(CP) $(TMPDIR)/removefile $(JREISHIELDDIR)/RemoveFile.idt
	$(CAT) $(CURRENTDIR)/../file.vbs | $(SED_BINARY_LOCATION) | $(SED_TEXT_LOCATION) > $(TMPDIR)/jrefile.vbs
	$(WSCRIPT) $(TMPDIR)/jrefile.vbs

#
# Copy base image to be bundled with installer.
#
setup_image: $(ZIPPER) 
	@$(ECHO) Cleaning old temporary install image
	$(CHMOD) -Rf 777 $(TMPJREIMAGE)
	$(RM) -r $(TMPJREIMAGE)
	$(MKDIR) -p $(TMPJREIMAGE)
ifneq ($(BASE_IMAGE_FULLVERSION), $(NEW_IMAGE_FULLVERSION))
	#
	# Patch build - copy cabs from old base image repository
	#
	$(CP) -r $(ALT_BASE_IMAGE_DIR)/j2re-repository-image/*.zip $(TMPJREIMAGE)
	$(CP) $(NEW_IMAGE_JRE_DIR)/COPYRIGHT $(TMPJREIMAGE)
else
	$(CP) -r $(NEW_IMAGE_JRE_DIR)/* $(TMPJREIMAGE)
	$(RM) -r $(TMPPACKED_JRE_JARDIR)
	$(MKDIR) -p $(TMPPACKED_JRE_JARDIR)
	$(MAKE) do_pack
	$(MAKE) do_zip
endif

#
# Pack jars in base image.
#

do_pack:
ifneq ($(BASE_IMAGE_FULLVERSION), $(NEW_IMAGE_FULLVERSION))
	@#we pack/unpack the TMPJREIMAGE
	cd $(BUILDDIR)/pack; \
	$(MAKE) PACK_OUTPUTDIR=$(TMPPACKED_JRE_JARDIR) \
		VAR_IMAGE_DIR=$(TMPJREIMAGE) \
		JARS_TO_PACK="$(JRE_PACKED_JARS)" \
		pack-jars
	for i in $(JRE_PACKED_JARS) ; do \
	    if [ -f $(BASE_IMAGE_JRE_DIR)/$$i ]; then \
		$(RM) $(TMPJREIMAGE)/$$i ; \
		$(CP) $(TMPPACKED_JRE_JARDIR)/`$(DIRNAME) $$i`/`$(BASENAME) $$i .jar`$(PACK_SUFFIX) $(TMPJREIMAGE)/`$(DIRNAME) $$i` ; \
	    fi ; \
	done
else
	for i in $(JRE_PACKED_JARS) ; do \
	    if [ -f $(JRE_IMAGE_DIR)/$$i ]; then \
		$(RM) $(TMPJREIMAGE)/$$i ; \
                $(CP) $(ALT_OUTPUTDIR)/pack/pack-jre-jars/`$(DIRNAME) $$i`/`$(BASENAME) $$i .jar`$(PACK_SUFFIX) $(TMPJREIMAGE)/`$(DIRNAME) $$i` ; \
	    fi ; \
	done
endif

##########################################################
#
# This is a workaround.  Install shield 7.04 makes creates
# a component for each sub-dir of our product, and therefore
# a .cab file for each component.  By zipping w/no compression
# we eliminate sub-dirs, and have less .cab files to host
do_zip: $(ZIPPER) $(TOUCHFILE)
	$(MV) $(TMPJREIMAGE)/bin $(TMPDIR)/tmpbin
	$(MV) $(TMPJREIMAGE) $(TMPDIR)/tmpjreimage
	$(MKDIR) $(TMPJREIMAGE)
	$(MV) $(TMPDIR)/tmpbin $(TMPJREIMAGE)/bin
	$(CD) $(TMPJREIMAGE) ; $(ZIPEXE) -0 -r $(TMPDIR)/core1.zip *
	$(RM) -r $(TMPJREIMAGE)
	$(MV) $(TMPDIR)/tmpjreimage $(TMPJREIMAGE)
	$(MV) $(TMPJREIMAGE)/lib/rt.pack $(TMPDIR)/tmprt.pack
	$(MV) $(TMPJREIMAGE) $(TMPDIR)/tmpjreimage
	$(MKDIR) -p $(TMPJREIMAGE)/lib
	$(MV) $(TMPDIR)/tmprt.pack $(TMPJREIMAGE)/lib/rt.pack
	$(CD) $(TMPJREIMAGE) ; $(ZIPEXE) -0 -r $(TMPDIR)/core2.zip *
	$(RM) -r $(TMPJREIMAGE)
	$(MV) $(TMPDIR)/tmpjreimage $(TMPJREIMAGE)
	$(MV) $(TMPJREIMAGE)/lib/ext/localedata.pack $(TMPJREIMAGE)/lib/charsets.pack $(TMPDIR)
	$(MV) $(TMPJREIMAGE)/lib/fonts $(TMPDIR)
	$(MV) $(TMPJREIMAGE)/COPYRIGHT $(TMPDIR)
	$(MKDIR) -p $(TMPJREIMAGE)/lib/fonts
	$(MV) $(TMPDIR)/fonts/LucidaSansRegular.ttf $(TMPJREIMAGE)/lib/fonts
	$(MV) $(TMPJREIMAGE)/lib/cmm/PYCC.pf $(TMPDIR)
	@if [ -x `$(ECHO) $(TMPJREIMAGE)/lib/audio` ]; then \
	    $(MV) $(TMPJREIMAGE)/lib/audio $(TMPDIR) ; \
	fi
	$(CD) $(TMPJREIMAGE) ; $(ZIPEXE) -0 -r $(TMPDIR)/core3.zip *
	$(RM) -r $(TMPJREIMAGE)/*
	$(MKDIR) -p $(TMPJREIMAGE)/lib/ext
	$(MV) $(TMPDIR)/localedata.pack $(TMPJREIMAGE)/lib/ext
	$(MV) $(TMPDIR)/charsets.pack $(TMPJREIMAGE)/lib
	$(CD) $(TMPJREIMAGE) ; $(ZIPEXE) -0 -r $(TMPDIR)/other.zip *
	$(RM) -r $(TMPJREIMAGE)/*
	$(MKDIR) -p $(TMPJREIMAGE)/lib/cmm
	$(MV) $(TMPDIR)/PYCC.pf $(TMPJREIMAGE)/lib/cmm
	$(MV) $(TMPDIR)/fonts $(TMPJREIMAGE)/lib
	@if [ -x `$(ECHO) $(TMPDIR)/audio` ]; then \
	    $(MV) $(TMPDIR)/audio $(TMPJREIMAGE)/lib ;	\
	fi
	$(CD) $(TMPJREIMAGE) ; $(ZIPEXE) -0 -r $(TMPDIR)/extra.zip *
	$(RM) -r $(TMPJREIMAGE)/*
	$(MV) $(TMPDIR)/core1.zip $(TMPDIR)/core2.zip $(TMPDIR)/core3.zip $(TMPDIR)/other.zip $(TMPDIR)/extra.zip $(TMPDIR)/COPYRIGHT $(TMPJREIMAGE)
	#
	# Touch all the zip files in the directory and set the creation date
	# to arbitary. This is important because InstallShield will only generate the 
	# same base image CAB if and only if the file content are the same and the file
	# creation date are the same.
	#
	# Notice that the arbitary date must be the same in the base image across
	# releases, or the Install-On-Demand may trigger base image to be downloaded
	# even if base image exists on the system. Currently, the arbitary file creation
	# date is 01/01/2003 8:00am. See #4796128.
	#
	# $(TOUCHFILE) $(TMPJREIMAGE)

create_removefile:
	$(RM) $(TMPDIR)/removefile
	$(CP) $(JREISHIELDDIR)/RemoveFile.idt $(TMPDIR)/removefile
	for dir in `find $(JRE_IMAGE_DIR) -type d` ; do \
	  dirbase=`$(BASENAME) $$dir` ; \
	  if [ "$$dirbase" = "j2re-image" ]; then \
	      dirbase=INSTALLDIR ; \
	  fi ; \
	  printf "$$dirbase\tjz$(JDK_MAJOR_VERSION)$(JDK_MINOR_VERSION)$(JDK_MICRO_VERSION)$(JDK_UPDATE_ISHIELD_VERSION)\t\t$$dirbase\t3\n" >> $(TMPDIR)/removefile; \
	  for file in `ls -p $$dir | grep -v "/"` ; do \
	    filebase=`$(BASENAME) $$file` ; \
	    filesafe=`$(BASENAME) $$filebase | $(TR) - j | $(TR) + k` ; \
	    LENGTH=`$(ECHO) $$filebase | $(NAWK) '{print length($$0);}'` ; \
	    if [ "$$LENGTH" -gt "7" ]; then \
		filebase=`$(ECHO) $$filebase | $(NAWK) '{print substr($$0,0,7);}'`* ; \
	    fi ; \
	    check=`$(ECHO) $$filebase | $(NAWK) '{print substr($$0,0,3);}'` ; \
	    if [ "$$check" = "GMT" ]; then \
		filebase=`$(ECHO) "GMT*"` ; \
	    fi ; \
	    printf "$$dirbase$$filesafe\tjz$(JDK_MAJOR_VERSION)$(JDK_MINOR_VERSION)$(JDK_MICRO_VERSION)$(JDK_UPDATE_ISHIELD_VERSION)\t$$filebase\t$$dirbase\t3\n" >> $(TMPDIR)/removefile; \
	  done; \
	done;

.PHONY: clean all create_bundle setup_installer_files setup_image do_pack do_zip
