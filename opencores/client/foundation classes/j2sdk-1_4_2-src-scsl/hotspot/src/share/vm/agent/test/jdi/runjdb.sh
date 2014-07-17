#!/bin/sh

#  jdb is a .c file that seems to discard the setting of CLASSPATH.
# So, we have to run jdb by calling java directly :-(

# License file for development version of dbx
LM_LICENSE_FILE=7588@extend.eng:/usr/dist/local/config/sparcworks/license.dat:7588@setlicense
export LM_LICENSE_FILE

doUsage()
{
   cat <<EOF
Usage:  runjdb.sh corefile -jdk jdk-pathname -sa sa-pathname
    sa-pathname is the path of a JDI-SA build dir.
EOF
}

jdk=
javaArgs=
args=
sa=
while [ $# != 0 ] ; do
    case $1 in
      -vv)
        set -x
        ;;
     -jdk)
        jdk=$2
        shift
        ;;
     -sa)
        sa=$2
        shift
        ;;
     -help | help)
        doUsage
        exit
        ;;
     -*)
        javaArgs="$javaArgs $1"
        ;;
     *)
        if [ ! -z "$args" ] ; then
            echo "Error: Only one core file or pid can be specified"
            exit 1
        fi
        echo "$1" | grep -s '^[0-9]*$' > /dev/null
        if [ $? = 0 ] ; then
            # it is a pid
            args="$args $1"
            echo "Error: A pid is not yet allowed"
            exit 1
        else
            # It is a core.        
            # We have to pass the name of the program that produced the
            # core, and the core file itself.
            args="$1"
        fi
        ;;
   esac
   shift
done

if [ -z "$jdk" ] ; then
    echo "Error:  -jdk jdk-pathname is required"
    exit 1
fi
if [ -z "$sa" ] ; then
    echo "Error:  -sa sa-pathname is required"
    exit 1
fi

if [ -z "$args" ] ; then
    echo "Error:  a core file or pid must be specified"
    exit 1
fi

set -x
$jdk/bin/jdb -J-Xbootclasspath/a:$sa  -connect \
  sun.jvm.hotspot.jdi.SACoreAttachingConnector:core=$args,javaExecutable=$jdk/bin/java


#$jdk/bin/java -Xbootclasspath/a:$mmm/ws/merlin-sa/build/agent \
#  com.sun.tools.example.debug.tty.TTY -connect \
#  sun.jvm.hotspot.jdi.SACoreAttachingConnector:core=sagcore,javaExecutable=$jdk/bin/java
