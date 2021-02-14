import daisy.lang._
import Real._


object NbodyKernel2 {
  def numerical_kernel2(mass: Real, position_i_x: Real, position_i_y: Real, position_i_z: Real, 
    position_j_x: Real, position_j_y: Real, position_j_z: Real, 
    acceleration_x: Real, acceleration_y: Real, acceleration_z: Real): (Real, Real, Real) = {
    // BB ranges
    require(-4.4E+7 <= position_i_x && position_i_x <= 5.59E+7 &&
      -4.59E+7 <= position_i_y && position_i_y <= 1.26E+8 && 
      -8.34E+7 <= position_i_z && position_i_z <= 7.2E+7 &&
      -4.40E+7 <= position_j_x && position_j_x <= 5.59E+7 && 
      -4.59E+7 <= position_j_y && position_j_y <= 1.26E+8 &&
      -8.34E+7 <= position_j_z && position_j_z <= 7.2E+7 &&
      -7.36E+6 <= acceleration_x && acceleration_x <= 8.84E+6 && 
      -7.18E+6 <= acceleration_y && acceleration_y <= 1.96E+7 &&
      -1.30E+7 <= acceleration_z && acceleration_z <= 1.15E+7 && 
      0.9 <= mass && mass <= 1.1)
      
      val GravConstant: Real = 0.01
  
      val a_x = position_i_x - position_j_x
      val a_y = position_i_y - position_j_y
      val a_z = position_i_z - position_j_z

      val mod = sqrt(a_x*a_x + a_y*a_y + a_z*a_z)   // this is most likely going to complain about sqrt of neg. number

      val scale = GravConstant*mass/(mod * mod * mod)

      val new_accel_x = (acceleration_x + (scale * (position_j_x - position_i_x)))
      val new_accel_y = (acceleration_y + (scale * (position_j_y - position_i_y)))
      val new_accel_z = (acceleration_z + (scale * (position_j_z - position_i_z)))

      (new_accel_x, new_accel_y, new_accel_z)
  }  

}
