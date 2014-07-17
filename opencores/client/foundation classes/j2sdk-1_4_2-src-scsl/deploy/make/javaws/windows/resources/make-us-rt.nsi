# Java Web Start 1.0.1_01 Installer Script
# @(#)make-us-rt.nsi	1.38 02/08/08

#################
# Header Settings
#################

Name "Java Web Start ##RELEASE##"
OutFile setup.exe
CRCCheck off
# 0x0409 is en_US
StringTable strings.txt 0x0409
LicenseText "$STRING_LIC_TXT"
LicenseData javaws-license.txt
ComponentText "$STRING_COMP_TXT"
InstType Normal
DirText "$STRING_DIR_TXT"
UninstallText "$STRING_UNINST_TXT"
UninstallExeName uninst-javaws.exe
InstallDir "$PROGRAMFILES\Java Web Start"
InstallDirRegKey HKEY_LOCAL_MACHINE "SOFTWARE\JavaSoft\Java Web Start\1.0.1" "Home"

##############
# File Section
##############

Section -post
SectionIn 1
SetCompress auto
SetOutPath $INSTDIR
File uncrunch.exe
File javaws.exe
File javaws.cfg
File javaws.out
File javaws.policy
File javawspl.dll
UnRegDLL $INSTDIR\javawebstart.dll
File javawebstart.dll
File JavaCup.ico
File cacerts
File Readme.html
File javalogo52x88.gif
File sunlogo64x30.gif
File helper.exe
File jre.exe
File jre.iss
SetOutPath $INSTDIR\resources
File resources\*.*
SetOutPath $INSTDIR
ExecWait '"$INSTDIR\uncrunch.exe" "$INSTDIR\javaws.out" "$INSTDIR\jawstmp.jar"'

###################
# Registry Settings
###################

# Tell Windows about our MIME type
WriteRegStr HKEY_CLASSES_ROOT  ".jnlp" "" "JNLPFile"
WriteRegStr HKEY_CLASSES_ROOT  "JNLPFile" "" "JNLP File"
WriteRegStr HKEY_CLASSES_ROOT  ".jnlp" "Content Type" "application/x-java-jnlp-file"
WriteRegBin HKEY_CLASSES_ROOT  "JNLPFile" "EditFlags" 00000100

# Tell Windows Explorer about how to launch a .jnlp file
WriteRegStr HKEY_CLASSES_ROOT  "JNLPFile\Shell\Open" "" "&Launch"
WriteRegStr HKEY_CLASSES_ROOT  "JNLPFile\Shell\Open\Command" "" '"$INSTDIR\javaws.exe" "%1"'

# Another way to specify a MIME type
WriteRegStr HKEY_CLASSES_ROOT  "MIME\Database\Content Type\application/x-java-jnlp-file" "Extension" ".jnlp"

# Java Web Start Home Home
WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\JavaSoft\Java Web Start\1.0.1" "Home" "$INSTDIR"
WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\JavaSoft\Java Web Start\1.0.1_02" "Home" "$INSTDIR"
WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\JavaSoft\Java Web Start\1.0.1_03" "Home" "$INSTDIR"
WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\JavaSoft\Java Web Start\1.2" "Home" "$INSTDIR"
WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\JavaSoft\Java Web Start\1.4.2" "Home" "$INSTDIR"

# Tell Netscape we are a trusted application
WriteRegStr HKEY_CURRENT_USER  "Software\Netscape\Netscape Navigator\User Trusted External Applications" '"$INSTDIR\javaws.exe"' "Yes"

# Tell Windows about the Path to Java Web Start
WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\App Paths\javaws.exe" "" "$INSTDIR\javaws.exe"
WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\App Paths\javaws.exe" "Path" "$INSTDIR"

# Uninstall Information
WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\Java Web Start" "DisplayName" "Java Web Start"
WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\Java Web Start" "UninstallString" '"$INSTDIR\uninst-javaws.exe"'

