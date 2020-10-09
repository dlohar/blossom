/* Code taken from Rosetta Code (https://rosettacode.org/wiki/N-body_problem#C).
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
#include <time.h>
#include <string.h>
#include <stdbool.h> 

typedef struct{
  double x,y,z;
} vector;

#define bodies 3
double GravConstant = 0.01;
int timeSteps = 10;
double masses[3];
vector positions[bodies];
vector velocities[bodies];
vector accelerations[bodies];

vector addVectors(vector a,vector b) {
  vector c = {a.x+b.x,a.y+b.y,a.z+b.z};

  return c;
}

vector scaleVector(double b,vector a) {
  vector c = {b*a.x,b*a.y,b*a.z};

  return c;
}

vector subtractVectors(vector a,vector b) {
  vector c = {a.x-b.x,a.y-b.y,a.z-b.z};

  return c;
}

double mod(vector a) {
  return sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
}

/* Kernel Function */
void numerical_kernel2(double v1_x, double v1_y, double v1_z, double v2_x, double v2_y, double v2_z, double v3_x, double v3_y, double v3_z,
	double a1_x, double a1_y, double a1_z, double a2_x, double a2_y, double a2_z, double a3_x, double a3_y, double a3_z) {
  for(int i=0;i<bodies;i++) {
    velocities[i] = addVectors(velocities[i],accelerations[i]); 
  }
}

vector numerical_kernel1(double mass, double i_x, double i_y, double i_z, double j_x, double j_y, double j_z, double acc_x, double acc_y, double acc_z) {
  vector position_i={i_x, i_y, i_z};
  vector position_j={j_x, j_y, j_z};
  vector acceleration={acc_x, acc_y, acc_z};
  vector acceleration_computed = addVectors(acceleration,scaleVector(GravConstant*mass/
  pow(mod(subtractVectors(position_i,position_j)),3),subtractVectors(position_j,position_i)));
  return acceleration_computed;
}

void resolveCollisions() {
  int i,j;
  for(i=0;i<bodies-1;i++) {
    for(j=i+1;j<bodies;j++){
      if(positions[i].x==positions[j].x && positions[i].y==positions[j].y && positions[i].z==positions[j].z){
        vector temp = velocities[i];
        velocities[i] = velocities[j];
        velocities[j] = temp;
      }
    }
  }
}

void computeAccelerations() {
  int i,j;
  for(i=0;i<bodies;i++){
    accelerations[i].x = 0;
    accelerations[i].y = 0;
    accelerations[i].z = 0;
    for(j=0;j<bodies;j++){
      if(i!=j){
        accelerations[i] = numerical_kernel1(masses[j], positions[i].x, positions[i].y, positions[i].z, positions[j].x, positions[j].y, positions[j].z, 			accelerations[i].x, accelerations[i].y, accelerations[i].z);
      }
    }
  }
}

void computePositions() {
  int i;

  for(i=0;i<bodies;i++)
    positions[i] = addVectors(positions[i],addVectors(velocities[i],scaleVector(0.5,accelerations[i])));
}

void simulate() {
  computeAccelerations();
  computePositions();
  numerical_kernel2(velocities[0].x,velocities[0].y,velocities[0].z,velocities[1].x,velocities[1].y,velocities[1].z,velocities[2].x,velocities[2].y,velocities[2].z,
	accelerations[0].x,accelerations[0].y,accelerations[0].z,accelerations[1].x,accelerations[1].y,accelerations[1].z,accelerations[2].x,accelerations[2].y,accelerations[2].z);
  resolveCollisions();
}

void original_program_main(double masses_in[], double positions_x_in[], double positions_y_in[], double positions_z_in[],
double velocities_x_in[], double velocities_y_in[], double velocities_z_in[]) {
    masses[0] = masses_in[0];
    masses[1] = masses_in[1];
    masses[2] = masses_in[2];

    positions[0].x = positions_x_in[0];
    positions[0].y = positions_y_in[0];
    positions[0].z = positions_z_in[0];
    velocities[0].x = velocities_x_in[0];
    velocities[0].y = velocities_y_in[0];
    velocities[0].z = velocities_z_in[0];
    positions[1].x = positions_x_in[1];
    positions[1].y = positions_y_in[1];
    positions[1].z = positions_z_in[1];
    velocities[1].x = velocities_x_in[1];
    velocities[1].y = velocities_y_in[1];
    velocities[1].z = velocities_z_in[1];
    positions[2].x = positions_x_in[2];
    positions[2].y = positions_y_in[2];
    positions[2].z = positions_z_in[2];
    velocities[2].x = velocities_x_in[2];
    velocities[2].y = velocities_y_in[2];
    velocities[2].z = velocities_z_in[2];

    for (int k = 0; k < timeSteps; k++) {
      simulate();
    }
}

