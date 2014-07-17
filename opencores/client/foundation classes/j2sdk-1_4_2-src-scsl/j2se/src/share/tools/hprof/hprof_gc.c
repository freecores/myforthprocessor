/*
 * @(#)hprof_gc.c	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "hprof.h"

void hprof_gc_start_event(JNIEnv *env_id)
{
    jlong start_time;
    hprof_thread_local_t *info; 

    /* ignore GC events if the VM shuts down */
    if (jvm_shut_down) return;

    /* grab the data_access_lock here, because obj_move and obj_free come
     * in single threaded mode and we could run into deadlocks by grabbing
     * the lock in those handlers. */
    CALL(RawMonitorEnter)(data_access_lock);  

    if (cpu_timing) {
        start_time = CALL(GetCurrentThreadCpuTime)();
	info = (hprof_thread_local_t *)(CALL(GetThreadLocalStorage)(env_id));
	if (info == NULL) {
	    fprintf(stderr, "HPROF ERROR: gc_start on an unknown thread %p\n",
		    env_id);
            CALL(RawMonitorExit)(data_access_lock); /* release lock before return */
	    return;
	}

	if (info->gc_start_time != (jlong)(-1)) {
	    fprintf(stderr, "HPROF ERROR: got gc_start instead of gc_end\n");
            CALL(RawMonitorExit)(data_access_lock); /* release lock before return */
	    return;
	}
    
	info->gc_start_time = start_time;
    }
}
 
void hprof_gc_finish_event(JNIEnv *env_id, jlong used_objects,
			   jlong used_object_space, jlong total_object_space)
{
    jlong gc_time;
    hprof_thread_local_t *info;

    /* ignore GC events if the VM shuts down */
    if (jvm_shut_down) return;

    if (cpu_timing) { /* for subtracting GC time from method time */
        info = (hprof_thread_local_t *)(CALL(GetThreadLocalStorage)(env_id));
    
	if (info == NULL) {
	    fprintf(stderr, "HPROF ERROR: gc_end on an unknown thread %p\n",
		    env_id);
	    return;
	}

	if (info->gc_start_time == (jlong)-1) {
	    fprintf(stderr, "HPROF ERROR: got gc_end instead of gc_start\n");
	    return;
	}

	gc_time = CALL(GetCurrentThreadCpuTime)() - info->gc_start_time;
	
        if ((info->stack_top - info->stack) > 0) {
	    (info->stack_top - 1)->time_in_gc +=  gc_time;
	}
	info->gc_start_time = (jlong)-1; /* reset gc_start_time */
    }
    
    CALL(RawMonitorExit)(data_access_lock); /* we grabbed this in gc_start */
}
    
    
  
