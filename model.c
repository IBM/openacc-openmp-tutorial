#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <signal.h>

#include "model.h"
#include "support.h"
#include "logging.h"

int NumInsects;
int NumLeaders;
#pragma acc declare create(NumInsects,params,NumLeaders)

struct model_parameters params;
struct leader_data *leaders;

struct insect_data *insects;
struct insect_action_data *actions;

#pragma acc routine
float distance(int a, int b, struct insect_data *restrict insects) {
	float dx,dy,dz,r;
	dx=insects[a].x-insects[b].x;
	dy=insects[a].y-insects[b].y;
	dz=insects[a].z-insects[b].z;
	r=sqrt(dx*dx+dy*dy+dz*dz);
	return r;
}

#pragma acc routine
int relevant_enemy(int leader, int target, struct insect_data *restrict insects) {
	return (distance(leader, target, insects)<=params.attack_radius) && (insects[target].leader_idx>=0);
}

#pragma acc routine
void repell_pair(int target, int partner, struct insect_data *restrict insects, struct insect_action_data *restrict actions) {
	//repell using capped coulomb force
	float dx,dy,dz,r,a,b,c,fx,fy,fz;
	float D=params.coulomb_constant;
	if (partner==target) return;
	dx=insects[target].x-insects[partner].x;
	dy=insects[target].y-insects[partner].y;
	dz=insects[target].z-insects[partner].z;
	r=sqrt(dx*dx+dy*dy+dz*dz);
	float r0=params.coulomb_radius;
	if (r<r0) r=r0;
	a  = D/(r*r*r);
	fx = dx*a;
	fy = dy*a;
	fz = dz*a;
	actions[target].fx+=fx;
	actions[target].fy+=fy;
	actions[target].fz+=fz;
/* don't apply force to partner since it will calculate this by itself
	actions[partner].fx-=fx;
	actions[partner].fy-=fy;
	actions[partner].fz-=fz;
*/
}

#pragma acc routine
void coulomb_repell(int i, struct insect_data *restrict insects, struct insect_action_data *restrict actions) {
	for (int partner=0; partner < NumInsects; partner++) {
		if (insects[i].leader_idx>=0)
			repell_pair(i,partner,insects,actions);
	}
}

void leader_collect_descendants(int leader_id, int insect_idx) {
	struct insect_data *p=&insects[insect_idx];
	struct leader_data *l=&leaders[leader_id];
	int n=p->nchildren;
	for (int i=0;i<p->nchildren;i++) {
		int idx=p->children[i];
		int n=l->num_descendants++;
		if (n>=MAX_NUM_DESCENDANTS)
			exit(-1);
		l->descendants[n]=idx;
		leader_collect_descendants(leader_id,idx);
	}
}

void leader_collect_enemies(int leader_id, int insect_idx) {
	struct insect_data *p=&insects[insect_idx];
	struct leader_data *l=&leaders[leader_id];
	int leader_idx=l->insect_idx;
	for (int i=0;i<NumLeaders;i++) {
		if (i==leader_id) continue;
		int n=l->num_enemies;
		for (int d=0;d<leaders[i].num_descendants;d++) {
			int target_idx=leaders[i].descendants[d];
			if (relevant_enemy(leader_idx, target_idx, insects)) {
				int n=leaders[leader_id].num_enemies++;
				leaders[leader_id].enemies[n]=target_idx;
			}
		}
	}
}

void identify_enemies() {
	int s=section_start("identify_enemies");
#pragma omp parallel for
	for (int i=0;i<NumLeaders;i++) {
		int leader_id=leaders[i].id;
		int leader_idx=leaders[i].insect_idx;
		leaders[i].num_descendants=1;
		leaders[i].descendants[0]=leader_idx;
		leader_collect_descendants(leader_id, leader_idx);
		//printf("leader %d (idx %d) has %d descendants\n",leader_id, leader_idx, leaders[i].num_descendants);
	}
#pragma omp parallel for
	for (int i=0;i<NumLeaders;i++) {
		int leader_id=leaders[i].id;
		int leader_idx=leaders[i].insect_idx;
		leaders[i].num_enemies=0;
		leader_collect_enemies(leader_id, leader_idx);
		//printf("leader %d (idx %d) has %d enemies\n",leader_id, leader_idx, leaders[i].num_enemies);
	}
	section_end(s);
}

