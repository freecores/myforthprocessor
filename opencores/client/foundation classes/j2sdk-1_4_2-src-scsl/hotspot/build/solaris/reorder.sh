#!/bin/sh -x
# 
# @(#)reorder.sh	1.9 03/01/23 10:50:54
# 
# Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
#

#  Generate the reorder data for hotspot.
#
#  Usage:
#
#	sh  reorder.sh  <test_sdk_workspace>  <test_sdk>  <jbb_dir>
#
#	<test_sdk_workspace> is a *built* SDK workspace which contains the
#	reordering tools for the SDK.  This script relies on lib_mcount.so
#	from this workspace.
#
#	**Note:  since the reordering tools are not yet integrated, it
#	is assumed that you will temporarily use the workspace
#	/net/kaiser.east/files3/ladybird for this purpose.
#
#	<test_sdk> is a working SDK which you can use to run the profiled
#	JVMs in to collect data.  You must be able to write to this SDK.
#
#	<jbb_dir> is a directory containing JBB test jar files and properties
#	which will be used to run the JBB test to provide reordering data
#	for the server VM.
#
#	Profiled builds of the VM are needed (before running this script),
#	build with PROFILE_PRODUCT=1:
#
#		gnumake profiled1 profiled PROFILE_PRODUCT=1
#
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

test_setup() {

  #   $1 = "client"  or  "server"
  #   $2 = name of reorder file to be generated.

  echo ""
  echo "Testing  $1  $2"
  echo ""
  libreldir=${ALT_OUTPUTDIR:-../../../build/solaris-$sdk_arch}/reorder
  libabsdir=${ALT_OUTPUTDIR:-$sdk_ws/build/solaris-$sdk_arch}/reorder
  ( cd $sdk_ws/make/tools/reorder ; gnumake $libreldir/$arch/libmcount.so )
  if [ "${arch}" = "i386" ] ; then
	# On Solaris/x86 we need to remove the symbol _mcount from the command
	( cd $sdk_ws/make/tools/reorder ; \
	    gnumake $libreldir/$arch/remove_mcount )
	echo Remove _mcount from java command.
	$libabsdir/$arch/remove_mcount $jre/bin/java
  fi
  ( cd $sdk_ws/make/tools/reorder ; gnumake tool_classes )
  ( cd $sdk_ws/make/tools/reorder ; gnumake test_classes )

  tests="Null Exit Hello Sleep IntToString \
	 LoadToolkit LoadFrame LoadJFrame JHello"
  swingset=$sdk/demo/jfc/SwingSet2/SwingSet2.jar
  java=$jre/bin/java
  if [ "X$LP64" != "X" ] ; then
    testjava="$jre/bin/sparcv9/java -$1"
  else
    testjava="$jre/bin/java -$1"
  fi
  mcount=$libabsdir/$arch/libmcount.so

  if [ ! -x $mcount ] ; then
    echo $mcount is missing!
    exit 1
  fi

  testpath=$libabsdir/classes

  reorder_file=$2
  
  rm -f ${reorder_file}
  rm -f ${reorder_file}_tmp2
  rm -f ${reorder_file}_tmp1

  echo "data = R0x2000;"				> ${reorder_file}
  echo "text = LOAD ?RXO;"				>> ${reorder_file}
  echo ""						>>  ${reorder_file}
  echo "text: .text%__1cTJVM_FunctionAtStart6F_pC_;"	>> ${reorder_file}
  echo ""						>>  ${reorder_file}
}

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

test_client() {

  # Run each of a set of tests, extract the methods called,
  # append the new functions to the reorder list.
  #   $1 = "client"  or  "server"
  #   $2 = name of reorder file to be generated.

  test_setup $1 $2

  for f in $tests ; do
    echo Running test $f.
    rm -f ${reorder_file}_tmp1
    echo "# Test $f" >> ${reorder_file}

    echo "Using LD_PRELOAD=$mcount"
    echo $testjava -classpath $testpath $f

    LD_PRELOAD=$mcount \
	    $testjava -classpath $testpath $f 2> ${reorder_file}_tmp1

    echo "Done."
    sed -n -e '/^text:/p' ${reorder_file}_tmp1 > ${reorder_file}_tmp2
    sed -e '/^text:/d' ${reorder_file}_tmp1
    LD_LIBRARY_PATH=$lib/server \
    $java -server -classpath $testpath Combine ${reorder_file} ${reorder_file}_tmp2 \
        > ${reorder_file}_tmp3
    mv ${reorder_file}_tmp3 ${reorder_file}
    rm -f ${reorder_file}_tmp2
    rm -f ${reorder_file}_tmp1
  done

  # Run SwingSet, extract the methods called,
  # append the new functions to the reorder list.

  echo "# SwingSet" >> ${reorder_file}

  echo ""
  echo ""
  echo "When SwingSet has finished drawing, " \
       "you may terminate it (with your mouse)."
  echo "Otherwise, it should be automatically terminated in 3 minutes."
  echo ""
  echo ""

  LD_PRELOAD=$mcount \
	  $testjava -classpath $testpath MaxTime \
		$swingset 180 2> ${reorder_file}_tmp1 

  sed -n -e '/^text:/p' ${reorder_file}_tmp1 > ${reorder_file}_tmp2

  LD_LIBRARY_PATH=$lib/server \
  $java -server -classpath $testpath Combine ${reorder_file} ${reorder_file}_tmp2  \
      > ${reorder_file}_tmp3
  mv ${reorder_file}_tmp3 ${reorder_file}
  rm -f ${reorder_file}_tmp2
  rm -f ${reorder_file}_tmp1
}

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

