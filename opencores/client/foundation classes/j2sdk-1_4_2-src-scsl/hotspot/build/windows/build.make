# @(#)build.make	1.14 03/01/23 10:51:02
# 
# Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

# Note: this makefile is invoked both from build.bat and from the J2SE
# control workspace in exactly the same manner; the required
# environment variables (Variant, WorkSpace, BootStrapDir, BuildID)
# are passed in as command line arguments.

# Allow control workspace to force Itanium builds with LP64
!ifdef LP64
!if "$(LP64)" == "1"
ARCH=ia64
!endif
!endif

# If we haven't set an ARCH yet use i486
# create.bat and build.bat will set it, if used.
!ifndef ARCH
ARCH=i486
!endif

# Supply these from the command line or the environment
#  It doesn't make sense to default this one
Variant=
#  It doesn't make sense to default this one
WorkSpace=

variantDir = windows_$(ARCH)_$(Variant)

defaultTarget: product

product: checks $(variantDir) $(variantDir)\local.make
	cd $(variantDir)
	nmake -f $(WorkSpace)\build\windows\makefiles\top.make ARCH=$(ARCH)

develop: checks $(variantDir) $(variantDir)\local.make
	cd $(variantDir)
	nmake -f $(WorkSpace)\build\windows\makefiles\top.make DEVELOP=1 ARCH=$(ARCH)

clean: checkVariant
	- rm -r -f $(variantDir)

$(variantDir):
	mkdir $(variantDir)

$(variantDir)\local.make: checks
	@ echo # Generated file					>  $@
	@ echo Variant=$(Variant)				>> $@
	@ echo WorkSpace=$(WorkSpace)				>> $@
	@ echo BootStrapDir=$(BootStrapDir)			>> $@
	@ echo BuildID=$(BuildID)				>> $@
        @ if "$(USERNAME)" NEQ "" echo BuildUser=$(USERNAME)	>> $@

checks: checkVariant checkWorkSpace

checkVariant:
	@ if "$(Variant)"=="" echo Need to specify "Variant=[compiler2|compiler1|core]" && false
	@ if "$(Variant)" NEQ "compiler2" if "$(Variant)" NEQ "compiler1" if "$(Variant)" NEQ "core"	\
          echo Need to specify "Variant=[compiler2|compiler1|core]" && false

checkWorkSpace:
	@ if "$(WorkSpace)"=="" echo Need to specify "WorkSpace=..." && false

checkBuildID:
	@ if "$(BuildID)"=="" echo Need to specify "BuildID=..." && false
