#include "model.h"
#include <stdio.h>

void print_leaders(FILE* f, int iteration);
void print_parent_chain(int p_idx);
void fprint_insect_data(FILE* stream, struct insect_data* p);
void fprint_insect_data_double(FILE* stream, struct insect_data_double* p);
void print_insect_data(struct insect_data* p);
void print_insect_action_data(struct insect_action_data* p);
void fprint_insect_action_data(FILE* stream, struct insect_action_data* p);
void print_model(int i, struct insect_data* p,struct insect_action_data* a);
void print_p(int i);
void log_iteration(int iteration);
void setup_logging();
void done_logging();
