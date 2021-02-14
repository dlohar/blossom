import daisy.lang._
import Real._


object LinearSVC {

  def numerical_kernel(feature0: Real, feature1: Real, feature2: Real, feature3: Real): Real = {
    // Astree ranges
    require(4.0 <= feature0 && feature0 <= 8.0 && 2.0 <= feature1 && feature1 <= 4.5 && 
      1.0 <= feature2 && feature2 <= 7.0 && 0.1 <= feature3 && feature3 <= 2.5)

  	val coeff0: Real = 0.1842392154564795
  	val coeff1: Real = 0.451227272556133
  	val coeff2: Real = -0.8079415307840663
  	val coeff3: Real = -0.45071538868087374
  
  	feature0 * coeff0 + feature1 * coeff1 + feature2 * coeff2 + feature3 * coeff3 
  }
}
