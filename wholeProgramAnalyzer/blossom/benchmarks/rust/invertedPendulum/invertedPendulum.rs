// global vars
const Vsat : f64 = 20.0;   // saturation voltage
const M : f64 = 0.6;       // mass of cart+pendulum
const m : f64 = 0.3;        // mass of pendulum
const l : f64 = 0.3;        // length of pendulum to CG
const g : f64 = 9.81;      // gravity
const Km : f64 = 2.0;         // motor torque constant
const Kg : f64 = 0.01;      // gear ratio
const R : f64 = 6.0;          // armiture resistance
const r : f64 = 0.01;        // drive radiu3
const K1 : f64 = 1.0 / 3.0;  //Km * Kg / (R * r); 
const K2 : f64 = 2.0 / 3.0;  // Km*Km * Kg*Kg / (R*r*r);
const I : f64 = 0.006;     // inertia of the pendulum
const L : f64 = 11.0 / 30.0;  //(I + m*l**2)/(m*l); 
const pi : f64 = 3.14159265358979;

const K : [f64; 4] = [-40.82482905, -30.66703711, -115.89445979, -19.424643];

// current (simulation) time
static mut t : f64 = 0.0;

fn modulo(x : f64, y : f64) -> f64 {
  return x - (x/y).floor()*y;  
}

fn constrain(mut theta : f64) -> f64 {
  theta = modulo(theta, (2.0 * pi));
  if (theta > pi) {
    return -2.0 * pi + theta;
  } else {
    return theta;
  }
}

fn cmp(a : f64, b : f64) -> i32 {

  if (a < b) { return -1; }
  else if (a == b) { return 0; }
  else { return 1; } // a > b

}

fn sat(Vsat_var : f64, V : f64) -> f64 {
  if (V.abs() > Vsat_var) {
    return Vsat_var * cmp(V, 0.0) as f64;
  } else {
    return V;
  }
}

fn average(x : f64, k1 : f64, k2 : f64, k3 : f64, k4 : f64) -> f64{
  
  return x + (k1 + 2.0 * (k3 + k4) +  k2) / 6.0;

}


fn cosine(x : f64) -> f64 {
  let res = x.cos();
  return res;
}

fn sine(x : f64) -> f64 {
  let res = x.sin();
  return res;
}

fn numerical_kernel1(input : &[f64], constraint : f64) -> f64{
  let E0 : f64 = 0.0;
  let k : f64 = 1.0;
  let w : f64 = (m*g*l/(4.0*I)).sqrt();
  let E : f64 = m*g*l * (0.5 * (input[3]/w)*(input[3]/w) + cosine(input[2]) - 1.0);
  let a : f64 = k * (E - E0)*cmp(input[3] * cosine(input[2]), 0.0) as f64;
  let F : f64 = M * a;
  let V : f64 = (F - K2 * constrain(input[2])) / K1;
  return V;
}

fn numerical_kernel2(input : &[f64], V: f64, out : &mut[f64]) {
  let x1 : f64 = input[0];
  let x2 : f64 = input[1];
  let x3 : f64 = input[2];
  let x4 : f64 = input[3];

  let x1_dt : f64 = x2;
  let x3_dt : f64 = x4;

  let x2_dt : f64 = (K1*V - K2*x2 - m*l*g*cosine(x3)*sine(x3)/L + m*l*sine(x3)*x4*x4) / (M - m*l*cosine(x3)*cosine(x3)/L);
  
  let x4_dt : f64 = (g*sine(x3) - m*l*x4*x4*cosine(x3)*sine(x3)/L - cosine(x3)*(K1*V + K2*x2)/M) / (L - m*l*cosine(x3)*cosine(x3)/M);
  out[0] = x1_dt;
  out[1] = x2_dt;
  out[2] = x3_dt;
  out[3] = x4_dt;
  
}

fn swing_up(input : &[f64]) -> f64 {
  let constraint : f64 = constrain(input[2]);
  let V : f64 = numerical_kernel1(input, constraint);
  return sat(Vsat, V);
}

fn control(input : &[f64]) -> f64{
  let c : f64 = constrain(input[2]);
  if (c > -pi/5.0 && c < pi/5.0) {
    return - (K[0]*input[0] + K[1]*input[1] + K[2]*c + K[3]*input[3]);
  } else {
    return swing_up(input);
  }
}

fn derivative(input : &[f64], out : &mut[f64]) {
  let V : f64 = sat(Vsat, control(input));
  numerical_kernel2(input, V, out); 
}

fn rk4_step(dt : f64, state : &mut[f64]) {
  let mut dx : [f64; 4] = [0.0,0.0,0.0,0.0];
  derivative(state, &mut dx);
 
  let mut k2 : [f64; 4] = [0.0,0.0,0.0,0.0];

  for i in 0 .. 4 {
    k2[i] = dx[i] * dt;
  }
  
  let mut xv : [f64; 4] = [0.0,0.0,0.0,0.0];
  for i in 0 .. 4 {
    xv[i] = state[i] + k2[i]/2.0;
  }

  derivative(&mut xv, &mut dx);
  let mut k3 : [f64; 4] = [0.0,0.0,0.0,0.0];
  for i in 0 .. 4 {
    k3[i] = dx[i] * dt;
  }
  
  for i in 0 .. 4 {
    xv[i] = state[i] + k3[i]/2.0;
  }

  derivative(&mut xv, &mut dx);
  let mut k4 : [f64; 4] = [0.0,0.0,0.0,0.0];
  for i in 0 .. 4 {
    k4[i] = dx[i] * dt;
  }

  for i in 0 .. 4 {
    xv[i] = state[i] + k4[i];
  }

  derivative(&mut xv, &mut dx);
  let mut k1 : [f64; 4] = [0.0,0.0,0.0,0.0];
  for i in 0 .. 4 {
    k1[i] = dt * dx[i];
  }
  // update time
  unsafe{
  t += dt;}

  // update state
  for i in 0 .. 4 {
    state[i] = average(state[i], k1[i], k2[i], k3[i], k4[i]);
  }
}

pub fn original_program_main(state : &mut[f64]) {
  let dt : f64 = 0.01;
  let end : f64 = 10.0;
  
  unsafe{
  t = 0.0;}
/* =========================================*/
  // integrate
    let mut k : i32 = 0;

    unsafe{
    while (t <= end) {
      k=k+1;
      rk4_step(dt, state);
    }
    }
}
