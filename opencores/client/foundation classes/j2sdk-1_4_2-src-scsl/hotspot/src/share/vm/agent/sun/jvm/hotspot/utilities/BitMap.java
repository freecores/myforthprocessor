/*
 * @(#)BitMap.java	1.3 03/01/23 11:51:07
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.utilities;

/** Manages a bitmap of the specified bit size */
public class BitMap {
  public BitMap(int sizeInBits) {
    this.size = sizeInBits;
    int nofWords = sizeInWords();
    data = new int[nofWords];
  }

  public int size() {
    return size;
  }

  // Accessors
  public boolean at(int offset) {
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(offset>=0 && offset < size(), "BitMap index out of bounds");
    }
    return Bits.isSetNthBit(wordFor(offset), offset % bitsPerWord);
  }

  public void atPut(int offset, boolean value) {
    int index = indexFor(offset);
    int pos   = offset % bitsPerWord;
    if (value) {
      data[index] = Bits.setNthBit(data[index], pos);
    } else {
      data[index] = Bits.clearNthBit(data[index], pos);
    }
  }

  public void clear() {
    for (int i = 0; i < sizeInWords(); i++) {
      data[i] = Bits.NoBits;
    }
  }

  public void iterate(BitMapClosure blk) {
    for (int index = 0; index < sizeInWords(); index++) {
      int rest = data[index];
      for (int offset = index * bitsPerWord; rest != Bits.NoBits; offset++) {
        if (rest % 2 == 1) {
          if (offset < size()) {
            blk.doBit(offset);
          } else {
            return; // Passed end of map
          }
        }
        rest = rest >>> 1;
      }
    }
  }

  /** Sets this bitmap to the logical union of it and the
      argument. Both bitmaps must be the same size. Returns true if a
      change was caused in this bitmap. */
  public boolean setUnion(BitMap other) {
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(size() == other.size(), "must have same size");
    }
    boolean changed = false;
    for (int index = 0; index < sizeInWords(); index++) {
      int temp = data[index] | other.data[index];
      changed = changed || (temp != data[index]);
      data[index] = temp;
    }
    return changed;
  }

  /** Sets this bitmap to the logical intersection of it and the
      argument. Both bitmaps must be the same size. */
  public void setIntersection(BitMap other) {
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(size() == other.size(), "must have same size");
    }
    for (int index = 0; index < sizeInWords(); index++) {
      data[index] = data[index] & (other.data[index]);
    }
  }

  /** Sets this bitmap to the contents of the argument. Both bitmaps
      must be the same size. */
  public void setFrom(BitMap other) {
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(size() == other.size(), "must have same size");
    }
    for (int index = 0; index < sizeInWords(); index++) {
      data[index] = other.data[index];
    }
  }

  /** Sets this bitmap to the logical difference between it and the
      argument; that is, any bits that are set in the argument are
      cleared in this bitmap. Both bitmaps must be the same size.
      Returns true if a change was caused in this bitmap. */
  public boolean setDifference(BitMap other) {
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(size() == other.size(), "must have same size");
    }
    boolean changed = false;
    for (int index = 0; index < sizeInWords(); index++) {
      int temp = data[index] & ~(other.data[index]);
      changed = changed || (temp != data[index]);
      data[index] = temp;
    }
    return changed;
  }

  /** Both bitmaps must be the same size. */
  public boolean isSame(BitMap other) {
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(size() == other.size(), "must have same size");
    }
    for (int index = 0; index < sizeInWords(); index++) {
      if (data[index] != (other.data[index])) return false;
    }
    return true;
  }

  //----------------------------------------------------------------------
  // Internals only below this point
  //
  private int   size; // in bits
  private int[] data;
  private static final int bitsPerWord = 32;

  private int sizeInWords() {
    return (size() + bitsPerWord - 1) / bitsPerWord;
  }

  private int indexFor(int offset) {
    return offset / bitsPerWord;
  }

  private int wordFor(int offset) {
    return data[offset / bitsPerWord];
  }
}
