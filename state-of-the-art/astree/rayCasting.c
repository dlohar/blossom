#include <stdio.h>
#include <stdlib.h>
#include <math.h>
 
typedef struct { double x, y; } vec;
typedef struct { int n; vec* v; } polygon_t, *polygon;
 
#define BIN_V(op, xx, yy) vec v##op(vec a,vec b){vec c;c.x=xx;c.y=yy;return c;}
#define BIN_S(op, r) double v##op(vec a, vec b){ return r; }
BIN_V(sub, a.x - b.x, a.y - b.y);
BIN_V(add, a.x + b.x, a.y + b.y);
BIN_S(dot, a.x * b.x + a.y * b.y);
BIN_S(cross, a.x * b.y - a.y * b.x);
 
vec numerical_kernel(vec a, double s, vec b)
{
  vec c;
  c.x = a.x + s * b.x;
  c.y = a.y + s * b.y;
  __ASTREE_assert((!(isinf(c.x))));
  __ASTREE_assert((!(isnan(c.x))));
  __ASTREE_assert((!(isinf(c.y))));
  __ASTREE_assert((!(isnan(c.y))));
  return c;
}
 
int intersect(vec x0, vec x1, vec y0, vec y1, double tol, vec *sect)
{
  vec dx = vsub(x1, x0), dy = vsub(y1, y0);
  double d = vcross(dy, dx), a;
  if (!d) return 0;
 
  a = (vcross(x0, dx) - vcross(y0, dx)) / d;
  if (sect)  {
    *sect = numerical_kernel(y0, a, dy);
  }
 
  if (a < -tol || a > 1 + tol) return -1;
  if (a < tol || a > 1 - tol) return 0;
 
  a = (vcross(x0, dy) - vcross(y0, dy)) / d;
  if (a < 0 || a > 1) return -1;
 
  return 1;
}
 
double dist(vec x, vec y0, vec y1, double tol)
{
  vec dy = vsub(y1, y0);
  vec x1, s;
  int r;
 
  x1.x = x.x + dy.y; x1.y = x.y - dy.x;
  r = intersect(x, x1, y0, y1, tol, &s);
  if (r == -1) return HUGE_VAL;
  s = vsub(s, x);
  return sqrt(vdot(s, s));
}
 
#define for_v(i, z, p) for(i = 0, z = p->v; i < p->n; i++, z++)
int inside(vec v, polygon p, double tol)
{
  int i, k, crosses, intersectResult;
  vec *pv;
  double min_x, max_x, min_y, max_y;
  __ASTREE_unroll((50))
  for (i = 0; i < p->n; i++) {
    k = (i + 1) % p->n;
    min_x = dist(v, p->v[i], p->v[k], tol);
    if (min_x < tol) return 0;
  }
 
  min_x = max_x = p->v[0].x;
  min_y = max_y = p->v[1].y;
  __ASTREE_unroll((50))
  for_v(i, pv, p) {
    if (pv->x > max_x) max_x = pv->x;
    if (pv->x < min_x) min_x = pv->x;
    if (pv->y > max_y) max_y = pv->y;
    if (pv->y < min_y) min_y = pv->y;
  }
  if (v.x < min_x || v.x > max_x || v.y < min_y || v.y > max_y)
    return -1;
 
  max_x -= min_x; max_x *= 2;
  max_y -= min_y; max_y *= 2;
  max_x += max_y;
 
  vec e;
  int loopBound = 5;
  __ASTREE_unroll((50))
  while (loopBound >= 0) {
    crosses = 0;
    e.x = v.x + (1 + rand() / (RAND_MAX + 1.)) * max_x;
    e.y = v.y + (1 + rand() / (RAND_MAX + 1.)) * max_x;
    __ASTREE_unroll((50))
    for (i = 0; i < p->n; i++) {
      k = (i + 1) % p->n;
      intersectResult = intersect(v, e, p->v[i], p->v[k], tol, 0);
      if (!intersectResult) break;
 
      if (intersectResult == 1) crosses++;
    }
    if (i == p->n) break;
  }
  return (crosses & 1) ? 1 : -1;
}
 
int main(void)
{
  double c1, c2;
  __ASTREE_modify((c1; [0.0, 10.0]));
  __ASTREE_modify((c2; [0.0, 5.0]));
  vec vsq[] = { {0,0}, {10,0}, {10,10}, {0,10},
      {2.5,2.5}, {7.5,0.1}, {7.5,7.5}, {2.5,7.5}};
 
  polygon_t sq = { 4, vsq }, 
  sq_hole = { 8, vsq }; 
  vec d = { 5, 5 };
  vec c = {c1, c2};

  inside(c, &sq, 1e-10);
  
  return 0;
}
