CC=pgcc
OPTFLAGS=-O3 -fast
GPUFLAGS=-ta=tesla:cc35,cc60,cuda9.0,pinned -Minfo=accel,inline
#GPUFLAGS=-ta=tesla:cc35,cc60,cuda9.0,managed -Minfo=accel,inline
CFLAGS=-mp $(OPTFLAGS) $(GPUFLAGS)
LDFLAGS=-mp $(GPUFLAGS)

SRCS=main.c

OBJS=$(SRCS:.c=.o)

all: main main-async

$(OBJS): Makefile

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

main: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

clean:
	rm -f $(OBJS) main main-async

run: main
	./$<

run-debug: main
	cuda-gdb ./$<

profile.nvvp: main
	nvprof -f -o $@ ./$<

profile-async.nvvp: main-async
	nvprof -f -o $@ ./$<

run-nvprof: profile.nvvp profile-async.nvvp
