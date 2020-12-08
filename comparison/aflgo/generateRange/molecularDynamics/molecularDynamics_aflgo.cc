# include <cstdlib>
# include <iostream>
# include <iomanip>
# include <ctime>
# include <cmath>
# include <stdio.h>
# include <stdlib.h>

//using namespace std;

typedef struct{
  double min, max;
  int init;
}__range_t;

/* NEW: Global variable to store the range */
__range_t range_kernel1[4];
__range_t range_kernel2[6];
__range_t range_kernel3[7];
/********************************/

/* NEW: Adding compute range function in this file to avoid linking conflict*/
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


void timestamp ( ) {
# define TIME_SIZE 40

  static char time_buffer[TIME_SIZE];
  const struct tm *tm;
  time_t now;

  now = time ( NULL );
  tm = localtime ( &now );
  return;
# undef TIME_SIZE
}

double cpu_time ( ) {
  double value;

  value = ( double ) clock ( ) / ( double ) CLOCKS_PER_SEC;

  return value;
}

double numerical_kernel1 ( int nd, double r1[], double r2[], double dr[] ) {

// DIST computes the displacement (and its norm) between two particles.
  double d;
  int i;

  /* NEW: Compute the range */
  for (i=0;i<2;i++) {
    compute_range(r1[i], &range_kernel1[i]);
    compute_range(r2[i], &range_kernel1[i+2]);
  }
  /********************************/

  d = 0.0;
  for ( i = 0; i < nd; i++ )
  {
    dr[i] = r1[i] - r2[i];
    d = d + dr[i] * dr[i];
  }
  d = sqrt ( d );

  return d;
}

void numerical_kernel2( double pos, double vel, double dt, double acc, double f,
  double rmass, double *results) {
  /* NEW: Broaden the range */
  compute_range(pos, &range_kernel2[0]);
  compute_range(vel, &range_kernel2[1]);
  compute_range(dt, &range_kernel2[2]);
  compute_range(acc, &range_kernel2[3]);
  compute_range(f, &range_kernel2[4]);
  compute_range(rmass, &range_kernel2[5]);
  /********************************/
  results[0] = pos + vel * dt + 0.5 * acc * dt * dt;
  results[1] = vel + 0.5 * dt * ( f * rmass + acc );
  results[2] = f * rmass;
}

void numerical_kernel3(double d2, double d, double f0, double f1,
  double rij0, double rij1, double *pot, double *res) {
  /* NEW: Broaden the range */
  compute_range(d2, &range_kernel3[0]);
  compute_range(d, &range_kernel3[1]);
  compute_range(f0, &range_kernel3[2]);
  compute_range(f1, &range_kernel3[3]);
  compute_range(rij0, &range_kernel3[4]);
  compute_range(rij1, &range_kernel3[5]);
  compute_range(*pot, &range_kernel3[6]);
  /********************************/
  *pot = *pot + 0.5 * pow ( sin ( d2 ), 2 );
  res[0] = f0 - rij0 * sin ( 2.0 * d2 ) / d;
  res[1] = f1 - rij1 * sin ( 2.0 * d2 ) / d;
}

void update ( int np, int nd, double pos[], double vel[], double f[], 
  double acc[], double mass, double dt ) {
  int i;
  int j;
  double rmass;
  double results[3];

  rmass = 1.0 / mass;
  for ( j = 0; j < np; j++ ) {
    for ( int i = 0; i < nd; i++ ) {
      numerical_kernel2(pos[i+j*nd], vel[i+j*nd], dt, acc[i+j*nd], f[i+j*nd], rmass, results);
      pos[i+j*nd] = results[0];
      vel[i+j*nd] = results[1];
      acc[i+j*nd] = results[2];
    }
  }

  return;
}

void compute ( int np, int nd, double pos[], double vel[], double mass, 
  double f[], double &pot, double &kin ) {
  double d;
  double d2;
  int i;
  int j;
  int k;
  double PI2 = 3.141592653589793 / 2.0;
  double rij[2];
  double res[2];
  pot = 0.0;
  kin = 0.0;

  for ( k = 0; k < np; k++ ) {
//  Compute the potential energy and forces.
    for ( i = 0; i < nd; i++ )
    {
      f[i+k*nd] = 0.0;
    }

    for ( j = 0; j < np; j++ )
    {
      if ( k != j )
      {
        d = numerical_kernel1 ( nd, pos+k*nd, pos+j*nd, rij );
        if ( d < PI2 )
        {
          d2 = d;
        }
        else
        {
          d2 = PI2;
        }

        numerical_kernel3(d2, d, f[k*nd], f[1+k*nd], rij[0], rij[1], &pot, res);
        f[k*nd] = res[0];
        f[1+k*nd] = res[1];
      }
    }
    for ( i = 0; i < nd; i++ )
    {
      kin = kin + vel[i+k*nd] * vel[i+k*nd];
    }
  }

  kin = kin * 0.5 * mass;

  return;
}

void r8mat_uniform_ab ( int m, int n, double a, double b, int &seed, double r[] ) {
  int i;
  const int i4_huge = 2147483647;
  int j;
  int k;

  if ( seed == 0 ) {
    exit ( 1 );
  }

  for ( j = 0; j < n; j++ )
  {
    for ( i = 0; i < m; i++ )
    {
      k = seed / 127773;

      seed = 16807 * ( seed - k * 127773 ) - k * 2836;

      if ( seed < 0 )
      {
        seed = seed + i4_huge;
      }

      r[i+j*m] = a + ( b - a ) * ( double ) ( seed ) * 4.656612875E-10;
    }
  }

  return;
}