void identify_leaders(int leader_layer) {
	NumLeaders=0;
	for (int i=0;i<NumInsects;i++) {
		int layer=0;
		int idx=i;
		while (idx>=0) {layer++; idx=insects[idx].parent;}
		if (layer==leader_layer) {
			if (NumLeaders==MAX_NUM_LEADERS) 
				exit(-1);
			int leader=NumLeaders++;
			leaders[leader].insect_idx=i;
		}
	}

	for (int i=0;i<NumLeaders;i++) {
		leaders[i].num_descendants=0;
		leaders[i].num_enemies=0;
		leaders[i].id=i;
		leaders[i].hue=1.0*(1+i)/(NumLeaders+1);
	}
}

int count_children(int idx) {
	struct insect_data *p=&insects[idx];
	int n=p->nchildren;
	for (int i=0;i<p->nchildren;i++)
		n+=count_children(p->children[i]);
	return n;
}

int make_leader(int idx, int leader_idx, int leader_id) {
	struct insect_data *p=&insects[idx];
	p->leader_idx=leader_idx;
	p->leader_id=leader_id;
	int n=1;
	for (int i=0;i<p->nchildren;i++)
		n+=make_leader(p->children[i],leader_idx,leader_id);
	return n;
}

double rand01() {
	return ((double)rand())/RAND_MAX;
}

void make_leaders() {
	int leader_idx, n;
	for (int i=0;i<NumInsects;i++) {
		insects[i].leader_id=-1;
		insects[i].leader_idx=-1;
	}

	for (int i=0;i<NumLeaders;i++) {
		leader_idx=leaders[i].insect_idx;
		int leader_id=i;
		n=make_leader(leader_idx,leader_idx,leader_id);
	}
}

void params_small_case(struct model_parameters* params) {
	params->lx=200;
	params->ly=200;
	params->lz=200;
	params->x0=0;
	params->y0=0;
	params->z0=0;
	params->r0=0.7;
	params->dt=0.05;

	params->grouping_constant=1;
	params->grouping_radius=1;

	params->coulomb_constant=10;
	params->coulomb_radius=2;

	params->damping_constant=sqrt(2*params->grouping_constant); //aperiodic
	
	params->attack_radius=0;
	params->attack_constant=0;
	params->defend_constant=0;
	params->fight_radius=0;
	params->fight_mass_rate=0;
	params->mass_min=0.1;
	params->surrender_mass_ratio=3;
	
	params->center_force_constant=0.1;

	params->output_dir="out";

	params->num_insects=10240;
	params->max_tree_depth=7;

	params->num_iterations=getenvl("ITERATIONS",16);
}

void params_large_case(struct model_parameters* params) {
	params->lx=200;
	params->ly=200;
	params->lz=200;
	params->x0=0;
	params->y0=0;
	params->z0=0;
	params->r0=0.7;
	params->dt=0.1;

	params->grouping_constant=1;
	params->grouping_radius=1;

	params->coulomb_constant=10;
	params->coulomb_radius=2;

	params->damping_constant=sqrt(2*params->grouping_constant); //aperiodic
	
	params->attack_radius=0;
	params->attack_constant=0;
	params->defend_constant=0;
	params->fight_radius=0;
	params->fight_mass_rate=0;
	params->mass_min=0.1;
	params->surrender_mass_ratio=3;
	
	params->center_force_constant=0.1;

	params->output_dir="out";

	params->num_insects=1<<14;
	params->max_tree_depth=9;

	params->num_iterations=getenvl("ITERATIONS",16);
}


void model_enable_rivalism() {
	printf("enabling rivalism\n");
	params.attack_constant=100;
	params.defend_constant=50;
	params.attack_radius=40;
	params.fight_radius=5;
	params.fight_mass_rate=0.01;
#pragma acc update device(params)
}

