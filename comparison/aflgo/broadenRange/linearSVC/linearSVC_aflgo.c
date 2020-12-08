#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "compute_range.h"

double coefficients[3][4] = {{0.1842392154564795, 0.451227272556133, -0.8079415307840663, -0.45071538868087374}, {0.05674216334215214, -0.8981194157558171, 0.4083958984619029, -0.9606849222530257}, {-0.8507702027651778, -0.9868272809923471, 1.3808941178761502, 1.8652996296254423}};
double intercepts[3] = {0.10956209204126635, 1.677892675644221, -1.7095873485238913};

/* NEW: Global variables to store the ranges */
/* 0: feature0, 1: feature1, 2: feature2, 3: feature3 */
__range_t range[4];

double numerical_kernel(double features[], int i){
  /* NEW - broaden range */
  for (int i = 0 ; i<4 ; i++) {
    broaden_range(features[i],&range[i]);
  }
  /* ------------------- END NEW ------------------- */
  double prob = 0.0;
  for (int j = 0, jl = sizeof(coefficients[0]) / sizeof (coefficients[0][0]); j < jl; j++) {
    prob += coefficients[i][j] * features[j];
  }
  return prob;
}


int predict (double features[]) {
  double class_val = -INFINITY;
  int class_idx = -1;
  int i, il, j, jl;
  for (i = 0, il = sizeof(coefficients) / sizeof (coefficients[0]); i < il; i++) {
    double prob = numerical_kernel(features, i);
    if (prob + intercepts[i] > class_val) {
      class_val = prob + intercepts[i];
      class_idx = i;
    }
  }
  return class_idx;
}
  
int main(void) {
  double features[4];
  int i, nb_scanf = 0;
  for (i = 0; i < 4; i++) {
    nb_scanf += scanf("%lf", &features[i]);
  }
  /* Range of the input */
  if ( (nb_scanf != 4) ||
       (features[0] < 4.0) || (features[0] > 8.0) ||
       (features[1] < 2.0) || (features[1] > 4.5) ||
       (features[2] < 1.0) || (features[2] > 7.0) ||
       (features[3] < 0.1) || (features[3] > 2.5) ||
       isnan(features[0]) || isnan(features[1]) ||
       isnan(features[2]) || isnan(features[3])) {
    exit(1);
  }

  /* NEW - GET BROAD RANGE: 
     Output file containing the fuzzed input values */
  FILE *file, *old_file;
  file = fopen("broad_kernel_range.dat","r");
  old_file = fopen("../kernel_range.dat","r");

  if ((file == NULL) || (old_file == NULL)) {
    puts("Error while opening kernel_range.dat");
    exit(1);
  }

  /* NEW - GET RANGE: 
     Read file containing the kernel ranges. 
     Initialize ranges if the file is empty. */
  for (i=0;i<4;i++) {
    nb_scanf = fscanf(file,"%le",&range[i].min);
    nb_scanf += fscanf(file,"%le",&range[i].max);
    if (nb_scanf != 2) {
      fscanf(old_file,"%le",&range[i].min);
      fscanf(old_file,"%le",&range[i].max);
    }
  }

  for(int j = 0; j < 1000;j++) {
    predict(features);
  }

  /* NEW - GET RANGE: 
     Print the final range in the file;  */
  freopen(NULL,"w+",file);
  for (i=0;i<4;i++) {
    fprintf(file,"%.20le %.20le\n",range[i].min,range[i].max);
  }
  
  /* NEW - GET RANGE: Close file */
  fclose(file);
  fclose(old_file);
  
  return 0;
}
