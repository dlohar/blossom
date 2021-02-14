import daisy.lang._
import Real._


object LuleshKernel1 {

  /* Scala only allows 22 function parameter. 
  z1 and z3 have same ranges with AFLGo, using z1 instead of z3.
  z5 and z7 have same ranges with AFLGo, using z5 instead of z7 */
  def numerical_kernel1(x0: Real, x1: Real, x2: Real, x3: Real, x4: Real, x5: Real, x6: Real, x7: Real,
    y0: Real, y1: Real, y2: Real, y3: Real, y4: Real, y5: Real, y6: Real, y7: Real,
    z0: Real, z1: Real, z2: Real, z4: Real, z5: Real, z6: Real): Real = {
 // BB+AFLGo/BB ranges   
    require(0.0 <= x0 && x0 <= 4.61 
    && 0.113 <= x1 && x1 <= 6.13 &&
    0.113 <= x2 && x2 <= 5.6 && 
    0.0 <= x3 && x3 <= 4.39 && 
    0.0 <= x4 && x4 <= 4.39 && 
    0.113 <= x5 && x5 <= 5.60 && 
    0.113 <= x6 && x6 <= 5.29 && 
    0.0 <= x7 && x7 <= 4.34 && 
    0.0 <= y0 && y0 <= 4.61 &&
    0.0 <= y1 && y1 <= 4.39 && 
    0.113 <= y2 && y2 <= 5.6 &&
    0.113 <= y3 && y3 <= 6.13 && 
    0.0 <= y4 && y4 <= 4.39 && 
    0.0 <= y5 && y5 <= 4.34 && 
    0.113<= y6 && y6 <= 5.29 && 
    0.113 <= y7 && y7 <= 5.60 && 
    0.0 <= z0 && z0 <= 4.61 && 
    0.0 <= z1 && z1 <= 4.39 && 
    0.0 <= z2 && z2 <= 4.34 && 
    0.113 <= z4 && z4 <= 6.13 && 
    0.113 <= z5 && z5 <= 5.6 && 
    0.113 <= z6 && z6 <= 5.29)

    val fjxxi = 0.125 * ( (x6-x0) + (x5-x3) - (x7-x1) - (x4-x2) )
    val fjxet = 0.125 * ( (x6-x0) - (x5-x3) + (x7-x1) - (x4-x2) )
    val fjxze = 0.125 * ( (x6-x0) + (x5-x3) + (x7-x1) + (x4-x2) )

    val fjyxi = 0.125 * ( (y6-y0) + (y5-y3) - (y7-y1) - (y4-y2) )
    val fjyet = 0.125 * ( (y6-y0) - (y5-y3) + (y7-y1) - (y4-y2) )
    val fjyze = 0.125 * ( (y6-y0) + (y5-y3) + (y7-y1) + (y4-y2) )

    val fjzxi = 0.125 * ( (z6-z0) + (z5-z1) - (z5-z1) - (z4-z2) )
    val fjzet = 0.125 * ( (z6-z0) - (z5-z1) + (z5-z1) - (z4-z2) )
    val fjzze = 0.125 * ( (z6-z0) + (z5-z1) + (z5-z1) + (z4-z2) )


    val cjxxi =    (fjyet * fjzze) - (fjzet * fjyze)
    val cjxet =  - (fjyxi * fjzze) + (fjzxi * fjyze)
    val cjxze =    (fjyxi * fjzet) - (fjzxi * fjyet)

    val cjyxi =  - (fjxet * fjzze) + (fjzet * fjxze)
    val cjyet =    (fjxxi * fjzze) - (fjzxi * fjxze)
    val cjyze =  - (fjxxi * fjzet) + (fjzxi * fjxet)

    val cjzxi =    (fjxet * fjyze) - (fjyet * fjxze)
    val cjzet =  - (fjxxi * fjyze) + (fjyxi * fjxze)
    val cjzze =    (fjxxi * fjyet) - (fjyxi * fjxet)

    8.0 * ( fjxet * cjxet + fjyet * cjyet + fjzet * cjzet)
  }
}