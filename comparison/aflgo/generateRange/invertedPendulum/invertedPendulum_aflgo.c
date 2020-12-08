#include <math.h>
#include <stdio.h>  // needed only to print the final result
#include <stdlib.h>
#include "compute_range.h"

// global vars
double Vsat = 20.0;   // saturation voltage
double M = 0.6;       // mass of cart+pendulum
double m = 0.3;        // mass of pendulum
double l = 0.3;        // length of pendulum to CG
double g = 9.81;      // gravity
double Km = 2;         // motor torque constant
double Kg = 0.01;      // gear ratio
double R = 6;          // armiture resistance
double r = 0.01;        // drive radiu3
double K1 = 1.0 / 3.0;  //Km * Kg / (R * r); 
double K2 = 2.0 / 3.0;  // Km*Km * Kg*Kg / (R*r*r);
double I = 0.006;     // inertia of the pendulum
double L = 11.0 / 30.0;  //(I + m*l**2)/(m*l); 
double pi = 3.14159265358979;

double K[4] = {-40.82482905, -30.66703711, -115.89445979, -19.424643};

/* NEW: Global variable to store the range */
/* 0-3: in[], 4: constraint */
__range_t range_kernel1[5];
/* 0-3: in[], 4: V */
__range_t range_kernel2[5];

// current (simulation) time
double t = 0.0;

double modulo(double x, double y) {
  return x - floor(x/y)*y;  
}

double constrain(double theta) {
  theta = modulo(theta, (2 * pi));
  if (theta > pi) {
    return theta = -2 * pi + theta;
  } else {
    return theta;
  }
}

int cmp(double a, double b) {

  if (a < b) { return -1; }
  else if (a == b) { return 0; }
  else { return 1; } // a > b

}

double sat(double Vsat, double V) {
  if (fabs(V) > Vsat) {
    return Vsat * cmp(V, 0);
  } else {
    return V;
  }
}

double average(double x, double k1, double k2, double k3, double k4) { 
  return x + (k1 + 2.0 * (k3 + k4) +  k2) / 6.0;

}

double cosine(double x) {
  double res = cos(x);
  return res;
}

double sine(double x) {
  double res = sin(x);
  return res;
}

double numerical_kernel1(double in[4], double constraint) {
  /* NEW - GET RANGE: 
     Compute the range */
  for (int i=0;i<4;i++) {
    compute_range(in[i],&range_kernel1[i]);
  }
  compute_range(constraint, &range_kernel1[4]);
  /* ------------------- END NEW ------------------- */
  
  double E0 = 0.0;
  double k = 1;
  double w = sqrt(m*g*l/(4*I));
  double E = m*g*l * (0.5 * (in[3]/w)*(in[3]/w) + cosine(in[2]) - 1);
  double a = k * (E - E0)*cmp(in[3] * cosine(in[2]), 0);
  double F = M * a;
  double V = (F - K2 * constraint) / K1;
  return V;
}

void numerical_kernel2(double in[4], double V, double out[4]) {
  /* NEW - GET RANGE: 
     Compute the range */
  for (int i=0;i<4;i++) {
    compute_range(in[i],&range_kernel2[i]);
  }
  compute_range(V,&range_kernel2[4]);
  /* ------------------- END NEW ------------------- */
  double x1 = in[0];
  double x2 = in[1];
  double x3 = in[2];
  double x4 = in[3];

  double x1_dt = x2;
  double x3_dt = x4;

  double div1 = (M - m*l*cosine(x3)*cosine(x3)/L);
  double div2 = (L - m*l*cosine(x3)*cosine(x3)/M);
  
  double x2_dt = (K1*V - K2*x2 - m*l*g*cosine(x3)*sine(x3)/L + m*l*sine(x3)*x4*x4) / (M - m*l*cosine(x3)*cosine(x3)/L);
  
  double x4_dt = (g*sine(x3) - m*l*x4*x4*cosine(x3)*sine(x3)/L - cosine(x3)*(K1*V + K2*x2)/M) / (L - m*l*cosine(x3)*cosine(x3)/M);
   
  out[0] = x1_dt;
  out[1] = x2_dt;
  out[2] = x3_dt;
  out[3] = x4_dt;
}

double swing_up(double in[4]) {
  double constraint = constrain(in[2]);
  double V = numerical_kernel1(in, constraint);
  return sat(Vsat, V);
}

