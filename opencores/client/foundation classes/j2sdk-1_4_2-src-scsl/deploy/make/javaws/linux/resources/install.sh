#!/bin/bash

#
# @(#)install.sh	1.29 03/01/23
# 
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

# Script to install Java(tm) Web Start and related files.
# Run in the directory you wish to have Java Web Start installed.
# Besides extracting the files, this will also update
# ~/.mailcap and ~/.mime.types accordingly.

PATH=/usr/bin:/usr/ucb/bin:/bin

# Functions

# Untar the attachment
extract() {
    echo 'Extracting...'
    outname=${INSTALLDIR}/temp.tar.Z
    tail +358 $0 > $outname
    (cd $INSTALLDIR; tar xf temp.tar.Z)
    rm -f ${outname}
}

# Creates the CFG for javasoft
createCFGForJS() {
    echo 'Creating javaws.cfg...'
    echo "javaws.cfg.jre.default = 0" > ${INSTALLDIR}/javaws.cfg

    echo "javaws.cfg.jre.0.platform=1.2" >> ${INSTALLDIR}/javaws.cfg
    echo "javaws.cfg.jre.0.product=1.2.2" >> ${INSTALLDIR}/javaws.cfg
    echo "javaws.cfg.jre.0.path=/usr/local/java/jdk1.2.2/linux/bin/java" >> ${INSTALLDIR}/javaws.cfg
}

