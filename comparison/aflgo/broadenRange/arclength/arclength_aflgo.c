#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "compute_range.h"

/* NEW: Global variable to store the range */
__range_t range[1];

double numerical_kernel(double x) {
  
  /* NEW - broaden range */
  broaden_range(x,range);
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

  /* NEW - GET BROAD RANGE: Output file containing the fuzzed input values */
  FILE *file, *old_file;
  file = fopen("broad_kernel_range.dat","r");
  old_file = fopen("../kernel_range.dat","r");  

  if ((file == NULL) || (old_file == NULL)) {
    puts("Error while opening kernel_range.dat");
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
    Read file containing the old kernel ranges
    and broad kernel ranges. */
  nb_scanf = fscanf(file,"%le",&range[0].min);
  nb_scanf += fscanf(file,"%le",&range[0].max);
  if (nb_scanf != 2) {
    fscanf(old_file,"%le",&range[0].min);
    fscanf(old_file,"%le",&range[0].max);
  }

  double h, t2, dppi, t_old;
  double s1;
  double t1 = -1.0;
  dppi = acos(t1);
  s1 = 0.0;
  t1 = 0.0;
  h = dppi / n;

  for( i = 1; i <= n; i++ ) {
    t2 = numerical_kernel (i * h);
    s1 = s1 + sqrt (h*h + (t2 - t1)*(t2 - t1));
    t1 = t2;
  }

  /* NEW - GET RANGE: 
     Print the final range in the file;  */
  freopen(NULL,"w+",file);
  fprintf(file,"%.20le %.20le\n",range[0].min,range[0].max);
  
  /* NEW - GET RANGE: Close file */
  fclose(file);
  fclose(old_file);

  return 0;
}
