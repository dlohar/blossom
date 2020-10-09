const COEFFICIENTS : [f64; 12] = [0.1842392154564795, 0.451227272556133, -0.8079415307840663, -0.45071538868087374, 0.05674216334215214, -0.8981194157558171, 0.4083958984619029, -0.9606849222530257, -0.8507702027651778, -0.9868272809923471, 1.3808941178761502, 1.8652996296254423];//3,4

const INTERCEPTS : [f64; 3] = [0.10956209204126635, 1.677892675644221, -1.7095873485238913];

fn numerical_kernel1(features : &[f64], i : i32) -> f64{
  let mut prob : f64 = 0.0;

  for j in 0 .. 4 {
    prob = prob + COEFFICIENTS[(i*4+j) as usize] * features[j as usize];
  }

  return prob;
}

fn predict (features : &[f64])->i32 {
  let mut class_val : f64 =  std::f64::NEG_INFINITY;
  let mut class_idx : i32 = -1;

  for i in 0 .. 3 {
    let prob : f64 = numerical_kernel1(features, i);

    if prob + INTERCEPTS[i as usize] > class_val {
      class_val = prob + INTERCEPTS[i as usize];
      class_idx = i;
    }
  }
  return class_idx;
}

pub fn original_program_main(features : &[f64]) {
    predict(&features);
}