# In the helper DLL we do the following:
# This tells Netscape to use Java Web Start to view files with MIME type application/x-java-jnlp-file
# where "TYPEXX" below is the next available TYPE value in the Viewers key (e.g. "TYPE01")
# HKEY_CURRENT_USER "Software\Netscape\Netscape Navigator\Viewers" "application/x-java-jnlp-file" '"$INSTDIR\javaws.exe"'
# HKEY_CURRENT_USER "Software\Netscape\Netscape Navigator\Viewers" "TYPEXX" "application/x-java-jnlp-file"

####################
# Post-Install Steps
####################

# Install JRE if necessary
# Detect if JRE is already installed
ExecWait '"$INSTDIR\helper.exe" 8 "1.2.2"'
If "$?==0"
ExecWait '"$INSTDIR\helper.exe" 8 "1.3"'
If "$?==0"
ExecWait '"$INSTDIR\helper.exe" 8 "1.3.1"'
If "$?==0"
ExecWait '"$INSTDIR\helper.exe" 8 "1.4"'
If "$?==0"
# If it's not, ask user to confirm installation directory
AltDirBox "$PROGRAMFILES\JavaSoft\JRE\1.3.1" "Java 2 Runtime Environment" "1.3.1_01"
If "$?!=0"
# If user didn't hit cancel, Install the JRE
ExecWait '"$INSTDIR\helper.exe" 6 "$ALTDIR" "1.3.1" "$INSTDIR"'
EndIf
EndIf
EndIf
EndIf
EndIf
# Create the ini file that the helper DLL will later create javaws.cfg from
WriteINIStr "$INSTDIR\foo.ini" foo jreVersion "1.3.1"
WriteINIStr "$INSTDIR\foo.ini" foo targetDir "$INSTDIR"
WriteINIStr "$INSTDIR\foo.ini" foo tempDir "$TEMP"

# This creates the initial CFG file
ExecWait '"$INSTDIR\helper.exe" 4 "$INSTDIR\foo.ini"'

# Delete intermediate files
Delete $INSTDIR\jre.exe
Delete $INSTDIR\jre.iss
Delete $INSTDIR\jre.iss2
Delete $INSTDIR\foo.ini
Delete $INSTDIR\uncrunch.exe
Delete $INSTDIR\javaws.out

# Register the javawebstart.dll
RegDLL $INSTDIR\javawebstart.dll

# Update the CFG file
# If we need a reboot, delay the update until next reboot
if "$NEEDSREBOOT==0"
ExecWait '"$INSTDIR\javaws.exe" -updateVersions'
EndIf
if "$NEEDSREBOOT==1"
WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\RunOnce" "Java Web Start" '"$INSTDIR\javaws.exe" -updateVersions'
EndIf

if "$DOCOMMONINSTALL==0"
# Create Shortcut in the Start Menu
MKDir "$USMPROGRAMS\Java Web Start"
CreateShortCut "$USMPROGRAMS\Java Web Start\Java Web Start.lnk" "$INSTDIR\javaws.exe" "" "$INSTDIR\JavaCup.ico" 0
if "$NODESKTOPICON==0"
# Create Shortcut on the Desktop
CreateShortCut "$UDESKTOP\Java Web Start.lnk" "$INSTDIR\javaws.exe" "" "$INSTDIR\JavaCup.ico" 0
EndIf
EndIf

if "$DOCOMMONINSTALL==1"
# Remove existing user shortcuts
Delete "$USMPROGRAMS\Java Web Start\Java Web Start.lnk"
RMDir "$USMPROGRAMS\Java Web Start"
# Create Shortcut in the Start Menu
MKDir "$CSMPROGRAMS\Java Web Start"
CreateShortCut "$CSMPROGRAMS\Java Web Start\Java Web Start.lnk" "$INSTDIR\javaws.exe" "" "$INSTDIR\JavaCup.ico" 0
if "$NODESKTOPICON==0"
# Remove existing user shortcuts
Delete "$UDESKTOP\Java Web Start.lnk"
# Create Shortcut on the Desktop
CreateShortCut "$CDESKTOP\Java Web Start.lnk" "$INSTDIR\javaws.exe" "" "$INSTDIR\JavaCup.ico" 0
EndIf
EndIf

