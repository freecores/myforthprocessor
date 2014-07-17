
GCC      = gcc

SOURCES  = proc_service.c          \
           utils.c                 \
           LinuxDebuggerLocal.c

OBJS     = $(SOURCES:.c=.o)

INCLUDES = -I${JAVA_HOME}/include -I${JAVA_HOME}/include/linux
LIBS     = -lelf -lthread_db

CFLAGS   = -c -g -D_GNU_SOURCE $(INCLUDES)

LIBSA = libsa.so

all: $(LIBSA)

$(LIBSA): $(OBJS)
	$(GCC) -shared -o $(LIBSA) $(OBJS) $(LIBS)

.c.obj:
	$(GCC) $(CFLAGS)

clean:
	rm -rf $(LIBSA)
	rm -rf $(OBJS)

test: proc_service.c utils.c test_debugger.c
	$(GCC) -o test -D_GNU_SOURCE $(INCLUDES) proc_service.c utils.c test_debugger.c $(LIBS)
