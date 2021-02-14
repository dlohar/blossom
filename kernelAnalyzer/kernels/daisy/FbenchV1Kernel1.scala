import daisy.lang._
import Real._


object FbenchV1Kernel1 {
// BB + AFLGo ranges
  def numerical_kernel1(x: Real): (Real, Real) = {
    require(0.0412 <= x && x <= 0.225)

    val z = x * x
    val b = ((((893025.0 * z + 49116375.0) * z + 425675250.0) * z +
            1277025750.0) * z + 1550674125.0) * z + 654729075.0
    val a = (((13852575.0 * z + 216602100.0) * z + 891080190.0) * z +
            1332431100.0) * z + 654729075.0
    (a, b)
  }
  
}
