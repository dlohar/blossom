import daisy.lang._
import Real._


object Arclength {

  def numerical_kernel(x: Real): Real = {
    // Astree range
    require(0.0 <= x && x <= 61599.86)

  	val d1: Real = 1.0
  	val t1 = x
  	val d2 = 2.0 * d1
  	val t2 =  t1 + sin (d1 * x) / d1
  	val d3 = 2.0 * d2
  	val t3 =  t2 + sin (d2 * x) / d2
  	val d4 = 2.0 * d3
  	val t4 =  t3 + sin (d3 * x) / d3
  	val d5 = 2.0 * d4
  	val t5 =  t4 + sin (d4 * x) / d4
  	val d6 = 2.0 * d5
  	t5 + sin (d5 * x) / d5
  }
}
