/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)NormalizerUtilities.java	1.4 03/01/23
 */

package sun.text;

public class NormalizerUtilities {

    public static int toLegacyMode(Normalizer.Mode mode) {
        // find the index of the legacy mode in the table;
        // if it's not there, default to Collator.NO_DECOMPOSITION (0)
        int legacyMode = legacyModeMap.length;
        while (legacyMode > 0) {
            --legacyMode;
            if (legacyModeMap[legacyMode] == mode) {
                break;
            }
        }
        return legacyMode;
    }

    public static Normalizer.Mode toNormalizerMode(int mode) {
        Normalizer.Mode normalizerMode;

        try {
            normalizerMode = legacyModeMap[mode];
        }
        catch(ArrayIndexOutOfBoundsException e) {
            normalizerMode = Normalizer.NO_OP;
        }
        return normalizerMode;

    }


    static Normalizer.Mode[] legacyModeMap = {
        Normalizer.NO_OP,           // Collator.NO_DECOMPOSITION
        Normalizer.DECOMP,          // Collator.CANONICAL_DECOMPOSITION
        Normalizer.DECOMP_COMPAT,   // Collator.FULL_DECOMPOSITION
    };

}


