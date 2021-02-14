import daisy.lang._
import Real._


object LuleshKernel4 {

  def numerical_kernel4(x0: Real, x1: Real, x2: Real, x3: Real, y0: Real, y1: Real, y2: Real, 
    y3: Real, z0: Real, z1: Real, z2: Real, z3: Real): Real = {

  // BB ranges

   require( 0.0 <= x0 && x0 <= 6.13 && 
      0.0 <= x1 && x1 <= 6.13  && 
      0.0 <= x2 && x2 <= 5.60 &&
      0.0 <= x3 && x3 <= 5.60 &&   
      0.0 <= y0 && y0 <= 6.13 && 
      0.0 <= y1 && y1 <= 6.13 && 
      0.0 <= y2 && y2 <= 5.60 && 
      0.0 <= y3 && y3 <= 6.13 &&
      0.0 <= z0 && z0 <= 6.13 && 
      0.0 <= z1 && z1 <= 5.60 && 
      0.0 <= z2 && z2 <= 6.13 &&
      0.0 <= z3 && z3 <= 6.13)


    val fx = (x2 - x0) - (x3 - x1)
    val fy = (y2 - y0) - (y3 - y1);
    val fz = (z2 - z0) - (z3 - z1);
    val gx = (x2 - x0) + (x3 - x1);
    val gy = (y2 - y0) + (y3 - y1);
    val gz = (z2 - z0) + (z3 - z1);
    val area = (fx * fx + fy * fy + fz * fz) * (gx * gx + gy * gy + gz * gz) -
      (fx * gx + fy * gy + fz * gz) * (fx * gx + fy * gy + fz * gz)
    area
  }
}
