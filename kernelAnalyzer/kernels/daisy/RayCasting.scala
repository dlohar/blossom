import daisy.lang._
import Real._


object Raycasting {

  def numerical_kernel(a_x: Real, a_y: Real, b_x: Real, b_y: Real, s: Real): (Real, Real) = {
    // ASTREE ranges
    require(0.0 <= a_x && a_x <= 10.0 && 0.0 <= a_y && a_y <= 10.0 && 
      -10.0 <= b_x && b_x <= 10.0 && -10.0 <= b_y && b_y <= 10.0 &&
      0.0 <= s && s <= 1.0)
    
    (a_x + s * b_x, a_y + s * b_y)
  }
}
