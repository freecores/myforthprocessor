/*
 * @(#)ScaledBlit.java	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d.loops;

import java.awt.Composite;
import java.lang.ref.WeakReference;
import sun.java2d.loops.GraphicsPrimitive;
import sun.java2d.SurfaceData;

/**
 * ScaledBlit
 * 1) copies rectangle of pixels from one surface to another
 *    while scaling the pixels to meet the sizes specified
 * 2) performs compositing of colors based upon a Composite
 *    parameter
 *
 * precise behavior is undefined if the source surface
 * and the destination surface are the same surface
 * with overlapping regions of pixels
 */

public class ScaledBlit extends GraphicsPrimitive
{
    public static final String methodSignature = "ScaledBlit(...)".toString();

    public static final int primTypeID = makePrimTypeID(); 

    private static RenderCache blitcache = new RenderCache(20);

    public static ScaledBlit locate(SurfaceType srctype,
			      CompositeType comptype,
			      SurfaceType dsttype)
    {
	return (ScaledBlit)
	    GraphicsPrimitiveMgr.locate(primTypeID,
					srctype, comptype, dsttype);
    }

    public static synchronized ScaledBlit getFromCache(SurfaceType src,
						 CompositeType comp,
						 SurfaceType dst)
    {
	Object o = blitcache.get(src, comp, dst);
	if (o != null) {
	    return (ScaledBlit) o;
	}
	ScaledBlit blit = locate(src, comp, dst);
	if (blit == null) {
	    /*
	    System.out.println("blit loop not found for:");
	    System.out.println("src:  "+src);
	    System.out.println("comp: "+comp);
	    System.out.println("dst:  "+dst);
	    */
	} else {
	    blitcache.put(src, comp, dst, blit);
	}
	return blit;
    }

    protected ScaledBlit(SurfaceType srctype,
		   CompositeType comptype,
		   SurfaceType dsttype)
    {
	super(methodSignature, primTypeID, srctype, comptype, dsttype);
    }

    public ScaledBlit(long pNativePrim,
		SurfaceType srctype,
		CompositeType comptype,
		SurfaceType dsttype)
    {
	super(pNativePrim, methodSignature, primTypeID, srctype, comptype, dsttype);
    }

    public native void Scale(SurfaceData src, SurfaceData dst,
			     Composite comp,
			     int srcx, int srcy,
			     int dstx, int dsty,
			     int srcW, int srcH,
			     int dstW, int dstH,
			     int clipX1, int clipY1,
			     int clipX2, int clipY2);
    static {
	GraphicsPrimitiveMgr.registerGeneral(new ScaledBlit(null, null, null));
    }

    public GraphicsPrimitive makePrimitive(SurfaceType srctype,
					   CompositeType comptype,
					   SurfaceType dsttype)
    {
        /*
	System.out.println("Constructing general blit for:");
	System.out.println("src:  "+srctype);
	System.out.println("comp: "+comptype);
	System.out.println("dst:  "+dsttype);
        */
	return null;
    }

    public GraphicsPrimitive traceWrap() {
	return new TraceScaledBlit(this);
    }

    private static class TraceScaledBlit extends ScaledBlit {
	ScaledBlit target;

	public TraceScaledBlit(ScaledBlit target) {
	    super(target.getSourceType(),
		  target.getCompositeType(),
		  target.getDestType());
	    this.target = target;
	}

	public GraphicsPrimitive traceWrap() {
	    return this;
	}

	public void Scale(SurfaceData src, SurfaceData dst, Composite comp,
			  int srcx, int srcy, int dstx, int dsty,
			  int srcW, int srcH, int dstW, int dstH,
			  int clipX1, int clipY1, int clipX2, int clipY2)
	{
	    tracePrimitive(target);
	    target.Scale(src, dst, comp,
			 srcx, srcy, dstx, dsty, srcW, srcH, dstW, dstH,
			 clipX1, clipY1, clipX2, clipY2);
	}
    }
}
