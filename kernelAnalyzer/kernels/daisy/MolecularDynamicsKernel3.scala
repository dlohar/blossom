import daisy.lang._
import Real._


object MolecularDynamicsKernel3 {

  def numerical_kernel3(d: Real, f0: Real, f1: Real, rij0: Real, rij1: Real, pot: Real) :
    (Real, Real, Real) = {
  // guided-BB / aflgo / bb ranges
    require(1.64E+00 <= d && d <= 1.12E+01 &&
      -7.16E-16 <= f0 && f0 <= 6.31E-16 &&
      -8.10E-16 <= f1 && f1 <= 6.40E-16 &&
      -8.54E+00 <= rij0 && rij0 <= 8.54E+00 &&
      -9.54E+00 <= rij1 && rij1 <= 9.54E+00 &&
      0.0 <= pot && pot <= 4.45E+01)
    val d2: Real = 1.57E+00
    val pot_res = pot + 0.5 * pow ( sin ( d2 ), 2 )
    val res0 = f0 - rij0 * sin ( 2.0 * d2 ) / d
    val res1 = f1 - rij1 * sin ( 2.0 * d2 ) / d
    (pot_res, res0, res1)
  }
}