#[derive(Clone, Copy)]
struct vector{
  x : f64,
  y : f64,
  z : f64
}

const bodies : i32 = 3;

const GravConstant : f64 = 0.01;
const timeSteps : i32 = 10;
static mut masses : [f64; 3] = [0.0, 0.0, 0.0];
static mut positions : Vec<vector> = Vec::new();
static mut velocities : Vec<vector> = Vec::new();
static mut accelerations : Vec<vector> = Vec::new();

fn addVectors(a : vector, b : vector) -> vector{
  return vector{x : a.x+b.x, y : a.y+b.y, z : a.z+b.z};
}

fn scaleVector(b : f64, a : vector) -> vector {
  return vector{x : b*a.x, y : b*a.y, z : b*a.z};
}

fn subtractVectors(a : vector, b : vector) -> vector{
  return vector{x : a.x-b.x, y : a.y-b.y, z : a.z-b.z};
}

fn modulus(a : vector) -> f64 {
  return (a.x*a.x + a.y*a.y + a.z*a.z).sqrt();
}

/* Kernel Function */
fn numerical_kernel2(v1_x : f64, v1_y : f64, v1_z : f64, v2_x : f64, v2_y : f64, v2_z : f64, v3_x : f64, v3_y : f64, v3_z : f64,
	a1_x : f64, a1_y : f64, a1_z : f64, a2_x : f64, a2_y : f64, a2_z : f64, a3_x : f64, a3_y : f64, a3_z : f64) {
unsafe{
  for i in 0 .. bodies {
    velocities[i as usize] = addVectors(velocities[i as usize],accelerations[i as usize]); 
  }
}
}

fn numerical_kernel1(mass : f64, i_x : f64, i_y : f64, i_z : f64, j_x : f64, j_y : f64, j_z : f64, acc_x : f64, acc_y : f64, acc_z : f64) -> vector {
  let position_i=vector{x:i_x, y:i_y, z:i_z};
  let position_j=vector{x:j_x, y:j_y, z:j_z};
  let acceleration=vector{x:acc_x, y:acc_y, z:acc_z};
  let acceleration_computed = addVectors(acceleration,scaleVector(GravConstant*mass/
  modulus(subtractVectors(position_i,position_j)).powf(3.0),subtractVectors(position_j,position_i)));
  return acceleration_computed;
}

fn resolveCollisions() {
unsafe{
  for i in 0 .. bodies-1 {
    for j in i+1 .. bodies {
      if(positions[i as usize].x==positions[j as usize].x && positions[i as usize].y==positions[j as usize].y && positions[i as usize].z==positions[j as usize].z){
        let temp = velocities[i as usize];
        velocities[i as usize] = velocities[j as usize];
        velocities[j as usize] = temp;
      }
    }
  }
}
}

fn computeAccelerations() {
unsafe{
  for i in 0 .. bodies{
    accelerations[i as usize].x = 0.0;
    accelerations[i as usize].y = 0.0;
    accelerations[i as usize].z = 0.0;

    for j in 0 .. bodies{
      if(i != j){
        accelerations[i as usize] = numerical_kernel1(masses[j as usize], positions[i as usize].x, positions[i as usize].y, positions[i as usize].z, positions[j as usize].x, positions[j as usize].y, positions[j as usize].z, 			accelerations[i as usize].x, accelerations[i as usize].y, accelerations[i as usize].z);
      }
    }
  }
}
}

fn computePositions() {
unsafe{
  for i in 0 .. bodies{
    positions[i as usize] = addVectors(positions[i as usize],addVectors(velocities[i as usize],scaleVector(0.5,accelerations[i as usize])));
  }
}
}

fn simulate() {
unsafe{
  computeAccelerations();
  computePositions();
  numerical_kernel2(velocities[0].x,velocities[0].y,velocities[0].z,velocities[1].x,velocities[1].y,velocities[1].z,velocities[2].x,velocities[2].y,velocities[2].z,
	accelerations[0].x,accelerations[0].y,accelerations[0].z,accelerations[1].x,accelerations[1].y,accelerations[1].z,accelerations[2].x,accelerations[2].y,accelerations[2].z);
  resolveCollisions();
}
}

pub fn original_program_main(masses_in : [f64; 3], positions_x_in : [f64; 3], positions_y_in : [f64; 3], positions_z_in : [f64; 3],
 velocities_x_in : [f64; 3], velocities_y_in : [f64; 3], velocities_z_in : [f64; 3]) {
    unsafe{
    masses[0] = masses_in[0];
    masses[1] = masses_in[1];
    masses[2] = masses_in[2];

    {
	positions.push(vector{x : 0.0, y : 0.0, z : 0.0});
	positions.push(vector{x : 0.0, y : 0.0, z : 0.0});	
	positions.push(vector{x : 0.0, y : 0.0, z : 0.0});
	velocities.push(vector{x : 0.0, y : 0.0, z : 0.0});
	velocities.push(vector{x : 0.0, y : 0.0, z : 0.0});
	velocities.push(vector{x : 0.0, y : 0.0, z : 0.0});
	accelerations.push(vector{x : 0.0, y : 0.0, z : 0.0});
	accelerations.push(vector{x : 0.0, y : 0.0, z : 0.0});
	accelerations.push(vector{x : 0.0, y : 0.0, z : 0.0});
    }

    positions[0].x = positions_x_in[0];
    positions[0].y = positions_y_in[0];
    positions[0].z = positions_z_in[0];
    velocities[0].x = velocities_x_in[0];
    velocities[0].y = velocities_y_in[0];
    velocities[0].z = velocities_z_in[0];
    positions[1].x = positions_x_in[1];
    positions[1].y = positions_y_in[1];
    positions[1].z = positions_z_in[1];
    velocities[1].x = velocities_x_in[1];
    velocities[1].y = velocities_y_in[1];
    velocities[1].z = velocities_z_in[1];
    positions[2].x = positions_x_in[2];
    positions[2].y = positions_y_in[2];
    positions[2].z = positions_z_in[2];
    velocities[2].x = velocities_x_in[2];
    velocities[2].y = velocities_y_in[2];
    velocities[2].z = velocities_z_in[2];

    for k in 0 .. timeSteps {
      simulate();
    }
}
}