void setup_model() {

	params_small_case(&params);
	//params_large_case(&params);
	NumInsects=params.num_insects;

	//setup insects
	insects=malloc(NumInsects*sizeof(struct insect_data));
	actions=malloc(NumInsects*sizeof(struct insect_action_data));
	float x,y,z;
	float lx=params.lx,ly=params.ly,lz=params.lz;
	float x0=-lx/2;
	float y0=-ly/2;
	float z0=-lz/2;

	//x in -lx/2..+lx/2
	for (int i=0;i<NumInsects;i++) {
		x = x0+lx*rand01();
		y = y0+ly*rand01();
		z = z0+lz*rand01();
		insects[i].x=x;
		insects[i].y=y;
		insects[i].z=z;
		insects[i].vx=0;
		insects[i].vy=0;
		insects[i].vz=0;
		insects[i].m=1+rand01();
		insects[i].parent=-2;
		insects[i].leader_idx=-1;
		insects[i].leader_id=-1;
		insects[i].nchildren=0;
	}
	insects[0].parent=-1;
	int current=0;
	int target_nchildren=4;
	int i=1;
	int level=0;
	int max_level=params.max_tree_depth;
	while (i<NumInsects) {
		if (insects[current].nchildren<target_nchildren&&level<max_level) {
			//add i to current
			int idx=insects[current].nchildren;
			insects[current].children[idx]=i;
			insects[current].nchildren=idx+1;
			insects[i].parent=current;
			//add upcoming insects to child i
			current=i;
			level++;
			i++;
		} else {
			//filled, try adding to parent
			current=insects[current].parent;
			level--;
			if (current==-1) {
				printf("tree full at insect %d\n",i); 
				exit(-1);
			}
		}
	}
	//setup leaders
	leaders=malloc(MAX_NUM_LEADERS*sizeof(struct leader_data));
	int leader_layer=5;
	identify_leaders(leader_layer);
	make_leaders();
#pragma acc update device(params,NumInsects,NumLeaders)
}

#pragma acc routine
void attack_defend_fight(int attack, int defend, struct insect_data *restrict insects, struct insect_action_data *restrict actions) {
	float dx,dy,dz,r,a,d;
	
	if (attack<0||defend<0||attack==defend) return;
	dx=insects[defend].x-insects[attack].x;
	dy=insects[defend].y-insects[attack].y;
	dz=insects[defend].z-insects[attack].z;
	r=sqrt(dx*dx+dy*dy+dz*dz);
	float r0=params.attack_radius;
	float rr=r;
	if (rr<r0) rr=r0;
	a = params.attack_constant/(rr*rr*rr);
	d = params.defend_constant/(rr*rr*rr);
#pragma acc atomic
	actions[attack].fx+=dx*a;
#pragma acc atomic
	actions[attack].fy+=dy*a;
#pragma acc atomic
	actions[attack].fz+=dz*a;
#pragma acc atomic
	actions[defend].fx+=dx*d;
#pragma acc atomic
	actions[defend].fy+=dy*d;
#pragma acc atomic
	actions[defend].fz+=dz*d;
	if (r<params.fight_radius) {
		float md=insects[defend].m;
		float ma=insects[attack].m;
		float ratio=params.surrender_mass_ratio;
		if (ma/md>ratio) {
			//attack wins
			//actions[defend].new_parent=attack;
			//actions[defend].rm+=.1/params.dt;
		} else 	if (md/ma>ratio) {
			//defend wins
			actions[attack].new_parent=defend;
#pragma acc atomic
			actions[attack].rm+=.1/params.dt;
#pragma acc atomic
			actions[defend].rm-=.1/params.dt;
		} else {
			//mass transfer to heavier one
			float dm=params.fight_mass_rate*(ma-md)/(ma+md);
#pragma acc atomic
			actions[defend].rm-=dm;
#pragma acc atomic
			actions[attack].rm+=dm;
		}
	}
}


