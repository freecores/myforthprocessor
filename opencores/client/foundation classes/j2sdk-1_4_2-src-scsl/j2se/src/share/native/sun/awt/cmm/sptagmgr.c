/*
 * @(#)sptagmgr.c	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
	Contains:	This module contains the profile tag manager.

				Created by lsh, October 18, 1993

	Written by:	The Kodak CMS MS Windows Team

	Copyright:	(C) 1993-1999 by Eastman Kodak Company, 
                            all rights reserved.

	Macintosh
	Change History (most recent first):

	Windows Revision Level:
		$Workfile: sptagmgr.c $
		$Logfile: /DLL/KodakCMS/SProfile/sptagmgr.c $
		$Revision: 2 $
		$Date: 5/14/99 1:37p $
		$Author: Msm $

	SCCS Revision:
		@(#)sptagmgr.c	1.41	1/8/99

	To Do:
 */


#include "sprof-pr.h"
#include <string.h>

/* Prototypes */
static void SpTagDelete (
				SpTagDirEntry_t FAR *Entry);

static SpStatus_t KSPAPI SpTagDeleteByIdImp (
				SpProfileData_t	FAR *ProfileData,
				SpTagId_t		TagId);

static SpStatus_t SpTagDeleteByIndex (
				SpTagDirEntry_t FAR *TagArray,
				KpInt32_t		TotalCount,
				KpUInt32_t		Index);

static SpStatus_t SpTagDirArrayInit (
				SpProfileData_t FAR *ProfileData);

static SpStatus_t SpTagDirEntryAssign (
				SpTagDirEntry_t FAR	*TagDirEntry,
				SpTagId_t			TagId,
				KpUInt32_t			DataSize,
				void				KPHUGE *Data);

static SpStatus_t SpTagDirEntryInit (
				SpTagDirEntry_t FAR *TagDirEntry);

static KpInt32_t SpTagFindByIndex (
				SpTagDirEntry_t	FAR *TagArray,
				KpInt32_t		Index,
				KpInt32_t		TotalCount);
				
static KpInt32_t SpTagGetFreeIndex (
				SpTagDirEntry_t FAR *TagArray,
				KpInt32_t TotalCount);

/* A free element of the tag directory array has the TagDataSize field == -1 */
const KpInt32_t SpFreeElement = -1;

/* The search routines will return the following if a match is not found */
const KpInt32_t SpNotFound = -1;


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Delete a tag directory entry.
 *
 * AUTHOR
 * 	mlb
 *
 * DATE CREATED
 *	October 17, 1994
 *------------------------------------------------------------------*/
static void SpTagDelete (SpTagDirEntry_t FAR *Entry)
{
	freeBuffer (Entry->TagData);
	Entry->TagData = NULL;
	
/* designate this element as a free element */
	Entry->TagDataSize = SpFreeElement;

} /* SpTagDelete */

#if !defined (SP_READONLY_PROFILES)

/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Delete a tag directory entry by Id.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	October 19, 1993
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpTagDeleteById (
				SpProfile_t	Profile,
				SpTagId_t	TagId)
{
	SpProfileData_t	FAR *ProfileData;
	SpStatus_t		Status;

	ProfileData = SpProfileLock (Profile);
	if (NULL == ProfileData)
		return SpStatBadProfile;

	Status = SpTagDeleteByIdImp (ProfileData, TagId);

	ProfileData->ProfChanged = KPTRUE;

	SpProfileUnlock (Profile);
	return Status;
	
} /* SpTagDeleteById */


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Delete a tag directory entry by Id.
 *	Implementation.
 *
 * AUTHOR
 * 	mlb
 *
 * DATE CREATED
 *	October 17, 1994
 *------------------------------------------------------------------*/
static SpStatus_t KSPAPI SpTagDeleteByIdImp (
				SpProfileData_t	FAR *ProfileData,
				SpTagId_t		TagId)
{
	SpTagDirEntry_t	FAR	*tagArray;
	KpInt32_t			index;
	SpStatus_t	status;

	/* Verify the Tag Array if Loaded.  Search Function
	   Only loads the Header */
	if (ProfileData->TagArray == NULL)
	{
		status = SpProfilePopTagArray(ProfileData);
		if (status != SpStatSuccess)
			return status;
	}

/* get a pointer to the head of the tag array */
	tagArray = (SpTagDirEntry_t FAR *) lockBuffer (ProfileData->TagArray);
	
/* look for tag in profile */
	index = SpTagFindById (tagArray, TagId, ProfileData->TotalCount);
	if (index == SpNotFound)
	{
		unlockBuffer (ProfileData->TagArray);
		return SpStatNotFound;
	}

/* free the tag directory entry */
	SpTagDelete (&tagArray[index]);
	
/* set FreeIndex if necessary */
	if (ProfileData->FreeIndex > index)
		ProfileData->FreeIndex = index;

/* unlock handle */
	unlockBuffer (ProfileData->TagArray);
	
	return SpStatSuccess;
	
} /* SpTagDeleteByIdImp */
#endif	/* !SP_READONLY_PROFILES */


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Delete a tag directory entry by Index.
 *	Implementation.
 *
 * AUTHOR
 * 	mlb
 *
 * DATE CREATED
 *	October 18, 1994
 *------------------------------------------------------------------*/
