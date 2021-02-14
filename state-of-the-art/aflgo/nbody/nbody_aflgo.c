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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
typedef struct{
  double x,y,z;
}vector;

int bodies,timeSteps;
double *masses,GravConstant;
vector *positions,*velocities,*accelerations;

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
  vector acceleration_computed = addVectors(acceleration,scaleVector(GravConstant*mass/
  pow(mod(subtractVectors(position_i,position_j)),3),subtractVectors(position_j,position_i)));
  /* Check for Nans */
  if (isnan(acceleration_computed.x) || isnan(acceleration_computed.y) || isnan(acceleration_computed.z) ){
    assert(0 && "NaN in the kernel 1");
  }
  /* Check for Inf */
  if (isinf(acceleration_computed.x) || isinf(acceleration_computed.y) || isinf(acceleration_computed.z) ){
    assert(0 && "INF in the kernel 1");
  }
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
  for(i=0;i<bodies;i++){
    velocities[i] = addVectors(velocities[i],accelerations[i]);
    /* Check for Nans */
    if (isnan(velocities[i].x) || isnan(velocities[i].y) || isnan(velocities[i].z) ){
      assert(0 && "NaN in the kernel 2");
    }
    /* Check for Inf */
    if (isinf(velocities[i].x) || isinf(velocities[i].y) || isinf(velocities[i].z) ){
      assert(0 && "INF in the kernel 2");
    }
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
  
  for(j=0;j<timeSteps;j++){
    simulate();
  }  
  return 0;
}
