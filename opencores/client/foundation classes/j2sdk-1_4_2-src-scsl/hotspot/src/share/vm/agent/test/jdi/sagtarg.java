// This is the target debuggee for sagtest.java.
// It just waits which lets the test call all the JDI
// methods on it.

interface MyInterface {
    public void myMethod();
}


abstract class MySuper implements MyInterface {
}

class sagtarg extends MySuper {
    public static void main(String[] args){
        System.out.println("Howdy!");
        String myStr = "";
        synchronized(myStr) {
            try {
                myStr.wait();
            } catch (InterruptedException ee) {
            }
        }
        System.out.println("Goodbye from sagtarg!");
    }
    
    public void myMethod() {
    }
}
