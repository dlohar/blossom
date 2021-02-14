import daisy.lang._
import Real._


object ReactorSimulationKernel2 {
  def numerical_kernel2( mu: Real, azm: Real, d: Real, x: Real, y: Real, z: Real) :
    (Real, Real, Real) = {
    // guided-BB ranges
    require( -1.0 <= mu && mu <= 1.0 &&
      5.85E-09 <= azm && azm <= 6.28E+00 &&
      1.03E-10 <= d && d <= 2.35E+02 &&
      0.0 <= x && x <= 5.00E+00 &&
      -1.47E+02 <= y && y <= 1.67E+02 &&
      -1.58E+02 <= z && z <= 1.55E+02)

    val s = sqrt ( 1.0 - mu * mu )
    val x_res = x + d * mu
    val y_res = y + d * s * cos ( azm )
    val z_res = z + d * s * sin ( azm )

    (x_res, y_res, z_res)
  }
}
