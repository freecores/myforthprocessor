REM set jre\bin and jre\bin\client (or server) in PATH
REM WINDBG_HOME must also be in PATH

%JAVA_HOME%\bin\java -showversion -Dsun.jvm.hotspot.debugger.useWindbgDebugger -Dsun.jvm.hotspot.debugger.windbg.imagePath=%PATH% -Djava.library.path=..\..\..\os\win32\agent\windbg -cp classes;lib\maf-1_0.jar;lib\jlfgr-1_0.jar sun.jvm.hotspot.HSDB %1 %2