static SpStatus_t SpTagDeleteByIndex (
				SpTagDirEntry_t FAR *tagArray,
				KpInt32_t		TotalCount,
				KpUInt32_t		Index)
{
	KpInt32_t			index;

/* look for tag in profile */
	index = SpTagFindByIndex (tagArray, Index, TotalCount);
	if (index == SpNotFound)
	{
		return SpStatNotFound;
	}

/* free the directory entry */
	SpTagDelete (&tagArray[index]);

	return SpStatSuccess;
	
} /* SpTagDeleteByIndex */

/*--------------------------------------------------------------------
 * DESCRIPTION
 *  Initialize entire tag directory entry array, TagArray
 *
 * AUTHOR
 * 	mlb
 *
 * DATE CREATED
 *	October 28, 1994
 *------------------------------------------------------------------*/
static SpStatus_t SpTagDirArrayInit (SpProfileData_t FAR *ProfileData)
{
	KpInt32_t index;
	SpTagDirEntry_t FAR *tagArray;
	
/* lock TagArray handle and return ptr */
	tagArray = (SpTagDirEntry_t FAR *) lockBuffer (ProfileData->TagArray);

	for (index = 0; index < ProfileData->TotalCount; index++)
		SpTagDirEntryInit (&tagArray[index]);

	unlockBuffer(ProfileData->TagArray);

	return SpStatSuccess;
	
} /* SpTagDirArrayInit */


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Add a tag directory entry for a tag.
 *
 * AUTHOR
 * 	mlb
 *
 * DATE CREATED
 *	October 20, 1994
 *------------------------------------------------------------------*/
SpStatus_t SpTagDirEntryAdd (
				SpProfileData_t FAR *ProfileData,
				SpTagId_t		TagId,
				KpUInt32_t		DataSize,
				void			KPHUGE *Data)
{
	KpInt32_t			index;
	KpInt32_t			loop;
	SpTagDirEntry_t	FAR *tagArray;
	KpHandle_t			newTagArrayHandle;
	SpTagDirEntry_t	FAR *newTagArray;
	SpStatus_t			status;

	/* Verify the Tag Array if Loaded.  Search Function
	   Only loads the Header */
	if (ProfileData->TagArray == NULL)
	{
		status = SpProfilePopTagArray(ProfileData);
		if (status != SpStatSuccess)
			return status;
	}

/* lock TagArray handle and return ptr */
	tagArray = (SpTagDirEntry_t FAR *) lockBuffer (ProfileData->TagArray);

/* look for tag in profile */
	index = SpTagFindById (tagArray, TagId, ProfileData->TotalCount);

/* TagId not found, add entry */
	if (index == SpNotFound)
	{
	
		if ((ProfileData->FreeIndex >= 0) &&
			(ProfileData->FreeIndex < ProfileData->TotalCount))
		{
			/* add tag to current array */
			status = SpTagDirEntryAssign (&tagArray[ProfileData->FreeIndex],
											TagId, DataSize, Data);
											
			ProfileData->FreeIndex = SpTagGetFreeIndex (tagArray,
													ProfileData->TotalCount);
		}
		else
		{
			/* add tag to enlarged array */
			newTagArrayHandle = allocBufferHandle (ProfileData->TotalCount *
												2 * sizeof (SpTagDirEntry_t));
			
			if (newTagArrayHandle == NULL)
			{
				unlockBuffer (ProfileData->TagArray);
				return SpStatMemory;
			}
								
			newTagArray = (SpTagDirEntry_t FAR *)
								lockBuffer (newTagArrayHandle);
								
			KpMemCpy (newTagArray, tagArray, ProfileData->TotalCount *
						sizeof (SpTagDirEntry_t FAR));
						
			/* free the old array */
			unlockBuffer (ProfileData->TagArray);
			freeBuffer (ProfileData->TagArray);
			
			index = ProfileData->TotalCount;
			ProfileData->TotalCount *= 2;
			for (loop = index; loop < ProfileData->TotalCount; loop++)
				SpTagDirEntryInit (&newTagArray[loop]);
			
			/* assign the old handle to the newly allocated space */	
			ProfileData->TagArray = newTagArrayHandle;
			status = SpTagDirEntryAssign (&newTagArray[index], TagId, DataSize, Data);
			ProfileData->FreeIndex = index + 1;
		}
	}
	else	/* TagId found, update tagArray element */
	{
	
	/* see if element is active */
		if (tagArray[index].TagDataSize != SpFreeElement)
		{
			/* delete current array element in order to replace it */
			SpTagDeleteByIndex (tagArray, ProfileData->TotalCount, 
                                            index);
		}
		
		/* add tag to current array (update it) */
		status = SpTagDirEntryAssign (&tagArray[index], TagId,
										DataSize, Data);
											
		ProfileData->FreeIndex = SpTagGetFreeIndex (tagArray,
													ProfileData->TotalCount);
													
	}

/* unlock handle */
	unlockBuffer (ProfileData->TagArray);
	return status;

} /* SpTagDirEntryAdd */


