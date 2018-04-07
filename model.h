#ifndef MODEL_H
#define MODEL_H

#include <stdio.h>

#define MAX_CHILDREN 8
#define MAX_NUM_LEADERS 1024

struct model_parameters {
	float x0,y0,z0,r0;
	float lx,ly,lz;
	float dt;

	float grouping_radius;
	float grouping_constant;
	
	float coulomb_constant;
	float coulomb_radius;

	float damping_constant;

	float attack_radius;
	float attack_constant;
	float defend_constant;

	float fight_radius;
	float fight_mass_rate;
	float surrender_mass_ratio;

	float center_force_constant;

	float mass_min;

	float num_insects;
	int max_tree_depth;

	int num_iterations;

	char* output_dir;
};

extern struct model_parameters params;

struct leader_data {
	int id;                      // the leader's index in leaders[]
	int insect_idx;              // the leader's index in insects[]
	float hue;                   // hue value for color of the leader's insects
};

extern struct leader_data *leaders;
extern int NumLeaders;

struct insect_data {
	float x,y,z,vx,vy,vz,m;      // 3D coordinates and velocities and mass
	int parent;                  // index to the parent insect
	int nchildren;               // number of children
	int children[MAX_CHILDREN];  // indices of the children
	int leader_idx;              // index to the leader insect
	int leader_id;               // the leader insect's index in leaders[]
};

struct insect_data_double {
	float x,y,z,vx,vy,vz,m;
	int parent;
	int nchildren;
	int children[MAX_CHILDREN];
	int leader_idx;
	int leader_id;
};

extern int NumInsects;

extern struct insect_data *insects;

struct insect_action_data {
	float fx,fy,fz,rm;           // 3D forces, and mass rate
	int new_parent;              // index of new parent in next iteration
};

extern struct insect_action_data *actions;

void setup_model();
void iteration();
int count_children(int idx);
void model_enable_rivalism();

#endif
