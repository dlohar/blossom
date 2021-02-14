import daisy.lang._
import Real._


object InvertedPendulumKernel2 {

// returns only the two changed values, because Daisy supports only 
  def numerical_kernel2(x1: Real, x2: Real, x3: Real, x4: Real, V: Real): (Real, Real) = {
    require(-2.27 <= x1 && x1 <= 1.11 && -3.25 <= x2 && x2 <= 3.56 && 
      -6.44 <= x3 && x3 <= 25.3 && -14.6 <= x4 && x4 <= 15.5 && -20.0 <= V && V <= 20.0)
    // BB + AFLGo ranges
    val K1: Real = 1.0 / 3.0
    val K2: Real = 2.0 / 3.0
    val L: Real = 11.0 / 30.0
    val l: Real = 0.3
    val m: Real = 0.3
    val g: Real = 9.81
    val M: Real = 0.6
  
    val x2_dt = (K1*V - K2*x2 - m*l*g*cos(x3)*sin(x3)/L +
      m*l*sin(x3)*x4*x4) / (M - m*l*cos(x3)*cos(x3)/L)
  
    val x4_dt = (g*sin(x3) - m*l*x4*x4*cos(x3)*sin(x3)/L - cos(x3)*(K1*V +
      K2*x2)/M) / (L - m*l*cos(x3)*cos(x3)/M)
    (x2_dt, x4_dt)
  }
}