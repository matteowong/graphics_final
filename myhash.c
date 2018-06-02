#include <stdio.h>
#include <stdlib.h>
#include "myhash.h"
#include "uthash.h"
#include "gmath.h"
#include "draw.h"
#include "display.h"
#include "ml6.h"

/*struct my_struct {
  double * vertex; //key char * s;
  double * vectors;
int num_vectors;
  //stuff
  UT_hash_handle hh;
  };*/

HASH * vector_hash=NULL;

void create_hash_table(struct matrix * polygons) {

  int i=0;
  while (i<polygons->lastcol) {
    //sprintf %3.3lf
    char key[22];

    //first vertex
    sprintf(key, "%3.3lf%3.3lf%3.3lf",polygons->m[0][i],polygons->m[1][i],polygons->m[2][i]);
    double *normal=calculate_normal(polygons, i);
    normalize(normal);
    double *normal_one=calculate_normal(polygons, i);
    normalize(normal_one);
    double *normal_two=calculate_normal(polygons, i);
    normalize(normal_two);
    //printf("[create hash] normalized first vector\n");
    add_point_hash(key, normal);
    printf("[create hash loop %d] added first point\n",i);
    
    //second vertex
    i++;
    //printf("i incremented\n");
    char key_one[22];
    //key_one[0]=0;
    sprintf(key_one,"%3.3lf%3.3lf%3.3lf",polygons->m[0][i],polygons->m[1][i],polygons->m[2][i]);
    
    //printf("[%s]\n",key_one);
    add_point_hash(key_one, normal_one);
    printf("[create hash loop %d] added second point\n",i);

    //third vertex
    i++;
    char key_two[22];
    //printf("made key_two\n");
    printf("[%3.3lf%3.3lf%3.3lf]\n",polygons->m[0][i],polygons->m[1][i],polygons->m[2][i]);
    sprintf(key_two, "%3.3lf%3.3lf%3.3lf",polygons->m[0][i],polygons->m[1][i],polygons->m[2][i]);
    printf("sprintfed\n");
    add_point_hash(key_two, normal_two);
    printf("[create hash loop %d] added third point\n",i);;
    i++;

  }
}

void add_point_hash(char * point, double *vector) {
  //calloc or realloc for vector
  HASH *s;

  HASH_FIND_STR(vector_hash, point, s);
  printf("[add_point_hash] hash_find_str done\n");

  if (s==NULL) {
    printf("[add_point_hash] s is NULL\n");
    s=(HASH *)malloc(sizeof(HASH));
    printf("[add_point_hash] malloced s\n");
    s->num_vectors=0;
    s->vertex=(char *)calloc(sizeof(char),strlen(point));
    strcpy(s->vertex,point);
    s->vectors=(double **)malloc(sizeof(double *));
    s->vectors[s->num_vectors]=(double *)malloc(sizeof(double)*3);
    s->vectors[s->num_vectors]=vector;
    s->num_vectors++;
    printf("[add_point_hash]almost there\n");
    HASH_ADD_KEYPTR(hh, vector_hash, s->vertex, strlen(s->vertex), s);
  } else {
    s->num_vectors++;
    s->vectors[s->num_vectors-1]=(double *)malloc(sizeof(double)*3);;
    /*double *new_vector
    new_vector[0]=vector[0];
    new_vector[1]=vector[1];
    new_vector[2]=vector[2];*/
    s->vectors[s->num_vectors-1]=vector;
    //s->vectors=realloc(s->vectors,s->num_vectors*sizeof(double*));
    //s->vectors[s->num_vectors-1]=vector;
    printf("[add_point_hash]updated [%s]\n",point);

  }
}

void print_vectors(double ** vectors, int num_vectors) {

  int i;
  for (i=0;i<num_vectors;i++) {
    printf("vector number %d\n",i);
    printf("x: %lf |", vectors[i][0]);
    printf("y: %lf |", vectors[i][1]);
    printf("z: %lf\n", vectors[i][2]);


  }


}

void print_hash() {

  HASH * s;
  for (s=vector_hash; s!=NULL; s=s->hh.next) {
    printf("point [%s]\n",s->vertex);
    print_vectors(s->vectors,s->num_vectors);
  }

}

void free_hash() {
  //HASH * current_point, *tmp;

  //HASH_ITER(hh, vector_hash, current_point, tmp) {
  HASH * current_point;
  for(current_point=vector_hash;current_point!=NULL; current_point=current_point->hh.next) {
    //free stuff
    
    printf("next point to free: [%s]\n",current_point->vertex);
    print_vectors(current_point->vectors,current_point->num_vectors);
    int i;
    for (i=0;i<current_point->num_vectors;i++) {
      free(current_point->vectors[i]);//error because they have been freed already
      printf("freed vector %d\n",i);
    }
    free(current_point->vectors);
    free(current_point->vertex);
    HASH_DEL(vector_hash,current_point);
    free(current_point);
  }
}

int main(int argc, char **argv) {
  struct matrix*polygons=new_matrix(4,1000);
  //add_box(polygons, 0, 10, 20, 30, 40, 50);
  add_sphere(polygons,0,0,0,40,5);
  printf("created sphere\n");
  create_hash_table(polygons);

  /*HASH * s;
  for (s=vector_hash; s!=NULL; s=s->hh.next) {
    printf("point [%s]\n",s->vertex);
    print_vectors(s->vectors,s->num_vectors);
    }*/
  print_hash();
  printf("\n\nprint two\n\n");
  print_hash();
  free_hash();
  return 0;
}
