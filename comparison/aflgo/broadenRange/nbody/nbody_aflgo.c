/*
  Code taken from Rosetta Code (https://rosettacode.org/wiki/N-body_problem#C).
  Simulates the interaction of several masses under gravity. 
  Takes as input a configuration file specifying the number of masses and their
  positions, in the following format:
  <Gravitational Constant> <Number of bodies(N)> <Time step>
  <Mass of M1>
  <Position of M1 in x,y,z co-ordinates>
  <Initial velocity of M1 in x,,y,z components>
  ...
  <And so on for N bodies>
*/

#include<stdlib.h>
#include<stdio.h>
#include<math.h>
#include "compute_range.h"

typedef struct{
  double x,y,z;
}vector;

int bodies,timeSteps;
double *masses,GravConstant;
vector *positions,*velocities,*accelerations;

/* NEW: Global variables to store the ranges */
/* 0: mass, 1-3: position_i xyz, 4-6: position_j xyz, 7-9: acceleration xyz */
__range_t range_kernel1[9];
/* body0: 0-2: velocities xyz, 3-5: accelerations xyz
   body1: ... */
__range_t range_kernel2[18];

vector addVectors(vector a,vector b){
  vector c = {a.x+b.x,a.y+b.y,a.z+b.z};

  return c;
}

vector scaleVector(double b,vector a){
  vector c = {b*a.x,b*a.y,b*a.z};

  return c;
}

vector subtractVectors(vector a,vector b){
  vector c = {a.x-b.x,a.y-b.y,a.z-b.z};

  return c;
}

double mod(vector a){
  return sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
}

void resolveCollisions(){
  int i,j;

  for(i=0;i<bodies-1;i++)
    for(j=i+1;j<bodies;j++){
      if(positions[i].x==positions[j].x && positions[i].y==positions[j].y && positions[i].z==positions[j].z){
        vector temp = velocities[i];
        velocities[i] = velocities[j];
        velocities[j] = temp;
      }
    }
}

vector numerical_kernel1(double mass, vector position_i, vector position_j, vector acceleration) {
  /* NEW - GET RANGE: 
     Check if x is in the range. If not, widen the range */
  /* broaden_range(mass,&range_kernel1[0]); */
  broaden_range(position_i.x,&range_kernel1[0]);
  broaden_range(position_i.y,&range_kernel1[1]);
  broaden_range(position_i.z,&range_kernel1[2]);
  broaden_range(position_j.x,&range_kernel1[3]);
  broaden_range(position_j.y,&range_kernel1[4]);
  broaden_range(position_j.z,&range_kernel1[5]);
  broaden_range(acceleration.x,&range_kernel1[6]);
  broaden_range(acceleration.y,&range_kernel1[7]);
  broaden_range(acceleration.z,&range_kernel1[8]);
  /* ------------------- END NEW ------------------- */

  vector acceleration_computed = addVectors(acceleration,scaleVector(GravConstant*mass/
  pow(mod(subtractVectors(position_i,position_j)),3),subtractVectors(position_j,position_i)));

  return acceleration_computed;
}

void computeAccelerations(){
  int i,j;
  for(i=0;i<bodies;i++){
    accelerations[i].x = 0;
    accelerations[i].y = 0;
    accelerations[i].z = 0;
    for(j=0;j<bodies;j++){
      if(i!=j){
        accelerations[i] = numerical_kernel1(masses[j], positions[i], positions[j], accelerations[i]);
      }
    }
  }
}

void computePositions(){
  int i;

  for(i=0;i<bodies;i++)
    positions[i] = addVectors(positions[i],addVectors(velocities[i],scaleVector(0.5,accelerations[i])));
}

void numerical_kernel2(){
  int i;
  /* NEW - GET RANGE: 
     Check if x is in the range. If not, widen the range */
  int j=0;
  for (i=0;i<bodies;i++) {
    broaden_range(velocities[i].x,&range_kernel2[i*6]);
    broaden_range(velocities[i].y,&range_kernel2[i*6+1]);
    broaden_range(velocities[i].z,&range_kernel2[i*6+2]);
    broaden_range(accelerations[i].x,&range_kernel2[i*6+3]);
    broaden_range(accelerations[i].y,&range_kernel2[i*6+4]);
    broaden_range(accelerations[i].z,&range_kernel2[i*6+5]);
  }
  /* ------------------- END NEW ------------------- */
  double x_old, y_old, z_old;

  for(i=0;i<bodies;i++){
    x_old = velocities[i].x;
    y_old = velocities[i].y;
    z_old = velocities[i].z;
    velocities[i] = addVectors(velocities[i],accelerations[i]);
  }
}

void simulate(){
  computeAccelerations();
  computePositions();
  numerical_kernel2();
  resolveCollisions();
}

