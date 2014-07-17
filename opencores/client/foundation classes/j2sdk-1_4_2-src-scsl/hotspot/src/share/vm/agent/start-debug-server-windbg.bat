set CLASSPATH=classes

%JAVA_HOME%\bin\java -classpath %CLASSPATH% -Djava.rmi.server.codebase=file:/%CLASSPATH% -Djava.security.policy=grantAll.policy -Dsun.jvm.hotspot.debugger.useWindbgDebugger -Djava.library.path=..\..\..\os\win32\agent\windbg -Dsun.jvm.hotspot.debugger.windbg.imagePath=%PATH% sun.jvm.hotspot.DebugServer %1 %2 %3 %4 %5 %6 %7 %8 %9
