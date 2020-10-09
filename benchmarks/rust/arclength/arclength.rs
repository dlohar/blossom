pub fn numerical_kernel(x : f64) -> f64 {
  let n : i32 = 5;
  let mut t1 : f64;
  let mut d1 : f64 = 1.0;
  t1 = x;

  for k in 1 .. (n+1) {
    d1 = 2.0 * d1;
    t1 = t1 + (d1 * x).sin() / d1;
  }

  return t1;
}

pub fn original_program_main(n : i32) {
  let h : f64;
  let mut t2 : f64;
  let dppi : f64;
  let mut s1 : f64;
  let mut t1 : f64 = -1.0;
  dppi = t1.acos();
  s1 = 0.0;
  t1 = 0.0;
  h = dppi / n as f64;

  for i in 1 .. (n+1) {
    t2 = numerical_kernel(i as f64 * h); 
    s1 = s1 + (h*h + (t2 - t1)*(t2 - t1)).sqrt();
    t1 = t2;
  }
}
