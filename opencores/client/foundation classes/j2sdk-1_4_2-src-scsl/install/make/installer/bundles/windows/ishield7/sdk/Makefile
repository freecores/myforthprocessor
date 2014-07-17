#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

# @(#)Makefile	1.42 03/04/15
#
# This is the makefile for building the J2SDK installer bundle after compiling
# the j2sdk sources. Listed below are some general notes in using this makefile.
#
# 1) Requires InstallShield Developer 7.04 defined as $(ALT_ISHIELD7DIR).
# 2) Requires InstallShield Developer 7.04 lang packs East and West
# 3) Requires MKS Toolkit (should be first thing in your PATH).
# 5) $(TMPDIR) is the temporary building directory.
# 6) $(BASE_IMAGE_SDK_DIR) is the base J2SDK image.
# 7) $(NEW_IMAGE_SDK_DIR) is the new J2SDK image.
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
  TSPACE=133000000
  DSPACE=29000000
  SSPACE=33000000
  PSPACE=192000000
else
  TSPACE=87000000
  DSPACE=13000000
  SSPACE=11000000
  PSPACE=146000000
endif


CURRENTDIR = $(shell $(PWD))
TMPSDKIMAGE = $(TMPDIR)/sdk-image
TMPPACKED_JDK_JARDIR = $(TMPDIR)/packed-sdk-jars

SED_TOOLS_RESERVE=$(SED) -e 's%TOOLSSPACE%$(TSPACE)%g'
SED_DEMO_RESERVE=$(SED) -e 's%DEMOSPACE%$(DSPACE)%g'
SED_SRC_RESERVE=$(SED) -e 's%SRCSPACE%$(SSPACE)%g'
SED_PUBLIC_RESERVE=$(SED) -e 's%PUBLICSPACE%$(PSPACE)%g'
SED_ALL_RESERVE=$(SED_TOOLS_RESERVE) | $(SED_DEMO_RESERVE) |$(SED_SRC_RESERVE) |$(SED_PUBLIC_RESERVE)

SED_BINARY_LOCATION = $(SED) -e 's%BINARYPROJECT%$(subst /,\\,$(TMPDIR)/sdk.ism)%g'
SED_TEXT_LOCATION = $(SED) -e 's%TEXTPROJECT%$(subst /,\\,$(TMPDIR)/sdk.isv)%g'
SED_CAB_LOCATION = $(SED) -e 's%CABLOCATION%$(SDK_CAB_URL)%g'

INSTALLERFILE=$(SDKISHIELDDIR)/sdk/singleexe/DiskImages/Disk1/setup.exe
INSTALLERFILES=$(SDKISHIELDDIR)/sdk/iftw/DiskImages/Disk1/

###################### all ################################
# 
# THIS IS THE ONLY VALID TARGET (with clean). Copy the sdk-image
# to the temp directory, setup the ishield 7 project, and build
# 
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
	$(CP) $(J2SE_TOPDIR)/src/share/doc/jdk/LICENSE.rtf.$(LICENSEEND) $(TMPDIR)/sdk-license.rtf

SDKISHIELDDIR = $(TMPDIR)/sdk

$(SDKISHIELDDIR):
	$(MKDIR) -p $@

ifneq ($(BASE_IMAGE_FULLVERSION), $(NEW_IMAGE_FULLVERSION))
    SDK_BUNDLE_NAME = j2sdk-$(BIN_BUNDLE_NAMEPART)-patch-$(BUNDLE_DATE)
    SDK_IFTW_BUNDLE_NAME = $(JDK_SHORT_BUNDLE_NAME)-patch-iftw
else
    SDK_BUNDLE_NAME = j2sdk-$(BIN_BUNDLE_NAMEPART)-$(BUNDLE_DATE)
    SDK_IFTW_BUNDLE_NAME = $(JDK_SHORT_BUNDLE_NAME)-iftw
endif

ZIPPER = $(TMPDIR)/zipper$(EXE_SUFFIX)

$(ZIPPER):
	$(CP) $(ALT_OUTPUTDIR)/tmp/sun.tools.zipper/$(OBJDIRNAME)/zipper$(EXE_SUFFIX) $@

