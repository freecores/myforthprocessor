#!/bin/ksh

# This script runs the test program, sagtest.java, with the regular
# JPDA jdi.
# It then starts up the debuggee part of the test, sagtarg.java,
# and calls gcore to create file sagcore for use in running
# the SA JDI client.

set -x
# jdk is a jdk with the vm from the sa workspace
if [ "$1" = -jdk ] ; then
    jdk=$2
    shift 2
else
    jdk=/java/re/jdk/1.4.0/latest/binaries/solsparc
fi

# First, run the sagtest.java with the regular JPDA jdi
workdir=./workdir
mkdir -p $workdir
CLASSPATH=$jdk/classes:$jdk/lib/tools.jar:$workdir
export CLASSPATH

$jdk/bin/javac -g  -classpath $jdk/classes:$jdk/lib/tools.jar:$workdir -J-Xms40m -d $workdir \
    TestScaffold.java \
    VMConnection.java \
    TargetListener.java \
    TargetAdapter.java \
    sagdoit.java \
    sagtarg.java \
    sagtest.java

if [ $? != 0 ] ; then
    exit 1
fi

$jdk/bin/java -Dtest.classes=$workdir sagtest

# Now run create a core file for use in running sa-jdi

if [ ! core.satest -nt sagtarg.class ] ; then
    tmp=/tmp/sagsetup
    rm -f $tmp
    $jdk/bin/java sagtarg > $tmp &
    pid=$!
    while [ ! -s $tmp ] ; do
        # Kludge alert!
        sleep 2
    done
    #rm -f $tmp
    gcore  $* $pid
    mv core.$pid sagcore
    kill -9 $pid
fi

