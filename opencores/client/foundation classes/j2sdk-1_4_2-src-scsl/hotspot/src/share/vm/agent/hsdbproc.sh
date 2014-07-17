#!/bin/sh
$JAVA_HOME/bin/java -showversion -Dsun.jvm.hotspot.debugger.useProcDebugger -Djava.library.path=../../../os/solaris/agent/proc -cp classes:lib/maf-1_0.jar:lib/jlfgr-1_0.jar sun.jvm.hotspot.HSDB

