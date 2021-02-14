#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
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

/* The  test  case  used  in  this program is the  design for a 4 inch
   achromatic telescope  objective  used  as  the  example  in  Wyld's
   classic  work  on  ray  tracing by hand, given in Amateur Telescope
   Making, Volume 3.  */

static double testcase[4][4] = {
  {27.05, 1.5137, 63.6, 0.52},
  {-16.68, 1, 0, 0.138},
  {-16.68, 1.6164, 36.7, 0.38},
  {-78.1, 1, 0, 0}
};

/*  Internal trig functions (used only if INTRIG is  defined).   These
    standard  functions  may be enabled to obtain timings that reflect
    the machine's floating point performance rather than the speed  of
    its trig function evaluation.  */

/*  The following definitions should keep you from getting intro trouble
    with compilers which don't let you redefine intrinsic functions.  */

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

/*  Coefficients for ATAN evaluation  */

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
  /* NEW: check for Nan and Infinity */
  if (isnan(*a) || isnan(*b)){
    assert(0 && "NaN in kernel1");
  }
  if (isinf(*a) || isinf(*b)){
    assert(0 && "INF in kernel1");
  }
}

double numerical_kernel5(double x) {
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
  /* NEW: check for Nan and Infinity */
  if (isnan(r)){
    assert(0 && "NaN in kernel5");
  }
  if (isinf(r)){
    assert(0 && "INF in kernel5");
  } 
  return r;  
}


/*  aint(x)       Return integer part of number.  Truncates towards 0    */

double aint(x)
     double x;
{
  long l;

  /*  Note that this routine cannot handle the full floating point
      number range.  This function should be in the machine-dependent
      floating point library!  */

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
  double y, r, z;

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
    exit(1);
  }
  return atan2(x, sqrt(1 - x * x));
}

/*            Calculate passage through surface

              If  the variable PARAXIAL is true, the trace through the
              surface will be done using the paraxial  approximations.
              Otherwise,  the normal trigonometric trace will be done.

              This routine takes the following inputs:

              RADIUS_OF_CURVATURE         Radius of curvature of surface
	      being crossed.  If 0, surface is
	      plane.

              OBJECT_DISTANCE             Distance of object focus from
	      lens vertex.  If 0, incoming
	      rays are parallel and
	      the following must be specified:

              RAY_HEIGHT                  Height of ray from axis.  Only
	      relevant if OBJECT.DISTANCE == 0

              AXIS_SLOPE_ANGLE            Angle incoming ray makes with axis
	      at intercept

              FROM_INDEX                  Refractive index of medium being left

              TO_INDEX                    Refractive index of medium being
	      entered.

              The outputs are the following variables:

              OBJECT_DISTANCE             Distance from vertex to object focus
	      after refraction.

              AXIS_SLOPE_ANGLE            Angle incoming ray makes with axis
	      at intercept after refraction.

*/

/*  If you are using an ancient compiler and get an error on
    the next line, add:
    #define void
    at the top of this file.  */

void numerical_kernel2(double *iang_sin, double from_index, double to_index,
  double slope_angle, double obj_dist, double old_ray_height,
// Values of the following variables are changed, hence passed as arguments
    double *rang_sin, double *old_axis_slope_angle) {
  if (to_index == 0.0){
    assert(0 && "Division by zero in kernel2");
  }
  *rang_sin = (from_index / to_index) * (*iang_sin);
  *old_axis_slope_angle = slope_angle;
  axis_slope_angle = slope_angle + (*iang_sin) - (*rang_sin);
  if (obj_dist != 0.0) {
    ray_height = obj_dist * (*old_axis_slope_angle);
  }
  /* NEW: check for division by zero */
  if (axis_slope_angle == 0.0){
    assert(0  && "Division by zero in kernel2");
  }

  object_distance = ray_height / axis_slope_angle;
  /* NEW: check for Nan and Infinity */
  if (isnan(object_distance) || isnan(*rang_sin) || isnan(axis_slope_angle) || isnan(*old_axis_slope_angle) || isnan(ray_height)){
    assert(0 && "NaN in kernel2");
  }
  if (isinf(object_distance) || isinf(*rang_sin) || isinf(axis_slope_angle) || isinf(*old_axis_slope_angle) || isinf(ray_height)){
    assert(0 && "INF in kernel2");
  }
}

