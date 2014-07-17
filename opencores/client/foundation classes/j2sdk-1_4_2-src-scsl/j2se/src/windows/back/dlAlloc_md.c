/*
 * @(#)dlAlloc_md.c	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * Platform-specific implementation for Doug Lea's malloc. See dlAlloc.c for details.
 */
 
/* 
  Emulation of sbrk for WIN32
  All code within the ifdef WIN32 is untested by me.
*/
#include "dlAlloc_md.h"

#define AlignPage(add) (((add) + (malloc_getpagesize-1)) & ~(malloc_getpagesize-1))

/* resrve 64MB to insure large contiguous space */ 
#define RESERVED_SIZE (1024*1024*64)
#define NEXT_SIZE (2048*1024)

#ifdef _M_IA64
#define TOP_MEMORY ((INTERNAL_SIZE_T)0x8000000000000000)
#else
#define TOP_MEMORY ((INTERNAL_SIZE_T)2*1024*1024*1024)
#endif

struct GmListElement;
typedef struct GmListElement GmListElement;

struct GmListElement 
{
        GmListElement* next;
        void* base;
};

static GmListElement* head = 0;
static INTERNAL_SIZE_T gNextAddress = 0;
static INTERNAL_SIZE_T gAddressBase = 0;
static INTERNAL_SIZE_T gAllocatedSize = 0;

static
GmListElement* makeGmListElement (void* bas)
{
        GmListElement* this;
        this = (GmListElement*)(void*)LocalAlloc (0, sizeof (GmListElement));
        ASSERT (this);
        if (this)
        {
                this->base = bas;
                this->next = head;
                head = this;
        }
        return this;
}

void gcleanup ()
{
        BOOL rval;
        ASSERT ( (head == NULL) || (head->base == (void*)gAddressBase));
        if (gAddressBase && (gNextAddress - gAddressBase))
        {
                rval = VirtualFree ((void*)gAddressBase, 
                                                        gNextAddress - gAddressBase, 
                                                        MEM_DECOMMIT);
        ASSERT (rval);
        }
        while (head)
        {
                GmListElement* next = head->next;
                rval = VirtualFree (head->base, 0, MEM_RELEASE);
                ASSERT (rval);
                LocalFree (head);
                head = next;
        }
}
                
static
void* findRegion (void* start_address, INTERNAL_SIZE_T size)
{
        MEMORY_BASIC_INFORMATION info;
        while ((INTERNAL_SIZE_T) start_address < TOP_MEMORY)
        {
                VirtualQuery (start_address, &info, sizeof (info));
                if (info.State != MEM_FREE)
                        start_address = (char*)info.BaseAddress + info.RegionSize;
                else if (info.RegionSize >= size)
                        return start_address;
                else
                        start_address = (char*)info.BaseAddress + info.RegionSize; 
        }
        return NULL;
        
}


void* wsbrk (INTERNAL_SSIZE_T size)
{
        void* tmp;
        if (size > 0)
        {
                if (gAddressBase == 0)
                {
                        gAllocatedSize = max (RESERVED_SIZE, AlignPage (size));
                        gNextAddress = gAddressBase = 
                                (INTERNAL_SIZE_T)VirtualAlloc (NULL, gAllocatedSize, 
                                                                                        MEM_RESERVE, PAGE_NOACCESS);
                } else if (AlignPage (gNextAddress + size) > (gAddressBase +
gAllocatedSize))
                {
                        INTERNAL_SIZE_T new_size = max (NEXT_SIZE, AlignPage (size));
                        void* new_address = (void*)(gAddressBase+gAllocatedSize);
                        do 
                        {
                                new_address = findRegion (new_address, new_size);
                                
                                if (new_address == 0)
                                        return (void*)-1;

                                gAddressBase = gNextAddress =
                                        (INTERNAL_SIZE_T)VirtualAlloc (new_address, new_size,
                                                                                                MEM_RESERVE, PAGE_NOACCESS);
                                // repeat in case of race condition
                                // The region that we found has been snagged 
                                // by another thread
                        }
                        while (gAddressBase == 0);

                        ASSERT (new_address == (void*)gAddressBase);

                        gAllocatedSize = new_size;

                        if (!makeGmListElement ((void*)gAddressBase))
                                return (void*)-1;
                }
                if ((size + gNextAddress) > AlignPage (gNextAddress))
                {
                        void* res;
                        res = VirtualAlloc ((void*)AlignPage (gNextAddress),
                                                                (size + gNextAddress - 
                                                                 AlignPage (gNextAddress)), 
                                                                MEM_COMMIT, PAGE_READWRITE);
                        if (res == 0)
                                return (void*)-1;
                }
                tmp = (void*)gNextAddress;
                gNextAddress = (INTERNAL_SIZE_T)tmp + size;
                return tmp;
        }
        else if (size < 0)
        {
                INTERNAL_SIZE_T alignedGoal = AlignPage (gNextAddress + size);
                /* Trim by releasing the virtual memory */
                if (alignedGoal >= gAddressBase)
                {
                        VirtualFree ((void*)alignedGoal, gNextAddress - alignedGoal,  
                                                 MEM_DECOMMIT);
                        gNextAddress = gNextAddress + size;
                        return (void*)gNextAddress;
                }
                else 
                {
                        VirtualFree ((void*)gAddressBase, gNextAddress - gAddressBase,
                                                 MEM_DECOMMIT);
                        gNextAddress = gAddressBase;
                        return (void*)-1;
                }
        }
        else
        {
                return (void*)gNextAddress;
        }
}