# Prompt for JDK and create javaws.cfg based on user input
createCFGFromUser() {
    more <<"EOF"
In order to use Java(tm) Web Start you must have a version of Java 2 standard edition
installed. If you do not, you can download one from http://java.sun.com.
EOF

    # Prompt for a default JDK
    javaPath=
    while [ x${javaPath} = x ]; do
	echo "Please enter the path of your installation" \
	     "(eg /usr/local/java/jdk1.2):"
	read path leftover
	if [ ! -d $path ]; then
	  echo 'Path is not valid.'
	elif [ ! -f ${path}/bin/java ]; then
	  echo 'Path does not contain java, please verify it is correct.'
	else
	  # determine version (notice the redirection here, for whatever reason
	  # we send version info to standard error)
	  echo "Obtaining version..."
	  javaPath=${path}
	  version=`${javaPath}/bin/java -version 2>&1 | head -1`
	  version=${version#*\"}
	  version=${version%%\"*}
	  echo "You appear to be running ${version}"
	  if [ -z "${version}" ]; then
	    echo 'Unable to determine version, please enter it now (eg 1.2):'
	    read version leftover
	  fi
	fi
    done

    # create javaws.cfg based on version info and javaPath. These are not completey correct.
    # The javaws -updateversion will fix them later.
    echo 'Creating javaws.cfg...'
    echo "javaws.cfg.jre.0.platform=${version}/bin/java" > ${INSTALLDIR}/javaws.cfg
    echo "javaws.cfg.jre.0.product=${version}/bin/java" >> ${INSTALLDIR}/javaws.cfg
    echo "javaws.cfg.jre.0.path=${javaPath}/bin/java" >> ${INSTALLDIR}/javaws.cfg
    echo "javaws.cfg.jre.default = 0" >> ${INSTALLDIR}/javaws.cfg
}

# Update .mailcap
updateMailcap() {
    echo 'Updating ~/.mailcap...'
    if [ -f ${HOME}/.mailcap ]; then
      jGrep=`grep -n  ${MIME_TYPE} ${HOME}/.mailcap`      
      if [ -n "$jGrep" ] ; then
        # Remove existing entry  - so we are sure the path is right
        grep -v ${MIME_TYPE} ${HOME}/.mailcap > ${INSTALLDIR}/.mailcap1
        grep -v "# Java Web Start" ${INSTALLDIR}/.mailcap1 > ${INSTALLDIR}/.mailcap2
        rm ${INSTALLDIR}/.mailcap1
        mv ${INSTALLDIR}/.mailcap2 ${HOME}/.mailcap
      fi
      # Doesn't contain application/x-java-jnlp-file file, add it.
      echo "# Java Web Start" >> ${HOME}/.mailcap
      echo "${MIME_TYPE}; $INSTALLDIR/javaws %s" >> ${HOME}/.mailcap
    else
      # .mailcap doesn't exist, create it.
      echo "# Java Web Start" > ${HOME}/.mailcap
      echo "${MIME_TYPE}; $INSTALLDIR/javaws %s" >> ${HOME}/.mailcap
    fi
}

# Update .mime.types
updateMimeTypes() {
    echo 'Updating ~/.mime.types...'
    if [ -f ${HOME}/.mime.types ]; then
      # the file exists, make sure it contains jnlp
      jGrep=`grep ${MIME_TYPE} ${HOME}/.mime.types`
      if [ -z "${jGrep}" ]; then
	# doesn't contain Javaws, add it
	echo "type=${MIME_TYPE} desc=\"Java Web Start\" exts=\"jnlp\"" >> \
	     ${HOME}/.mime.types
      fi
    else
      # Doesn't exist so create it, even though if it doesn't exist it
      # implies you are not using Communicator, but we'll create it on the
      # chance that you do install Communicator.
      echo '#--Netscape Communications Corporation MIME Information' > \
	   ${HOME}/.mime.types
      echo '#Do not delete the above line. It is used to identify the file type.' \
	   >> ${HOME}/.mime.types
      echo '#mime types added by Netscape Helper' >> ${HOME}/.mime.types
      echo 'type=application/x-java-jnlp-file desc="Java Web Start" exts="jnlp"' >> \
	   ${HOME}/.mime.types
    fi
}

# Software licensing
showLicensing() {
more <<"EOF"

                                  Pre-Release 
                          Software Evaluation Agreement

SUN MICROSYSTEMS, INC. ("SUN") IS WILLING TO LICENSE THE JAVA (TM) WEB START,
VERSION 1.2 PRE-RELEASE SOFTWARE TO LICENSEE  ONLY UPON THE CONDITION THAT 
LICENSEE ACCEPTS ALL OF THE TERMS CONTAINED IN THIS LICENSE AGREEMENT 
("AGREEMENT").  PLEASE READ THE TERMS AND CONDITIONS OF THIS AGREEMENT 
CAREFULLY.  BY INSTALLING  THIS SOFTWARE, LICENSEE ACCEPTS THE TERMS AND 
CONDITIONS OF THIS LICENSE AGREEMENT, INDICATE ACCEPTANCE BY SELECTING THE 
"ACCEPT" BUTTON AT THE BOTTOM OF THIS AGREEMENT. IF LICENSEE IS NOT WILLING TO 
BE BOUND  BY ALL THE TERMS, SELECT THE "DECLINE" BUTTON AT THE BOTTOM OF THE 
AGREEMENT AND THE INSTALLATION PROCESS WILL NOT CONTINUE. 

1.0 DEFINITIONS 
"Licensed Software" means the Java Web Start, Version 1.2 Pre-Release Software
in binary and source code forms, and any user manuals, programming guides and 
other documentation provided to Licensee by Sun Microsystems, Inc. under this 
Agreement.

2.0 LIMITED LICENSE 
Sun Microsystems, Inc. grants to Licensee, a non-exclusive, non-transferable, 
royalty-free and limited license to view the source code portions of the 
Licensed Software and use the binary code portions of the Licensed Software 
internally for the purposes of evaluation only. No license is granted to 
Licensee for any other purpose. Licensee may not sell, rent, loan or otherwise
encumber or transfer Licensed Software in whole or in part, to any third party.

3.0 LICENSE RESTRICTIONS 
3.1 Licensee may not duplicate Licensed Software, except for one (1) copy of 
Licensed Software and only for archival purposes.  Licensee agrees to 
reproduce all copyright and any other proprietary right notices on any such 
copy. 
3.2 Except as otherwise provided by law, Licensee may not modify or create 
derivative works of the Licensed Software, reverse engineer, disassemble or 
decompile binary portions of the Licensed Software, or otherwise attempt to 
derive the source code from such portions. 
3.3 No right, title, or interest in or to Licensed Software, any trademarks, 
service marks, or trade names of Sun or Sun's licensors is granted under this 
Agreement. 
3.4 Licensed Software may only be used at the location identified above. 
3.5 Licensee shall have no right to use the Licensed Software for productive 
or commercial use. 

4.0 NO SUPPORT 
Sun is under no obligation to support Licensed Software or to provide Licensee
with updates or error corrections (collectively "Software Updates"). If Sun, 
at its sole option, supplies Software Updates to Licensee, the Software
Updates will be considered part of Licensed Software, and subject to the 
terms of this Agreement. 

5.0 LICENSEE DUTIES 
Licensee agrees to evaluate and test the Licensed Software for use with 
Licensee's products and to provide feedback to Sun's email alias: 
javawebstart-feedback@sun.com.  Sun shall treat any oral or written feedback 
or results of Licensee's testing of the Licensed Software which Licensee 
provides to Sun as Sun's Confidential Information (defined in Section 7
below). 

6.0 TERM AND TERMINATION OF AGREEMENT 
6.1 This Agreement will commence on the date on which Licensee receives 
Licensed Software (the "Effective Date") and will expire sixty (60) days from 
the Effective Date, unless terminated earlier as provided herein. 
6.2 Either party may terminate this Agreement upon ten (10) days' written 
notice to the other party. However, Sun may terminate this Agreement 
immediately should any Licensed Software become, or in Sun's opinion be 
likely to become, the subject of a claim of infringement of a patent, trade 
secret or copyright. 
6.3 Sun may terminate this Agreement immediately should Licensee materially 
breach any of its provisions or take any action in derogation of Sun's rights 
to the Confidential Information licensed to Licensee. 
6.4 Upon termination or expiration of this Agreement, Licensee will 
immediately cease use of and destroy Licensed Software, any copies thereof 
and provide to Sun  a written statement certifying that Licensee has complied
 with the foregoing obligations. 
6.5 Rights and obligations under this Agreement which by their nature should 
survive, will remain in effect after termination or expiration hereof. 

7.0 CONFIDENTIAL INFORMATION 
7.1 For purposes of this Agreement, "Confidential Information" means:  (i) 
business and technical information and any source code or binary code, which 
Sun discloses to Licensee related to Licensed Software; (ii) Licensee's 
feedback based on Licensed Software; and (iii) the terms, conditions, and 
existence of this Agreement.  Licensee may not disclose or use Confidential 
Information, except for the purposes specified in this Agreement.  Licensee 
will protect the Confidential Information with the same degree of care, but 
not less than a reasonable degree of care, as Licensee uses to protect its 
own Confidential Information. Licensee's obligations regarding Confidential 
Information will expire no less than five (5) years from the date of receipt 
of the Confidential Information, except for Sun source code which will be 
protected in perpetuity.  Licensee agrees that Licensed Software contains Sun 
trade secrets. 
7.2 Notwithstanding any provisions contained in this Agreement concerning 
nondisclosure and non-use of the Confidential Information, the nondisclosure 
obligations of Section 7.1 will not apply to any portion of Confidential 
Information that Licensee can demonstrate in writing is:  (i) now, or 
hereafter through no act or failure to act on the part of Licensee becomes, 
generally known to the general public; (ii) known to Licensee at the time of 
receiving the Confidential Information without an obligation of 
confidentiality; (iii) hereafter rightfully furnished to Licensee by a third 
party without restriction on disclosure; or (iv) independently developed by 
Licensee without any use of the Confidential Information. 
7.3 Licensee must restrict access to Confidential Information to its 
employees or contractors with a need for this access to perform their 
employment or contractual obligations and who have agreed in writing to be 
bound by a confidentiality obligation, which incorporates the protections and 
restrictions substantially as set forth in this Agreement. 

8.0 DISCLAIMER OF WARRANTY 
8.1 Licensee acknowledges that Licensed Software may contain errors and is 
not designed, licensed, or intended for use in the design, construction, 
operation or maintenance of any nuclear facility ("High Risk Activities"). 
Sun disclaims any express or implied warranty of fitness for such uses.  
Licensee represents and warrants to Sun that it will not use, distribute or 
license the Licensed Software for High Risk Activities. 
8.2 LICENSED SOFTWARE IS PROVIDED "AS IS". ALL EXPRESS OR IMPLIED CONDITIONS, 
REPRESENTATIONS, AND WARRANTIES, INCLUDING ANY IMPLIED WARRANTY OF 
MERCHANTABILITY, SATISFACTORY QUALITY, FITNESS FOR A PARTICULAR PURPOSE, OR 
NON-INFRINGEMENT, ARE DISCLAIMED, EXCEPT TO THE EXTENT THAT SUCH DISCLAIMERS 
ARE HELD TO BE LEGALLY INVALID. 

9.0 LIMITATION OF LIABILITY 
9.1 Licensee acknowledges that the Licensed Software may be experimental and 
that the Licensed Software may have defects or deficiencies, which cannot or 
will not be corrected by Sun.  Licensee will hold Sun harmless from any 
claims based on Licensee's use of the Licensed Software for any purposes 
other than those of internal evaluation, and from any claims that later 
versions or releases of any Licensed Software furnished to Licensee are 
incompatible with the Licensed Software provided to Licensee under this 
Agreement. 
9.2 To the extent not prohibited by law, in no event will Sun be liable for 
any indirect, punitive, special, incidental or consequential damage in 
connection with or arising out of this Agreement (including loss of business, 
revenue, profits, use, data or other economic advantage), however it arises, 
whether for breach or in tort, even if the other party has been previously 
advised of the possibility of such damage. 

10.0 U.S.  GOVERNMENT RESTRICTED RIGHTS 
10.1 If this Software is being acquired by or on behalf of the U.S. Government
or by a U.S. Government prime contractor or subcontractor (at any tier), then 
the Government's rights in the Software and accompanying documentation shall 
be only as set forth in this license; this is in accordance with 48 C.F.R. 
227.7202-4 (for Department of Defense (DOD) acquisitions) and with 48 C.F.R. 
2.101 and 12.212 (for non-DOD acquisitions). 

11.0 GENERAL TERMS 
11.1 Any action relating to or arising out of this Agreement will be governed 
by California law and controlling U.S.  federal law.  The U.N.  Convention 
for the International Sale of Goods and the choice of law rules of any 
jurisdiction will not apply. 
11.2 Licensed Software and technical data delivered under this Agreement are 
subject to U.S.  export control laws and may be subject to export or import 
regulations in other countries.  Licensee agrees to comply strictly with all 
such laws and regulations and acknowledges that it has the responsibility to 
obtain such licenses to export, re-export or import as may be required after 
delivery to Licensee. 
11.3 It is understood and agreed that, notwithstanding any other provision of 
this Agreement, Licensee's breach of the provisions of Section 7 of this 
Agreement will cause Sun irreparable damage for which recovery of money 
damages would be inadequate, and that Sun will therefore be entitled to seek 
timely injunctive relief to protect Sun's rights under this Agreement in 
addition to any and all remedies available at law. 
11.4 Neither party may assign or otherwise transfer any of its rights or 
obligations under this Agreement, without the prior written consent of the 
other party, except that Sun may assign this Agreement to an affiliated 
company. 
11.5 This Agreement is the parties' entire agreement relating to its subject 
matter.  It supersedes all prior or contemporaneous oral or written 
communications, proposals, conditions, representations and warranties and 
prevails over any conflicting or additional terms of any quote, order, 
acknowledgment, or other communication between the parties relating to its 
subject matter during the term of this Agreement.  No modification to this 
Agreement will be binding, unless in writing and signed by an authorized 
representative of each party.

(LFI#113567/Form ID#011801)
EOF
agreed=
while [ x$agreed = x ]; do
    echo
    echo "Do you agree to the above license terms? [yes or no] "
    read reply leftover
    case $reply in
	y* | Y*)
	    agreed=1;;
	n* | N*)
    echo "If you don't agree to the license you can't install this sofware";
    exit 1;;
    esac
done
}

# Updates the version strings in the config file.
updateVersions() {
    echo 'Determining full JRE versions...'
    ${INSTALLDIR}/javaws -updateVersions
}


# Where to install jn.dtet
INSTALLDIR=`pwd`/javaws

# Mime type registered under.
MIME_TYPE=application/x-java-jnlp-file

# Show licensing
showLicensing

# Create the install directory
if [ ! -d ${INSTALLDIR} ]; then
  mkdir ${INSTALLDIR}
fi

if [ ! -d ${INSTALLDIR} ]; then
  echo 'Unable to create javaws directory'
  exit 0
fi

if [ -f /usr/local/java/jdk1.2.2/linux/bin/java ]; then
  createCFGForJS
else
  createCFGFromUser
fi
extract
updateMailcap
updateMimeTypes
updateVersions
exit 0
