#include <stdlib.h>
#include <sys/time.h>
#include <stdarg.h>
#include <string.h>
#include <cuda.h>
#include <driver_types.h>

#include "support.h"
#include "nvtx_markers.h"

int getenvl(const char* name, int def) {
  char *a=getenv(name);
  if (a) return atol(a); else return def;
}

void setup_devices() {
  size_t myStackSize = 12*1024;
  cudaError_t stat = cudaDeviceSetLimit (cudaLimitStackSize, myStackSize);
}


double now() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec+tv.tv_usec*1e-6;
}

int num_sections=0;
struct section sections[MAX_SECTIONS];

int section_indexOf(const char * name) {
	for (int i=0;i<num_sections;i++){
		if (strcmp(name,sections[i].name)==0) return i;
	}
	return -1;
}

int section_add(const char *name) {
	struct section *s=&sections[num_sections];
	s->name=name;
	s->count=0;
	s->count_iteration=0;
	s->total=0;
	s->total_iteration=0;
	return num_sections++;
}

int section_start(const char *name) {
	int i=section_indexOf(name);
	if (i<0) i=section_add(name);
	struct section *s=&sections[i];
	s->start=now();
	NV_PUSH_RANGE(name);
	return i;
}

void section_end(int i) {
	struct section *s=&sections[i];
	NV_POP_RANGE();
	double end=now();
	double dt=end-s->start;
	s->total+=dt;
	s->total_iteration+=dt;
	s->end=end;
	s->count++;
	s->count_iteration++;
}

void sections_next_iteration() {
	for (int i=0;i<num_sections;i++) {
		sections[i].count_iteration=0;
		sections[i].total_iteration=0;
	}
}

