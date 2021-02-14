#include <stdlib.h>
#include <stdio.h>
#include <math.h>

double coefficients[3][4] = {{0.1842392154564795, 0.451227272556133, -0.8079415307840663, -0.45071538868087374}, {0.05674216334215214, -0.8981194157558171, 0.4083958984619029, -0.9606849222530257}, {-0.8507702027651778, -0.9868272809923471, 1.3808941178761502, 1.8652996296254423}};
double intercepts[3] = {0.10956209204126635, 1.677892675644221, -1.7095873485238913};

double numerical_kernel(double features[], int i){
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

void original_program_main(double features[4]) {
    predict(features);
}
