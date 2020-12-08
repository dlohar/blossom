#include <stdio.h>
#include <math.h>

double Vsat = 20.0;   
double M = 0.6;       
double m = 0.3;        
double l = 0.3;        
double g = 9.81;      
double Km = 2;         
double Kg = 0.01;      
double R = 6;          
double r = 0.01;        
double K1 = 1.0 / 3.0; 
double K2 = 2.0 / 3.0;  
double I = 0.006;     
double L = 11.0 / 30.0;  
double pi = 3.14159265358979;

double K[4] = {-40.82482905, -30.66703711, -115.89445979, -19.424643};

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
  else { return 1; } 

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
  double x1 = in[0];
  double x2 = in[1];
  double x3 = in[2];
  double x4 = in[3];

  double x1_dt = x2;
  double x3_dt = x4;
  
  double x2_dt = (K1*V - K2*x2 - m*l*g*cosine(x3)*sine(x3)/L + m*l*sine(x3)*x4*x4) / (M - m*l*cosine(x3)*cosine(x3)/L);
  
  double x4_dt = (g*sine(x3) - m*l*x4*x4*cosine(x3)*sine(x3)/L - cosine(x3)*(K1*V + K2*x2)/M) / (L - m*l*cosine(x3)*cosine(x3)/M);
  
  out[0] = x1_dt;
  out[1] = x2_dt;
  out[2] = x3_dt;
  out[3] = x4_dt;
}

double swing_up(double in[4]) {
  double constraint = constrain(in[2]);
  __ASTREE_log_vars((in[0];inter));
  __ASTREE_log_vars((in[1];inter));
  __ASTREE_log_vars((in[2]; inter));
  __ASTREE_log_vars((in[3]; inter));
  __ASTREE_log_vars((constraint; inter));
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
  __ASTREE_log_vars((in[0];inter));
  __ASTREE_log_vars((in[1];inter));
  __ASTREE_log_vars((in[2]; inter));
  __ASTREE_log_vars((in[3]; inter));
  __ASTREE_log_vars((V; inter));
  numerical_kernel2(in, V, out); 
}

void rk4_step(double dt, double state[4]) {
  double dx[4]; 
  derivative(state, dx);
 
  double k2[4];
  __ASTREE_unroll((50))
  for (int i = 0; i < 4; i++) {
    k2[i] = dx[i] * dt;
  }
  
  double xv[4];
  __ASTREE_unroll((50))
  for (int i = 0; i < 4; i++) {
    xv[i] = state[i] + k2[i]/2.0;
  }

  derivative(xv, dx);
  double k3[4];
  __ASTREE_unroll((50))
  for (int i = 0; i < 4; i++) {
    k3[i] = dx[i] * dt;
  }
  __ASTREE_unroll((50))
  for (int i = 0; i < 4; i++) {
    xv[i] = state[i] + k3[i]/2.0;
  }

  derivative(xv, dx);
  double k4[4];
  __ASTREE_unroll((50))
  for (int i = 0; i < 4; i++) {
    k4[i] = dx[i] * dt;
  }
  __ASTREE_unroll((50))
  for (int i = 0; i < 4; i++) {
    xv[i] = state[i] + k4[i];
  }

  derivative(xv, dx);
  double k1[4];
  __ASTREE_unroll((50))
  for (int i = 0; i < 4; i++) {
    k1[i] = dt * dx[i];
  }
  t += dt;
  __ASTREE_unroll((50))
  for (int i = 0; i < 4; i++) {
    state[i] = average(state[i], k1[i], k2[i], k3[i], k4[i]);
  }
}

int main(void) {
  double dt = 0.01;
  double state[4];
  __ASTREE_modify((state[0]; [0.0, 0.0009]));
  __ASTREE_modify((state[1]; [0.0, 0.0009]));
  __ASTREE_modify((state[2]; [pi/2, pi]));
  __ASTREE_modify((state[3]; [0.0, 0.0009]));
  double end = 10;

  __ASTREE_unroll((50))
  while (t <= end) {
    rk4_step(dt, state);
  }
  return 0;
}