double control(double in[4]) {
  double c = constrain(in[2]);
  if (c > -pi/5.0 && c < pi/5.0) {
    return - (K[0]*in[0] + K[1]*in[1] + K[2]*c + K[3]*in[3]);
  } else {
    return swing_up(in);
  }
}

void derivative(double in[4], double out[4]) {
  double V = sat(Vsat, control(in));
  numerical_kernel2(in, V, out);   
}

void rk4_step(double dt, double state[4]) {
  double dx[4]; 
  derivative(state, dx);
 
  double k2[4];
  for (int i = 0; i < 4; i++) {
    k2[i] = dx[i] * dt;
  }
  
  double xv[4];
  for (int i = 0; i < 4; i++) {
    xv[i] = state[i] + k2[i]/2.0;
  }

  derivative(xv, dx);
  double k3[4];
  for (int i = 0; i < 4; i++) {
    k3[i] = dx[i] * dt;
  }
  
  for (int i = 0; i < 4; i++) {
    xv[i] = state[i] + k3[i]/2.0;
  }

  derivative(xv, dx);
  double k4[4];
  for (int i = 0; i < 4; i++) {
    k4[i] = dx[i] * dt;
  }

  for (int i = 0; i < 4; i++) {
    xv[i] = state[i] + k4[i];
  }

  derivative(xv, dx);
  double k1[4];
  for (int i = 0; i < 4; i++) {
    k1[i] = dt * dx[i];
  }
  // update time
  t += dt;
  // update state
  for (int i = 0; i < 4; i++) {
    state[i] = average(state[i], k1[i], k2[i], k3[i], k4[i]);
  }
}

// main function
int main(void) {
  double dt = 0.01; 
  t = 0.0;
  double state[4];  
  double end = 10;

  /* NEW - GET RANGE: 
     Output file containing the fuzzed input values */
  FILE *file1, *file2;
  file1 = fopen("kernel1_range.dat","r");
  file2 = fopen("kernel2_range.dat","r");
  if ((file1 == NULL) || (file2 == NULL)){
    puts("Error while opening file kernel_range.dat");
    exit(1);
  }
  /* NEW: Scan the fuzzed input */
  int nb_scanf = 0;
  nb_scanf = scanf("%lf",&state[0]);
  nb_scanf += scanf("%lf",&state[1]);
  nb_scanf += scanf("%lf",&state[2]);
  nb_scanf += scanf("%lf",&state[3]);
  
  if ( (nb_scanf != 4) ||
       (state[0] < 0.0) || (state[0] > 0.0009) ||
       (state[1] < 0.0) || (state[1] > 0.0009) ||
       (state[2] < pi/2) || (state[2] > pi) ||
       (state[3] < 0.0) || (state[3] > 0.0009) ||
       isnan(state[0]) || isnan(state[1]) ||
       isnan(state[2]) || isnan(state[3])) {
    exit(1);
  }

  /* NEW - GET RANGE: 
     Read file containing the kernel ranges. 
     Initialize ranges if the file is empty. */
  int i;
  for (i=0;i<5;i++) {
    nb_scanf = fscanf(file1,"%le",&range_kernel1[i].min);
    nb_scanf += fscanf(file1,"%le",&range_kernel1[i].max);
    nb_scanf += fscanf(file2,"%le",&range_kernel2[i].min);
    nb_scanf += fscanf(file2,"%le",&range_kernel2[i].max);
    if (nb_scanf != 4) {
      init_range(&range_kernel1[i]);
      init_range(&range_kernel2[i]);
    } else {
      range_kernel1[i].init = 1;
      range_kernel2[i].init = 1;
    }
  }
  
  // integrate
  while (t <= end) {
    rk4_step(dt, state);
  }

  /* NEW - GET RANGE: 
     Print the final range in the file;  */
  freopen(NULL,"w+",file1);
  freopen(NULL,"w+",file2);
  for (i=0;i<5;i++) {
    fprintf(file1,"%.20le %.20le\n",range_kernel1[i].min,range_kernel1[i].max);
    fprintf(file2,"%.20le %.20le\n",range_kernel2[i].min,range_kernel2[i].max);
  }  
  /* NEW - GET RANGE: Close file */
  fclose(file1);
  fclose(file2);  
  return 0;
}
