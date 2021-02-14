import daisy.lang._
import Real._


object FbenchV1Kernel2 {
def numerical_kernel2(iang_sin: Real, from_index: Real, to_index: Real,
    slope_angle: Real, obj_dist: Real, old_ray_height: Real): Real = {
    // BB ranges
    require(-0.217475732986908 <= iang_sin && iang_sin <= 0.0739371534195933 && 1.0 <= from_index && from_index <= 1.6164 &&
      1.0 <= to_index && to_index <= 1.6164 && 0.0 <= slope_angle && slope_angle <= 0.0991744034677826 &&
      0.0 <= obj_dist && obj_dist <= 121.112061135518 && 0.0 <= old_ray_height && old_ray_height <= 2.0)

    val rang_sin = (from_index / to_index) * (iang_sin)
  
    val old_axis_slope_angle = slope_angle
  
    val axis_slope_angle = slope_angle + (iang_sin) - (rang_sin)
    val ray_height = if (obj_dist != 0.0) {   // this is essentially useless, since Daisy's domain cannot capture this
      obj_dist * (old_axis_slope_angle)
    } else {
      old_ray_height
    }
    val object_distance = ray_height / axis_slope_angle

    object_distance
  }
  
}
