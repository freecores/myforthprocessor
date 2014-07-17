/*
 * @(#)CodingChooser.java	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.pack;

import java.io.*;
import java.util.*;
import java.util.zip.*;

class CodingChooser implements Constants {
    static int verbose;
    int effort;
    Choice[] choices;
    ByteArrayOutputStream context;

    // Element in sorted set of coding choices:
    private static class Choice {
	final Coding coding;
	final int index;       // index in choices
	final int[] distance;  // cache of distance
	Choice(Coding coding, int index, int[] distance) {
	    this.coding   = coding;
	    this.index    = index;
	    this.distance = distance;
	}
	// These variables are reset and reused:
	int searchOrder; // order in which it is checked
	int minDistance; // min distance from already-checked choices
	int zipSize;     // size of encoding in sample, zipped output
	int byteSize;    // size of encoding in sample (debug only)

	void reset() {
	    searchOrder = Integer.MAX_VALUE;
	    minDistance = Integer.MAX_VALUE;
	    zipSize = byteSize = -1;
	}

	public String toString() {
	    return stringForDebug();
	}

	private String stringForDebug() {
	    String s = "";
	    if (searchOrder < Integer.MAX_VALUE)
		s += " so: "+searchOrder;
	    if (minDistance < Integer.MAX_VALUE)
		s += " md: "+minDistance;
	    if (zipSize > 0)
		s += " zs: "+zipSize;
	    if (byteSize > 0)
		s += " bs: "+byteSize;
	    return "Choice["+index+"] "+s+" "+coding;
	}
    }

    CodingChooser(int effort, Coding[] allChoices) {
	this.effort = effort;
	// The following line "makes sense" but is too much
	// work for a simple heuristic.
	//if (effort > 5)  zipDef.setLevel(effort);
	int nc = 0;
	for (int i = 0; i < allChoices.length; i++) {
	    if (allChoices[i] == null)  continue;
	    nc++;
	}
	choices = new Choice[nc];
	nc = 0;
	for (int i = 0; i < allChoices.length; i++) {
	    if (allChoices[i] == null)  continue;
	    int[] distance = new int[choices.length];
	    choices[nc++] = new Choice(allChoices[i], i, distance);
	}
	for (int i = 0; i < choices.length; i++) {
	    Coding ci = choices[i].coding;
	    if(D)assert0(ci.distanceFrom(ci) == 0);
	    for (int j = 0; j < i; j++) {
		Coding cj = choices[j].coding;
		int dij = ci.distanceFrom(cj);
		if(D)assert0(dij > 0);
		if(D)assert0(dij == cj.distanceFrom(ci));
		choices[i].distance[j] = dij;
		choices[j].distance[i] = dij;
	    }
	}
    }

    ByteArrayOutputStream getContext() {
	if (context == null)
	    context = new ByteArrayOutputStream(1 << 16);
	return context;
    }

    private int searchOrder;

    public static final int MIN_EFFORT = 1;
    public static final int MAX_EFFORT = 9;

    Coding choose(int[] values, int start, int end, Coding regular, int[] dstate, int[] sizes) {
	if (effort <= MIN_EFFORT) {
	    if (sizes != null)
		computeSize(regular, dstate, values, start, end, sizes);
	    return regular;
	}

	int min = Integer.MAX_VALUE;
	int max = Integer.MIN_VALUE;
	for (int i = start; i < end; i++) {
	    int val = values[i];
	    if (min > val)  min = val;
	    if (max < val)  max = val;
	}
	Choice regularChoice = null;
	int numChoices = 0;
	for (int i = 0; i < choices.length; i++) {
	    Choice c = choices[i];
	    c.reset();
	    if (!c.coding.canRepresent(min, max)) {
		// Mark as already visited:
		c.searchOrder = -1;
		continue;
	    }
	    if (c.coding == regular)
		regularChoice = c;
	    numChoices++;
	}
	if (regularChoice == null) {
	    for (int i = 0; i < choices.length; i++) {
		Choice c = choices[i];
		if (c.searchOrder != -1) {
		    regularChoice = c;  // arbitrary pick
		    break;
		}
	    }
	    if (verbose > 1) {
		System.out.println("*** cannot find regular choice "+regular);
		System.out.println("    using instead "+regularChoice.coding);
	    }
	}
	searchOrder = 0;
	if (verbose > 2) {
	    System.out.println("chooser: #choices="+numChoices+" ["+min+".."+max+"]");
	    if (verbose > 4) {
		for (int i = 0; i < choices.length; i++) {
		    Choice c = choices[i];
		    if (c.searchOrder >= 0)
			System.out.println("  "+c);
		}
	    }
	}

	double searchScale = 1.0;
	for (int x = effort; x < MAX_EFFORT; x++) {
	    searchScale /= 1.414;  // every 2 effort points doubles work
	}
	int searchOrderLimit = (int)( numChoices * searchScale );

	// Start by evaluating the "regular" choice.
	Choice bestChoice = regularChoice;
	int maxd = evaluate(regularChoice, dstate, values, start, end);
	regularChoice.zipSize -= 1;  // give credit for being the default
	int dscale = 1;
	// Continually select a new choice to evaluate.
	while (searchOrder < searchOrderLimit) {
	    if (dscale > maxd)  dscale = 1;  // cycle dscale values!
	    int dhi = maxd / dscale;
	    int dlo = maxd / (dscale *= 2) + 1;
	    Choice nextChoice = findChoiceNear(bestChoice, dhi, dlo);
	    if (nextChoice == null)  continue;
	    if(D)assert0(nextChoice.coding.canRepresent(min, max));
	    int nextMaxd = evaluate(nextChoice, dstate, values, start, end);
	    if (bestChoice.zipSize > nextChoice.zipSize) {
		if (verbose > 2)
		    System.out.println("better after "+searchOrder+" steps at jump of "+bestChoice.distance[nextChoice.index]+" by "+((int)((((double)bestChoice.zipSize / nextChoice.zipSize)-1)*10000+0.5)/100.0)+"%");
		bestChoice = nextChoice;
		maxd = nextMaxd;
		if (verbose > 5)  System.out.println("maxd = "+maxd);
	    }
	}
	Coding best = bestChoice.coding;
	if (verbose > 1) {
	    System.out.println("chooser: result="+bestChoice+" after "+bestChoice.searchOrder+" rounds, "+(regularChoice.zipSize-bestChoice.zipSize)+" fewer bytes than regular "+regular);
	}
	return best;
    }
    Coding choose(int[] values, int start, int end, Coding regular, int[] dstate) {
	return choose(values, start, end, regular, dstate, null);
    }
    Coding choose(int[] values, Coding regular, int[] sizes) {
	return choose(values, 0, values.length, regular, null, sizes);
    }
    Coding choose(int[] values, Coding regular) {
	return choose(values, 0, values.length, regular, null, null);
    }

    // Find an arbitrary choice at least dlo away from a previously
    // evaluated choices, and at most dhi.  Try also to regulate its
    // min distance to all previously evaluated choices, in this range.
    private Choice findChoiceNear(Choice near, int dhi, int dlo) {
	if (verbose > 5)
	    System.out.println("findChoice "+dhi+".."+dlo+" near: "+near);
	int[] distance = near.distance;
	Choice found = null;
	for (int i = 0; i < choices.length; i++) {
	    Choice c = choices[i];
	    if (c.searchOrder < searchOrder)
		continue;  // already searched
	    // Distance from "near" guy must be in bounds:
	    if (distance[i] >= dlo && distance[i] <= dhi) {
		// Try also to keep min-distance from other guys in bounds:
		if (c.minDistance >= dlo && c.minDistance <= dhi) {
		    if (verbose > 5)
			System.out.println("findChoice => good "+c);
		    return c;
		}
		found = c;
	    }
	}
	if (verbose > 5)
	    System.out.println("findChoice => found "+found);
	return found;
    }

    private int evaluate(Choice c, int[] dstate, int[] values, int start, int end) {
	if(D)assert0(c.searchOrder == Integer.MAX_VALUE);
	c.searchOrder = searchOrder++;
	int[] sizes = { 0, 0 };
	if (dstate != null)  dstate = (int[]) dstate.clone();
	computeSize(c.coding, dstate, values, start, end, sizes);
	c.byteSize = sizes[0];
	c.zipSize  = sizes[1];
	if (verbose > 3)
	    System.out.println("evaluate "+c);
	// update all minDistance values in still unevaluated choices
	int[] distance = c.distance;
	int maxd = 0;  // how far is c from everybody else?
	for (int i = 0; i < choices.length; i++) {
	    Choice c2 = choices[i];
	    if (c2.searchOrder < searchOrder)
		continue;
	    int d = distance[i];
	    if (verbose > 5)
		System.out.println("evaluate dist "+d+" to "+c2);
	    int mind = c2.minDistance;
	    if (mind > d)
		c2.minDistance = mind = d;
	    if (maxd < d)
		maxd = d;
	}
	// Now maxd has the distance of the farthest outlier
	// from all evaluated choices.
	if (verbose > 5)
	    System.out.println("evaluate maxd => "+maxd);
	return maxd;
    }

    // Compute the coded size of a sequence of values.
    // The first int is the size in uncompressed bytes.
    // The second is an estimate of the compressed size of these bytes.
    public void computeSize(Coding c, int[] dstate, int[] values, int start, int end, int[] sizes) {
	try {
	    resetData();
	    c.writeArrayTo(byteSizer, values, start, end, dstate);
	    sizes[0] = getByteSize();
	    sizes[1] = getZipSize();
	} catch (IOException ee) {
	    throw new RuntimeException(ee); // cannot happen
	}
    }
    public void computeSize(Coding c, int[] values, int[] sizes) {
	computeSize(c, null, values, 0, values.length, sizes);
    }
    public int[] computeSize(Coding c, int[] values, int start, int end) {
	int[] sizes = { 0, 0 };
	computeSize(c, null, values, start, end, sizes);
	return sizes;
    }
    public int[] computeSize(Coding c, int[] values) {
	return computeSize(c, values, 0, values.length);
    }

    class Sizer extends OutputStream {
	final OutputStream out;  // if non-null, copy output here also
	Sizer(OutputStream out) {
	    this.out = out;
	}
	Sizer() {
	    this(null);
	}
	private int count;
	public void write(int b) throws IOException {
	    count++;
	    if (out != null)  out.write(b);
	}
	public void write(byte b[], int off, int len) throws IOException {
	    count += len;
	    if (out != null)  out.write(b, off, len);
	}
	public void reset() {
	    count = 0;
	}
	public int getSize() { return count; }

	public String toString() {
	    if (D)  return stringForDebug();
	    return super.toString();
	}
	String stringForDebug() {
	    return "<Sizer "+getSize()+">";
	}
    }

    private Sizer zipSizer  = new Sizer();
    private Deflater zipDef = new Deflater();
    private DeflaterOutputStream zipOut = new DeflaterOutputStream(zipSizer, zipDef);
    private Sizer byteSizer = new Sizer(zipOut);

    private void resetData() {
	flushData();
	zipDef.reset();
	if (context != null) {
	    // Prepend given salt to the test output.
	    try {
		context.writeTo(byteSizer);
	    } catch (IOException ee) {
		throw new RuntimeException(ee); // cannot happen
	    }
	}
	zipSizer.reset();
	byteSizer.reset();
    }
    private void flushData() {
	try {
	    zipOut.finish();
	} catch (IOException ee) {
	    throw new RuntimeException(ee); // cannot happen
	}
    }
    private int getByteSize() {
	return byteSizer.getSize();
    }
    private int getZipSize() {
	flushData();
	return zipSizer.getSize();
    }

    static void assert0(boolean z) {
	if (!z) throw new RuntimeException("assert failed");
    }
}
