#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef struct vector {
  double x,y,z;
}vector;

#define bodies 3

double GravConstant = 0.01;
int timeSteps = 10;

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

void resolveCollisions(vector * positions, vector * velocities){
  int i,j;
  __ASTREE_unroll((50))
  for(i=0;i<bodies-1;i++)
    __ASTREE_unroll((50))
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
  __ASTREE_assert((!(isinf(acceleration_computed.x))));
  __ASTREE_assert((!(isnan(acceleration_computed.x))));
  __ASTREE_assert((!(isinf(acceleration_computed.y))));
  __ASTREE_assert((!(isnan(acceleration_computed.y))));
  __ASTREE_assert((!(isinf(acceleration_computed.z))));
  __ASTREE_assert((!(isnan(acceleration_computed.z))));
  return acceleration_computed;
}

void computeAccelerations(double masses[], vector * positions, vector * accelerations){
  int i,j;
  __ASTREE_unroll((50))
  for(i=0;i<bodies;i++){
    accelerations[i].x = 0;
    accelerations[i].y = 0;
    accelerations[i].z = 0;
    __ASTREE_unroll((50))
    for(j=0;j<bodies;j++){
      if(i!=j){
        accelerations[i] = numerical_kernel1(masses[j], positions[i], positions[j], accelerations[i]);
      }
    }
  }
}

void computePositions(vector *positions, vector *velocities, vector *accelerations){
  int i;
  __ASTREE_unroll((50))
  for(i=0;i<bodies;i++)
    positions[i] = addVectors(positions[i],addVectors(velocities[i],scaleVector(0.5,accelerations[i])));
}

void numerical_kernel2(vector *velocities, vector *accelerations){
  int i;

  __ASTREE_unroll((50))
  for(i=0;i<bodies;i++) {
    velocities[i] = addVectors(velocities[i],accelerations[i]);
    __ASTREE_assert((!(isinf(velocities[i].x))));
    __ASTREE_assert((!(isnan(velocities[i].x))));
    __ASTREE_assert((!(isinf(velocities[i].y))));
    __ASTREE_assert((!(isnan(velocities[i].y))));
    __ASTREE_assert((!(isinf(velocities[i].z))));
    __ASTREE_assert((!(isnan(velocities[i].z))));
  }
}

void simulate(double masses[], vector *positions, vector *velocities, vector *accelerations){
  computeAccelerations(masses, positions, accelerations);
  computePositions(positions, velocities, accelerations);
  numerical_kernel2(velocities, accelerations);
  resolveCollisions(positions, velocities);
}

int main(void) {
  int i;
  double masses[3];
  vector positions[3];
  vector velocities[3];
  vector accelerations[3];
  __ASTREE_modify((masses[0]; [0.9, 1.1]));
  __ASTREE_modify((positions[0].x; [0.0, 0.5]));
  __ASTREE_modify((positions[0].y; [0.0, 0.5]));
  __ASTREE_modify((positions[0].z; [0.0, 0.5]));
  __ASTREE_modify((velocities[0].x; [0.001, 0.019]));
  __ASTREE_modify((velocities[0].y; [0.0, 0.5]));
  __ASTREE_modify((velocities[0].z; [0.0, 0.5]));

  __ASTREE_modify((masses[1]; [0.001, 0.9]));
  __ASTREE_modify((positions[1].x; [0.9, 1.1]));
  __ASTREE_modify((positions[1].y; [0.9, 1.1]));
  __ASTREE_modify((positions[1].z; [0.0, 0.5]));
  __ASTREE_modify((velocities[1].x; [0.0, 0.5]));
  __ASTREE_modify((velocities[1].y; [0.0, 0.5]));
  __ASTREE_modify((velocities[1].z; [0.01, 0.03]));

  __ASTREE_modify((masses[2]; [0.0001, 0.0019]));
  __ASTREE_modify((positions[2].x; [0.0, 0.5]));
  __ASTREE_modify((positions[2].y; [0.9, 1.1]));
  __ASTREE_modify((positions[2].z; [0.9, 1.1]));
  __ASTREE_modify((velocities[2].x; [0.001, 0.019]));
  __ASTREE_modify((velocities[2].y; [-0.019, -0.001]));
  __ASTREE_modify((velocities[2].z; [-0.019, -0.001]));

  __ASTREE_unroll((50))
  for(i=0;i<timeSteps;i++){
    simulate(masses, positions, velocities, accelerations);
  }
  return 0;
}

