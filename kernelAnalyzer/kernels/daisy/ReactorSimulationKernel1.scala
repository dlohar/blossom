import daisy.lang._
import Real._


object ReactorSimulationKernel1 {

  def numerical_kernel1 (u: Real) : Real = {
    // guided-BB ranges
    require( 9.69E-09 <= u && u <= 1.00E+00 )

    val emax: Real = 2.5
    val emin: Real = 1.0E-3
    val c = 1.0 / ( 2.0 * ( sqrt ( emax ) - sqrt ( emin ) ) );

    ( u / ( 2.0 * c ) + sqrt ( emin ) ) * ( u / ( 2.0 * c ) + sqrt ( emin ) )
  }

}