/*--------------------------------------------------------------------
 * DESCRIPTION
 *  Assign tag directory information to a tag directory entry.
 *
 * AUTHOR
 * 	mlb
 *
 * DATE CREATED
 *	October 20, 1994
 *------------------------------------------------------------------*/
static SpStatus_t SpTagDirEntryAssign (
				SpTagDirEntry_t FAR	*TagDirEntry,
				SpTagId_t			TagId,
				KpUInt32_t			DataSize,
				void				KPHUGE *Data)
{
	void KPHUGE *ptr;
	
	TagDirEntry->TagId = TagId;
	TagDirEntry->TagDataSize = DataSize;
	
	/* allocate memory for TagData */
	TagDirEntry->TagData = allocBufferHandle (DataSize);
	if (TagDirEntry->TagData == NULL)
		return SpStatMemory;
	ptr = (void KPHUGE *) lockBuffer (TagDirEntry->TagData);
	KpMemCpy (ptr, Data, DataSize);
	
	unlockBuffer(TagDirEntry->TagData);

	return SpStatSuccess;
	
} /* SpTagDirEntryAssign */


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Initialize a tag directory entry element of the TagArray.  Setting
 *  TagDataSize = -1 means that the element is available.  After an
 *  element is deleted, for example, the TagDataSize field will be set
 *  to -1 -- thus signifying that the element is again available.
 *
 * AUTHOR
 * 	mlb
 *
 * DATE CREATED
 *	October 14, 1994
 *------------------------------------------------------------------*/
static SpStatus_t SpTagDirEntryInit (SpTagDirEntry_t FAR *TagDirEntry)
{
	TagDirEntry->TagId = 0;
	TagDirEntry->TagData = NULL;
	TagDirEntry->TagDataSize = SpFreeElement;

	return SpStatSuccess;
	
} /* SpTagDirEntryInit */


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Find a tag directory entry given a TagId.  If DataSize == -1, then
 *  this means that the element has been previously deleted and is
 *  now available for reuse.  Thus, even if the TagId of such an element
 *  is matched, no real match has occured since this is an available
 *  element.
 *
 * AUTHOR
 * 	mlb
 *
 * DATE CREATED
 *	October 14, 1994
 *------------------------------------------------------------------*/
KpInt32_t SpTagFindById (
				SpTagDirEntry_t FAR *TagArray,
				SpTagId_t		TagId,
				KpInt32_t		TotalCount)
{
	KpInt32_t			index;
	SpTagDirEntry_t	FAR	*curr;
	
	curr = TagArray;
	
/* search for matching TagId */
	for (index = 0; index < TotalCount; index++)
	{
		if ((curr[index].TagId == TagId) && (curr[index].TagDataSize != SpFreeElement))
			return index;
	}

	return SpNotFound;
	
} /* SpTagFindById */


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Find a tag directory entry given it index.  If the TagDataSize field of the array
 * element == -1, then this element is currently available.  Thus, no match is
 * possible at this array position.
 *
 * AUTHOR
 * 	mlb
 *
 * DATE CREATED
 *	October 14, 1994
 *------------------------------------------------------------------*/
static KpInt32_t SpTagFindByIndex (
				SpTagDirEntry_t	FAR *TagArray,
				KpInt32_t		Index,
				KpInt32_t		TotalCount)
{
	SpTagDirEntry_t FAR *curr;
	
	curr = TagArray;
	
	if ((Index >= 0) && (Index < TotalCount) && (curr[Index].TagDataSize != SpFreeElement))
		return Index;
	else
		return SpNotFound;
		
} /* SpTagFindByIndex */


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Get number of active tag directory entries in the profile.
 *
 * AUTHOR
 * 	mlb
 *
 * DATE CREATED
 *	October 14, 1994
 *------------------------------------------------------------------*/