COPIER = $(TMPDIR)/copier$(EXE_SUFFIX)

$(COPIER):
	$(CP) $(ALT_OUTPUTDIR)/tmp/sun.tools.copier/$(OBJDIRNAME)/copier$(EXE_SUFFIX) $@

TOUCHFILE = $(TMPDIR)/touchfile$(EXE_SUFFIX)

$(TOUCHFILE):
	$(CP) $(ALT_OUTPUTDIR)/tmp/sun.tools.touchfile/$(OBJDIRNAME)/touchfile$(EXE_SUFFIX) $@

PATCHER = $(TMPDIR)/patchsdk$(EXE_SUFFIX)

$(PATCHER):
# 
# Use stub patcher to reduce patcher size for download if no patch should be applied.
#
ifneq ($(BASE_IMAGE_FULLVERSION), $(NEW_IMAGE_FULLVERSION))
	$(CP) $(ALT_OUTPUTDIR)/tmp/patcher/sdk/$(OBJDIRNAME)/j2sdk-patcher$(EXE_SUFFIX) $@
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
	$(MAKE) singleexe
ifeq ($(ARCH_DATA_MODEL),32)
	$(MAKE) iftw
endif

singleexe:
ifeq ($(ARCH_DATA_MODEL),32)
	$(CP) -f $(TMPDIR)/jre-full-setup.exe $(TMPDIR)/setup.exe
endif
	$(BUILDER) -p $(TMPDIR)/sdk.ism -r "singleexe" -c COMP
	$(CP) $(INSTALLERFILE) $(BIN_BUNDLEDIR)/$(SDK_BUNDLE_NAME).exe

iftw:
	$(CP) -f $(TMPDIR)/jre-iftw-setup.exe $(TMPDIR)/setup.exe
	$(BUILDER) -p $(TMPDIR)/sdk.ism -r "iftw" -c COMP
	$(MV) $(INSTALLERFILES)/setup.exe $(INSTALLERFILES)/$(SDK_IFTW_BUNDLE_NAME).exe
	$(RM) $(INSTALLERFILES)/setup.bmp
	$(RM) $(TMPDIR)/setup.exe
	$(CD) $(INSTALLERFILES)/ ; $(ZIPEXE) -r $(BIN_BUNDLEDIR)/$(SDK_BUNDLE_NAME).zip *