int main(int argC,char* argV[])
{
  int i,j;

  /* NEW: Fix the following variables*/
  GravConstant = 0.01;
  bodies = 3;
  timeSteps = 10;

  masses = (double*)malloc(bodies*sizeof(double));
  positions = (vector*)malloc(bodies*sizeof(vector));
  velocities = (vector*)malloc(bodies*sizeof(vector));
  accelerations = (vector*)malloc(bodies*sizeof(vector));

  /* NEW - GET RANGE: 
     Output file containing the fuzzed input values */
  FILE *file1, *file2, *old_file1, *old_file2;
  file1 = fopen("broad_kernel1_range.dat","r");
  file2 = fopen("broad_kernel2_range.dat","r");
  old_file1 = fopen("../kernel1_range.dat","r");
  old_file2 = fopen("../kernel2_range.dat","r");
  if ((file1 == NULL) || (file2 == NULL) ||
      (old_file1 == NULL) || (old_file2 == NULL)){
    puts("Error while opening file kernel_range.dat");
    exit(1);
  }
  
  /* NEW:Instead of using a file containing system configuration data, scan the input from AFLGO stream */
  int nb_scanf = 0;
  for(i=0;i<bodies;i++){
    /* Scan data */
    nb_scanf += scanf("%lf",&masses[i]);
    nb_scanf += scanf("%lf",&positions[i].x);
    nb_scanf += scanf("%lf",&positions[i].y);
    nb_scanf += scanf("%lf",&positions[i].z);
    nb_scanf += scanf("%lf",&velocities[i].x);
    nb_scanf += scanf("%lf",&velocities[i].y);
    nb_scanf += scanf("%lf",&velocities[i].z);
  }
  
  if ( (nb_scanf != 21) ||
       (masses[0] < 0.9) || (masses[0] > 1.1) ||
       (positions[0].x < 0.0) || (positions[0].x > 0.5) ||
       (positions[0].y < 0.0) || (positions[0].y > 0.5) ||
       (positions[0].z < 0.0) || (positions[0].z > 0.5) ||
       (velocities[0].x < 0.001) || (velocities[0].x > 0.019) ||
       (velocities[0].y < 0.0) || (velocities[0].y > 0.5) ||
       (velocities[0].z < 0.0) || (velocities[0].z > 0.5) ||
       (masses[1] < 0.001) || (masses[1] > 0.9) ||
       (positions[1].x < 0.9) || (positions[1].x > 1.1) ||
       (positions[1].y < 0.9) || (positions[1].y > 1.1) ||
       (positions[1].z < 0.0) || (positions[1].z > 0.5) ||
       (velocities[1].x < 0.0) || (velocities[1].x > 0.5) ||
       (velocities[1].y < 0.0) || (velocities[1].y > 0.5) ||
       (velocities[1].z < 0.01) || (velocities[1].z > 0.03) ||
       (masses[2] < 0.0001) || (masses[2] > 0.0019) ||
       (positions[2].x < 0.0) || (positions[2].x > 0.5) ||
       (positions[2].y < 0.9) || (positions[2].y > 1.1) ||
       (positions[2].z < 0.9) || (positions[2].z > 1.1) ||
       (velocities[2].x < 0.001) || (velocities[2].x > 0.019) ||
       (velocities[2].y < -0.019) || (velocities[2].y > -0.001) ||
       (velocities[2].z < -0.019) || (velocities[2].z > -0.001) ||
       isnan(masses[0]) || isnan(positions[0].x) ||
       isnan(positions[0].y) || isnan(positions[0].z) ||
       isnan(velocities[0].x) || isnan(velocities[0].y) ||
       isnan(velocities[0].z) ||
       isnan(masses[1]) || isnan(positions[1].x) ||
       isnan(positions[1].y) || isnan(positions[1].z) ||
       isnan(velocities[1].x) || isnan(velocities[1].y) ||
       isnan(velocities[1].z) ||
       isnan(masses[2]) || isnan(positions[2].x) ||
       isnan(positions[2].y) || isnan(positions[2].z) ||
       isnan(velocities[2].x) || isnan(velocities[2].y) ||
       isnan(velocities[2].z)) {
    exit(1);
  }

  /* NEW - GET RANGE: 
     Read file containing the kernel ranges. 
     Initialize ranges if the file is empty. */
  for (i=0;i<9;i++) {
    nb_scanf = fscanf(file1,"%le",&range_kernel1[i].min);
    nb_scanf += fscanf(file1,"%le",&range_kernel1[i].max);
    if (nb_scanf != 2) {
      fscanf(old_file1,"%le",&range_kernel1[i].min);
      fscanf(old_file1,"%le",&range_kernel1[i].max);
    }
  }
  for (i=0;i<18;i++) {
    nb_scanf = fscanf(file2,"%le",&range_kernel2[i].min);
    nb_scanf += fscanf(file2,"%le",&range_kernel2[i].max);
    if (nb_scanf != 2) {
      fscanf(old_file2,"%le",&range_kernel2[i].min);
      fscanf(old_file2,"%le",&range_kernel2[i].max);
    }
  }

  for(j=0;j<timeSteps;j++){
    simulate();
  }

  /* NEW - GET RANGE: 
     Print the final range in the file;  */
  freopen(NULL,"w+",file1);
  freopen(NULL,"w+",file2);
  for (i=0;i<9;i++) {
    fprintf(file1,"%.20le %.20le\n",range_kernel1[i].min,range_kernel1[i].max);
  }
  for (i=0;i<18;i++) {
    fprintf(file2,"%.20le %.20le\n",range_kernel2[i].min,range_kernel2[i].max);
  }

  /* NEW - close all files */
  fclose(file1);
  fclose(file2);
  fclose(old_file1);
  fclose(old_file2);
  
  return 0;
}