# Check to see if we need to reboot
ExecWait '"$INSTDIR\helper.exe" 3 "$INSTDIR"'
If "$?==1"
RebootBox
EndIf

# Check to see if Netscape is running
ExecWait '"$INSTDIR\helper.exe" 5 "$INSTDIR"'
If "$?==1"
MessageBox MB_OK "$STRING_NETSCAPE_TXT"
EndIf

# Ask the user if they want to view the README
if "$AUTOINSTALL==0"
MessageBox MB_YESNO "$STRING_DONE_TXT"
If "$?!=0"
Browser "$INSTDIR\$STRING_README_TXT"
EndIf
EndIf

###################
# Uninstall section
###################

Section Uninstall
StringTable strings.txt 0x0409
ExecWait '"$INSTDIR\javaws.exe" -uninstall'
DeleteRegKey HKEY_CLASSES_ROOT  ".jnlp"
DeleteRegKey HKEY_CLASSES_ROOT  "JNLPFile\Shell\Open\Command"
DeleteRegKey HKEY_CLASSES_ROOT  "JNLPFile\Shell\Open"
DeleteRegKey HKEY_CLASSES_ROOT  "JNLPFile\Shell"
DeleteRegKey HKEY_CLASSES_ROOT  "JNLPFile"
DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\JavaSoft\Java Web Start\1.0.1"
DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\JavaSoft\Java Web Start\1.0.1_02"
DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\JavaSoft\Java Web Start\1.0.1_03"
DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\JavaSoft\Java Web Start\1.2"
DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\JavaSoft\Java Web Start\1.4.2"
DeleteRegKey HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\App Paths\javaws.exe"
DeleteRegKey HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\Java Web Start"
DeleteRegKey HKEY_CLASSES_ROOT  "MIME\Database\Content Type\application/x-java-jnlp-file"
DeleteRegValue HKEY_CURRENT_USER "Software\Netscape\Netscape Navigator\User Trusted External Applications" "$INSTDIR\javaws.exe"
# Next line Deletes \\HKEY_CURRENT_USER\\Software\Netscape\Netscape Navigator\Viewers keys
ExecWait '"$INSTDIR\helper.exe" 7'
Delete $INSTDIR\javaws.exe
Delete $INSTDIR\javaws.cfg
Delete $INSTDIR\javaws.jar
Delete $INSTDIR\javaws.policy
Delete $INSTDIR\javawspl.dll
UnRegDLL $INSTDIR\javawebstart.dll
Delete $INSTDIR\javawebstart.dll
Delete $INSTDIR\JavaCup.ico
Delete $INSTDIR\cacerts
Delete $INSTDIR\Readme.html
Delete $INSTDIR\javalogo52x88.gif
Delete $INSTDIR\sunlogo64x30.gif
Delete $INSTDIR\uninst-javaws.exe
Delete $INSTDIR\helper.exe
Delete $INSTDIR\setup.log
Delete "$UDESKTOP\Java Web Start.lnk"
Delete "$USMPROGRAMS\Java Web Start\Java Web Start.lnk"
RMDir "$USMPROGRAMS\Java Web Start"
Delete "$CDESKTOP\Java Web Start.lnk"
Delete "$CSMPROGRAMS\Java Web Start\Java Web Start.lnk"
RMDir "$CSMPROGRAMS\Java Web Start"
RMDirRecursive $USERPROFILE\.javaws
RMDirRecursive $INSTDIR\.javaws
Delete $INSTDIR\resources\*.*
RMDir $INSTDIR\resources
RMDir $INSTDIR
MessageBox MB_OK "$STRING_UNDONE_TXT"
