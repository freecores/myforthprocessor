/*
 * @(#)GenerateChecksums.java	1.2 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.Properties;
import java.util.zip.Adler32;

/**
 * Calculates Adler-32 checksums for the files given in the
 * argument list and writes the file-name-to-checksum mapping
 * in properties file format to standard output.
 */
public class GenerateChecksums {
    
    public static void main(String[] args) throws IOException {
        Properties checksums = new Properties();
        
        for (int i = 0; i < args.length; i++) {
            File file = new File(args[i]);
            FileInputStream stream = new FileInputStream(file);
            checksums.put(file.getName(), Long.toString(calculateChecksum(stream)));
            stream.close();
        }
        
        checksums.store(System.out, null);
    }

    private static long calculateChecksum(FileInputStream in) throws IOException {
        Adler32 checksum = new Adler32();
        byte[] buffer = new byte[1024];
        int length = 0;
        while ((length = in.read(buffer)) != -1) {
            checksum.update(buffer, 0, length);
        }
        return checksum.getValue();
    }
}

