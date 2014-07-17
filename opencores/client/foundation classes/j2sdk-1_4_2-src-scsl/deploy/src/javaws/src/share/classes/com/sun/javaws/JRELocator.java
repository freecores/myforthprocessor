/*
 * @(#)JRELocator.java	1.17 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;

import com.sun.javaws.*;
import com.sun.javaws.debug.*;
import java.io.*;

/**
 * Utility class to search for JREs.
 *
 * @version 1.17 01/23/03
 */
public class JRELocator {
    public static final int DEFAULT_TIMEOUT = 15000;

    private static final String PRODUCT_ID = "productVersion=";
    private static final String PLATFORM_ID = "platformVersion=";


    public static ConfigProperties.JREInformation getVersion(File javaPath) {
        return getVersion(javaPath, DEFAULT_TIMEOUT);
    }

    /**
     * Returns the version for the JVM at <code>path</code>.
     * This will return null if there is problem with executing
     * the path. This gives the child process at most <code>msTimeout</code>
     * mili-seconds to execute before assuming it is bogus.
     */
    public static ConfigProperties.JREInformation getVersion(File javaPath,
                                                             int msTimeout) {
        // Execute the process
        String[] result = execute(new String[]
                  { javaPath.getPath(), "-classpath", getClassPath(javaPath),
                    JRELocator.class.getName() }, msTimeout);
        ConfigProperties.JREInformation jre = null;

        if (result != null) {
            jre = extractVersion(javaPath.getPath(), result[0]);

            if (jre == null) {
                jre = extractVersion(javaPath.getPath(), result[1]);
            }

            if (jre != null && jre.getPlatformVersionId().equals("1.2")) {
                // 1.2 platform versions don't contain full version
                // extract from -fullversion
                result = execute(new String[] { javaPath.getPath(),
                                                "-fullversion" }, msTimeout);

                if (result != null) {
                    jre = extractVersionFor12(javaPath.getPath(), result[0]);

                    if (jre == null) {
                        jre = extractVersionFor12(javaPath.getPath(),
                                                  result[1]);
                    }
                }
            }
        }
        if (Globals.TraceJRESearch) {
            Debug.println("\tjre search returning: " + jre);
        }
        return jre;
    }

    private static String[] execute(String[] commands, int msTimeout) {
        Process p = null;
        boolean done = false;

        if (Globals.TraceJRESearch) {
            Debug.println("jre search executing");
            for (int counter = 0; counter < commands.length; counter++) {
                Debug.println(counter + ": " + commands[counter]);
            }
        }
        try {
            p = Runtime.getRuntime().exec(commands);
        } catch (IOException ioe) {
            done = true;
        }
        int exitValue = -1;
        int waitCount = msTimeout / 100;
        while (!done) {
            // Wait a bit.
            try {
                Thread.sleep(100);
            }
            catch (InterruptedException ite) {}

            // Check if done.
            try {
                exitValue = p.exitValue();
                done = true;
                if (Globals.TraceJRESearch) {
                    Debug.println("\tfinished executing " + exitValue);
                }
            }
            catch (IllegalThreadStateException itse) {
                if (--waitCount == 0) {
                    // Give up on it!
                    done = true;
                    if (Globals.TraceJRESearch) {
                        Debug.println("\tfailed " + exitValue);
                    }
                    p.destroy();
                }
            }
        }
        if (done && exitValue == 0) {
            String[] results = new String[2];

            results[0] = readFromStream(p.getErrorStream());
            results[1] = readFromStream(p.getInputStream());
            if (Globals.TraceJRESearch) {
                Debug.println("result: " + results[0]);
                Debug.println("result: " + results[1]);
            }
            return results;
        }
        return null;
    }

    /**
     * Continually executes read on <code>is</code> until -1 is returned,
     * returning the read result.
     */
    private static String readFromStream(InputStream is) {
        StringBuffer sb = new StringBuffer();
        try {
            byte[] buff = new byte[80];
            boolean done = false;

            while (!done) {
                int amount = is.read(buff, 0, 80);
                if (amount == -1) {
                    done = true;
                }
                else if (amount > 0) {
                    sb.append(new String(buff, 0, amount));
                }
            }
        } catch (IOException ioe) { }
        try {
            is.close();
        }
        catch (IOException ioe) {}
        return sb.toString();
    }

    /**
     * This extracts the JRE version from the passed in string.
     * <p>
     * This returns null if the product or platform could not be determined.
     */
    private static ConfigProperties.JREInformation extractVersion(
                         String path, String vString) {
        String platform = extractString(PLATFORM_ID, vString);
        String product = extractString(PRODUCT_ID, vString);

        if (platform != null && product != null) {
            return new ConfigProperties.JREInformation(platform, product,
						       null, path,
						       ConfigProperties.JREInformation.SRC_USER,
						       true,
						       ConfigProperties.getNextAvailableUserJREIndex(), null, null);
        }
        return null;
    }

    /**
     * Extracts the string after <code>id</code> in the <code>string</code>.
     * The returned string ends with the next newline after <code>id</code>,
     * or the end of the string.
     * <p>
     * If <code>id</code> is not found, null will be returned.
     */
    private static String extractString(String id, String string) {
        int index = string.indexOf(id);

        if (index != -1) {
            int end = string.indexOf('\n', index);
            String result;

            if (end != -1) {
                result = string.substring(index + id.length(), end);
            }
            else {
                result = string.substring(index + id.length());
            }
            if (result.length() > 0 && result.charAt(result.length() - 1) ==
                '\r') {
                result = result.substring(0, result.length() - 1);
            }
            return result;
        }
        return null;
    }

    /**
     * This extracts the JRE version from the passed in string. This is
     * only used for vms with 1.2, in which the complete product version was
     * not specified.
     * <p>
     * This will have to evolve as the version strings change.
     */
    private static ConfigProperties.JREInformation extractVersionFor12(
                                     String path, String vString) {
        int index = vString.indexOf("1.2");
        int length = vString.length();

        if (index != -1 && index < (length - 1)) {
            int endIndex = vString.indexOf('"', index);

            if (endIndex != -1) {
                String version = vString.substring(index, endIndex);

                return new ConfigProperties.JREInformation("1.2", version,
							   null, path,
							   ConfigProperties.JREInformation.SRC_USER,
							   true,
							   ConfigProperties.getNextAvailableUserJREIndex(), null, null);
            }
        }
        return null;
    }

    /**
     * Returns the classpath to use when determining the java version.
     */
    private static String getClassPath(File javaPath) {
        File file = javaPath;

        file = file.getParentFile();
        if (file != null) {
            file = file.getParentFile();

            if (file != null) {
                file = new File(file, "lib");

                if (file != null && file.exists()) {
                    file = new File(file, "classes.zip");

                    if (file != null && file.exists()) {
                        return ConfigProperties.getJavaWSJarFilePath() +
                               File.pathSeparator + file.getPath();
                    }
                }
            }
        }
        return ConfigProperties.getJavaWSJarFilePath();
    }


    public static void main(String[] args) {
        write(PLATFORM_ID, System.getProperty("java.specification.version"));
        write(PRODUCT_ID, System.getProperty("java.version"));
    }

    private static void write(String left, String right) {
        if (right != null) {
            System.out.println(left + right);
        }
    }
}
