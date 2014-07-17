/*
 * @(#)hprof_cpu.c	1.15 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jvmpi.h"
#include "hprof.h"

static JVMPI_RawMonitor hprof_cpu_lock;
static int              pause_cpu_sampling = FALSE;

void
hprof_cpu_sample_off(hprof_objmap_t *thread_id)
{
    int keep_sampling = FALSE;

    if (thread_id != NULL) {
        if (live_thread_list != NULL) {
	    live_thread_t *list;
	    CALL(RawMonitorEnter)(thread_list_lock);
	    for(list = live_thread_list; list; list = list->next) {
	        if (list->tid == thread_id) {
		    list->cpu_sampled = 0;
		}
		if (list->cpu_sampled) {
                    /* still sampling on at least one thread */
                    keep_sampling = TRUE;
		}
	    }
	    CALL(RawMonitorExit)(thread_list_lock);
	}
    }
    pause_cpu_sampling = !keep_sampling;
}

/* handler for enabling cpu sampling */
void
hprof_cpu_sample_on(hprof_objmap_t *thread_id)
{
    hprof_start_cpu_sampling_thread();
    if (thread_id == NULL) {
        cpu_sampling = TRUE;
        pause_cpu_sampling = FALSE;
    } else {
        /* turn on cpu sampling for the specified thread */
        if (live_thread_list) {
	    live_thread_t *list;
	    CALL(RawMonitorEnter)(thread_list_lock);
	    for(list = live_thread_list; list; list = list->next) {
	        if (list->tid == thread_id) {
		    list->cpu_sampled = 1;
		    cpu_sampling = TRUE;
                    pause_cpu_sampling = FALSE;
		}
	    }
	    CALL(RawMonitorExit)(thread_list_lock);
	}
    }
    if (hprof_cpu_lock) {
        /* notify the CPU sampling thread */
        CALL(RawMonitorEnter)(hprof_cpu_lock);
	CALL(RawMonitorNotifyAll)(hprof_cpu_lock);
	CALL(RawMonitorExit)(hprof_cpu_lock);
    }
}

