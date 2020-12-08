#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "compute_range.h"

/* NEW: Global variable to store the range */
__range_t range[1];

/* Kernel is the whole function */
double numerical_kernel(double x) {
  
  /* NEW - GET RANGE: 
     compute the range */
  compute_range(x,range);
  /* ------------------- END NEW ------------------- */

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

int main(int argc, char **argv) {
  int i, n = 1, nb_scanf = 0;;

  /* NEW - GET RANGE: 
     Output file containing the fuzzed input values */
  FILE *file;
  file = fopen("kernel_range.dat","r");  

  if (file == NULL){
    puts("Error while opening file kernel_range.dat");
    exit(1);
  }

  /* Scan the fuzzed input */
  nb_scanf = scanf("%d",&n);
  
  /* Exit if the input is not in the proposed interval */
  if ( (nb_scanf !=1) ||
       (n < 1) || (n > 1000000) ) {
    exit(1);
  }

 /* NEW - GET RANGE: 
    Read file containing the kernel ranges. 
    Initialize ranges if the file is empty. */
  nb_scanf = fscanf(file,"%le",&range[0].min);
  nb_scanf += fscanf(file,"%le",&range[0].max);
  if (nb_scanf != 2) {
    init_range(range);
  } else {
    range[0].init = 1;
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
  // final answer is stored in variable s1

  /* NEW - GET RANGE: 
     Print the final range in the file;  */
  freopen(NULL,"w+",file);
  fprintf(file,"%.20le %.20le\n",range[0].min,range[0].max);
  
  /* NEW - GET RANGE: Close file */
  fclose(file);

  return 0;
}
