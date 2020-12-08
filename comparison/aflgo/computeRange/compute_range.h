#ifndef __COMPUTE_RANGE_H__
#define __COMPUTE_RANGE_H__

#ifndef __MATH_H__
#define __MATH_H__
#include <math.h>
#endif /* __MATH_H__ */
typedef struct{
  double min, max;
  int init;
}__range_t;
void init_range (__range_t *);
void compute_range (double, __range_t *);
void broaden_range (double, __range_t *);
#endif /* __COMPUTE_RANGE_H__ */
