#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdbool.h> 

#define cot(x) (1.0 / tan(x))

#define TRUE  1
#define FALSE 0

#define max_surfaces 10

/*  Local variables  */

static char tbfr[132];

static short current_surfaces;
static short paraxial;

static double clear_aperture;

static double aberr_lspher;
static double aberr_osc;
static double aberr_lchrom;

static double max_lspher;
static double max_osc;
static double max_lchrom;

static double radius_of_curvature;
static double object_distance;
static double ray_height;
static double axis_slope_angle;
static double from_index;
static double to_index;

static double spectral_line[9];
static double s[max_surfaces][5];
static double od_sa[2][2];




static char outarr[8][80];         /* Computed output of program goes here */

int itercount;                     /* The iteration counter for the main loop
                                      in the program is made global so that
                                      the compiler should not be allowed to
                                      optimise out the loop over the ray
                                      tracing code. */

#define ITERATIONS 5
int niter = ITERATIONS;            /* Iteration counter */

static char *refarr[] = {          /* Reference results.  These happen to
                                      be derived from a run on Microsoft
                                      Quick BASIC on the IBM PC/AT. */

        "   Marginal ray          47.09479120920   0.04178472683",
        "   Paraxial ray          47.08372160249   0.04177864821",
        "Longitudinal spherical aberration:        -0.01106960671",
        "    (Maximum permissible):                 0.05306749907",
        "Offense against sine condition (coma):     0.00008954761",
        "    (Maximum permissible):                 0.00250000000",
        "Axial chromatic aberration:                0.00448229032",
        "    (Maximum permissible):                 0.05306749907"
};

#define sin I_sin
#define cos I_cos
#define tan I_tan
#define sqrt I_sqrt
#define atan I_atan
#define atan2 I_atan2
#define asin I_asin

#define fabs(x)  ((x < 0.0) ? -x : x)

#define pic 3.1415926535897932

/*  Commonly used constants  */

static double pi = pic,
              twopi = pic * 2.0,
              piover4 = pic / 4.0,
              fouroverpi = 4.0 / pic,
              piover2 = pic / 2.0;

