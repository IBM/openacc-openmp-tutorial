#ifndef LOGGING_H
#define LOGGING_H

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <float.h>

#include "model.h"
#include "support.h"
#include "logging.h"


FILE* fp_log;
FILE* fp_log_leaders;
FILE* fp_log_timings;

void setup_logging() {
      char filename[4096];
      sprintf(filename,"%s/log.txt",params.output_dir);
      fp_log=fopen(filename, "w+");
      sprintf(filename,"%s/log-leaders.txt",params.output_dir);
      fp_log_leaders=fopen(filename, "w+");
      sprintf(filename,"%s/log-timings.txt",params.output_dir);
      fp_log_timings=fopen(filename, "w+");
}

void done_logging() {
      fclose(fp_log);
      fclose(fp_log_leaders);
      fclose(fp_log_timings);
}

void print_leaders(FILE* f, int iteration) {
	if (iteration==0) {
		fprintf(f,"# iteration [insect_counts]\n");
	}
	int leader_idx, n, ntot=0;
	fprintf(f,"%i ",iteration);
	for (int i=0;i<NumLeaders;i++) {
		leader_idx=leaders[i].insect_idx;
		int leader_id=i;
		n=1+count_children(leader_idx);
		fprintf(f," %d",n);
		ntot+=n;
	}
	fprintf(f,"\n");
}

void print_parent_chain(int p_idx) {
	int i=p_idx;
	while (i>=0) {
		printf("               %6d (leader_id=%d, leader_idx=%d, nchildren=%d, children={",i,insects[i].leader_id,insects[i].leader_idx,insects[i].nchildren);
		for (int k=0;k<insects[i].nchildren;k++) {
			if (k>0) printf(", ");
			printf("%5d",insects[i].children[k]);
		}
		printf("})\n");
		i=insects[i].parent;
	}
}

int replacechar(char *str, char orig, char rep) {
    char *ix = str;
    int n = 0;
    while((ix = strchr(ix, orig)) != NULL) {
        *ix++ = rep;
        n++;
    }
    return n;
}

void print_timings(FILE *f,int iteration) {
	char nn[4096];
	if (iteration==0) {
		fprintf(f,"# iteration");
		for (int i=0;i<num_sections;i++) {
			strcpy(nn,sections[i].name);
			if (strcmp(nn,"model")==0 || strcmp(nn,"image")==0) {
				replacechar(nn,' ','_');
				fprintf(f," %s.count %s.time %s.count_total %s.time_total",nn,nn,nn,nn);
			}
		}
		fprintf(f,"\n");
	}
	fprintf(f,"%3d ",iteration);
	for (int i=0;i<num_sections;i++) {
		const char *n=sections[i].name;
		if (strcmp(n,"model")==0 || strcmp(n,"image")==0) {
			fprintf(f,"%2d %e %2d %e ",sections[i].count_iteration, sections[i].total_iteration, sections[i].count, sections[i].total);
		}
	}
	fprintf(f,"\n");
}

void log_iteration(int iteration) {
	print_leaders(fp_log_leaders,iteration);
	print_timings(fp_log_timings,iteration);
	sections_next_iteration();
	FILE *f=fp_log;
	if (iteration==0) {
		fprintf(f,"# iteration");
		//fprintf(f," mass_min mass_max");
		//fprintf(f," max_force_x max_force_y max_force_z");
		//fprintf(f," total_force_x total_force_y total_force_z");
		fprintf(f," cms_x cms_y cms_z cms_vy cms_vy cms_vz mass");
		fprintf(f," kinetic_energy");
		fprintf(f,"\n");
	}
	struct insect_data_double cms={0};
	struct insect_action_data sum={0};
	struct insect_action_data max={0};
	float minm=+INFINITY;
	float maxm=-INFINITY;
	double E=0;
	for (int i=0;i<NumInsects;i++) {
		struct insect_action_data* a=&actions[i];
		struct insect_data* p=&insects[i];
		sum.fx+=a->fx;
		sum.fy+=a->fy;
		sum.fz+=a->fz;
		max.fx=MAX(max.fx,a->fx);
		max.fy=MAX(max.fy,a->fy);
		max.fz=MAX(max.fz,a->fz);
		minm=MIN(minm,p->m);
		maxm=MAX(maxm,p->m);
		cms.x +=p->x *p->m;
		cms.y +=p->y *p->m;
		cms.z +=p->z *p->m;
		cms.vx+=p->vx*p->m;
		cms.vy+=p->vy*p->m;
		cms.vz+=p->vz*p->m;
		E+=0.5*p->m*(p->vx*p->vx+p->vy*p->vy+p->vz*p->vz);
		cms.m+=p->m;
	}
	//fprintf(fp_log," %e %e ",minm,maxm);
	//fprint_insect_action_data(fp_log,&max);
	//fprintf(fp_log," ");
	//fprint_insect_action_data(fp_log,&sum);
	fprintf(fp_log," %4d ",iteration);
	cms.x /=cms.m;
	cms.y /=cms.m;
	cms.z /=cms.m;
	cms.vx/=cms.m;
	cms.vy/=cms.m;
	cms.vz/=cms.m;
	fprint_insect_data_double(fp_log,&cms);
	fprintf(fp_log," %.*le",DECIMAL_DIG,E);
	fprintf(fp_log,"\n");
	fflush(fp_log);
	fflush(fp_log_leaders);
	fflush(fp_log_timings);
	printf("iteration %d\n",iteration);
}

void fprint_insect_data_double(FILE* stream, struct insect_data_double* p) {
	fprintf(stream,"%+.*le %+.*le %+.*le %+.*le %+.*le %+.*le %.*le",DECIMAL_DIG,p->x,DECIMAL_DIG,p->y,DECIMAL_DIG,p->z,DECIMAL_DIG,p->vx,DECIMAL_DIG,p->vy,DECIMAL_DIG,p->vz, DECIMAL_DIG,p->m);
}

void fprint_insect_data(FILE* stream, struct insect_data* p) {
	fprintf(stream,"%+.*e %+.*e %+.*e %+.*e %+.*e %+.*e %.*e",DECIMAL_DIG,p->x,DECIMAL_DIG,p->y,DECIMAL_DIG,p->z,DECIMAL_DIG,p->vx,DECIMAL_DIG,p->vy,DECIMAL_DIG,p->vz, DECIMAL_DIG,p->m);
}

void print_insect_data(struct insect_data* p) {
	fprint_insect_data(stdout,p);
}

void print_insect_action_data(struct insect_action_data* p) {
	fprint_insect_action_data(stdout,p);
}

void fprint_insect_action_data(FILE *stream,struct insect_action_data* p) {
	fprintf(stream,"%+.*e %+.*e %+.*e",DECIMAL_DIG,p->fx,DECIMAL_DIG,p->fy,DECIMAL_DIG,p->fz);
}

void print_model(int i, struct insect_data* p,struct insect_action_data* a) {
	printf("%d %+.9e %+.9e %+.9e  %+.9e %+.9e %+.9e  %+.9e %+.9e\n",i, p->x,p->y,p->z,p->vx,p->vy,p->vz,a->fx,a->fy,a->fz);
}

void print_p(int i) {
	struct insect_data* p=&insects[i];
	struct insect_action_data* a=&actions[i];
	printf("%d %+.9e %+.9e %+.9e  %+.9e %+.9e %+.9e  %+.9e %+.9e\n",i, p->x,p->y,p->z,p->vx,p->vy,p->vz,a->fx,a->fy,a->fz);
}


#endif
