#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#define cot(x) (1.0 / tan(x))

#define TRUE  1
#define FALSE 0

#define max_surfaces 10
#define ITERATIONS 5

#define fabs(x)  ((x < 0.0) ? -x : x)

#define pic 3.1415926535897932

char tbfr[132];

short current_surfaces;
short paraxial;

double clear_aperture;

double aberr_lspher;
double aberr_osc;
double aberr_lchrom;

double max_lspher;
double max_osc;
double max_lchrom;

double radius_of_curvature;
double object_distance;
double ray_height;
double axis_slope_angle;
double from_index;
double to_index;

double s[max_surfaces][5];
double od_sa[2][2];

char outarr[8][80];         
int itercount;                     
int niter = ITERATIONS;            

char *refarr[] = {          

        "   Marginal ray          47.09479120920   0.04178472683",
        "   Paraxial ray          47.08372160249   0.04177864821",
        "Longitudinal spherical aberration:        -0.01106960671",
        "    (Maximum permissible):                 0.05306749907",
        "Offense against sine condition (coma):     0.00008954761",
        "    (Maximum permissible):                 0.00250000000",
        "Axial chromatic aberration:                0.00448229032",
        "    (Maximum permissible):                 0.05306749907"
};

double pi = pic;
double twopi = pic * 2.0;
double piover4 = pic / 4.0;
double fouroverpi = 4.0 / pic;
double piover2 = pic / 2.0;

double atanc[] = {
  0.0,
  0.4636476090008061165,
  0.7853981633974483094,
  0.98279372324732906714,
  1.1071487177940905022,
  1.1902899496825317322,
  1.2490457723982544262,
  1.2924966677897852673,
  1.3258176636680324644
};


void numerical_kernel1(double *iang_sin, double from_index, double to_index,
  double slope_angle, double obj_dist, double ray_height,
    double *rang_sin, double *old_axis_slope_angle) {
  *rang_sin = (from_index / to_index) * (*iang_sin);
  __CPROVER_assert(!(isinf(&rang_sin)), "infinity");

  *old_axis_slope_angle = slope_angle;
  axis_slope_angle = slope_angle + (*iang_sin) - (*rang_sin);
  __CPROVER_assert(!(isinf(axis_slope_angle)), "infinity");

  if (obj_dist != 0.0) {
    ray_height = obj_dist * (*old_axis_slope_angle);
  }
  object_distance = ray_height / axis_slope_angle;
  __CPROVER_assert(!(isinf(object_distance)), "infinity");
  __CPROVER_assert(!(isnan(object_distance)), "nan");
}

void numerical_kernel2(double *iang_sin, double from_index, double to_index,
  double slope_angle, double rad_curvature,  
    double *iang, double *rang_sin, double *old_axis_slope_angle, double *sagitta) {
  *iang = asin(*iang_sin);
  __CPROVER_assert(!(isinf(&iang)), "infinity");
  *rang_sin = (from_index / to_index) * (*iang_sin);
  __CPROVER_assert(!(isinf(&rang_sin)), "infinity");
  *old_axis_slope_angle = slope_angle;
  axis_slope_angle = slope_angle + (*iang) - asin(*rang_sin);
  __CPROVER_assert(!(isinf(axis_slope_angle)), "infinity");
  *sagitta = sin((*old_axis_slope_angle + (*iang)) / 2.0);
  *sagitta = 2.0 * radius_of_curvature*(*sagitta)*(*sagitta);
  __CPROVER_assert(!(isinf(&sagitta)), "infinity");
  object_distance = ((rad_curvature * sin(*old_axis_slope_angle + (*iang))) *
              cot(axis_slope_angle)) + (*sagitta);
  __CPROVER_assert(!(isinf(object_distance)), "infinity");
  __CPROVER_assert(!(isnan(object_distance)), "nan");

}

void numerical_kernel3(double from_index, double to_index, 
  double slope_angle, double obj_dist,
    double *rang) {
  *rang = -asin((from_index / to_index) * sin(slope_angle));
  __CPROVER_assert(!(isinf(&rang)), "infinity");
  object_distance = obj_dist * ((to_index * cos(-(*rang))) / 
    (from_index * cos(slope_angle)));
  __CPROVER_assert(!(isinf(object_distance)), "infinity");
  __CPROVER_assert(!(isnan(object_distance)), "nan");
  axis_slope_angle = -(*rang);
}

