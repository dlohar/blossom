import daisy.lang._
import Real._


object MolecularDynamicsKernel1 {

  def numerical_kernel1 (r10: Real, r11: Real, r20: Real, r21: Real) : Real = {
  // guided-BB / aflgo / bb ranges
    require(4.38E-01 <= r10 && r10 <= 8.98E+00 &&
      1.84E-02 <= r11 && r11 <= 9.56E+00 &&
      4.38E-01 <= r20 && r20 <= 8.98E+00 &&
      1.84E-02 <= r21 && r21 <= 9.56E+00)

    sqrt((r10 - r20) * (r10 - r20) + (r11 - r21) * (r11 - r21))
  }

}