#ifndef MYHASH_H
#define MYHASH_H
#include <limits.h>
#include "uthash.h"
#include "matrix.h"
#include "gmath.h"

typedef struct {
  char * vertex;
  double **vectors;
  int num_vectors;
  UT_hash_handle hh;
} HASH;

extern HASH * vector_hash;

void add_point_hash(char * point, double * vector);
void create_hash_table(struct matrix * polygons);

#endif
