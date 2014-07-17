/**
 *  @test @(#)sagtest.java	1.2 02/03/03
 *  @bug 0000000
 *  @summary This is just an exercise of various JDI elements for use in 
 *           testing the SA/JDI client
 *
 *  @author jjh
 *
 *  @run build TestScaffold VMConnection TargetListener TargetAdapter sagdoit
 *  @run compile -g sagtarg.java
 *  @run main sagtest
 */
import com.sun.jdi.*;
import com.sun.jdi.event.*;
import com.sun.jdi.request.*;

import java.util.*;

    /********** target program **********/

// The target program is sagtarg.java

    /********** test program **********/

public class sagtest extends TestScaffold {
    ReferenceType targetClass;
    ThreadReference mainThread;

    sagtest (String args[]) {
        super(args);
    }

    public static void main(String[] args)	throws Exception {
        new sagtest(args).startTests();
    }

    /********** event handlers **********/

    
    /********** test core **********/

    protected void runTests() throws Exception {
        /*
         * Get to the top of main() 
         * to determine targetClass and mainThread
         */
        BreakpointEvent bpe = startToMain("sagtarg");
        targetClass = bpe.location().declaringType();
        mainThread = bpe.thread();
        EventRequestManager erm = vm().eventRequestManager();
        
        sagdoit mine = new sagdoit(vm());        
        mine.doAll();

        if (!testFailed) {
            println("sagtest: passed");
        } else {
            throw new Exception("sagtest: failed");
        }
    }
}
