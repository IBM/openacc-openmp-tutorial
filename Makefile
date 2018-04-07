
.PHONY: clean all video run run-debug run-nvprof results validation

COMPILER=pgi

LIBS=-lm -lpng
ifeq ($(COMPILER),gnu)
	CC=gcc
	#OPTFLAGS=-O0 -g
	OPTFLAGS=-O3 -g
	CFLAGS=-std=c99 -fopenmp $(OPTFLAGS)
	LDFLAGS=$(LIBS) -fopenmp
	#CFLAGS=-std=c99 $(OPTFLAGS)
	#LDFLAGS=$(LIBS)
endif
ifeq ($(COMPILER),pgi)
	CC=pgcc
	OPTFLAGS=-O3 -fast
#	GPUFLAGS=-ta=tesla:cc35,cc60,cuda9.0 -Minfo=accel,inline
	GPUFLAGS=-ta=tesla:cc35,cc60,cuda9.0,pinned -Minfo=accel,inline
#	GPUFLAGS=-ta=tesla:cc35,cc60,cuda9.0,managed -Minfo=accel,inline
	CFLAGS=-mp $(OPTFLAGS) $(GPUFLAGS)
	LDFLAGS=$(LIBS) -mp $(GPUFLAGS)
endif

SRCS=main.c support.c model.c writepng.c render.c logging.c

OBJS=$(SRCS:.c=.o)

all: main

video: out/out.mp4

out/out.mp4: $(wildcard out/iteration*.png)
	ffmpeg -r 30 -i out/iteration.%04d.png -c:v libx264 -movflags faststart -profile:v high -bf 2 -g 15 -coder 1 -crf 18 -pix_fmt yuv420p -r 30 $@ -y

$(OBJS): Makefile

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

main: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

clean:
	rm -f $(OBJS) main

run: out/log.txt

out/log.txt: main
	./run

run-debug: main
	SANDBOX=cuda-gdb ./run

run-nvprof: main
	SANDBOX="nvprof -f -o profile.nvvp" ./run

results: validation

out/log.txt: main
validation: out/log.txt out/log-timings.txt
	@python -c "import tabulate" 2>/dev/null || pip install --user tabulate
	@python ./validate.py

model.o: model.h
main.o: main.h
render.o: render.h
support.o: support.h
writepng.o: writepng.h
logging.o: logging.h
