CC=pgcc
OPTFLAGS=-O3 -fast
GPUFLAGS=-ta=tesla:cc35,cc60,cuda9.0,pinned -Minfo=accel,inline
#GPUFLAGS=-ta=tesla:cc35,cc60,cuda9.0,managed -Minfo=accel,inline
CFLAGS=-mp $(OPTFLAGS) $(GPUFLAGS)
LDFLAGS=-mp $(GPUFLAGS)

SRCS=main.c

OBJS=$(SRCS:.c=.o)

TARGETS=$(OBJS) main main-async

.PHONY: all clean run run-debug run-nvprof

all: main main-async

$(TARGETS): Makefile

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

main: main.o $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

main-async: main-async.o $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

clean:
	rm -f $(TARGETS)

run: main
	${SUBMIT_COMMAND} ./run

run-debug: main
	SANDBOX=cuda-gdb ${SUBMIT_COMMAND} ./run

profile.nvvp: main
	SANDBOX="nvprof -f -o $@" ${SUBMIT_COMMAND} ./run

profile-async.nvvp: main-async
	SANDBOX="nvprof -f -o $@" ${SUBMIT_COMMAND} ./run

run-nvprof: profile.nvvp profile-async.nvvp