static void
hprof_cpu_loop(void *p)
{
    float avg_sample_time = 1;
    jlong last_sample_time = 1;
    jlong pause_time = 1;
    JNIEnv *my_env;
    
    (*jvm)->GetEnv(jvm, (void **)&my_env, JNI_VERSION_1_2);
    
    hprof_cpu_lock = CALL(RawMonitorCreate)("_Hprof CPU sampling lock");

    CALL(RawMonitorEnter)(hprof_cpu_lock);

    while (1) {
        CALL(RawMonitorEnter)(hprof_dump_lock);
        if (dump_in_process) {
            pause_cpu_sampling = TRUE;
        }
        CALL(RawMonitorExit)(hprof_dump_lock);

	avg_sample_time = last_sample_time * 0.1 + avg_sample_time * 0.9;

        if (!pause_cpu_sampling) {
	    /* Adjust the sampling interval according to the cost of
	     * each sample.
	     */
	    jlong avg_time = (jlong) avg_sample_time;
	    if (avg_time == 0) {
	        avg_time = 1;
	    }

	    if (avg_time > pause_time * 2) {
	        pause_time = avg_time;
	    }
	    if (pause_time > 1 && avg_time < pause_time) {
	        pause_time = avg_time;
	    }
	    CALL(RawMonitorWait)(hprof_cpu_lock, pause_time);
	} else {
            /*
             * Pause sampling for now. Reset sample controls if
             * sampling is resumed again.
             */
            CALL(RawMonitorWait)(hprof_cpu_lock, 0);
            avg_sample_time = 1;
            last_sample_time = 1;
            pause_time = 1;
	    continue;
	}

	CALL(RawMonitorEnter)(thread_list_lock);
	
	last_sample_time = hprof_get_timemillis();
	
	{
            static jint     listSize = 0;
            static JNIEnv **reqList = NULL;
            static jint    *results = NULL;

	    live_thread_t *list;
	    live_thread_t *suspended_list = NULL;
	    int n_traces = 0;
	    int i, j;
	    JVMPI_CallTrace *traces;
            jint thread_status;
            jint reqCnt;

            if (can_use_suspend_resume_thread_lists) {
                int needAlloc = 0;

                if (listSize == 0) {
                    /* set initial list size (which may not be big enough) */
                    listSize = HPROF_THREAD_LIST_SIZE;
                    needAlloc = 1;
                } else if (num_live_threads > listSize) {
                    /* old list size is not big enough so free old lists */
                    hprof_free(results);
                    hprof_free(reqList);
                    needAlloc = 1;
                }
                /* make the initial or new list size big enough */
                while (num_live_threads > listSize) {
                    listSize *= 2;
                }
                if (needAlloc) {
                    /* only allocate space when we need it */
                    results = HPROF_CALLOC(ALLOC_TYPE_ARRAY,
                                            listSize * sizeof (jint));
                    reqList = HPROF_CALLOC(ALLOC_TYPE_ARRAY,
                                           listSize * sizeof (JNIEnv *));
                }
            }

	    /* Allocate space for all the traces we might collect, maximum
	       value is number of live threads */
	    traces = HPROF_CALLOC(ALLOC_TYPE_CALLTRACE,
		num_live_threads * sizeof(JVMPI_CallTrace));
	    for (i = 0; i < num_live_threads; i++) {
	        traces[i].frames = HPROF_CALLOC(ALLOC_TYPE_CALLFRAME,
		    max_trace_depth * sizeof(JVMPI_CallFrame));
	    }
	    
            if (can_use_suspend_resume_thread_lists) {
                jint doneCnt;

                reqCnt = 0;
                /* get all runnable threads on a list */
                for (list = live_thread_list; list; list = list->next) {
                    if (list->cpu_sampled && list->env != my_env) {
                        thread_status = CALL(GetThreadStatus)(list->env);
                        if ((thread_status & ~JVMPI_THREAD_INTERRUPTED) == JVMPI_THREAD_RUNNABLE) {
                            reqList[reqCnt++] = list->env;
                        }
                    }
                }

                if (!gc_okay) {
                    CALL(DisableGC)();
                }
                /* suspend all the runnable threads in one shot */
                CALL(SuspendThreadList)(reqCnt, reqList, results);

                /*
                 * Put the suspended threads on the suspended list
                 * and on the request list for the resume below
                 */
                doneCnt = 0;
                i = 0;
                for (list = live_thread_list; list && i < reqCnt; list = list->next) {
                    if (list->env == reqList[i]) {
                        if (results[i] == 0) {
                            list->nextSuspended = suspended_list;
                            suspended_list = list;
                            if (doneCnt != i) {
                                reqList[doneCnt] = reqList[i];
                            }
                            doneCnt++;
                        }
                        i++;
                    }
                }
                reqCnt = doneCnt;  /* set count for resume below */
            } else {
                if (!gc_okay) {
                    CALL(DisableGC)();
                }
                /* suspend all the runnable threads */
                for (list = live_thread_list; list; list = list->next) {
                    if (list->cpu_sampled && list->env != my_env) {
                        thread_status = CALL(GetThreadStatus)(list->env);
                        if ((thread_status & ~JVMPI_THREAD_INTERRUPTED) == JVMPI_THREAD_RUNNABLE) {
                            CALL(SuspendThread)(list->env);
                            list->nextSuspended = suspended_list;
                            suspended_list = list;
                        }
                    }
                }
            }
	    
	    /* get traces for all the threads that were actually running */
	    for (list = suspended_list; list; list = list->nextSuspended) {
		if (CALL(ThreadHasRun)(list->env)) {
		    traces[n_traces].env_id = list->env;
		    CALL(GetCallTrace)(&(traces[n_traces]),
				       max_trace_depth);
		    if (traces[n_traces].num_frames > 0) {
		        n_traces++;
		    }
		}
	    }
       	    
            if (can_use_suspend_resume_thread_lists) {
                CALL(ResumeThreadList)(reqCnt, reqList, results);
            } else {
                /* resume all the suspended threads */
                for (list = suspended_list; list; list = list->nextSuspended) {
                    CALL(ResumeThread)(list->env);
                }
            }
            if (!gc_okay) {
                CALL(EnableGC)();
            }

            /*
             * If a dump request came in after we checked at the top of
             * the "while (1)" loop, then we catch that fact here. We
             * don't want to perturb the data that is being dumped so
             * we just ignore the data from this sampling loop.
             */
            CALL(RawMonitorEnter)(hprof_dump_lock);
            if (dump_in_process) {
                pause_cpu_sampling = TRUE;
            }
            CALL(RawMonitorExit)(hprof_dump_lock);
	    
            /* record all the traces if we aren't dumping at the moment */
            if (n_traces > 0 && !pause_cpu_sampling) {
                CALL(RawMonitorEnter)(data_access_lock);
		for (i = 0; i < n_traces; i++) {
		    JVMPI_CallTrace *jtrace = &(traces[i]);
		    hprof_trace_t *htrace = 
		        hprof_intern_jvmpi_trace(jtrace->frames, 
						 jtrace->num_frames, 
						 jtrace->env_id);
		    if (htrace == NULL) {
		        fprintf(stderr,
				"HPROF ERROR: NULL trace in cpu_sample\n");
		    } else {
		        htrace->num_hits++;
			htrace->cost++;
		    }
		}
                CALL(RawMonitorExit)(data_access_lock);
	    }

	    /* free up the space allocated to collect traces */
	    for(i = 0; i < num_live_threads; i++) {
	        hprof_free(traces[i].frames);
	    }
	    hprof_free(traces);
	}
		
	last_sample_time = hprof_get_timemillis() - last_sample_time;

	CALL(RawMonitorExit)(thread_list_lock);
    }
    /* never reached */
}

void hprof_start_cpu_sampling_thread(void)
{
    static int started = FALSE;
    if (started || cpu_sampling == FALSE)
        return;
    started = TRUE;
    /* start the cpu sampling thread */
    if (CALL(CreateSystemThread)("HPROF CPU profiler",
				 JVMPI_MAXIMUM_PRIORITY,
				 hprof_cpu_loop) < 0) {
        fprintf(stderr, "HPROF ERROR: unable to create CPU sampling thread\n");
    }
}