#pragma acc routine
int engage_descendants(int target_idx, int insect_idx, int leader_idx, struct insect_data *restrict insects, struct insect_action_data *restrict actions) {
	//insect engages target and all of its descendants that are a relevant enemy to leader
	int n=0;
	//engage target
	if (relevant_enemy(leader_idx,target_idx,insects)) {
		attack_defend_fight(insect_idx,target_idx,insects,actions);
		n=1;
	}
	//engage target's descendants
	struct insect_data *target=&insects[target_idx];
	for (int i=0;i<target->nchildren;i++) {
		int child_idx=target->children[i];
		n+=engage_descendants(child_idx,insect_idx,leader_idx,insects,actions);
	}
	return n;
}

#pragma acc routine
void engage_enemies(int insect_idx, struct insect_data *restrict insects, struct insect_action_data *restrict actions) {
	struct insect_data *parent,*leader,*insect;
	
	insect=&insects[insect_idx];
	int leader_idx=insect->leader_idx;
	if (leader_idx<0) return;
	leader=&insects[leader_idx];

	int node_idx=leader_idx;
	int parent_idx=leader->parent;
	int n=0;
	while (parent_idx>=0) {
		parent=&insects[parent_idx];
		//all peers of node are enenmies, i.e. all children of parent except node
		for (int i=0;i<parent->nchildren;i++) {
			int child_idx=parent->children[i];
			if (child_idx!=node_idx) {
				n+=engage_descendants(child_idx,insect_idx,leader_idx,insects,actions);
			}
		}
		//ascend to parent
		node_idx=parent_idx;
		parent_idx=parent->parent;
	}
}

#pragma acc routine
void center_force(int insect_idx, struct insect_data *restrict insects, struct insect_action_data *restrict actions) {
	struct insect_data *insect=&insects[insect_idx];
	struct insect_action_data *action=&actions[insect_idx];
	float dx,dy,dz,a;
	dx=insect->x;
	dy=insect->y;
	dz=insect->z;
	a=params.center_force_constant;
	action->fx+=-dx*a;
	action->fy+=-dy*a;
	action->fz+=-dz*a;
}

void spring_force(int target, int peer, float r0, float D, struct insect_data*restrict insects, struct insect_action_data*restrict actions) {
	float dx,dy,dz,r,a,b,c,fx,fy,fz;
	dx=insects[target].x-insects[peer].x;
	dy=insects[target].y-insects[peer].y;
	dz=insects[target].z-insects[peer].z;
	r=sqrt(dx*dx+dy*dy+dz*dz);
	a  = D*(r-r0)/r;
	fx = -dx*a;
	fy = -dy*a;
	fz = -dz*a;
#pragma omp atomic
	actions[target].fx+=fx;
#pragma omp atomic
	actions[target].fy+=fy;
#pragma omp atomic
	actions[target].fz+=fz;
#pragma omp atomic
	actions[peer].fx-=fx;
#pragma omp atomic
	actions[peer].fy-=fy;
#pragma omp atomic
	actions[peer].fz-=fz;
}

void tree_force(int a, int b) {
	if (a<0||b<0||a==b) return;
	spring_force(a,b,params.grouping_radius,params.grouping_constant,insects,actions);
}

void add_child(int p_idx, int c_idx) {
	struct insect_data *p=&insects[p_idx];
	int n=p->nchildren;
	if (insects[p_idx].leader_idx<0) {
		printf("adding insect to non-leader\n");
		exit(-1);
	}
	if (n<MAX_CHILDREN) {
		p->nchildren++;
		p->children[n]=c_idx;
		insects[c_idx].parent=p_idx;
		//update children of c to new leader
		make_leader(c_idx,insects[p_idx].leader_idx,insects[p_idx].leader_id);	
	} else {
		add_child(p->children[0],c_idx);
	}
}

int child_index(int p_idx, int c_idx) {
	struct insect_data *p=&insects[p_idx];
	int n=p->nchildren;
	for (int i=0;i<n;i++) 
		if (p->children[i]==c_idx) 
			return i;
	return -1;
}

