/*
 * @(#)hsGFontScaler.cpp	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Copyright (C) 1996-1997 all rights reserved by HeadSpin Technology Inc. Chapel Hill, NC USA
 *
 * This software is the property of HeadSpin Technology Inc. and it is furnished
 * under a license and may be used and copied only in accordance with the
 * terms of such license and with the inclusion of the above copyright notice.
 * This software or any other copies thereof may not be provided or otherwise
 * made available to any other person or entity except as allowed under license.
 * No title to and ownership of the software or intellectual property
 * therewithin is hereby transferred.
 *
 * HEADSPIN MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY
 * OF THE SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, OR NON-INFRINGEMENT. SUN SHALL NOT BE LIABLE FOR
 * ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR
 * DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.
 *
 * This information in this software is subject to change without notice
*/

#include "hsGFontScaler.h"
#include "hsTemplates.h"

//////////////////////////////////////////////////////////////////////////////////

struct FontScalerPair {
	UInt32		fScalerID;
	hsGFontScaler*	fScaler;

	FontScalerPair()
	{
	}

	FontScalerPair(UInt32 scalerID, hsGFontScaler* scaler) : fScalerID(scalerID), fScaler(scaler)
	{
	}

	friend int operator==(const FontScalerPair& a, const FontScalerPair& b)
	{
		return a.fScalerID == b.fScalerID;
	}
};

static hsDynamicArray<FontScalerPair>*	gScalerList = nil;

//////////////////////////////////////////////////////////////////////////////////

void hsGFontScaler::Register(UInt32 scalerID, hsGFontScaler* scaler)
{
  if( NULL != scaler ) {       // I suppose, DVF
    int    found = 0;
    if (gScalerList == nil) {
      gScalerList = new hsDynamicArray<FontScalerPair>;  // Is it ok?
    } else {
      found = (0 <= gScalerList->Find(FontScalerPair(scalerID, nil)) );
    }
    if( 0 == found ) {
      scaler->Ref();
      gScalerList->Append(FontScalerPair(scalerID, scaler));
    }
  }
}

void hsGFontScaler::UnRegister(UInt32 scalerID)
{
    if (gScalerList != nil) {
        Int32	index = gScalerList->Find(FontScalerPair(scalerID, nil));

        if (index >= 0) {
            gScalerList->Get(index).fScaler->UnRef();
            gScalerList->Remove(index);
            if (gScalerList->GetCount() == 0) {
                delete gScalerList;
                gScalerList = nil;
            }
            return;
        }
    }
    hsDebugMessage("can't unregister unknown scalerID", scalerID);
}

hsGFontScaler* hsGFontScaler::Find(UInt32 scalerID)
{
    if (gScalerList != nil) {
       	Int32	index = gScalerList->Find(FontScalerPair(scalerID, nil));

        if (index >= 0) {
            return gScalerList->Get(index).fScaler;
        }
    }
    return nil;
}

