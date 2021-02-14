import daisy.lang._
import Real._


object LuleshKernel2 {
  def numerical_kernel2_dvdx(x0: Real, x1: Real, x2: Real, x3: Real, x4: Real, x5: Real,
    y0: Real, y1: Real, y2: Real, y3: Real, y4: Real, y5: Real,
    z0: Real, z1: Real, z2: Real, z3: Real, z4: Real, z5: Real): Real = {
    // BB ranges
    require(0.0 <= x0 && x0 <= 6.09 && 
      0.0 <= x1 && x1 <= 6.09 && 
      0.0 <= x2 && x2 <= 6.09 && 
      0.0 <= x3 && x3 <= 6.09 &&
      0.0 <= x4 && x4 <= 6.09 && 
      0.0 <= x5 && x5 <= 6.09 &&
      0.0 <= y0 && y0 <= 6.09 && 
      0.0 <= y1 && y1 <= 6.09 && 
      0.0 <= y2 && y2 <= 6.09 && 
      0.0 <= y3 && y3 <= 6.09 &&
      0.0 <= y4 && y4 <= 6.09 && 
      0.0 <= y5 && y5 <= 6.09 &&
      0.0 <= z0 && z0 <= 6.09 && 
      0.0 <= z1 && z1 <= 6.09 && 
      0.0 <= z2 && z2 <= 6.09 && 
      0.0 <= z3 && z3 <= 6.09 &&
      0.0 <= z4 && z4 <= 6.09 && 
      0.0 <= z5 && z5 <= 6.09)

    val twelve: Real = 12.0
    val one: Real = 1.0
    val twelfth = one / twelve

    twelfth * (y1 + y2) * (z0 + z1) - (y0 + y1) * (z1 + z2) +
      (y0 + y4) * (z3 + z4) - (y3 + y4) * (z0 + z4) -
      (y2 + y5) * (z3 + z5) + (y3 + y5) * (z2 + z5)
  }

  def numerical_kernel2_dvdy(x0: Real, x1: Real, x2: Real, x3: Real, x4: Real, x5: Real,
    y0: Real, y1: Real, y2: Real, y3: Real, y4: Real, y5: Real,
    z0: Real, z1: Real, z2: Real, z3: Real, z4: Real, z5: Real): Real = {
     // BB ranges
  require(0.0 <= x0 && x0 <= 6.09 &&
        0.0 <= x1 && x1 <= 6.09 &&
        0.0 <= x2 && x2 <= 6.09 &&
        0.0 <= x3 && x3 <= 6.09 &&
        0.0 <= x4 && x4 <= 6.09 &&
        0.0 <= x5 && x5 <= 6.09 &&
        0.0 <= y0 && y0 <= 6.09 &&
        0.0 <= y1 && y1 <= 6.09 &&
        0.0 <= y2 && y2 <= 6.09 &&
        0.0 <= y3 && y3 <= 6.09 &&
        0.0 <= y4 && y4 <= 6.09 &&
        0.0 <= y5 && y5 <= 6.09 &&
        0.0 <= z0 && z0 <= 6.09 &&
        0.0 <= z1 && z1 <= 6.09 &&
        0.0 <= z2 && z2 <= 6.09 &&
        0.0 <= z3 && z3 <= 6.09 &&
        0.0 <= z4 && z4 <= 6.09 &&
        0.0 <= z5 && z5 <= 6.09)

    val twelve: Real = 12.0
    val one: Real = 1.0
    val twelfth = one / twelve

    twelfth * (- (x1 + x2) * (z0 + z1) + (x0 + x1) * (z1 + z2) -
      (x0 + x4) * (z3 + z4) + (x3 + x4) * (z0 + z4) +
      (x2 + x5) * (z3 + z5) - (x3 + x5) * (z2 + z5))
  }

  def numerical_kernel2_dvdz(x0: Real, x1: Real, x2: Real, x3: Real, x4: Real, x5: Real,
    y0: Real, y1: Real, y2: Real, y3: Real, y4: Real, y5: Real,
    z0: Real, z1: Real, z2: Real, z3: Real, z4: Real, z5: Real): Real = {
     // BB ranges
    require(0.0 <= x0 && x0 <= 6.09 &&
        0.0 <= x1 && x1 <= 6.09 &&
        0.0 <= x2 && x2 <= 6.09 &&
        0.0 <= x3 && x3 <= 6.09 &&
        0.0 <= x4 && x4 <= 6.09 &&
        0.0 <= x5 && x5 <= 6.09 &&
        0.0 <= y0 && y0 <= 6.09 &&
        0.0 <= y1 && y1 <= 6.09 &&
        0.0 <= y2 && y2 <= 6.09 &&
        0.0 <= y3 && y3 <= 6.09 &&
        0.0 <= y4 && y4 <= 6.09 &&
        0.0 <= y5 && y5 <= 6.09 &&
        0.0 <= z0 && z0 <= 6.09 &&
        0.0 <= z1 && z1 <= 6.09 &&
        0.0 <= z2 && z2 <= 6.09 &&
        0.0 <= z3 && z3 <= 6.09 &&
        0.0 <= z4 && z4 <= 6.09 &&
        0.0 <= z5 && z5 <= 6.09)
    val twelve: Real = 12.0
    val one: Real = 1.0
    val twelfth = one / twelve

    twelfth * (- (y1 + y2) * (x0 + x1) + (y0 + y1) * (x1 + x2) -
      (y0 + y4) * (x3 + x4) + (y3 + y4) * (x0 + x4) +
      (y2 + y5) * (x3 + x5) - (y3 + y5) * (x2 + x5))
  }
}