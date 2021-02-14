import daisy.lang._
import Real._


object ReactorSimulationKernel3 {
  def numerical_kernel3(e: Real) : Real = {
    // guided-BB ranges
    require( 6.76E-08 <= e && e <= 2.50E+00 )
    val s = (sin ( 100.0 * ( exp ( e ) - 1.0 ) ) + sin ( 18.81 * ( exp(e) - 1.0 ) ) )
    val s_abs = if (s > 0.0) {s} else {-s}
    val y: Real = if (s_abs > 0.02) { s_abs } else { 0.02 }

    (10.0 * exp ( -0.1 / y ))

  }
}
