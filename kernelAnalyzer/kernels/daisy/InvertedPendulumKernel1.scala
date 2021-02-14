import daisy.lang._
import Real._


object InvertedPendulumKernel1 {
def numerical_kernel1(in_0: Real, in_1: Real, in_2: Real, in_3: Real, constrain: Real): Real = {
 //BB + AFLGo ranges
  require(-2.27 <= in_0 && in_0 <= 1.10230642732939 && -1.91 <= in_1 && in_1 <= 2.29 && 
    -5.65 <= in_2 && in_2 <= 24.5 && -14.6 <= in_3 && in_3 <= 15.5 &&
    -3.14 <= constrain && constrain <= 3.14)
    val l: Real = 0.3
    val m: Real = 0.3
    val g: Real = 9.81
    val I: Real = 0.006
    val M: Real = 0.6
    val K1: Real = 1.0 / 3.0
    val K2: Real = 2.0 / 3.0

    val E0: Real = 0.0
    val k: Real = 1

    val w = sqrt(m*g*l/(4*I))
    val E = m*g*l * (0.5 * (in_3/w)*(in_3/w) + cos(in_2) - 1)
    
    val tmp = in_3 * cos(in_2)
    val cmp: Real = if (tmp < 0.0) { -1 }
      else if (tmp == 0.0) { 0 }
      else { 1 } // a > b

    val a = k * (E - E0) * cmp
    
    val F = M * a
    
    val V = (F - K2 * constrain) / K1
    V
  }

}