void initialize ( int np, int nd, double pos[], double vel[], double acc[] ) {
  int i;
  int j;
  int seed;
  seed = 123456789;
  r8mat_uniform_ab ( nd, np, 0.0, 10.0, seed, pos );

  for ( j = 0; j < np; j++ )
  {
    for ( i = 0; i < nd; i++ )
    {
      vel[i+j*nd] = 0.0;
    }
  }

  for ( j = 0; j < np; j++ )
  {
    for ( i = 0; i < nd; i++ )
    {
      acc[i+j*nd] = 0.0;
    }
  }
  return;
}
/* originally, mass = 1.0, dt = 0.1, step_num = 500 */
int main (void) {
  int nd = 2; // This needs to be fixed, since we need the size of the arrays
  int np = 10; // This needs to be fixed, since we need the size of the arrays

/* NEW - GET RANGE: 
     Output file containing the fuzzed input values */
  FILE *file1, *file2, *file3;
  file1 = fopen("kernel1_range.dat","r");
  file2 = fopen("kernel2_range.dat","r");
  file3 = fopen("kernel3_range.dat","r");

  if ( (file1 == NULL) || (file2 == NULL) || (file3 == NULL) ) {
    puts("Error while opening file kernel_range.dat");
    exit(1);
  }
  /* NEW: Scan the fuzzed input */
  int nb_scanf = 0;
  double mass, dt;
  int step_num, i;
  nb_scanf = scanf("%lf",&mass);
  nb_scanf += scanf("%d",&step_num);
  nb_scanf += scanf("%lf",&dt);

  if ( (nb_scanf != 3) ||
       (mass > 2.0) || (mass < 0.5) ||
       (step_num > 50) || (step_num < 10) ||
       (dt > 0.1) || (dt < 0.01) ||
       isnan(mass) || isnan(dt) ||
       isnan(step_num)) {
    exit(1);
  }
  for (i=0;i<4;i++) {
    nb_scanf = fscanf(file1,"%le",&range_kernel1[i].min);
    nb_scanf += fscanf(file1,"%le",&range_kernel1[i].max);
    nb_scanf += fscanf(file2,"%le",&range_kernel2[i].min);
    nb_scanf += fscanf(file2,"%le",&range_kernel2[i].max);
    nb_scanf += fscanf(file3,"%le",&range_kernel3[i].min);
    nb_scanf += fscanf(file3,"%le",&range_kernel3[i].max);
    if (nb_scanf != 6) {
      init_range(&range_kernel1[i]);
      init_range(&range_kernel2[i]);
      init_range(&range_kernel3[i]);

    } else {
      range_kernel1[i].init = 1;
      range_kernel2[i].init = 1;
      range_kernel3[i].init = 1;

    }
  }
  for (i=4;i<6;i++) {
    nb_scanf = fscanf(file2,"%le",&range_kernel2[i].min);
    nb_scanf += fscanf(file2,"%le",&range_kernel2[i].max);
    nb_scanf += fscanf(file3,"%le",&range_kernel3[i].min);
    nb_scanf += fscanf(file3,"%le",&range_kernel3[i].max);
    if (nb_scanf != 4) {
      init_range(&range_kernel2[i]);
      init_range(&range_kernel3[i]);

    } else {
      range_kernel2[i].init = 1;
      range_kernel3[i].init = 1;
    }
  }
  nb_scanf = fscanf(file3,"%le",&range_kernel3[6].min);
  nb_scanf += fscanf(file3,"%le",&range_kernel3[6].max);
  if (nb_scanf != 2) {
    init_range(&range_kernel3[6]);
  } else {
    range_kernel3[6].init = 1;
  }
  double *acc;
  double ctime;
  double e0;
  double *force;
  double kinetic;
  double *pos;
  double potential;
  int step;
  int step_print;
  int step_print_index;
  int step_print_num;
  double *vel;

  timestamp ( );

  acc = new double[nd*np];
  force = new double[nd*np];
  pos = new double[nd*np];
  vel = new double[nd*np];

  step_print = 0;
  step_print_index = 0;
  step_print_num = 10;

  ctime = cpu_time ( );

  for ( step = 0; step <= step_num; step++ )
  {
    if ( step == 0 )
    {
      initialize ( np, nd, pos, vel, acc );
    }
    else
    {
      update ( np, nd, pos, vel, force, acc, mass, dt );
    }

    compute ( np, nd, pos, vel, mass, force, potential, kinetic );

    if ( step == 0 )
    {
      e0 = potential + kinetic;
    }

    if ( step == step_print ) {
      step_print_index = step_print_index + 1;
      step_print = ( step_print_index * step_num ) / step_print_num;
    }

  }

  ctime = cpu_time ( ) - ctime;
  delete [] acc;
  delete [] force;
  delete [] pos;
  delete [] vel;

//  Terminate.
  timestamp ( );

  freopen(NULL,"w+",file1);
  freopen(NULL,"w+",file2);
  freopen(NULL,"w+",file3);

  for (i=0;i<4;i++) {
    fprintf(file1,"%.20le %.20le\n",range_kernel1[i].min,range_kernel1[i].max);
    fprintf(file2,"%.20le %.20le\n",range_kernel2[i].min,range_kernel2[i].max);
    fprintf(file3,"%.20le %.20le\n",range_kernel3[i].min,range_kernel3[i].max);
  }
  for (i=4;i<6;i++) {
    fprintf(file2,"%.20le %.20le\n",range_kernel2[i].min,range_kernel2[i].max);
    fprintf(file3,"%.20le %.20le\n",range_kernel3[i].min,range_kernel3[i].max);
  }
  fprintf(file3,"%.20le %.20le\n",range_kernel3[6].min,range_kernel3[6].max);
  fclose(file1);
  fclose(file2);
  fclose(file3);
  return 0;
}