KpInt32_t SpTagGetCount (SpProfileData_t	FAR *ProfileData)
{
	SpTagDirEntry_t	FAR *tagArray;
	KpInt32_t		index;
	KpInt32_t		activeCount = 0;
	SpStatus_t		status;

	/* Verify the Tag Array if Loaded.  Search Function
	   Only loads the Header */
	if (ProfileData->TagArray == NULL)
	{
		status = SpProfilePopTagArray(ProfileData);
		if (status != SpStatSuccess)
			return activeCount;
	}

/* get a pointer to the head of the tag array */
	tagArray = (SpTagDirEntry_t FAR *) lockBuffer (ProfileData->TagArray);
		
/* determine the number of active TagArray elements */
	for (index = 0; index < ProfileData->TotalCount; index++)
		if (tagArray[index].TagDataSize == SpFreeElement)
			continue;
		else
			activeCount++;
	
	unlockBuffer (ProfileData->TagArray);

	return activeCount;
	
} /* SpTagGetCount */


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Find the next free element in the array of tag directory entries.
 *
 * AUTHOR
 * 	mlb
 *
 * DATE CREATED
 *	October 28, 1994
 *------------------------------------------------------------------*/
static KpInt32_t SpTagGetFreeIndex (SpTagDirEntry_t FAR *TagArray,
								KpInt32_t TotalCount)
{
	KpInt32_t index;
	
	for (index = 0; index < TotalCount; index++)
	{
		if (TagArray[index].TagDataSize == SpFreeElement)
			return index;
	}
	
	return TotalCount;
	
} /* SpTagGetFreeIndex */


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Get raw profile header data.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	August 16, 1994
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpRawHeaderGet (
				SpProfile_t	Profile,
				KpUInt32_t	BufferSize,
				void		KPHUGE *Buffer)
{
	SpStatus_t	Status;
	SpHeader_t	Header;
	KpUInt32_t	ProfSize;
	char		KPHUGE *Ptr = Buffer;
	SpProfileData_t	FAR *ProfileData;

	Status = SpProfileGetHeader (Profile, &Header);

	if (SpStatSuccess == Status)
		Status = SpHeaderFromPublic (&Header, BufferSize, Buffer);

	if (SpStatSuccess != Status)
		return Status;

	ProfileData = SpProfileLock (Profile);
	if (NULL == ProfileData)
		return SpStatBadProfile;

	if ((ProfileData->ProfChanged == KPTRUE) ||
	    (ProfileData->ProfileSize == 128))
		Status = SpProfileGetProfileSize(Profile, &ProfSize);
	else
		ProfSize = ProfileData->ProfileSize;

	if (SpStatSuccess == Status)
		SpPutUInt32(&Ptr, ProfSize);

	return Status;
	
} /* SpRawHeaderGet */


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Free raw tag data.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	August 11, 1994
 *------------------------------------------------------------------*/
void KSPAPI SpRawTagDataFree (
				SpProfile_t	Profile,
				SpTagId_t	TagId,
				void		KPHUGE *Data)
{

/* this function does nothing */
/* it is here because the SGI version needs to free tag data */

	Profile = Profile;
	TagId = TagId;
	Data = Data;
	
} /* SpRawTagDataFree */


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Get raw tag data.
 *
 * AUTHOR
 * 	mlb
 *
 * DATE CREATED
 *	October 19, 1994
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpRawTagDataGet (
				SpProfile_t	Profile,
				SpTagId_t	TagId,
				KpUInt32_t	FAR *DataSize,
				void		KPHUGE * FAR *Data)
{
	SpStatus_t		status;
	KpInt32_t		index;
	SpProfileData_t FAR *profileData;
	SpTagDirEntry_t	FAR *tagArray;

/* convert profile handle to pointer to locked memory */
	profileData = SpProfileLock (Profile);
	if (NULL == profileData)
		return SpStatBadProfile;

	/* Verify the Tag Array if Loaded.  Search Function
	   Only loads the Header */
	if (profileData->TagArray == NULL)
	{
		status = SpProfilePopTagArray(profileData);
		if (status != SpStatSuccess)
			return status;
	}

/* locate the tag data */
	tagArray = (SpTagDirEntry_t FAR *) lockBuffer (profileData->TagArray);
	index = SpTagFindById (tagArray, TagId, profileData->TotalCount);
	if (SpNotFound == index)
	{
		*DataSize = 0;
		*Data = NULL;
		status = SpStatNotFound;
	}
	else
	{
		*DataSize = tagArray[index].TagDataSize;
		*Data = tagArray[index].TagData;
		status = SpStatSuccess;
	}

/* unlock handles */
	unlockBuffer (profileData->TagArray);
	SpProfileUnlock (Profile);
	return status;
	
} /* SpRawTagDataGet */


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Get raw tag data size.
 *
 * AUTHOR
 * 	mlb
 *
 * DATE CREATED
 *	October 19, 1994
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpRawTagDataGetSize (
				SpProfile_t	Profile,
				SpTagId_t	TagId,
				KpUInt32_t	FAR *DataSize)
{
	SpStatus_t		status;
	KpInt32_t		index;
	SpProfileData_t FAR *profileData;
	SpTagDirEntry_t	FAR *tagArray;

/* convert profile handle to pointer to locked memory */
	profileData = SpProfileLock (Profile);
	if (NULL == profileData)
		return SpStatBadProfile;

	/* Verify the Tag Array if Loaded.  Search Function
	   Only loads the Header */
	if (profileData->TagArray == NULL)
	{
		status = SpProfilePopTagArray(profileData);
		if (status != SpStatSuccess)
			return status;
	}

/* locate the tag data */
	tagArray = (SpTagDirEntry_t	FAR *) lockBuffer (profileData->TagArray);
	index = SpTagFindById (tagArray, TagId, profileData->TotalCount);
	if (SpNotFound == index) {
		*DataSize = 0;
		status = SpStatNotFound;
	}
	else {
		*DataSize = tagArray[index].TagDataSize;
		status = SpStatSuccess;
	}

/* unlock handles */
	unlockBuffer(profileData->TagArray);
	SpProfileUnlock (Profile);
	
	return status;
	
} /* SpRawTagDataGetSize */


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Set raw tag data.
 *
 * AUTHOR
 * 	lsh
 *
 * DATE CREATED
 *	August 11, 1994
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpRawTagDataSet (
				SpProfile_t	Profile,
				SpTagId_t	TagId,
				KpUInt32_t	DataSize,
				void		KPHUGE *Data)
{
	SpStatus_t		Status;
	SpProfileData_t	FAR *ProfileData;

	ProfileData = SpProfileLock (Profile);
	if (NULL == ProfileData)
		return SpStatBadProfile;

	Status = SpTagDirEntryAdd (ProfileData, TagId, DataSize, Data);

	ProfileData->ProfChanged = KPTRUE;

	SpProfileUnlock (Profile);
	return Status;
	
} /* SpRawTagDataSet */


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Allocate an empty profile.
 *
 * AUTHOR
 * 	mlb
 *
 * DATE CREATED
 *	October 18, 1994
 *------------------------------------------------------------------*/
