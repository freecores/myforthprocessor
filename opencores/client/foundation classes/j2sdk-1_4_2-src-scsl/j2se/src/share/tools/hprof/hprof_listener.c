/*
 * @(#)hprof_listener.c	1.12 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "hprof.h"

#define HPROF_CMD_EOF ((unsigned char)-1)  /* pseudo command */

static jint 
recv_fully(int f, char *buf, int len)
{
    jint nbytes = 0;
    while (nbytes < len) {
        int res = recv(f, buf + nbytes, (SIZE_T)(len - nbytes), 0);
	if (res < 0) {
	    if (hprof_is_on) {
	        hprof_is_on = FALSE;
	        fprintf(stderr,
			"HPROF ERROR: failed to read cmd from socket\n");
                CALL(ProfilerExit)((jint)1);
	    }
            /*
             * hprof was disabled before we returned from recv() above.
             * This means the command socket is closed so we let that
             * trickle back up the command processing stack.
             */
            break;
	}
	nbytes += res;
    }
    return nbytes;
}

static unsigned char recv_u1(void)
{
    unsigned char c;
    jint nbytes = recv_fully(hprof_fd, (char *)&c, sizeof(unsigned char));
    if (nbytes == 0) {
        c = HPROF_CMD_EOF;
    }
    return c;
}

static unsigned short recv_u2(void)
{
    unsigned short s;
    jint nbytes = recv_fully(hprof_fd, (char *)&s, sizeof(unsigned short));
    if (nbytes == 0) {
        s = (unsigned short)-1;
    }
    return ntohs(s);
}

static unsigned int recv_u4(void)
{
    unsigned int i;
    jint nbytes = recv_fully(hprof_fd, (char *)&i, sizeof(unsigned int));
    if (nbytes == 0) {
        i = (unsigned int)-1;
    }
    return ntohl(i);
}

static void * recv_id(void)
{
    void *result;
    jint nbytes = recv_fully(hprof_fd, (char *)&result, sizeof(void *));
    if (nbytes == 0) {
        result = (void *)-1;
    }
    return result;
}

/* the callback thread */
static void
hprof_callback(void *p)
{
    while (hprof_is_on) {
        jint          length;
        jint          seq_num;
        unsigned char tag;

        tag = recv_u1();
        if (tag == HPROF_CMD_EOF) {
            /* the cmd socket has closed so the listener thread is done */
            break;
        }
        seq_num = recv_u4();
        length  = recv_u4();

	switch (tag) {
	case HPROF_CMD_GC:
	    CALL(RunGC)();
	    break;
	case HPROF_CMD_DUMP_HEAP: {
	    hprof_get_heap_dump();
	    break;
	}
	case HPROF_CMD_ALLOC_SITES: {
	    unsigned short flags = recv_u2();
	    unsigned int i_tmp = recv_u4();
	    float ratio = *(float *)(&i_tmp);
	    hprof_output_sites(flags, ratio);
	    break;
	}
	case HPROF_CMD_HEAP_SUMMARY: {
	    CALL(RawMonitorEnter)(data_access_lock);
	    hprof_write_header(HPROF_HEAP_SUMMARY, 24);
	    hprof_write_u4(total_live_bytes);
	    hprof_write_u4(total_live_instances);
	    hprof_write_u4(jlong_high(total_alloced_bytes));
	    hprof_write_u4(jlong_low(total_alloced_bytes));
	    hprof_write_u4(jlong_high(total_alloced_instances));
	    hprof_write_u4(jlong_low(total_alloced_instances));
	    CALL(RawMonitorExit)(data_access_lock);
	    break;
	}
	case HPROF_CMD_EXIT:
	    hprof_is_on = FALSE;
	    fprintf(stderr, 
		    "HPROF: received exit event, exiting ...\n");
	    CALL(ProfilerExit)((jint)0);
	case HPROF_CMD_DUMP_TRACES:
	    CALL(RawMonitorEnter)(data_access_lock);
	    hprof_output_unmarked_traces();
	    CALL(RawMonitorExit)(data_access_lock);
	    break;
	case HPROF_CMD_CPU_SAMPLES: {
	    unsigned short flags = recv_u2();
	    unsigned int i_tmp = recv_u4();
	    float ratio = *(float *)(&i_tmp);
	    hprof_output_trace_cost(ratio, CPU_SAMPLES_RECORD_NAME);
	    break;
	}
	case HPROF_CMD_CONTROL: {
	    unsigned short cmd = recv_u2();
	    if (cmd == 0x0001) {
	        CALL(EnableEvent)(JVMPI_EVENT_OBJ_ALLOC, NULL);
	    } else if (cmd == 0x0002) {
	        CALL(DisableEvent)(JVMPI_EVENT_OBJ_ALLOC, NULL);
	    } else if (cmd == 0x0003) {
	        hprof_objmap_t *thread_id = recv_id(); 
		hprof_cpu_sample_on(thread_id);
	    } else if (cmd == 0x0004) {
		hprof_objmap_t *thread_id = recv_id();
		hprof_cpu_sample_off(thread_id);
	    } else if (cmd == 0x0005) {
	        CALL(RawMonitorEnter)(data_access_lock);
	        hprof_clear_trace_cost();
		CALL(RawMonitorExit)(data_access_lock);
	    } else if (cmd == 0x0006) {
	        CALL(RawMonitorEnter)(data_access_lock);
		hprof_clear_site_table();
		CALL(RawMonitorExit)(data_access_lock);
	    } else if (cmd == 0x0007) {
	        max_trace_depth = recv_u2();
	    }
	    break;
	}
	default:
	    if (hprof_is_on) {
		hprof_is_on = FALSE;
	        fprintf(stderr,
			"HPROF ERROR : failed to recognize cmd %d, exiting..\n",
			(int)tag);
	    }
	    CALL(ProfilerExit)((jint)1);
	}
	CALL(RawMonitorEnter)(data_access_lock);
	hprof_flush();
	CALL(RawMonitorExit)(data_access_lock);
    }
}

void hprof_start_listener_thread(void)
{
    if (hprof_socket_p) {
        if (CALL(CreateSystemThread)("Hprof listener",
				     JVMPI_MAXIMUM_PRIORITY, 
				     hprof_callback) == JNI_ERR) {
	    fprintf(stderr, "HPROF ERROR: unable to create listener thread\n");
	}
    }
}