ifeq ($(ARCH_DATA_MODEL),32)
setup_installer_files: $(COPIER) $(PATCHER) $(UNPACKER) $(REGUTILS) $(SDKISHIELDDIR)
else
setup_installer_files: $(UNPACKER) $(REGUTILS) $(SDKISHIELDDIR)
endif
	$(CP) $(CURRENTDIR)/sdk.isv $(TMPDIR)
	$(CD) $(CURRENTDIR)/sdk/; $(FIND) . -depth | $(GREP) -v "SCCS" | $(CPIO) -pdum $(SDKISHIELDDIR)/
	$(CHMOD) 777 $(SDKISHIELDDIR)/*
	$(SED_CAB_LOCATION) $(CURRENTDIR)/sdk/ISReleaseExtended.idt > $(SDKISHIELDDIR)/ISReleaseExtended.idt
	$(CAT) $(CURRENTDIR)/sdk/Property.idt | $(PROCESS_FILES_COMP) | $(SED_UPDATE_VERSION) | $(SED_FULL_VERSION) > $(SDKISHIELDDIR)/Property.idt
	$(CAT) $(CURRENTDIR)/sdk/_SummaryInformation.idt | $(PROCESS_FILES_COMP) | $(SED_UPDATE_VERSION) > $(SDKISHIELDDIR)/_SummaryInformation.idt
	$(SED_FULL_VERSION) $(CURRENTDIR)/sdk/Directory.idt | $(SED_BASE_IMAGE_FULL_VERSION) | $(SED_NEW_IMAGE_FULL_VERSION) > $(SDKISHIELDDIR)/Directory.idt
	$(CAT) $(CURRENTDIR)/sdk/File.idt | $(PROCESS_FILES_COMP) > $(SDKISHIELDDIR)/File.idt
	$(CAT) $(CURRENTDIR)/sdk/Component.idt | $(PROCESS_FILES_COMP) > $(SDKISHIELDDIR)/Component.idt
	$(CAT) $(CURRENTDIR)/sdk/ReserveCost.idt | $(PROCESS_FILES_COMP) | $(SED_ALL_RESERVE) > $(SDKISHIELDDIR)/ReserveCost.idt
	$(CAT) $(CURRENTDIR)/sdk/FeatureComponents.idt | $(PROCESS_FILES_COMP) > $(SDKISHIELDDIR)/FeatureComponents.idt
	$(CAT) $(CURRENTDIR)/sdk/ISComponentExtended.idt | $(PROCESS_FILES_COMP) > $(SDKISHIELDDIR)/ISComponentExtended.idt
	$(CAT) $(CURRENTDIR)/sdk/CustomAction.idt | $(PROCESS_FILES_COMP) > $(SDKISHIELDDIR)/CustomAction.idt
	$(CAT) $(CURRENTDIR)/sdk/ISString.idt | $(SED_FULL_VERSION) > $(SDKISHIELDDIR)/ISString.idt
	$(MAKE) create_removefile
	$(CP) $(TMPDIR)/removefile $(SDKISHIELDDIR)/RemoveFile.idt
	$(CAT) $(CURRENTDIR)/../file.vbs | $(SED_BINARY_LOCATION) | $(SED_TEXT_LOCATION) > $(TMPDIR)/sdkfile.vbs
ifeq ($(ARCH_DATA_MODEL),64)
	$(CP) -fr $(CURRENTDIR)/64bit/* $(SDKISHIELDDIR)/
	$(SED_FULL_VERSION) $(CURRENTDIR)/64bit/Directory.idt | $(SED_BASE_IMAGE_FULL_VERSION) | $(SED_NEW_IMAGE_FULL_VERSION) > $(SDKISHIELDDIR)/Directory.idt
	$(RM) -rf $(SDKISHIELDDIR)/SCCS
endif
	$(WSCRIPT) $(TMPDIR)/sdkfile.vbs

#
# Copy base image to be bundled with installer.
#
ifeq ($(ARCH_DATA_MODEL),32)
setup_image: $(ZIPPER) 
else
setup_image:
endif
	@$(ECHO) Cleaning old temporary install image
	$(CHMOD) -Rf 777 $(TMPSDKIMAGE)
	$(RM) -r $(TMPSDKIMAGE)
	$(MKDIR) -p $(TMPSDKIMAGE)
ifneq ($(BASE_IMAGE_FULLVERSION), $(NEW_IMAGE_FULLVERSION))
	#
	# Patch build - copy cabs from old base image repository
	#
	$(CP) -r $(ALT_BASE_IMAGE_DIR)/j2sdk-repository-image/*.zip $(TMPSDKIMAGE)	
	$(CP) $(NEW_IMAGE_SDK_DIR)/COPYRIGHT $(TMPSDKIMAGE)
else
	$(CP) -r $(NEW_IMAGE_SDK_DIR)/* $(TMPSDKIMAGE)
	$(RM) -r $(TMPPACKED_JDK_JARDIR)
	$(MKDIR) -p $(TMPPACKED_JDK_JARDIR)
	$(MAKE) do_pack
ifeq ($(ARCH_DATA_MODEL),32)
	$(MAKE) do_zip
endif
endif

do_pack:
ifneq ($(BASE_IMAGE_FULLVERSION), $(NEW_IMAGE_FULLVERSION))
	@#we pack/unpack the BASE_IMAGE_SDK_DIR, extra flag to keep debug info
	cd $(BUILDDIR)/pack; \
	$(MAKE) PACK_OUTPUTDIR=$(TMPPACKED_JDK_JARDIR) \
		VAR_IMAGE_DIR=$(TMPSDKIMAGE) \
		JARS_TO_PACK="$(JDK_PACKED_JARS)" \
		PACK_FLAGS+="-C -D" \
		pack-jars
	for i in $(JDK_PACKED_JARS) ; do \
	    if [ -f $(BASE_IMAGE_SDK_DIR)/$$i ]; then \
		$(RM) $(TMPSDKIMAGE)/$$i ; \
		$(CP) $(TMPPACKED_JDK_JARDIR)/`$(DIRNAME) $$i`/`$(BASENAME) $$i .jar`$(PACK_SUFFIX) $(TMPSDKIMAGE)/`$(DIRNAME) $$i` ; \
	    fi ; \
	done
else
	for i in $(JDK_PACKED_JARS) ; do \
	    $(RM) $(TMPSDKIMAGE)/$$i ; \
	    $(CP) $(ALT_OUTPUTDIR)/pack/pack-jdk-jars/`$(DIRNAME) $$i`/`$(BASENAME) $$i .jar`$(PACK_SUFFIX) $(TMPSDKIMAGE)/`$(DIRNAME) $$i` ; \
	done
endif

##########################################################
#
# This is a workaround.  Install shield 7.04 makes creates
# a component for each sub-dir of our product, and therefore
# a .cab file for each component.  By zipping w/no compression
# we eliminate sub-dirs, and have less .cab files to host
do_zip: $(ZIPPER) $(TOUCHFILE)
	$(MV) $(TMPSDKIMAGE)/src.zip $(TMPSDKIMAGE)/demo $(TMPSDKIMAGE)/COPYRIGHT $(TMPDIR)
	$(CD) $(TMPSDKIMAGE) ; $(ZIPEXE) -0 -r $(TMPDIR)/tools.zip *
	$(RM) -rf $(TMPSDKIMAGE)/*
	$(MV) $(TMPDIR)/demo $(TMPSDKIMAGE)
	$(CD) $(TMPSDKIMAGE) ; $(ZIPEXE) -0 -r $(TMPDIR)/demos.zip *
	$(RM) -rf $(TMPSDKIMAGE)/*
	$(MV) $(TMPDIR)/tools.zip $(TMPDIR)/demos.zip $(TMPDIR)/src.zip $(TMPDIR)/COPYRIGHT $(TMPSDKIMAGE)
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
	#$(TOUCHFILE) $(TMPSDKIMAGE)

create_removefile:
	$(RM) $(TMPDIR)/removefile
	$(CP) $(SDKISHIELDDIR)/RemoveFile.idt $(TMPDIR)/removefile
	for dir in `$(CD) $(JDK_IMAGE_DIR) ; find * -type d` ; do \
	  dirkey=`$(ECHO) $$dir | $(TR) -d "/"` ; \
	  printf "$$dirkey\tsz$(JDK_MAJOR_VERSION)$(JDK_MINOR_VERSION)$(JDK_MICRO_VERSION)$(JDK_UPDATE_ISHIELD_VERSION)\t\t$$dirkey\t3\n" >> $(TMPDIR)/removefile; \
	  for file in `$(CD) $(JDK_IMAGE_DIR)/$$dir ; $(LS) -p | grep -v "/"` ; do \
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
	    printf "$$dirkey$$filesafe\tsz$(JDK_MAJOR_VERSION)$(JDK_MINOR_VERSION)$(JDK_MICRO_VERSION)$(JDK_UPDATE_ISHIELD_VERSION)\t$$filebase\t$$dirkey\t3\n" >> $(TMPDIR)/removefile; \
	  done; \
	done;
	for filesafe in `$(CD) $(JDK_IMAGE_DIR) ; $(LS) -p | grep -v "/"` ; do \
	  LENGTH=`$(ECHO) $$filesafe | $(NAWK) '{print length($$0);}'` ; \
	  file=`$(ECHO) $$filesafe` ; \
	  if [ "$$LENGTH" -gt "7" ]; then \
	    file=`$(ECHO) $$filesafe | $(NAWK) '{print substr($$0,0,7);}'`* ; \
	  fi ; \
	  printf "$$filesafe\tsz$(JDK_MAJOR_VERSION)$(JDK_MINOR_VERSION)$(JDK_MICRO_VERSION)$(JDK_UPDATE_ISHIELD_VERSION)\t$$file\tINSTALLDIR\t3\n" >> $(TMPDIR)/removefile; \
	done;

.PHONY: clean all create_bundle setup_installer_files setup_image do_zip do_pack

