#include <stdlib.h>
#include <math.h>

double numerical_kernel(double x) {
  int k, n = 5;
  double t1;
  double d1 = 1.0L;
  t1 = x;
  __ASTREE_unroll((50))
  for( k = 1; k <= n; k++ ) {
    d1 = 2.0 * d1;
    t1 = t1 + sin (d1 * x) / d1;
  }
  __ASTREE_assert((!(isinf(t1))));
  __ASTREE_assert((!(isnan(t1))));
  return t1;
}

int main(void) {
  int n;
  __ASTREE_modify((n; [1, 1000000]));
  int i;
  double h, t2, dppi,x;
  double s1;
  double t1 = -1.0;
  dppi = acos(t1);
  s1 = 0.0;
  t1 = 0.0;
  h = dppi / n;
  __ASTREE_unroll((50))
  for( i = 1; i <= n; i++ ) {
    x = i*h;
    t2 = numerical_kernel(x);
    s1 = s1 + sqrt (h*h + (t2 - t1)*(t2 - t1));
    t1 = t2;
  }
  return 0;
}
