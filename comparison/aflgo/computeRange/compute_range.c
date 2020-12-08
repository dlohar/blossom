#include "compute_range.h"

void init_range (__range_t* range) {
  range[0].init = 0;
  range[0].min = +INFINITY;
  range[0].max = -INFINITY;
}

void compute_range (double new_value, __range_t* range) {
  if (range[0].init == 0) {
    range[0].min = new_value;
    range[0].max = new_value;
    range[0].init = 1;
  } else if (new_value < range[0].min) {
    range[0].min = new_value;
  } else if (new_value > range[0].max) {
    range[0].max = new_value;
  }
  
}

void broaden_range (double new_value, __range_t* range) {
  if (new_value < range[0].min) {
    range[0].min = new_value;
  } else if (new_value > range[0].max) {
    range[0].max = new_value;
  }
}
