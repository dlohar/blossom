import daisy.lang._
import Real._


object FbenchV2Kernel2 {
def numerical_kernel2(iang_sin: Real, from_index: Real, to_index: Real,
    slope_angle: Real, rad_curvature: Real, radius_of_curvature: Real): Real = {  
    // BB ranges

    require(-0.219 <= iang_sin && iang_sin <= 0.0739371534195933 && 1.0 <= from_index && from_index <= 1.63 &&
      1.0 <= to_index && to_index <= 1.63 && 0.0 <= slope_angle && slope_angle <= 0.102 && 
      -78.0999999999999 <= rad_curvature && rad_curvature <= 27.05 &&
      -78.1 <= radius_of_curvature && radius_of_curvature <= 27.01)

    val iang = asin(iang_sin)
    val rang_sin = (from_index / to_index) * (iang_sin)
    val old_axis_slope_angle = slope_angle
    val axis_slope_angle = slope_angle + (iang) - asin(rang_sin)

    val sagitta = sin((old_axis_slope_angle + (iang)) / 2.0)
    val sagitta2 = 2.0 * radius_of_curvature * (sagitta) * (sagitta)

    val object_distance = ((rad_curvature * sin(old_axis_slope_angle + (iang))) *
              (1.0 / tan(axis_slope_angle))) + (sagitta2)
    object_distance
  }
}