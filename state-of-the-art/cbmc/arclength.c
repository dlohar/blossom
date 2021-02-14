#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>

double numerical_kernel(double x) {
  int k, n = 5;
  double t1;
  double d1 = 1.0L;
  t1 = x;
  for( k = 1; k <= n; k++ ) {
    d1 = 2.0 * d1;
    t1 = t1 + sin (d1 * x) / d1;
    __CPROVER_assert(!(isinf(t1)), "infinity");
    __CPROVER_assert(!(isnan(t1)),"nan");
  }

  return t1;
}

int main(void) {
  int n;
  __CPROVER_assume((n >= 0) && (n <= 1000000));

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
  return 0;
}
