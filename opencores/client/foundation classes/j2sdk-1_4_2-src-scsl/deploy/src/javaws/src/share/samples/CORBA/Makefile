#
# @(#)Makefile	1.3 03/01/23
# 
# Copyright (c) 2003 Sun Microsystems, Inc. All Rights Reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 
# -Redistribution of source code must retain the above copyright notice, this
#  list of conditions and the following disclaimer.
# 
# -Redistribution in binary form must reproduce the above copyright notice, 
#  this list of conditions and the following disclaimer in the documentation
#  and/or other materials provided with the distribution.
# 
# Neither the name of Sun Microsystems, Inc. or the names of contributors may 
# be used to endorse or promote products derived from this software without 
# specific prior written permission.
# 
# This software is provided "AS IS," without a warranty of any kind. ALL 
# EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND WARRANTIES, INCLUDING
# ANY IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
# OR NON-INFRINGEMENT, ARE HEREBY EXCLUDED. SUN MIDROSYSTEMS, INC. ("SUN")
# AND ITS LICENSORS SHALL NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY LICENSEE
# AS A RESULT OF USING, MODIFYING OR DISTRIBUTING THIS SOFTWARE OR ITS
# DERIVATIVES. IN NO EVENT WILL SUN OR ITS LICENSORS BE LIABLE FOR ANY LOST 
# REVENUE, PROFIT OR DATA, OR FOR DIRECT, INDIRECT, SPECIAL, CONSEQUENTIAL, 
# INCIDENTAL OR PUNITIVE DAMAGES, HOWEVER CAUSED AND REGARDLESS OF THE THEORY 
# OF LIABILITY, ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, 
# EVEN IF SUN HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
# 
# You acknowledge that this software is not designed, licensed or intended
# for use in the design, construction, operation or maintenance of any
# nuclear facility.
#

JAVA_HOME=/home/raghuv/j2sdk1_3/jdk1.3.1

## NOT TO BE CHANGED

JAVA=$(JAVA_HOME)/bin/java
JAVAC=$(JAVA_HOME)/bin/javac
JDB=$(JAVA_HOME)/bin/jdb

CLASSPATH=.:./class:$(JAVA_HOME)
JAVACFLAGS=-d . -classpath $(CLASSPATH)

.SUFFIXES: .class .java .java~

.java.class:
	$(JAVAC) $(JAVACFLAGS) $<

EXE_FLAGS = \
        -ORBInitialHost $(ORB_INITIAL_HOST) \
        -ORBInitialPort $(ORB_INITIAL_PORT)

client = HelloClient.class
server = HelloServer.class

LD_LIBRARY_PATH=$(JAVA_HOME)/lib/
IOSER=$(LD_LIBRARY_PATH)/libioser.so
JNDI_PROPS=-Djava.naming.factory.initial=com.sun.jndi.cosnaming.CNCtxFactory -Djava.naming.provider.url=iiop://${ORB_INITIAL_HOST}:${ORB_INITIAL_PORT} 

ORB_INITIAL_PORT=10000
ORB_ACTIVATION_PORT=9999
ORB_INITIAL_HOST=129.148.70.86


ORBD=${JAVA_HOME}/bin/orbd -ORBInitialPort ${ORB_INITIAL_PORT} -port ${ORB_ACTIVATION_PORT} -serverPollingTime 200 -serverStartupDelay 1000


ORB_PROPS=-Dorg.omg.CORBA.ORBInitialHost=${ORB_INITIAL_HOST} -Dorg.omg.CORBA.ORBInitialPort=${ORB_INITIAL_PORT}

JAVAFLAGS=$(ORB_PROPS) -classpath $(CLASSPATH) 

all : clean build run

stubs:  HelloInterface.java
	$(JAVA_HOME)/bin/rmic -iiop -keepgenerated HelloImpl

build: stubs $(client) $(server)

run: runns runserver runclient

clean:
	rm *.class

#
# Targets to run the client.
#
runclient:
	$(JAVA) $(JNDI_PROPS) $(JAVAFLAGS) HelloClient 

runserver:
	$(JAVA) $(JAVAFLAGS) $(JNDI_PROPS) HelloServer &

runorbd:
	$(ORBD)	


