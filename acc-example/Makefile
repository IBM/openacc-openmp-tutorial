CC=pgcc
OPTFLAGS=-O3 -fast
GPUFLAGS=-ta=tesla:cc35,cc60,cuda9.0,pinned -Minfo=accel,inline
#GPUFLAGS=-ta=tesla:cc35,cc60,cuda9.0,managed -Minfo=accel,inline
CFLAGS=-mp $(OPTFLAGS) $(GPUFLAGS)
LDFLAGS=-mp $(GPUFLAGS)

SRCS=main.c

OBJS=$(SRCS:.c=.o)

TARGETS=$(OBJS) main

.PHONY: all clean run run-debug run-nvprof

all: main

$(TARGETS): Makefile

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

main: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

clean:
	rm -f $(TARGETS)

run: main
	${SUBMIT_COMMAND} ./run

run-debug: main
	SANDBOX=cuda-gdb ${SUBMIT_COMMAND} ./run

run-nvprof: main
	SANDBOX="nvprof -f -o profile.nvvp" ${SUBMIT_COMMAND} ./run
