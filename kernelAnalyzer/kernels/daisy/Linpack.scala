import daisy.lang._
import Real._


object Linpack {
  def numerical_kernel(dx_0: Real, dx_1: Real, dx_2: Real, dx_3: Real, 
    dy_0: Real, dy_1: Real, dy_2:Real, dy_3:Real): Real = {
    //Guided BB ranges
    require(-1.0 <= dx_0 && dx_0 <= 1.0 && 
       -1.0 <= dx_1 && dx_1 <= 1.18E+58 && 
      -1.77 <= dx_2 && dx_2 <= 3.92E+57 && 
      -2.29 <= dx_3 && dx_3 <= 2.48 &&   
      -4.41E+6 <= dy_0 && dy_0 <= 2.91E+6 && 
      -4.41E+6 <= dy_1 && dy_1 <= 3.87E+6 && 
     -8.45E+269 <= dy_2 && dy_2 <= 9.49E+170 && 
      -8.45E+269 <= dy_3 && dy_3 <= 9.49E+170)
    val dtemp: Real = 0.0
    dtemp + dx_0 * dy_0 + dx_1 * dy_1 + dx_2 * dy_2 + dx_3 * dy_3
  }
}