void numerical_kernel3(double *iang_sin, double from_index, double to_index,
  double slope_angle, double rad_curvature,  
// Values of the following variables are changed, hence passed as arguments
    double *iang, double *rang_sin, double *old_axis_slope_angle, double *sagitta) { 
  /* NEW: check for NaN (avoid segmentation fault) */
  if (isnan(*iang_sin)){
    assert(0 && "NaN in kernel3");
  }
  *iang = asin(*iang_sin);
  /* NEW: check for division by zero */
  if (to_index == 0.0){
    assert(0 && "Division by zero in kernel3");
  }
  *rang_sin = (from_index / to_index) * (*iang_sin);
  /* NEW: check for NaN (avoid segmentation fault) */
  if (isnan(*rang_sin)){
    assert(0 && "NaN in kernel3");
  }
  *old_axis_slope_angle = slope_angle;
  axis_slope_angle = slope_angle + (*iang) - asin(*rang_sin);
  *sagitta = sin((*old_axis_slope_angle + (*iang)) / 2.0);
  *sagitta = 2.0 * radius_of_curvature*(*sagitta)*(*sagitta);
  object_distance = ((rad_curvature * sin(*old_axis_slope_angle + (*iang))) *
              cot(axis_slope_angle)) + (*sagitta);
  /* NEW: check for Nan and Infinity */
  if (isnan(object_distance) || isnan(axis_slope_angle) || isnan(*old_axis_slope_angle) || isnan(*sagitta)){
    assert(0 && "NaN in kernel3");
  }
  if (isinf(object_distance) || isinf(axis_slope_angle) || isinf(*old_axis_slope_angle) || isinf(*sagitta) || isinf(*iang_sin) || isinf(*rang_sin)){
    assert(0 && "INF in kernel3");
  }
}

void numerical_kernel4(double from_index, double to_index, 
  double slope_angle, double obj_dist,
  // Values of the following variables are changed, hence passed as arguments
    double *rang) {
  *rang = -asin((from_index / to_index) * sin(slope_angle));
  object_distance = obj_dist * ((to_index * cos(-(*rang))) / 
    (from_index * cos(slope_angle)));
  axis_slope_angle = -(*rang);
  /* NEW: check for Nan and Infinity */
  if (isnan(object_distance) || isnan(*rang) || isnan(axis_slope_angle)){
    assert(0 && "NaN in kernel4");
  }
  if (isinf(object_distance) || isinf(*rang) || isinf(axis_slope_angle)){
    assert(0 && "INF in kernel4");
  }

}

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
      } else
	iang_sin = ((object_distance -
		     radius_of_curvature) / radius_of_curvature) *
	  axis_slope_angle;
      
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

/*  Initialise when called the first time  */

int main(argc, argv)
     int argc;
     char *argv[];
{
  int i, j, k, errors;
  double od_fline, od_cline;
  long passes;

  int nb_scanf = 0;
  for (i=1; i<=8; i++) {
    nb_scanf += scanf("%lf",&spectral_line[i]);
  }

  /* NEW: Exit if the input is not in the range */
  if ( (nb_scanf != 8) ||
       ((spectral_line[1] < 7521.0) || (spectral_line[1] > 7621.0)) ||
       ((spectral_line[2] < 6769.955) || (spectral_line[2] > 6869.955)) ||
       ((spectral_line[3] < 6462.816) || (spectral_line[3] > 6562.816)) ||
       ((spectral_line[4] < 5795.944) || (spectral_line[4] > 5895.944)) ||
       ((spectral_line[5] < 5169.557) || (spectral_line[5] > 5269.557)) ||
       ((spectral_line[6] < 4761.344) || (spectral_line[6] > 4861.344)) ||
       ((spectral_line[7] < 4240.477) || (spectral_line[7] > 4340.477)) ||
       ((spectral_line[8] < 3868.494) || (spectral_line[8] > 3968.494)) ||
       isnan(spectral_line[1]) || isnan(spectral_line[2]) ||
       isnan(spectral_line[3]) || isnan(spectral_line[4]) ||
       isnan(spectral_line[5]) || isnan(spectral_line[6]) ||
       isnan(spectral_line[7]) || isnan(spectral_line[8])) {
    exit(1);
  }
  
  clear_aperture = 4.0;
  /* Load test case into working array */

  current_surfaces = 4;
  for (i = 0; i < current_surfaces; i++)
    for (j = 0; j < 4; j++)
      s[i + 1][j + 1] = testcase[i][j];

  /* Perform ray trace the specified number of times. */
  passes = 0;
  while(passes < niter ) {
    passes++;
    for (itercount = 0; itercount < niter; itercount++) {    
      for (paraxial = 0; paraxial <= 1; paraxial++) {
      
	/* Do main trace in D light */
      
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

    /* Now evaluate the accuracy of the results from the last ray trace */

    /* Now compare the edited results with the master values from
       reference executions of this program. */

    errors = 0;
    for (i = 0; i < 8; i++) {
      if (strcmp(outarr[i], refarr[i]) != 0) {
	       k = strlen(refarr[i]);
	       for (j = 0; j < k; j++) {
	         if (refarr[i][j] != outarr[i][j]) errors++;
	       }
      }
    }
  }
  return 0;
}
