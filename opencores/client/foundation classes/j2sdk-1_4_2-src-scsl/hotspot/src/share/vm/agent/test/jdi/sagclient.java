
import com.sun.jdi.*;
import com.sun.jdi.connect.*;

import java.util.Map;
import java.util.List;
import java.util.Iterator;
import java.io.IOException;

public class sagclient {
    static AttachingConnector myConn;
    static VirtualMachine vm;
    static VirtualMachineManager vmmgr;
    
    public static void println(String msg) {
        System.out.println("jj: " + msg);
    }


    public static void main(String args[]) {
        vmmgr = Bootstrap.virtualMachineManager();
        List attachingConnectors = vmmgr.attachingConnectors();
        if (attachingConnectors.isEmpty()) {
            System.err.println( "ERROR: No attaching connectors");
            return;
        }
        Iterator myIt = attachingConnectors.iterator();
        while (myIt.hasNext()) {
            myConn = (AttachingConnector)myIt.next();
            if (myConn.name().equals(
                "sun.jvm.hotspot.jdi.SACoreAttachingConnector")) {
                break;
            }
        }
        String execPath = null;
        String pidText = null;
        String coreFilename = null;
        int pid = 0;
        switch (args.length) {
        case (0):
            break;
        case (1):
            // If all numbers, it is a PID to attach to
            // Else, it is a pathname to a .../bin/java for a core file.
            try {
                pidText = args[0];
                pid = Integer.parseInt(pidText);
            } catch (NumberFormatException e) {
                execPath = pidText;
                coreFilename = "core";
                pidText = null;
            }
            break;
            
        case (2):
            execPath = args[0];
            coreFilename = args[1];
            break;
        }
        System.out.println( "pid: " + pid);
        System.out.println( "jdk: " + execPath);
        System.out.println( "core: " + coreFilename);
        
        if (pidText != null) {
            System.out.println("Cannot attach to process " + pid + ".  Not yet implemented.");
            return;
        }

        Map connArgs = myConn.defaultArguments();
        System.out.println("connArgs = " + connArgs);
        
        Connector.StringArgument connArg = (Connector.StringArgument)connArgs.get("core");
        connArg.setValue(coreFilename);

        connArg =  (Connector.StringArgument)connArgs.get("javaExecutable");
        connArg.setValue(execPath);
        try {
            vm = myConn.attach(connArgs);
        } catch (IOException ee) {
            System.err.println("ERROR: myConn.attach got IO Exception:" + ee);
            vm = null;
        } catch (IllegalConnectorArgumentsException ee) {
            System.err.println("ERROR: myConn.attach got illegal args exception:" + ee);
            vm = null;
        }
        if (vm != null) {
            System.out.println("sagclient: attached ok!");
            sagdoit mine = new sagdoit(vm);
            mine.doAll();
            vm.exit(0);
        }
    }
}