test_server() {

  # Run the JBB script, collecting data on the way.
  #   $1 = "client"  or  "server"
  #   $2 = name of reorder file to be generated.

  test_setup $1 $2

  echo Running JBB.
  rm -f ${reorder_file}_tmp1
  rm -f ${reorder_file}_tmp2
  heap=200m

  CLASSPATH=jbb.jar:jbb_no_precompile.jar:check.jar:reporter.jar

    ( cd $jbb_dir; LD_PRELOAD=$mcount MCOUNT_ORDER_BY_COUNT=1 \
        $testjava -classpath $CLASSPATH -Xms${heap} -Xmx${heap} \
	spec.jbb.JBBmain -propfile SPECjbb.props ) 2> ${reorder_file}_tmp1

  sed -n -e '/^text:/p' ${reorder_file}_tmp1 > ${reorder_file}_tmp2
  sed -e '/^text:/d' ${reorder_file}_tmp1
  cat ${reorder_file}_tmp2		>> ${reorder_file}
  rm -f ${reorder_file}_tmp2
  rm -f ${reorder_file}_tmp1
}

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# Rename the old VMs, copy the new in, run the test, and put the
# old one back.

copy_and_test() {

  #   $1 = "client"  or  "server"
  #   $2 = name of reorder file to be generated.
  #   $3 = profiled mv to copy in

  if [ -d $lib/$1.save ] ; then
    echo "$lib/$1.save directory already exists?  Please remove or rename."
    exit 1
  fi

  mv $lib/$1 $lib/$1.save
  mkdir $lib/$1
  cp $3 $lib/$1
  test_$1 $1 $2
  rm -r $lib/$1
  mv $lib/$1.save $lib/$1
}

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


# Check arguments:

if [ $# != 3 ] ; then
  echo ""
  echo "Usage:"
  echo "   sh  reorder.sh  <test_sdk_workspace>  <test_sdk>  <jbb_dir>"
  echo ""
  exit 1
fi

sdk_ws=$1
if [ ! -r $sdk_ws/make/tools/reorder/Makefile ] ; then
  echo ""
  echo "test workspace "$sdk_ws" does not contain the reordering tools."
  echo ""
  exit 1
fi

sdk=$2
jre=$sdk/jre
arch=`uname -p`
sdk_arch=$arch

if [ $arch = sparc -a "X$LP64" != "X" ] ; then
  arch=sparcv9
fi

if [ $arch = "i386" ] ; then
  hotspot_arch=i486
else
  hotspot_arch=$arch
fi

lib=$jre/lib/$arch
if [ ! -r $jre/lib/rt.jar ] ; then
  echo ""
  echo "test SDK "$sdk" is not a suitable SDK."
  echo ""
  exit 1
fi

jbb_dir=$3
if [ ! -r $jbb_dir/jbb.jar ] ; then
  echo ""
  echo "jbb.jar not present in $jbb_dir"
  echo ""
  exit 1
fi


# Were profiled VMs built?

if [    ! -r solaris_${hotspot_arch}_compiler1/profiled/libjvm.so  \
     -o ! -r solaris_${hotspot_arch}_compiler2/profiled/libjvm.so ] ; then
  echo ""
  echo "Profiled builds of compiler1 and compiler2 are needed first."
  exit 1
fi


# Compiler1 - not supported in 64-bit (b69 java launcher rejects it).

if [ "X$LP64" = "X" ] ; then
  #gnumake profiled1
  echo Copying profiled client VM.
  echo
  copy_and_test client \
                reorder_COMPILER1_$hotspot_arch \
                solaris_${hotspot_arch}_compiler1/profiled/libjvm.so
fi

#gnumake profiled
echo Copying profiled server VM.
echo
copy_and_test server \
              reorder_COMPILER2_$hotspot_arch \
              solaris_${hotspot_arch}_compiler2/profiled/libjvm.so
