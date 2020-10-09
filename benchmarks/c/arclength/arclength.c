#include <stdio.h>
#include <math.h>
#include <stdlib.h>

/* func function */
double numerical_kernel(double x) {
  int k, n = 5;
  double t1;
  double d1 = 1.0L;
  t1 = x;
  for( k = 1; k <= n; k++ ) {
    d1 = 2.0 * d1;
    t1 = t1 + sin (d1 * x) / d1;
  }
  return t1;
}

void original_program_main(int n) {
  int i;
  double h, t2, dppi;
  double s1;
  double t1 = -1.0;
  dppi = acos(t1);
  s1 = 0.0;
  t1 = 0.0;
  h = dppi / n;
  for( i = 1; i <= n; i++ ) {
    t2 = numerical_kernel(i * h); 
    s1 = s1 + sqrt (h*h + (t2 - t1)*(t2 - t1));
    t1 = t2;
  }
}