SpStatus_t SpProfileAlloc (
				SpCallerId_t	CallerId,
				SpProfile_t		FAR *Profile,
				SpProfileData_t	FAR *FAR *ProfileDataPtr)
{
	SpStatus_t		Status;
	SpProfileData_t	FAR *ProfileData;

	*Profile = NULL;
	*ProfileDataPtr = NULL;

	Status = SpCallerIdValidate (CallerId);
	if (SpStatSuccess != Status)
		return Status;

	ProfileData = SpMalloc (sizeof (*ProfileData));
	if (NULL == ProfileData)
		return SpStatMemory;

/* clear entire header */
	KpMemSet (ProfileData, 0, sizeof (*ProfileData));

/* mark memory with a signature */
	ProfileData->Signature = SpProfileDataSig;

/* save CallerId */
	ProfileData->CallerId = CallerId;

/* Initialize LockCount */
	ProfileData->LockCount = 1;

/* Initialize Size Info */
	ProfileData->ProfileSize = 128;
	ProfileData->ProfChanged = KPTRUE;

/* allocate and initialize space for the tags */
	ProfileData->TotalCount = SPNUMTAGS;
	ProfileData->TagArray = allocBufferHandle (ProfileData->TotalCount *
												sizeof (SpTagDirEntry_t FAR));
												
	if (ProfileData->TagArray == NULL)
		return SpStatMemory;
		
	SpTagDirArrayInit (ProfileData);

/* initially no tags in profile */
	ProfileData->FreeIndex = 0;
	
/* give caller a handle to the profile data */
	*Profile = (SpProfile_t) getHandleFromPtr (ProfileData);

	*ProfileDataPtr = ProfileData;

	return SpStatSuccess;
	
} /* SpProfileAlloc */


