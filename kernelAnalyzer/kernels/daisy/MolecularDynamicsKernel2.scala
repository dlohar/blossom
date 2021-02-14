import daisy.lang._
import Real._


object MolecularDynamicsKernel2 {

  def numerical_kernel2( pos: Real, vel: Real, dt: Real, acc: Real, f: Real, rmass: Real): 
    (Real, Real, Real) = {
  // guided-BB / aflgo / bb ranges
    require( 1.84E-02 <= pos && pos <= 9.56E+00 &&
      -8.97E-15 <= vel && vel <= 6.55E-15 &&
      1.00E-02 <= dt && dt <= 1.00E-01 &&
      -1.86E-15 <= acc && acc <= 1.36E-15 &&
      -9.31E-16 <= f && f <= 6.79E-16 &&
      5.00E-01 <= rmass && rmass <= 2.00E+00)

    val res0 = pos + vel * dt + 0.5 * acc * dt * dt
    val res1 = vel + 0.5 * dt * ( f * rmass + acc )
    val res2 = f * rmass
    (res0, res1, res2)
  }
}