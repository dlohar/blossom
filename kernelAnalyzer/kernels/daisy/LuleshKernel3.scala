import daisy.lang._
import Real._


object LuleshKernel3 {
  def numerical_kernel3(x1: Real, x2: Real, x3: Real, y1: Real, y2: Real, y3: Real, 
    z1: Real, z2: Real, z3: Real): Real = {
  // BB ranges
    require( -3.26 <= x1 && x1 <= 2.34 && 
     -3.26 <= x2 && x2 <= 2.34 &&
     -3.26 <= x3 && x3 <= 2.34  &&  
     -1.35 <= y1 && y1 <= 0.99 && 
     -1.35 <= y2 && y2 <= 0.99  &&
     -1.35 <= y3 && y3 <= 0.99  && 
     -1.35 <= z1 && z1 <= 1.15 && 
     -1.35 <= z2 && z2 <= 1.15  &&
     -1.35 <= z3 && z3 <= 1.15 )

    ((x1)*((y2)*(z3) - (z2)*(y3)) + (x2)*((z1)*(y3) - (y1)*(z3)) + (x3)*((y1)*(z2) - (z1)*(y2)))
  }
}