#if !defined (SP_READONLY_PROFILES)
/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Create an empty profile.
 *
 * AUTHOR
 * 	kfc
 *
 * DATE CREATED
 *	October 19, 1993
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileCreate (
				SpCallerId_t	CallerId,
				SpProfile_t		FAR *Profile)
{
	return SpProfileCreateDT (CallerId, SP_ICC_TYPE_0, Profile);
}


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Create an empty profile.
 *
 * AUTHOR
 * 	kfc (based on routine by lsh)
 *
 * DATE CREATED
 *	Nov 10, 1998
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileCreateDT (
				SpCallerId_t	CallerId,
				KpUInt32_t		DataType,
				SpProfile_t		FAR *Profile)
{
	SpProfileData_t FAR *ProfileData;
	SpStatus_t		Status;
	struct kpTm		Now;
	SpTagValue_t	Tag;
	char			CopyRightStr[75];
	char			YearStr[5];

/* allocate a new profile */
	Status = SpProfileAlloc (CallerId, Profile, &ProfileData);
	if (SpStatSuccess != Status)
		return Status;

/* set the 'standard' profile values */
	KpMemSet (&ProfileData->Header, 0, sizeof (ProfileData->Header));
    ProfileData->Header.CMMType = SpCMMType;
    ProfileData->Header.ProfileVersion = SpProfileVersion;
    ProfileData->Header.DeviceClass = SpProfileClassDisplay;
    ProfileData->Header.DataColorSpace = SpSpaceRGB;
    ProfileData->Header.InterchangeColorSpace = SpSpaceXYZ;

/* set creation date/time */
	KpGetLocalTime (&Now);
	ProfileData->Header.DateTime.Year   = (KpUInt16_t) (1900 + Now.year);
	ProfileData->Header.DateTime.Month  = (KpUInt16_t) (Now.mon + 1);
	ProfileData->Header.DateTime.Day    = (KpUInt16_t) Now.mday;
	ProfileData->Header.DateTime.Hour   = (KpUInt16_t) Now.hour;
	ProfileData->Header.DateTime.Minute = (KpUInt16_t) Now.min;
	ProfileData->Header.DateTime.Second = (KpUInt16_t) Now.sec;

/* Specify the primary platform/operating system of the profile */
#if defined (KPMAC)
    ProfileData->Header.Platform = SpSigMake ('A', 'P', 'P', 'L');
#elif defined (KPSGI) || defined(KPSGIALL)
    ProfileData->Header.Platform = SpSigMake ('S', 'G', 'I', ' ');
#elif defined (KPSUN)
    ProfileData->Header.Platform = SpSigMake ('S', 'U', 'N', 'W');
#else
    ProfileData->Header.Platform = SpSigMake ('M', 'S', 'F', 'T');
#endif

    ProfileData->Header.Flags = 0;
    ProfileData->Header.DeviceManufacturer = SpSigMake ('n', 'o', 'n', 'e');
    ProfileData->Header.DeviceModel = SpSigMake ('n', 'o', 'n', 'e');
    ProfileData->Header.DeviceAttributes.hi = 0;
    ProfileData->Header.DeviceAttributes.lo = 0;
    ProfileData->Header.RenderingIntent = 0;

/* set Illuminant (D5000) */
    ProfileData->Header.Illuminant.X = KpF15d16FromDouble (0.9642);
    ProfileData->Header.Illuminant.Y = KpF15d16FromDouble (1.0);
    ProfileData->Header.Illuminant.Z = KpF15d16FromDouble (0.8249);

	switch (DataType) {
	case SP_ICC_TYPE_0:
		ProfileData->Header.Originator = SpSigOrgKodak1_Ver_0;
		break;
	case SP_ICC_TYPE_1:
		ProfileData->Header.Originator = SpSigOrgKodak_Type_1;
		break;
	case SP_ICC_TYPE_CURRENT:
	default:
		ProfileData->Header.Originator = SpSigOrgKodak;
		break;
	}

/* set default copyright */
	Tag.TagId = SpTagCopyRight;
	Tag.TagType = Sp_AT_Text;

	KpItoa ((KpInt32_t)ProfileData->Header.DateTime.Year, YearStr);
	strcpy (CopyRightStr, "Copyright (c) Eastman Kodak Company, ");
	strcat (CopyRightStr, YearStr);
	strcat (CopyRightStr, ", all rights reserved.");
	Tag.Data.Text = CopyRightStr;

	Status = SpTagSet (*Profile, &Tag);

	SpProfileUnlock (*Profile);

	return Status;

} /* SpProfileCreateDT */
#endif /* !SP_READONLY_PROFILES */


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Free a profile.
 *
 * AUTHOR
 * 	mlb
 *
 * DATE CREATED
 *	October 18, 1994
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileFree (SpProfile_t FAR *Profile)
{
	KpInt32_t			index;
	char				FAR *fileName;
	SpTagDirEntry_t		FAR *tagArray;
	SpProfileData_t		FAR *ProfileData;

	ProfileData = SpProfileLock (*Profile);
	if (NULL == ProfileData)
		return SpStatBadProfile;

	if (NULL != ProfileData->TagArray)
	{
/* lock down handle to TagArray */
		tagArray = (SpTagDirEntry_t FAR *) 
			   lockBuffer (ProfileData->TagArray);

/* free all tags in profile */
		for (index = 0; index < ProfileData->TotalCount; index++)
			SpTagDeleteByIndex (tagArray, 
   					ProfileData->TotalCount, index);

/* free tag memory */
		freeBuffer (ProfileData->TagArray);
		tagArray = NULL;
	}

/* lock Filename handle and free memory */
	fileName = (char FAR *) lockBuffer (ProfileData->FileName);
	if (fileName)
		SpFree (fileName);

	SpFree (ProfileData);
	*Profile = NULL;
	return SpStatSuccess;

} /* SpProfileFree */


