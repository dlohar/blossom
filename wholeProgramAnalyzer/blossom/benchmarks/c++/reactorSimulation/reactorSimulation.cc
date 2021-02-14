# include <cstdlib>
# include <iostream>
# include <iomanip>
# include <cmath>
# include <ctime>

using namespace std;

void timestamp ( ) {
# define TIME_SIZE 40

  static char time_buffer[TIME_SIZE];
  const struct std::tm *tm_ptr;
  std::time_t now;

  now = std::time ( NULL );
  tm_ptr = std::localtime ( &now );
  return;
# undef TIME_SIZE
}

// Can Daisy handle this??
double r8_uniform_01 ( int &seed ) {
//      seed = ( 16807 * seed ) mod ( 2^31 - 1 )
//      u = seed / ( 2^31 - 1 )

  int i4_huge = 2147483647;
  int k;
  double r;
  k = seed / 127773;
  seed = 16807 * ( seed - k * 127773 ) - k * 2836;
  if ( seed < 0 ) {
    seed = seed + i4_huge;
  }
  r = ( double ) ( seed ) * 4.656612875E-10;
  return r;
}

int absorb ( int &seed ) {
  double pa = 0.1;
  double u;
  int value;
  u = r8_uniform_01 ( seed );
  if ( u <= pa ){
    value = 1;
  } else {
    value = 0;
  }
  return value;
}

double r8_abs ( double x ) {
  double value;
  if ( 0.0 <= x ){
    value = + x;
  } else {
    value = - x;
  }
  return value;
}

double r8_max ( double x, double y ) {
  double value;
  if ( y < x ) {
    value = x;
  } else {
    value = y;
  }
  return value;
}

double numerical_kernel1 (double u) {
  double c, value;
  double emax = 2.5;
  double emin = 1.0E-03;
  c = 1.0 / ( 2.0 * ( sqrt ( emax ) - sqrt ( emin ) ) );
  value = ( u / ( 2.0 * c ) + sqrt ( emin ) );
  value = value * value;
  return value;
}

void numerical_kernel2 (double mu, double azm, double d, double &x, double &y, double &z ) {

  double s;
  s = sqrt ( 1.0 - mu * mu );
  x = x + d * mu;
  y = y + d * s * cos ( azm );
  z = z + d * s * sin ( azm );
  return;
}

// Can Daisy handle this??
double numerical_kernel3 ( double e ) {
  double s;
  double value;
  double y;

  s = r8_abs ( sin ( 100.0 * ( exp ( e ) - 1.0 ) )
    + sin ( 18.81 * ( exp ( e ) - 1.0 ) ) );

  y = r8_max ( 0.02, s );

  value = 10.0 * exp ( -0.1 / y );

  return value;
}

double dist2c ( double e, int &seed ) {
  double u;
  double value;

  u = r8_uniform_01 ( seed );

  value = - log ( u ) / numerical_kernel3 ( e );

  return value;
}


double energy ( int &seed ) {
  double u;
  double value;

  u = r8_uniform_01 ( seed );
  value = numerical_kernel1(u);
  return value;
}

void output ( int na, double ea, double sa, int nr, double er, double sr, 
  int nt, double et, double st, int ntot ) {
  double ea_ave;
  double er_ave;
  double et_ave;
  double etot;
  double pa;
  double pr;
  double pt;
  double ptot;

  etot = ea + er + et;

  if ( 0 < na ) {
    ea_ave = ea / ( double ) ( na );
    sa = sqrt ( sa / ( double ) ( na ) - ea_ave * ea_ave );
  }
  else {
    ea_ave = 0.0;
  }

  pa = ( double ) ( na * 100 ) / ( double ) ( ntot );

  if ( 0 < nr ) {
    er_ave = er / ( double ) ( nr );
    sr = sqrt ( sr / ( double ) ( nr ) - er_ave * er_ave );
  } else {
    er_ave = 0.0;
  }

  pr = ( double ) ( nr * 100 ) / ( double ) ( ntot );

  if ( 0 < nt ) {
    et_ave = et / ( double ) ( nt );
    st = sqrt ( st / ( double ) ( nt ) - et_ave * et_ave );
  } else {
    et_ave = 0.0;
  }

  pt = ( double ) ( nt * 100 ) / ( double ) ( ntot );

  ptot = 100.0;

  return;
}

void scatter ( int &seed, double &e, double &mu, double &azm ) {
  double pi = 3.141592653589793;
  double u;

  u = r8_uniform_01 ( seed );
  mu = - 1.0 + 2.0 * u;

  u = r8_uniform_01 ( seed );
  azm = u * 2.0 * pi;

  u = r8_uniform_01 ( seed );
  e = ( u * 0.7 + 0.3 ) * e;

  return;
}

void source ( int &seed, double &e, double &mu, double &azm, double &x, 
  double &y, double &z ) {
  double pi = 3.141592653589793;
  double u;

  u = r8_uniform_01 ( seed );
  mu = u;

  u = r8_uniform_01 ( seed );
  azm = u * 2.0 * pi;

  x = 0.0;
  y = 0.0;
  z = 0.0;

  e = energy ( seed );

  return;
}

// initially thick = 2.0, ntot = 100000, test_num = 5
int original_program_main (double thick, int ntot, int test_num, int seed) {
  double azm;
  double d;
  double e;
  double ea;
  double er;
  double et;
  double mu;
  int na;
  int nr;
  int nt;
  int part;
  double sa;
  double sr;
  double st;
  int test;
  double x;
  double y;
  double z;

  timestamp ( );

  // seed = 123456789;

  for ( test = 1; test <= test_num; test++ ) {
    ea = 0.0;
    er = 0.0;
    et = 0.0;
    na = 0;
    nr = 0;
    nt = 0;
    sa = 0.0;
    sr = 0.0;
    st = 0.0;
//  Loop over the particles.
    for ( part = 1; part <= ntot; part++ ) {
//  Generate a new particle.
      source ( seed, e, mu, azm, x, y, z );

      while ( 1 ) {
//  Compute the distance that the particle can travel through the slab,
//  based on its current energy.
        d = dist2c ( e, seed );
//  Update the particle's position by D units.
        numerical_kernel2 ( mu, azm, d, x, y, z );
//  The particle was reflected by the shield, and this path is complete.

        if ( x < 0.0 ) {
          nr = nr + 1;
          er = er + e;
          sr = sr + e * e;
          break;
        }

//  The particle was transmitted through the shield, and this path is complete.

        else if ( thick < x ) {
          nt = nt + 1;
          et = et + e;
          st = st + e * e;
          break;
        }
//  The particle collided with the shield, and was absorbed.  This path is done.
        else if ( absorb ( seed ) )
        {
          na = na + 1;
          ea = ea + e;
          sa = sa + e * e;
          break;
        }
//  The particle collided with the shield and was scattered.
//  Find the scattering angle and energy, and continue along the new path.
        else {
          scatter ( seed, e, mu, azm );
        }
      }
    }
    output ( na, ea, sa, nr, er, sr, nt, et, st, ntot );

  }
//  Terminate.
  timestamp ( );

  return 0;
}
