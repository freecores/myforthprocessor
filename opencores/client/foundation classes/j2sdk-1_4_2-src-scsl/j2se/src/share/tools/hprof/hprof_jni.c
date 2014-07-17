/*
 * @(#)hprof_jni.c	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "hprof.h"

static hprof_globalref_t *hprof_globalrefs = NULL;

static void
hprof_globalref_add(hprof_objmap_t *obj_id, 
		    jobject ref_id, 
		    unsigned int trace_serial_num)
{
    hprof_globalref_t *globalref =
	HPROF_CALLOC(ALLOC_TYPE_GLOBALREF, sizeof(hprof_globalref_t));
    globalref->obj_id = obj_id;
    globalref->ref_id = ref_id;
    globalref->trace_serial_num = trace_serial_num;
    globalref->next = hprof_globalrefs;
    hprof_globalrefs = globalref;
}

static void 
hprof_globalref_del(jobject ref_id)
{
    hprof_globalref_t **p = &hprof_globalrefs;
    hprof_globalref_t *globalref;
    while ((globalref = *p)) {
        if (globalref->ref_id == ref_id) {
	    *p = globalref->next;
	    hprof_free(globalref);
	    return;
	}
	p = &(globalref->next);
    }
}

hprof_globalref_t * hprof_globalref_find(jobject ref_id)
{
    hprof_globalref_t *globalref = hprof_globalrefs;
    while (globalref) {
        if (globalref->ref_id == ref_id) {
	    return globalref;
	}
	globalref = globalref->next;
    }
    return NULL;
}

void hprof_jni_globalref_alloc_event(JNIEnv *env_id, jobjectID obj_id, jobject ref_id)
{
    hprof_trace_t *htrace;
    hprof_objmap_t *objmap;
    
    CALL(RawMonitorEnter)(data_access_lock);
    htrace = hprof_get_trace(env_id, max_trace_depth);
    if (htrace == NULL) {
        fprintf(stderr, "HPROF ERROR: got NULL trace in jni_globalref_alloc\n");
	goto globalref_alloc_done;
    } 
    
    objmap = hprof_fetch_object_info(obj_id);
    if (objmap == NULL) {
        fprintf(stderr, "HPROF ERROR: unable to map JVMPI obj ID to hprof "
		"obj ID in globalref_alloc \n");
	goto globalref_alloc_done;
    }
   
    hprof_globalref_add(objmap, ref_id, htrace->serial_num);

 globalref_alloc_done:
    CALL(RawMonitorExit)(data_access_lock);
}
    
void hprof_jni_globalref_free_event(JNIEnv *env_id, jobject ref_id)
{
    hprof_trace_t *htrace;
    
    CALL(RawMonitorEnter)(data_access_lock); 
    hprof_globalref_del(ref_id);
    CALL(RawMonitorExit)(data_access_lock);
}