/*--------------------------------------------------------------------
 * DESCRIPTION
 *	Validate that profile is valid.
 *
 * AUTHOR
 * 	mlb
 *
 * DATE CREATED
 *	October 19, 1994
 *------------------------------------------------------------------*/
SpStatus_t SpProfileValidate (SpProfileData_t FAR *ProfileData)
{
	SpTagDirEntry_t	FAR *tagArray;
	SpStatus_t	status;

	/* Verify the Tag Array if Loaded.  Search Function
  	   Only loads the Header */
	if (ProfileData->TagArray == NULL)
	{
		status = SpProfilePopTagArray(ProfileData);
		if (status != SpStatSuccess)
			return status;
	}
/* check for atleast one tag */
	if (0 == ProfileData->TotalCount)
		return SpStatBadProfile;

/* lock TagArray handle and return useable ptr */
	tagArray = (SpTagDirEntry_t FAR *) lockBuffer (ProfileData->TagArray);
	
/* check for tags that are required by all profiles */
	if (SpNotFound == SpTagFindById (tagArray, SpTagCopyRight, 
		ProfileData->TotalCount))
	{
		unlockBuffer (ProfileData->TagArray);
		return SpStatReqTagMissing;
	}

	if (SpNotFound == SpTagFindById (tagArray, SpTagProfileDesc,
		ProfileData->TotalCount))
	{
		unlockBuffer (ProfileData->TagArray);
		return SpStatReqTagMissing;
	}

/* unlock handle */
	unlockBuffer (ProfileData->TagArray);

	return SpStatSuccess;

} /* SpProfileValidate */


/*--------------------------------------------------------------
 * DESCRIPTION
 *      Validate that profile is valid.
 *
 * AUTHOR
 *      mlb
 *
 * DATE CREATED
 *      October 19, 1994
 *--------------------------------------------------------------*/

SpStatus_t SpProfilePopTagArray(SpProfileData_t *ProfileData)
{
SpStatus_t	status;
KpMapFile_t	MapFileCtl;
KpFileProps_t	props;
char		FAR *fileName;

	/* allocate and initialize space for the tags */
	ProfileData->TotalCount = SPNUMTAGS;
	ProfileData->TagArray = allocBufferHandle
				(ProfileData->TotalCount *
				sizeof (SpTagDirEntry_t FAR));
 
	if (ProfileData->TagArray == NULL)
		return SpStatMemory;
 
	SpTagDirArrayInit (ProfileData);
 
	/* initially no tags in profile */
	ProfileData->FreeIndex = 0;

	/* lock FileName handle and return ptr */
	fileName = (char *)lockBuffer(ProfileData->FileName);
 
	/* Convert file information into useable form */
        SpCvrtSpFileProps(&ProfileData->Props, &props);

	/* Map the file */
	if (NULL == KpMapFileEx(fileName, &props, 
				"r", &MapFileCtl))
		return SpStatFileNotFound;

	/* unlock handles */
	unlockBuffer (ProfileData->FileName);

	/* get profile header into internal format */
	status = SpProfileLoadFromBufferImp (ProfileData,  
				(char *)MapFileCtl.Ptr);

	KpUnMapFile (&MapFileCtl);

	return status;
}

#if !defined (SP_READONLY_PROFILES)

static SpStatus_t convertMFT (SpProfile_t pf, SpTagId_t tagID, SpSig_t newOrg);

/*--------------------------------------------------------------------
 * DESCRIPTION (Public)
 *	Set the profile to a new data type 
 *
 * AUTHOR
 * 	King F. Choi
 *
 * DATE CREATED
 *	November 5, 1998
 *------------------------------------------------------------------*/