void remove_child(int p_idx, int c_idx) {
	if (p_idx<0) {
		printf("unable to remove from root insect");
		exit(-1);
	}
	struct insect_data *p=&insects[p_idx];
	struct insect_data *c=&insects[c_idx];
	int n=p->nchildren;
	int idx=child_index(p_idx,c_idx);
	if (idx>=0) {
		//promote child 0 of c into position of c
		if (c->nchildren==0) {
			//if no child to promote, just remove c from p
			for (int i=idx;i<n-1;i++)
				p->children[i]=p->children[i+1];
			p->nchildren--;
		} else {
			int is_leader=(c->leader_idx==c_idx);
			int promote_idx=c->children[0];
			p->children[idx]=promote_idx;
			insects[promote_idx].parent=p_idx;
			//and attach remaining children of c to promote
			for (int i=1;i<c->nchildren;i++) {
				add_child(promote_idx,c->children[i]);
			}
			c->nchildren=0;
			//if c was a leader, then make promoted this leader
			if (is_leader) {
				int leader_id=insects[c_idx].leader_id;
				leaders[leader_id].insect_idx=promote_idx;
				make_leader(promote_idx,promote_idx,leader_id);
			}
		}
		//free insect from parents or leaders
		insects[c_idx].parent=-1;
		insects[c_idx].leader_id=-1;
		insects[c_idx].leader_idx=-1;
	} else {
		printf("insect %d not a child of %d\n",c_idx,p_idx);
		exit(-1);
	}
}

void apply_velocities() {
	float dt=params.dt;
	for (int i=0;i<NumInsects;i++) {
		insects[i].x+=insects[i].vx*dt;
		insects[i].y+=insects[i].vy*dt;
		insects[i].z+=insects[i].vz*dt;
	}
}

void apply_forces() {
	float dt=params.dt;
	float beta=params.damping_constant;
	for (int i=0;i<NumInsects;i++) {
		insects[i].vx+=dt*(actions[i].fx/insects[i].m-insects[i].vx*beta);
		insects[i].vy+=dt*(actions[i].fy/insects[i].m-insects[i].vy*beta);
		insects[i].vz+=dt*(actions[i].fz/insects[i].m-insects[i].vz*beta);
		insects[i].m +=dt*(actions[i].rm);
		insects[i].m  =MAX(insects[i].m,params.mass_min);
		int npar=actions[i].new_parent;
		if (npar>=0) {
			int par=insects[i].parent;
			remove_child(par,i);
			add_child(npar,i);
		}
	}
}

void engage(struct insect_data *restrict insects, struct insect_action_data *restrict actions,struct leader_data *restrict leaders) {
#pragma acc data present(insects[0:NumInsects]) present(actions[0:NumInsects]) present(leaders[0:NumLeaders])
#pragma acc kernels
#pragma acc loop parallel
	for (int i=0;i<NumLeaders;i++) {
		struct leader_data *l=&leaders[i];
		int ne=l->num_enemies;
		int nd=l->num_descendants;
#pragma acc loop collapse(2)
		for (int d=0;d<nd;d++) {
			for (int e=0;e<ne;e++) {
				int attack=l->descendants[d];
				int defend=l->enemies[e];
				attack_defend_fight(attack,defend,insects,actions);
			}
		}
	}
}

void calculate_forces() {
	for (int i=0;i<NumInsects;i++) {
		actions[i].fx=0;
		actions[i].fy=0;
		actions[i].fz=0;
		actions[i].rm=0;
		actions[i].new_parent=-1;
	}
	identify_enemies();
	int s=section_start("tree_force");
#pragma omp parallel for
	for (int i=0;i<NumInsects;i++) {
		int parent=insects[i].parent;
		tree_force(i, parent);
	}
	section_end(s);
	s=section_start("center+coulomb_force");
#pragma acc data copyin(insects[0:NumInsects]) copy(actions[0:NumInsects]) copyin(leaders[0:NumLeaders])
{
#pragma acc kernels
#pragma acc loop parallel
	for (int i=0;i<NumInsects;i++) {
		center_force(i,insects,actions);
		coulomb_repell(i,insects,actions);
	}
	section_end(s);
	s=section_start("engage_enemies");
	engage(insects,actions,leaders);
}
	section_end(s);
}

void iteration()
{
	int s=section_start("model");
	//leap-frog method
	apply_velocities();
	calculate_forces();
	apply_forces();
	section_end(s);
}

