import daisy.lang._
import Real._


object NbodyKernel1 {
def numerical_kernel1(velocity_x1: Real, velocity_y1: Real, velocity_z1: Real,
    velocity_x2: Real, velocity_y2: Real, velocity_z2: Real,
    velocity_x3: Real, velocity_y3: Real, velocity_z3: Real,
    accelerations_x1: Real, accelerations_y1: Real, accelerations_z1: Real,
    accelerations_x2: Real, accelerations_y2: Real, accelerations_z2: Real,
    accelerations_x3: Real, accelerations_y3: Real, 
    accelerations_z3: Real): (Real, Real, Real) = {
    // guided BB ranges
    require(-1.07E+6 <= velocity_x1 && velocity_x1 <= 2.03E+6 &&
      -2.61E+6 <= velocity_x2 && velocity_x2 <= 1.72E+6 &&
      -6.82E+6 <= velocity_x3 && velocity_x3 <= 8.84E+6 &&
      -4.57E+5 <= velocity_y1 && velocity_y1 <= 3.9E+5 &&
      -7.32E+5 <= velocity_y2 && velocity_y2 <= 6.77E+5 &&
      -7.18E+6 <= velocity_y3 && velocity_y3 <= 1.96E+7 &&
      -1.02E+6 <= velocity_z1 && velocity_z1 <= 4.07E+5 &&
      -7.74E+5 <= velocity_z2 && velocity_z2 <= 1.56E+6 &&
      -1.3E+7 <= velocity_z3 && velocity_z3 <= 1.14E+7 &&
      -1.07E+6 <= accelerations_x1 && accelerations_x1 <= 2.03E+6 &&
      -2.61E+6 <= accelerations_x2 && accelerations_x2 <= 1.72E+6 &&
      -6.82E+6 <= accelerations_x3 && accelerations_x3 <= 8.84E+6 &&
      -4.57E+5 <= accelerations_y1 && accelerations_y1 <= 3.9E+5 &&
      -7.32E+5 <= accelerations_y2 && accelerations_y2 <= 6.77E+5 &&
      -7.18E+6 <= accelerations_y3 && accelerations_y3 <= 1.96E+7 &&
      -1.02E+6 <= accelerations_z1 && accelerations_z1 <= 4.07E+5 &&
      -7.74E+5 <= accelerations_z2 && accelerations_z2 <= 1.56E+6 &&
      -1.30E+7 <= accelerations_z3 && accelerations_z3 <= 1.14E+7)
      
    val velocities_x1 = velocity_x1 + accelerations_x1
    val velocities_x2 = velocity_x2 + accelerations_x2
    val velocities_x3 = velocity_x3 + accelerations_x3
    val velocities_y1 = velocity_y1 + accelerations_y1
    val velocities_y2 = velocity_y2 + accelerations_y2
    val velocities_y3 = velocity_y3 + accelerations_y3
    val velocities_z1 = velocity_z1 + accelerations_z1
    val velocities_z2 = velocity_z2 + accelerations_z2
    val velocities_z3 = velocity_z3 + accelerations_z3
    (velocities_x1, velocities_y1, velocities_z1) // only returning the first co-ordinate
  }


}