SpStatus_t KSPAPI SpProfileSetDT (
				KpUInt32_t		DataType,
				SpProfile_t		Profile)
{
	SpHeader_t		hdr;
	SpSig_t			newOrg;
	KpInt16_t		convertFlag = 0;
	SpStatus_t		st;

	st = SpProfileGetHeader (Profile, &hdr);
	if (st != SpStatSuccess)
		return st;

	switch (DataType) {
	case SP_ICC_TYPE_0:
		st = SpStatNotImp;
		break;
	case SP_ICC_TYPE_1:
		newOrg = SpSigOrgKodak_Type_1;
		if ((hdr.Originator == SpSigOrgKodak1_Ver_0) ||
			(hdr.Originator == SpSigOrgKodak2_Ver_0))
			convertFlag = 1;
		st = SpStatSuccess;
		break;
	case SP_ICC_TYPE_CURRENT:
		newOrg = SpSigOrgKodak;
		if ((hdr.Originator == SpSigOrgKodak1_Ver_0) ||
			(hdr.Originator == SpSigOrgKodak2_Ver_0))
			convertFlag = 1;
		st = SpStatSuccess;
		break;
	default:
		st = SpStatNotImp;
		break;
	}

	if (convertFlag) {
		st = convertMFT (Profile, SpTagAToB0, newOrg);
		if (st != SpStatSuccess) {
			return st;
		}
		st = convertMFT (Profile, SpTagAToB1, newOrg);
		if (st != SpStatSuccess) {
			return st;
		}
		st = convertMFT (Profile, SpTagAToB2, newOrg);
		if (st != SpStatSuccess) {
			return st;
		}
		st = convertMFT (Profile, SpTagBToA0, newOrg);
		if (st != SpStatSuccess) {
			return st;
		}
		st = convertMFT (Profile, SpTagBToA1, newOrg);
		if (st != SpStatSuccess) {
			return st;
		}
		st = convertMFT (Profile, SpTagBToA2, newOrg);
		if (st != SpStatSuccess) {
			return st;
		}
		st = convertMFT (Profile, SpTagGamut, newOrg);
		if (st != SpStatSuccess) {
			return st;
		}
		st = convertMFT (Profile, SpTagPreview0, newOrg);
		if (st != SpStatSuccess) {
			return st;
		}
		st = convertMFT (Profile, SpTagPreview1, newOrg);
		if (st != SpStatSuccess) {
			return st;
		}
		st = convertMFT (Profile, SpTagPreview2, newOrg);
		if (st != SpStatSuccess) {
			return st;
		}
		hdr.Originator = newOrg;
		st = SpProfileSetHeader (Profile, &hdr);
	}

	return st;
}


/*--------------------------------------------------------------------
 *	Subroutine:		convertMFT
 *	Function:		Convert profile to type 1 format
 *	Inputs:			pf - profile (SpProfile_t)
 *					Tag - profile tag (SpTagValue_t)
 *					newOrg - new originator (SpSig_t)
 *	Outputs:		none
 *	Return value:	none
 *	Author:			King F. Choi
 *	Date:			October 16, 1998
 *------------------------------------------------------------------*/
static SpStatus_t convertMFT (SpProfile_t pf, SpTagId_t tagID, SpSig_t newOrg)
{
	SpTagValue_t	tag;
	SpHeader_t		hdr;
	SpSig_t			oldOrg;
	SpTransRender_t	render = SpTransRenderPerceptual;
	SpTransType_t	transform = SpTransTypeIn;
	SpStatus_t		st;
	SpXform_t		xform;
	SpSig_t			LutType;

	st = SpTagGetById (pf, tagID, &tag);
	if (st != SpStatSuccess) {
		return 	SpStatSuccess;				/* assume no Lut of this type, nothing to do */
	}
	
	LutType = tag.Data.Lut.LutType;
	SpTagFree(&tag);
	
	if (LutType != SpTypeLut16) {
		return 	SpStatSuccess;					/* only convert Lut16 */
	}
	
	switch ((KpInt32_t)tagID) {
	case (KpInt32_t)SpTagAToB0:
		render = SpTransRenderPerceptual;
		transform = SpTransTypeIn;
		break;
		
	case (KpInt32_t)SpTagAToB1:
		render = SpTransRenderColorimetric;
		transform = SpTransTypeIn;
		break;
		
	case (KpInt32_t)SpTagAToB2:
		render = SpTransRenderSaturation;
		transform = SpTransTypeIn;
		break;
		
	case (KpInt32_t)SpTagBToA0:
		render = SpTransRenderPerceptual;
		transform = SpTransTypeOut;
		break;
		
	case (KpInt32_t)SpTagBToA1:
		render = SpTransRenderColorimetric;
		transform = SpTransTypeOut;
		break;
		
	case (KpInt32_t)SpTagBToA2:
		render = SpTransRenderSaturation;
		transform = SpTransTypeOut;
		break;

	case (KpInt32_t)SpTagGamut:
		render = SpTransRenderAny;
		transform = SpTransTypeGamut;
		break;

	case (KpInt32_t)SpTagPreview0:
		render = SpTransRenderPerceptual;
		transform = SpTransTypeSim;
		break;

	case (KpInt32_t)SpTagPreview1:
		render = SpTransRenderColorimetric;
		transform = SpTransTypeSim;
		break;

	case (KpInt32_t)SpTagPreview2:
		render = SpTransRenderSaturation;
		transform = SpTransTypeSim;
		break;

	default:
		st = SpStatOutOfRange;
		return st;
	}
	
	SpXformGet (pf, render, transform, &xform);
	st = SpProfileGetHeader (pf, &hdr);
	if (st == SpStatSuccess) {
		oldOrg = hdr.Originator;
	    hdr.Originator = newOrg;
		st = SpProfileSetHeader (pf, &hdr);
	}
	if (st == SpStatSuccess) {
		SpXformSet (pf, 16, render, transform, xform);
	    hdr.Originator = oldOrg;
		st = SpProfileSetHeader (pf, &hdr);
	}
	SpXformFree(&xform);
	return st;
}


#endif
