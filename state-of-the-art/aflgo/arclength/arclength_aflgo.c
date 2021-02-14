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
  }
  /* NEW: Check for Nans and Inf */
  if (isnan(t1)) {
    assert(0 && "NaN in kernel");
  }
  if (isinf(t1)) {
    assert(0 && "INF in kernel");
  }
  return t1;
}

int main(int argc, char **argv) {
  int i, n = 1, nb_scanf = 0;
  /* NEW: Scan the fuzzed input */
  nb_scanf = scanf("%d",&n);
  
  /* NEW: Exit if the input is not in the range */
  if ( (nb_scanf !=1) ||
       (n < 1) || (n > 1000000) ) {
    exit(1);
  }
  double h, t2, dppi, t_old;
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