void transit_surface() {
  double iang,               
         rang,               
         iang_sin,           
         rang_sin,         
         old_axis_slope_angle, sagitta;
  if (paraxial) {
    if (radius_of_curvature != 0.0) {
      if (object_distance == 0.0) {
        axis_slope_angle = 0.0;
        iang_sin = ray_height / radius_of_curvature;
      } else {
        iang_sin = ((object_distance -
            radius_of_curvature) / radius_of_curvature) * axis_slope_angle;
      }

      numerical_kernel1(&iang_sin, from_index, to_index, axis_slope_angle,
      object_distance, ray_height, &rang_sin, &old_axis_slope_angle);
      return;
    }
    object_distance = object_distance * (to_index / from_index);
    axis_slope_angle = axis_slope_angle * (from_index / to_index);
    return;
  }

  if (radius_of_curvature != 0.0) {
    if (object_distance == 0.0) {
      axis_slope_angle = 0.0;
      iang_sin = ray_height / radius_of_curvature;
    } else {
      iang_sin = ((object_distance - radius_of_curvature) / 
        radius_of_curvature) * sin(axis_slope_angle);
    }
    numerical_kernel2(&iang_sin, from_index, to_index, axis_slope_angle, 
      radius_of_curvature, &iang, &rang_sin, &old_axis_slope_angle, &sagitta);
    return;          
  }
  numerical_kernel3(from_index, to_index, axis_slope_angle, object_distance, &rang);        
}

void trace_line(int line, double ray_h, double spectral_line[]) {
  int i;

  object_distance = 0.0;
  ray_height = ray_h;
  from_index = 1.0;
  for (i = 1; i <= current_surfaces; i++) {
    radius_of_curvature = s[i][1];
    to_index = s[i][2];
    if (to_index > 1.0) {
      to_index = to_index + ((spectral_line[4] -
          spectral_line[line]) /
            (spectral_line[3] - spectral_line[6])) * ((s[i][2] - 1.0) /
              s[i][3]);
    }
    transit_surface();
    from_index = to_index;
    if (i < current_surfaces) {
      object_distance = object_distance - s[i][4];
    }
  }
}

int main(void)
{
  int i, j, k, errors;
  double od_fline, od_cline;
  long passes;
  double spectral_line[9];

  __CPROVER_assume((spectral_line[1] >= 7521.0) && (spectral_line[1] <= 7621.0));
  __CPROVER_assume((spectral_line[2] >= 6769.955) && (spectral_line[2] <= 6869.955));
  __CPROVER_assume((spectral_line[3] >= 6462.816) && (spectral_line[3] <= 6562.816));
  __CPROVER_assume((spectral_line[4] >= 5795.944) && (spectral_line[4] <= 5895.944));
  __CPROVER_assume((spectral_line[5] >= 5169.557) && (spectral_line[5] <= 5269.557));
  __CPROVER_assume((spectral_line[6] >= 4761.344) && (spectral_line[6] <= 4861.344));
  __CPROVER_assume((spectral_line[7] >= 4240.477) && (spectral_line[7] <= 4340.477));
  __CPROVER_assume((spectral_line[8] >= 3868.494) && (spectral_line[8] <= 3968.494));  

  clear_aperture = 4.0;
  s[1][1] = 27.05;
  s[1][2] = 1.5137;
  s[1][3] = 63.6;
  s[1][4] = 0.52;
  s[2][1] = -16.68;
  s[2][2] = 1;
  s[2][3] = 0;
  s[2][4] = 0.138;
  s[3][1] = -16.68;
  s[3][2] = 1.6164;
  s[3][3] = 36.7;
  s[3][4] = 0.38;
  s[4][1] = -78.1;
  s[4][2] = 1;
  s[4][3] = 0;
  s[4][4] = 0;
  
  passes = 0;
  current_surfaces = 4;

  while(passes < niter ) {
    passes++;    
    for (itercount = 0; itercount < niter; itercount++) {
      for (paraxial = 0; paraxial <= 1; paraxial++) {
        trace_line(4, clear_aperture / 2.0, spectral_line);
        od_sa[paraxial][0] = object_distance;
        od_sa[paraxial][1] = axis_slope_angle;
      }
      paraxial = FALSE;


      trace_line(3, clear_aperture / 2.0, spectral_line);
      od_cline = object_distance;


      trace_line(6, clear_aperture / 2.0, spectral_line);
      od_fline = object_distance;

      aberr_lspher = od_sa[1][0] - od_sa[0][0];
      aberr_osc = 1.0 - (od_sa[1][0] * od_sa[1][1]);
      aberr_lchrom = od_fline - od_cline;
      max_lspher = sin(od_sa[0][1]);

      max_lspher = 0.0000926;
      max_osc = 0.0025;
      max_lchrom = max_lspher;
    }
        
    errors = 0;
    for (i = 0; i < 8; i++) {
      if (strcmp(outarr[i], refarr[i]) != 0) {
        k = strlen(refarr[i]);
        for (j = 0; j < k; j++) {
          if (refarr[i][j] != outarr[i][j]) {
            errors++;
          }
        }
      }
    }
  }
  return 0;
}