static double atanc[] = {
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

void numerical_kernel1(double x,  
// Values of the following variables are changed, hence passed as arguments
  double *a, double *b) {

  double z = x * x;
  *b = ((((893025.0 * z + 49116375.0) * z + 425675250.0) * z +
            1277025750.0) * z + 1550674125.0) * z + 654729075.0;
  *a = (((13852575.0 * z + 216602100.0) * z + 891080190.0) * z +
            1332431100.0) * z + 654729075.0;
}

double numerical_kernel5(double x) {
/* =========================================================== */
  double r, y, z;
  if (x < piover4) {
    y = x * fouroverpi;
    z = y * y;
    r = y * (((((((-0.202253129293E-13 * z + 0.69481520350522E-11) * z -
          0.17572474176170806E-8) * z + 0.313361688917325348E-6) * z -
          0.365762041821464001E-4) * z + 0.249039457019271628E-2) * z -
          0.0807455121882807815) * z + 0.785398163397448310);
  } else {
    y = (piover2 - x) * fouroverpi;
    z = y * y;
    r = ((((((-0.38577620372E-12 * z + 0.11500497024263E-9) * z -
      0.2461136382637005E-7) * z + 0.359086044588581953E-5) * z -
      0.325991886926687550E-3) * z + 0.0158543442438154109) * z -
      0.308425137534042452) * z + 1.0;
  }
  return r;
}


double aint(x)
double x;
{
        long l;
        l = x;
        if ((int)(-0.5) != 0  &&  l < 0 )
           l++;
        x = l;
        return x;
}

/*  sin(x)        Return sine, x in radians  */

static double sin(x)
double x;
{
        int sign;
        double r;

        x = (((sign= (x < 0.0)) != 0) ? -x: x);

        if (x > twopi)
           x -= (aint(x / twopi) * twopi);

        if (x > pi) {
           x -= pi;
           sign = !sign;
        }

        if (x > piover2)
           x = pi - x;

        r = numerical_kernel5(x);
        return sign ? -r : r;
}

/*  cos(x)        Return cosine, x in radians, by identity  */

static double cos(x)
double x;
{
        x = (x < 0.0) ? -x : x;
        if (x > twopi)                /* Do range reduction here to limit */
           x = x - (aint(x / twopi) * twopi); /* roundoff on add of PI/2    */
        return sin(x + piover2);
}

/*  tan(x)        Return tangent, x in radians, by identity  */

static double tan(x)
double x;
{
        return sin(x) / cos(x);
}

/*  sqrt(x)       Return square root.  Initial guess, then Newton-
                  Raphson refinement  */

double sqrt(x)
double x;
{
        double c, cl, y;
        int n;

        if (x == 0.0)
           return 0.0;

        if (x < 0.0) {
           exit(1);
        }

        y = (0.154116 + 1.893872 * x) / (1.0 + 1.047988 * x);

        c = (y - x / y) / 2.0;
        cl = 0.0;
        for (n = 50; c != cl && n--;) {
           y = y - c;
           cl = c;
           c = (y - x / y) / 2.0;
        }
        return y;
}

/*  atan(x)       Return arctangent in radians,
                  range -pi/2 to pi/2  */


static double atan(x)
double x;
{
        int sign, l, y;
        double a, b, z;

        x = (((sign = (x < 0.0)) != 0) ? -x : x);
        l = 0;

        if (x >= 4.0) {
           l = -1;
           x = 1.0 / x;
           y = 0;
           goto atl;
        } else {
           if (x < 0.25) {
              y = 0;
              goto atl;
           }
        }

        y = aint(x / 0.5);
        z = y * 0.5;
        x = (x - z) / (x * z + 1);

atl:
      numerical_kernel1(x, &a, &b);
        a = (a / b) * x + atanc[y];
        if (l)
           a=piover2 - a;
        return sign ? -a : a;
}

/*  atan2(y,x)    Return arctangent in radians of y/x,
                  range -pi to pi  */

static double atan2(y, x)
double y, x;
{
        double temp;
        if (x == 0.0) {
           if (y == 0.0)   /*  Special case: atan2(0,0) = 0  */
              return 0.0;
           else if (y > 0)
              return piover2;
           else
              return -piover2;
        }
        temp = atan(y / x);
        if (x < 0.0) {
           if (y >= 0.0)
              temp += pic;
           else
              temp -= pic;
        }
        return temp;
}

/*  asin(x)       Return arcsine in radians of x  */

static double asin(x)
double x;
{
        if (fabs(x)>1.0) {
          printf("x is bigger than 1.0! x = %f\n", x);
           exit(1);
        }
        return atan2(x, sqrt(1 - x * x));
}

void numerical_kernel2(double *iang_sin, double from_index, double to_index,
  double slope_angle, double obj_dist, double old_ray_height,
// Values of the following variables are changed, hence passed as arguments
    double *rang_sin, double *old_axis_slope_angle) {
  *rang_sin = (from_index / to_index) * (*iang_sin);
  *old_axis_slope_angle = slope_angle;
  axis_slope_angle = slope_angle + (*iang_sin) - (*rang_sin);
  if (obj_dist != 0.0) {
    ray_height = obj_dist * (*old_axis_slope_angle);
  }
  object_distance = ray_height / axis_slope_angle;
}

void numerical_kernel3(double *iang_sin, double from_index, double to_index,
  double slope_angle, double rad_curvature,  
// Values of the following variables are changed, hence passed as arguments
    double *iang, double *rang_sin, double *old_axis_slope_angle, double *sagitta) {
/* =========================================================== */
  *iang = asin(*iang_sin);
  *rang_sin = (from_index / to_index) * (*iang_sin);
  *old_axis_slope_angle = slope_angle;
  axis_slope_angle = slope_angle + (*iang) - asin(*rang_sin);
  *sagitta = sin((*old_axis_slope_angle + (*iang)) / 2.0);
  *sagitta = 2.0 * radius_of_curvature*(*sagitta)*(*sagitta);
  object_distance = ((rad_curvature * sin(*old_axis_slope_angle + (*iang))) *
              cot(axis_slope_angle)) + (*sagitta);

}

void numerical_kernel4(double from_index, double to_index, 
  double slope_angle, double obj_dist,
  // Values of the following variables are changed, hence passed as arguments
    double *rang) {

  *rang = -asin((from_index / to_index) * sin(slope_angle));
  object_distance = obj_dist * ((to_index * cos(-(*rang))) / 
    (from_index * cos(slope_angle)));
  axis_slope_angle = -(*rang);
}
/*  If you are using an ancient compiler and get an error on
    the next line, add:
       #define void
    at the top of this file.  */
static void transit_surface() {
        double iang,               /* Incidence angle */
               rang,               /* Refraction angle */
               iang_sin,           /* Incidence angle sin */
               rang_sin,           /* Refraction angle sin */
               old_axis_slope_angle, sagitta;

        if (paraxial) {
           if (radius_of_curvature != 0.0) {
              if (object_distance == 0.0) {
                 axis_slope_angle = 0.0;
                 iang_sin = ray_height / radius_of_curvature;
              } else {
                  iang_sin = ((object_distance -
                    radius_of_curvature) / radius_of_curvature) *
                    axis_slope_angle;
                }
              numerical_kernel2(&iang_sin, from_index, to_index, axis_slope_angle,
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
              iang_sin = ((object_distance -
                 radius_of_curvature) / radius_of_curvature) *
                 sin(axis_slope_angle);
           }
           numerical_kernel3(&iang_sin, from_index, to_index, axis_slope_angle, 
            radius_of_curvature, &iang, &rang_sin, &old_axis_slope_angle, &sagitta);
           return;
        }


        numerical_kernel4(from_index, to_index, axis_slope_angle, 
          object_distance, &rang);
}

/*  Perform ray trace in specific spectral line  */

static void trace_line(line, ray_h)
int line;
double ray_h;
{
        int i;
        object_distance = 0.0;
        ray_height = ray_h;
        from_index = 1.0;

        for (i = 1; i <= current_surfaces; i++) {
           radius_of_curvature = s[i][1];
           to_index = s[i][2];

           if (to_index > 1.0)
              to_index = to_index + ((spectral_line[4] -
                 spectral_line[line]) /
                 (spectral_line[3] - spectral_line[6])) * ((s[i][2] - 1.0) /
                 s[i][3]);
           transit_surface();
           from_index = to_index;
           if (i < current_surfaces)
              object_distance = object_distance - s[i][4];
        }
}

void original_program_main(double arg1, double arg2, double arg3, double arg4, double arg5, double arg6, double arg7, double arg8) {
  int i, j, k, errors;
  double od_fline, od_cline;
  long passes;

  /* Generating ranges randomly */  
    spectral_line[1] = arg1;
    spectral_line[2] = arg2;
    spectral_line[3] = arg3;
    spectral_line[4] = arg4;
    spectral_line[5] = arg5;
    spectral_line[6] = arg6;
    spectral_line[7] = arg7;
    spectral_line[8] = arg8;
  /* =========================================*/
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
            trace_line(4, clear_aperture / 2.0);
            od_sa[paraxial][0] = object_distance;
            od_sa[paraxial][1] = axis_slope_angle;
          }
          paraxial = FALSE;

          /* Trace marginal ray in C */

          trace_line(3, clear_aperture / 2.0);
          od_cline = object_distance;

          /* Trace marginal ray in F */

          trace_line(6, clear_aperture / 2.0);
          od_fline = object_distance;

          aberr_lspher = od_sa[1][0] - od_sa[0][0];
          aberr_osc = 1.0 - (od_sa[1][0] * od_sa[1][1]);
          aberr_lchrom = od_fline - od_cline;
          max_lspher = sin(od_sa[0][1]);

          /* D light */
          max_lspher = 0.0000926;
          max_osc = 0.0025;
          max_lchrom = max_lspher;
        }
        errors = 0;
        for (i = 0; i < 8; i++) {
          if (strcmp(outarr[i], refarr[i]) != 0) {
            k = strlen(refarr[i]);
            for (j = 0; j < k; j++) {
              if (refarr[i][j] != outarr[i][j])
                errors++;
            }
          }
        }
    }
}
