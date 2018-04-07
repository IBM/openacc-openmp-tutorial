#ifndef SUPPORT_H
#define SUPPORT_H

#include <assert.h>
#include <stdio.h>

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

extern int ThisTask;

struct section {
	double total,total_iteration;
	double start;
	double end;
	int count,count_iteration;
	const char* name;
};

#define MAX_SECTIONS 1024
extern int num_sections;
extern struct section sections[MAX_SECTIONS];

int getenvl(const char* name, int def);
void setup_devices();
double now();
int section_start(const char *name);
void section_end(int i);
void sections_next_iteration();

#ifdef DEBUG
#define printd(format,...) printf(format,__VA_ARGS__)
#else
#define printd(format,...)
#endif

#endif
