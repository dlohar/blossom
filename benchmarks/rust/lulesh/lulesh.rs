#![allow(dead_code,non_camel_case_types,non_snake_case,unused_parens)]

pub type Index_t = usize;
pub const ELEMENTS_PER_NODE : Index_t = 8;
pub type Real_t = f64;
pub type Int_t = i64;
pub type Etn_t = [Index_t; ELEMENTS_PER_NODE];
pub type Node_t = [Real_t; 8];
pub const NODE_EMPTY: Node_t = [ 0.0 ; 8];
pub type B_t = [Node_t; 3];

pub const VOLUME_ERROR: i32 = 5;
pub const QSTOP_ERROR : i32 = 6;

use std::process;
use std::vec::Vec;
use std::iter::FromIterator;

#[inline(always)]
fn max(a: f64, b: f64) -> f64 {
    if (a > b) {
        a
    } else {
        b
    }
}

fn numerical_kernel3		(x1: f64, y1: f64, z1: f64,
                                 x2: f64, y2: f64, z2: f64,
                                 x3: f64, y3: f64, z3: f64) -> f64 {
    return ((x1)*((y2)*(z3) 
                  - (z2)*(y3)) 
            + (x2)*((z1)*(y3)
                    - (y1)*(z3)) + (x3)*((y1)*(z2) - (z1)*(y2)));
}

#[inline]
pub fn CalcElemVolume( x: &[Real_t; 8], y: &[Real_t; 8], z: &[Real_t; 8] ) -> Real_t {
    let one: Real_t = 1.0;
    let twelve: Real_t = 12.0;
    let twelveth: Real_t = one/twelve;

    let dx61 = x[6] - x[1];
    let dy61 = y[6] - y[1];
    let dz61 = z[6] - z[1];

    let dx70 = x[7] - x[0];
    let dy70 = y[7] - y[0];
    let dz70 = z[7] - z[0];

    let dx63 = x[6] - x[3];
    let dy63 = y[6] - y[3];
    let dz63 = z[6] - z[3];

    let dx20 = x[2] - x[0];
    let dy20 = y[2] - y[0];
    let dz20 = z[2] - z[0];

    let dx50 = x[5] - x[0];
    let dy50 = y[5] - y[0];
    let dz50 = z[5] - z[0];

    let dx64 = x[6] - x[4];
    let dy64 = y[6] - y[4];
    let dz64 = z[6] - z[4];

    let dx31 = x[3] - x[1];
    let dy31 = y[3] - y[1];
    let dz31 = z[3] - z[1];

    let dx72 = x[7] - x[2];
    let dy72 = y[7] - y[2];
    let dz72 = z[7] - z[2];

    let dx43 = x[4] - x[3];
    let dy43 = y[4] - y[3];
    let dz43 = z[4] - z[3];

    let dx57 = x[5] - x[7];
    let dy57 = y[5] - y[7];
    let dz57 = z[5] - z[7];

    let dx14 = x[1] - x[4];
    let dy14 = y[1] - y[4];
    let dz14 = z[1] - z[4];

    let dx25 = x[2] - x[5];
    let dy25 = y[2] - y[5];
    let dz25 = z[2] - z[5];

    let volume =
        numerical_kernel3(dx31 + dx72, dx63, dx20,
                       dy31 + dy72, dy63, dy20,
                       dz31 + dz72, dz63, dz20) +
        numerical_kernel3(dx43 + dx57, dx64, dx70,
                       dy43 + dy57, dy64, dy70,
                       dz43 + dz57, dz64, dz70) +
        numerical_kernel3(dx14 + dx25, dx61, dx50,
                       dy14 + dy25, dy61, dy50,
                       dz14 + dz25, dz61, dz50);

    // println!("volume = {}", volume * twelveth);
    return volume * twelveth;
}


#[inline]
fn CalcPressureForElems(p_new: &mut [Real_t], bvc: &mut [Real_t],
                        pbvc: &mut [Real_t], e_old: &[Real_t],
                        compression: &[Real_t], vnewc: &[Real_t],
                        pmin: Real_t ,
                        p_cut: Real_t , eosvmax_in: Option<Real_t> ,
                        length: Index_t )
{
    let c1s: Real_t = (2.0 as Real_t)/(3.0 as Real_t) ;
    for i in 0..length {
        bvc[i] = c1s * (compression[i] + (1. as Real_t));
        pbvc[i] = c1s;
    }

    for i in 0..length {
        p_new[i] = bvc[i] * e_old[i] ;

        if    ((p_new[i]).abs() <  p_cut   ) {
            p_new[i] = (0.0 as Real_t) ;
        }

        if let Some(eosvmax) = eosvmax_in {
            if vnewc[i] >= eosvmax { /* impossible condition here? */
                p_new[i] = (0.0 as Real_t) ;
            }
        }

        if    (p_new[i]       <  pmin) {
            p_new[i]   = pmin ;
        }
    }
}


#[inline]
pub fn CalcEnergyForElems(p_new: &mut [Real_t], e_new: &mut [Real_t], q_new: &mut [Real_t],
                          bvc: &mut [Real_t], pbvc: &mut [Real_t],
                          p_old: &mut [Real_t], e_old: &mut [Real_t], q_old: &mut [Real_t],
                          compression: &mut [Real_t], compHalfStep: &mut [Real_t],
                          vnewc: &[Real_t], work: &mut [Real_t], delvc: &mut [Real_t], pmin: Real_t ,
                          p_cut: Real_t , e_cut: Real_t  , q_cut: Real_t , emin: Real_t ,
                          qq: &[Real_t], ql: &[Real_t],
                          rho0: Real_t ,
                          eosvmax: Option<Real_t> ,
                          length: Index_t )
{
    let sixth: Real_t = (1.0 as Real_t) / (6.0 as Real_t) ;
    let mut pHalfStep = vec![0 as Real_t; length];

    for i in 0..length {
        e_new[i] = e_old[i] - (0.5 as Real_t) * delvc[i] * (p_old[i] + q_old[i])
            + (0.5 as Real_t) * work[i];

        if (e_new[i]  < emin ) {
            e_new[i] = emin ;
        }
    }

    CalcPressureForElems(&mut pHalfStep[..], bvc, pbvc, e_new, compHalfStep, vnewc,
                         pmin, p_cut, eosvmax, length);

    for i in 0..length {
        let vhalf = (1. as Real_t) / ((1. as Real_t) + compHalfStep[i]) ;

        if ( delvc[i] > (0. as Real_t) ) {
            q_new[i] /* = qq[i] = ql[i] */ = (0. as Real_t) ;
        } else {
            let mut ssc: Real_t = ( pbvc[i] * e_new[i]
                                    + vhalf * vhalf * bvc[i] * pHalfStep[i] ) / rho0 ;

            if ( ssc <= (0.111111e-36 as Real_t) ) {
                ssc =(0.333333e-18 as Real_t) ;
            } else {
                ssc =(ssc.sqrt()) ;
            }

            q_new[i] = (ssc*ql[i] + qq[i]) ;
        }

        e_new[i] = e_new[i] + (0.5 as Real_t) * delvc[i]
        * (  (3.0 as Real_t)*(p_old[i]     + q_old[i])
             - (4.0 as Real_t)*(pHalfStep[i] + q_new[i])) ;
    }

    for i in 0..length {

        e_new[i] += (0.5 as Real_t) * work[i];

        if ((e_new[i]).abs() < e_cut) {
            e_new[i] = (0. as Real_t)  ;
        }
        if (     e_new[i]  < emin ) {
            e_new[i] = emin ;
        }
    }

    CalcPressureForElems(p_new, bvc, pbvc, e_new, compression, vnewc,
                         pmin, p_cut, eosvmax, length);

    for i in 0..length {
        let q_tilde = match(delvc[i]) {
            d @ _ if d > 0.0 => 0.0,
            _ => {
                let mut ssc: Real_t = ( pbvc[i] * e_new[i]
                                        + vnewc[i] * vnewc[i] * bvc[i] * p_new[i] ) / rho0 ;

                if ( ssc <= (0.111111e-36 as Real_t) ) {
                    ssc = (0.333333e-18 as Real_t) ;
                } else {
                    ssc =(ssc.sqrt()) ;
                }

                (ssc*ql[i] + qq[i])
            }
        };

        e_new[i] = e_new[i] - (  (7.0 as Real_t)*(p_old[i]     + q_old[i])
                                 - (8.0 as Real_t)*(pHalfStep[i] + q_new[i])
                                 + (p_new[i] + q_tilde)) * delvc[i]*sixth ;

        if ((e_new[i]).abs() < e_cut) {
            e_new[i] = (0. as Real_t)  ;
        }
        if (     e_new[i]  < emin ) {
            e_new[i] = emin ;
        }
    }

    CalcPressureForElems(p_new, bvc, pbvc, e_new, compression, vnewc,
                         pmin, p_cut, eosvmax, length);

    for i in 0..length {

        if ( delvc[i] <= (0. as Real_t) ) {
            let mut ssc: Real_t = ( pbvc[i] * e_new[i]
                                    + vnewc[i] * vnewc[i] * bvc[i] * p_new[i] ) / rho0 ;

            if ( ssc <= 0.111111e-36 ) {
                ssc = 0.333333e-18;
            } else {
                ssc =(ssc.sqrt()) ;
            }

            q_new[i] = (ssc*ql[i] + qq[i]) ;

            if ((q_new[i]).abs() < q_cut) {q_new[i] = (0. as Real_t) ;}
        }
    }
}

#[inline]
pub fn SumElemFaceNormal(B: &mut B_t,
                         node0: Index_t, node1: Index_t, node2: Index_t, node3: Index_t, 
                         x0: Real_t, y0: Real_t, z0: Real_t,
                         x1: Real_t, y1: Real_t, z1: Real_t,
                         x2: Real_t, y2: Real_t, z2: Real_t,
                         x3: Real_t, y3: Real_t, z3: Real_t)
{
    let bisectX0: Real_t = (0.5 as Real_t) * (x3 + x2 - x1 - x0);
    let bisectY0: Real_t = (0.5 as Real_t) * (y3 + y2 - y1 - y0);
    let bisectZ0: Real_t = (0.5 as Real_t) * (z3 + z2 - z1 - z0);
    let bisectX1: Real_t = (0.5 as Real_t) * (x2 + x1 - x3 - x0);
    let bisectY1: Real_t = (0.5 as Real_t) * (y2 + y1 - y3 - y0);
    let bisectZ1: Real_t = (0.5 as Real_t) * (z2 + z1 - z3 - z0);
    let areaX: Real_t = (0.25 as Real_t) * (bisectY0 * bisectZ1 - bisectZ0 * bisectY1);
    let areaY: Real_t = (0.25 as Real_t) * (bisectZ0 * bisectX1 - bisectX0 * bisectZ1);
    let areaZ: Real_t = (0.25 as Real_t) * (bisectX0 * bisectY1 - bisectY0 * bisectX1);

    B[0][node0] += areaX;
    B[0][node1] += areaX;
    B[0][node2] += areaX;
    B[0][node3] += areaX;

    B[1][node0] += areaY;
    B[1][node1] += areaY;
    B[1][node2] += areaY;
    B[1][node3] += areaY;

    B[2][node0] += areaZ;
    B[2][node1] += areaZ;
    B[2][node2] += areaZ;
    B[2][node3] += areaZ;
}

#[inline]
pub fn CalcElemNodeNormals(pf: &mut B_t,
                           x: &[Real_t; 8],
                           y: &[Real_t; 8],
                           z: &[Real_t; 8])
{
    for i in 0..8 {
        pf[0][i] = 0.0;
        pf[1][i] = 0.0;
        pf[2][i] = 0.0;
    }
    /* evaluate face one: nodes 0, 1, 2, 3 */
    SumElemFaceNormal(pf,
                      0, 1, 2, 3,
                      x[0], y[0], z[0], x[1], y[1], z[1],
                      x[2], y[2], z[2], x[3], y[3], z[3]);
    /* evaluate face two: nodes 0, 4, 5, 1 */
    SumElemFaceNormal(pf,
                      0, 4, 5, 1,
                      x[0], y[0], z[0], x[4], y[4], z[4],
                      x[5], y[5], z[5], x[1], y[1], z[1]);
    /* evaluate face three: nodes 1, 5, 6, 2 */
    SumElemFaceNormal(pf,
                      1, 5, 6, 2,
                      x[1], y[1], z[1], x[5], y[5], z[5],
                      x[6], y[6], z[6], x[2], y[2], z[2]);
    /* evaluate face four: nodes 2, 6, 7, 3 */
    SumElemFaceNormal(pf,
                      2, 6, 7, 3,
                      x[2], y[2], z[2], x[6], y[6], z[6],
                      x[7], y[7], z[7], x[3], y[3], z[3]);
    /* evaluate face five: nodes 3, 7, 4, 0 */
    SumElemFaceNormal(pf,
                      3, 7, 4, 0,
                      x[3], y[3], z[3], x[7], y[7], z[7],
                      x[4], y[4], z[4], x[0], y[0], z[0]);
    /* evaluate face six: nodes 4, 7, 6, 5 */
    SumElemFaceNormal(pf,
                      4, 7, 6, 5,
                      x[4], y[4], z[4], x[7], y[7], z[7],
                      x[6], y[6], z[6], x[5], y[5], z[5]);
}

#[inline]
pub fn numerical_kernel1( x: &Node_t,
                                         y: &Node_t,
                                         z: &Node_t,
                                         b: &mut B_t,
                                         volume: &mut Real_t )
{
    let x0 = x[0] ;   let x1 = x[1] ;
    let x2 = x[2] ;   let x3 = x[3] ;
    let x4 = x[4] ;   let x5 = x[5] ;
    let x6 = x[6] ;   let x7 = x[7] ;

    let y0 = y[0] ;   let y1 = y[1] ;
    let y2 = y[2] ;   let y3 = y[3] ;
    let y4 = y[4] ;   let y5 = y[5] ;
    let y6 = y[6] ;   let y7 = y[7] ;

    let z0 = z[0] ;   let z1 = z[1] ;
    let z2 = z[2] ;   let z3 = z[3] ;
    let z4 = z[4] ;   let z5 = z[5] ;
    let z6 = z[6] ;   let z7 = z[7] ;

    let fjxxi = 0.125 * ( (x6-x0) + (x5-x3) - (x7-x1) - (x4-x2) );
    let fjxet = 0.125 * ( (x6-x0) - (x5-x3) + (x7-x1) - (x4-x2) );
    let fjxze = 0.125 * ( (x6-x0) + (x5-x3) + (x7-x1) + (x4-x2) );

    let fjyxi = 0.125 * ( (y6-y0) + (y5-y3) - (y7-y1) - (y4-y2) );
    let fjyet = 0.125 * ( (y6-y0) - (y5-y3) + (y7-y1) - (y4-y2) );
    let fjyze = 0.125 * ( (y6-y0) + (y5-y3) + (y7-y1) + (y4-y2) );

    let fjzxi = 0.125 * ( (z6-z0) + (z5-z3) - (z7-z1) - (z4-z2) );
    let fjzet = 0.125 * ( (z6-z0) - (z5-z3) + (z7-z1) - (z4-z2) );
    let fjzze = 0.125 * ( (z6-z0) + (z5-z3) + (z7-z1) + (z4-z2) );

    /* compute cofactors */
    let cjxxi =    (fjyet * fjzze) - (fjzet * fjyze);
    let cjxet =  - (fjyxi * fjzze) + (fjzxi * fjyze);
    let cjxze =    (fjyxi * fjzet) - (fjzxi * fjyet);

    let cjyxi =  - (fjxet * fjzze) + (fjzet * fjxze);
    let cjyet =    (fjxxi * fjzze) - (fjzxi * fjxze);
    let cjyze =  - (fjxxi * fjzet) + (fjzxi * fjxet);

    let cjzxi =    (fjxet * fjyze) - (fjyet * fjxze);
    let cjzet =  - (fjxxi * fjyze) + (fjyxi * fjxze);
    let cjzze =    (fjxxi * fjyet) - (fjyxi * fjxet);

    /* calculate partials :
       this need only be done for l = 0,1,2,3   since , by symmetry ,
       (6,7,4,5) = - (0,1,2,3) .
       */
    b[0][0] =   -  cjxxi  -  cjxet  -  cjxze;
    b[0][1] =      cjxxi  -  cjxet  -  cjxze;
    b[0][2] =      cjxxi  +  cjxet  -  cjxze;
    b[0][3] =   -  cjxxi  +  cjxet  -  cjxze;
    b[0][4] = -b[0][2];
    b[0][5] = -b[0][3];
    b[0][6] = -b[0][0];
    b[0][7] = -b[0][1];

    b[1][0] =   -  cjyxi  -  cjyet  -  cjyze;
    b[1][1] =      cjyxi  -  cjyet  -  cjyze;
    b[1][2] =      cjyxi  +  cjyet  -  cjyze;
    b[1][3] =   -  cjyxi  +  cjyet  -  cjyze;
    b[1][4] = -b[1][2];
    b[1][5] = -b[1][3];
    b[1][6] = -b[1][0];
    b[1][7] = -b[1][1];

    b[2][0] =   -  cjzxi  -  cjzet  -  cjzze;
    b[2][1] =      cjzxi  -  cjzet  -  cjzze;
    b[2][2] =      cjzxi  +  cjzet  -  cjzze;
    b[2][3] =   -  cjzxi  +  cjzet  -  cjzze;
    b[2][4] = -b[2][2];
    b[2][5] = -b[2][3];
    b[2][6] = -b[2][0];
    b[2][7] = -b[2][1];

    /* calculate jacobian determinant (volume) */
    *volume = (8. as Real_t) * ( fjxet * cjxet + fjyet * cjyet + fjzet * cjzet);
}

#[inline]
pub fn SumElemStressesToNodeForces( B: &B_t,
                                    stress_xx: Real_t,
                                    stress_yy: Real_t,
                                    stress_zz: Real_t,
                                    fx: &mut [Real_t; 8],
                                    fy: &mut [Real_t; 8],
                                    fz: &mut [Real_t; 8] )
{
    let pfx0 = B[0][0] ;   let pfx1 = B[0][1] ;
    let pfx2 = B[0][2] ;   let pfx3 = B[0][3] ;
    let pfx4 = B[0][4] ;   let pfx5 = B[0][5] ;
    let pfx6 = B[0][6] ;   let pfx7 = B[0][7] ;

    let pfy0 = B[1][0] ;   let pfy1 = B[1][1] ;
    let pfy2 = B[1][2] ;   let pfy3 = B[1][3] ;
    let pfy4 = B[1][4] ;   let pfy5 = B[1][5] ;
    let pfy6 = B[1][6] ;   let pfy7 = B[1][7] ;

    let pfz0 = B[2][0] ;   let pfz1 = B[2][1] ;
    let pfz2 = B[2][2] ;   let pfz3 = B[2][3] ;
    let pfz4 = B[2][4] ;   let pfz5 = B[2][5] ;
    let pfz6 = B[2][6] ;   let pfz7 = B[2][7] ;

    fx[0] = -( stress_xx * pfx0 );
    fx[1] = -( stress_xx * pfx1 );
    fx[2] = -( stress_xx * pfx2 );
    fx[3] = -( stress_xx * pfx3 );
    fx[4] = -( stress_xx * pfx4 );
    fx[5] = -( stress_xx * pfx5 );
    fx[6] = -( stress_xx * pfx6 );
    fx[7] = -( stress_xx * pfx7 );

    fy[0] = -( stress_yy * pfy0  );
    fy[1] = -( stress_yy * pfy1  );
    fy[2] = -( stress_yy * pfy2  );
    fy[3] = -( stress_yy * pfy3  );
    fy[4] = -( stress_yy * pfy4  );
    fy[5] = -( stress_yy * pfy5  );
    fy[6] = -( stress_yy * pfy6  );
    fy[7] = -( stress_yy * pfy7  );

    fz[0] = -( stress_zz * pfz0 );
    fz[1] = -( stress_zz * pfz1 );
    fz[2] = -( stress_zz * pfz2 );
    fz[3] = -( stress_zz * pfz3 );
    fz[4] = -( stress_zz * pfz4 );
    fz[5] = -( stress_zz * pfz5 );
    fz[6] = -( stress_zz * pfz6 );
    fz[7] = -( stress_zz * pfz7 );
}


#[inline]
pub fn CalcElemVelocityGrandient( xvel: &Node_t,
                                  yvel: &Node_t,
                                  zvel: &Node_t,
                                  b: &B_t,
                                  detJ: Real_t) -> (Real_t, Real_t, Real_t,
                                                    Real_t, Real_t, Real_t)

{
    let inv_detJ: Real_t = (1.0 as Real_t) / detJ ;
    let pfx = &b[0];
    let pfy = &b[1];
    let pfz = &b[2];


    let x = inv_detJ * ( pfx[0] * (xvel[0]-xvel[6])
                         + pfx[1] * (xvel[1]-xvel[7])
                         + pfx[2] * (xvel[2]-xvel[4])
                         + pfx[3] * (xvel[3]-xvel[5]) );

    let y = inv_detJ * ( pfy[0] * (yvel[0]-yvel[6])
                         + pfy[1] * (yvel[1]-yvel[7])
                         + pfy[2] * (yvel[2]-yvel[4])
                         + pfy[3] * (yvel[3]-yvel[5]) );

    let z = inv_detJ * ( pfz[0] * (zvel[0]-zvel[6])
                         + pfz[1] * (zvel[1]-zvel[7])
                         + pfz[2] * (zvel[2]-zvel[4])
                         + pfz[3] * (zvel[3]-zvel[5]) );

    let dyddx  = inv_detJ * ( pfx[0] * (yvel[0]-yvel[6])
                              + pfx[1] * (yvel[1]-yvel[7])
                              + pfx[2] * (yvel[2]-yvel[4])
                              + pfx[3] * (yvel[3]-yvel[5]) );

    let dxddy  = inv_detJ * ( pfy[0] * (xvel[0]-xvel[6])
                              + pfy[1] * (xvel[1]-xvel[7])
                              + pfy[2] * (xvel[2]-xvel[4])
                              + pfy[3] * (xvel[3]-xvel[5]) );

    let dzddx  = inv_detJ * ( pfx[0] * (zvel[0]-zvel[6])
                              + pfx[1] * (zvel[1]-zvel[7])
                              + pfx[2] * (zvel[2]-zvel[4])
                              + pfx[3] * (zvel[3]-zvel[5]) );

    let dxddz  = inv_detJ * ( pfz[0] * (xvel[0]-xvel[6])
                              + pfz[1] * (xvel[1]-xvel[7])
                              + pfz[2] * (xvel[2]-xvel[4])
                              + pfz[3] * (xvel[3]-xvel[5]) );

    let dzddy  = inv_detJ * ( pfy[0] * (zvel[0]-zvel[6])
                              + pfy[1] * (zvel[1]-zvel[7])
                              + pfy[2] * (zvel[2]-zvel[4])
                              + pfy[3] * (zvel[3]-zvel[5]) );

    let dyddz  = inv_detJ * ( pfz[0] * (yvel[0]-yvel[6])
                              + pfz[1] * (yvel[1]-yvel[7])
                              + pfz[2] * (yvel[2]-yvel[4])
                              + pfz[3] * (yvel[3]-yvel[5]) );
    let dx2  = ( 0.5 as Real_t) * ( dxddy + dyddx );
    let dy2  = ( 0.5 as Real_t) * ( dxddz + dzddx );
    let dz2  = ( 0.5 as Real_t) * ( dzddy + dyddz );
    return (x, y, z, dx2, dy2, dz2);
}

#[inline]
fn numerical_kernel4( x0: Real_t, x1: Real_t,
             x2: Real_t, x3: Real_t,
             y0: Real_t, y1: Real_t,
             y2: Real_t, y3: Real_t,
             z0: Real_t, z1: Real_t,
             z2: Real_t, z3: Real_t) -> Real_t
{
    let fx: Real_t = (x2 - x0) - (x3 - x1);
    let fy: Real_t = (y2 - y0) - (y3 - y1);
    let fz: Real_t = (z2 - z0) - (z3 - z1);
    let gx: Real_t = (x2 - x0) + (x3 - x1);
    let gy: Real_t = (y2 - y0) + (y3 - y1);
    let gz: Real_t = (z2 - z0) + (z3 - z1);
    let area: Real_t =
        (fx * fx + fy * fy + fz * fz) *
        (gx * gx + gy * gy + gz * gz) -
        (fx * gx + fy * gy + fz * gz) *
        (fx * gx + fy * gy + fz * gz);
    return area ;
}

#[inline]
pub fn CalcElemCharacteristicLength( x: &[Real_t; 8],
                                     y: &[Real_t; 8],
                                     z: &[Real_t; 8],
                                     volume: Real_t ) -> Real_t
{
    let mut charLength: Real_t = 0.;

    let mut a = numerical_kernel4(x[0],x[1],x[2],x[3],
                         y[0],y[1],y[2],y[3],
                         z[0],z[1],z[2],z[3]) ;
    charLength = max(a,charLength) ;

    a = numerical_kernel4(x[4],x[5],x[6],x[7],
                 y[4],y[5],y[6],y[7],
                 z[4],z[5],z[6],z[7]) ;
    charLength = max(a,charLength) ;

    a = numerical_kernel4(x[0],x[1],x[5],x[4],
                 y[0],y[1],y[5],y[4],
                 z[0],z[1],z[5],z[4]) ;
    charLength = max(a,charLength) ;

    a = numerical_kernel4(x[1],x[2],x[6],x[5],
                 y[1],y[2],y[6],y[5],
                 z[1],z[2],z[6],z[5]) ;
    charLength = max(a,charLength) ;

    a = numerical_kernel4(x[2],x[3],x[7],x[6],
                 y[2],y[3],y[7],y[6],
                 z[2],z[3],z[7],z[6]) ;
    charLength = max(a,charLength) ;

    a = numerical_kernel4(x[3],x[0],x[4],x[7],
                 y[3],y[0],y[4],y[7],
                 z[3],z[0],z[4],z[7]) ;
    charLength = max(a,charLength) ;

    charLength = (4.0 as Real_t) * volume / charLength.sqrt();

    return charLength;
}

#[inline]
fn numerical_kernel2(x0: Real_t, x1: Real_t, x2: Real_t,
           x3: Real_t, x4: Real_t, x5: Real_t,
           y0: Real_t, y1: Real_t, y2: Real_t,
           y3: Real_t, y4: Real_t, y5: Real_t,
           z0: Real_t, z1: Real_t, z2: Real_t,
           z3: Real_t, z4: Real_t, z5: Real_t,
           dvdx: &mut Real_t, dvdy: &mut Real_t, dvdz: &mut Real_t)
{
    let twelfth: Real_t = (1.0 as Real_t) / (12.0 as Real_t) ;

    *dvdx =
        (y1 + y2) * (z0 + z1) - (y0 + y1) * (z1 + z2) +
        (y0 + y4) * (z3 + z4) - (y3 + y4) * (z0 + z4) -
        (y2 + y5) * (z3 + z5) + (y3 + y5) * (z2 + z5);
    *dvdy =
        - (x1 + x2) * (z0 + z1) + (x0 + x1) * (z1 + z2) -
        (x0 + x4) * (z3 + z4) + (x3 + x4) * (z0 + z4) +
        (x2 + x5) * (z3 + z5) - (x3 + x5) * (z2 + z5);

    *dvdz =
        - (y1 + y2) * (x0 + x1) + (y0 + y1) * (x1 + x2) -
        (y0 + y4) * (x3 + x4) + (y3 + y4) * (x0 + x4) +
        (y2 + y5) * (x3 + x5) - (y3 + y5) * (x2 + x5);

    *dvdx *= twelfth;
    *dvdy *= twelfth;
    *dvdz *= twelfth;
}

#[inline]
pub fn CalcElemVolumeDerivative(dvdx: &mut [Real_t; 8],
                                dvdy: &mut [Real_t; 8],
                                dvdz: &mut [Real_t; 8],
                                x: &[Real_t; 8],
                                y: &[Real_t; 8],
                                z: &[Real_t; 8])
{
    numerical_kernel2(x[1], x[2], x[3], x[4], x[5], x[7],
            y[1], y[2], y[3], y[4], y[5], y[7],
            z[1], z[2], z[3], z[4], z[5], z[7],
            &mut dvdx[0], &mut dvdy[0], &mut dvdz[0]);
    numerical_kernel2(x[0], x[1], x[2], x[7], x[4], x[6],
            y[0], y[1], y[2], y[7], y[4], y[6],
            z[0], z[1], z[2], z[7], z[4], z[6],
            &mut dvdx[3], &mut dvdy[3], &mut dvdz[3]);
    numerical_kernel2(x[3], x[0], x[1], x[6], x[7], x[5],
            y[3], y[0], y[1], y[6], y[7], y[5],
            z[3], z[0], z[1], z[6], z[7], z[5],
            &mut dvdx[2], &mut dvdy[2], &mut dvdz[2]);
    numerical_kernel2(x[2], x[3], x[0], x[5], x[6], x[4],
            y[2], y[3], y[0], y[5], y[6], y[4],
            z[2], z[3], z[0], z[5], z[6], z[4],
            &mut dvdx[1], &mut dvdy[1], &mut dvdz[1]);
    numerical_kernel2(x[7], x[6], x[5], x[0], x[3], x[1],
            y[7], y[6], y[5], y[0], y[3], y[1],
            z[7], z[6], z[5], z[0], z[3], z[1],
            &mut dvdx[4], &mut dvdy[4], &mut dvdz[4]);
    numerical_kernel2(x[4], x[7], x[6], x[1], x[0], x[2],
            y[4], y[7], y[6], y[1], y[0], y[2],
            z[4], z[7], z[6], z[1], z[0], z[2],
            &mut dvdx[5], &mut dvdy[5], &mut dvdz[5]);
    numerical_kernel2(x[5], x[4], x[7], x[2], x[1], x[3],
            y[5], y[4], y[7], y[2], y[1], y[3],
            z[5], z[4], z[7], z[2], z[1], z[3],
            &mut dvdx[6], &mut dvdy[6], &mut dvdz[6]);
    numerical_kernel2(x[6], x[5], x[4], x[3], x[2], x[0],
            y[6], y[5], y[4], y[3], y[2], y[0],
            z[6], z[5], z[4], z[3], z[2], z[0],
            &mut dvdx[7], &mut dvdy[7], &mut dvdz[7]);
}

#[inline]
pub fn CalcElemFBHourglassForce(xd: &[Real_t], yd: &[Real_t], zd: &[Real_t],  hourgam0: &[Real_t],
                                hourgam1: &[Real_t], hourgam2: &[Real_t], hourgam3: &[Real_t],
                                hourgam4: &[Real_t], hourgam5: &[Real_t], hourgam6: &[Real_t],
                                hourgam7: &[Real_t], coefficient: Real_t,
                                hgfx: &mut [Real_t], hgfy: &mut [Real_t], hgfz: &mut [Real_t] )
{
    let i00: Index_t =0;
    let i01: Index_t =1;
    let i02: Index_t =2;
    let i03: Index_t =3;

    let mut h00: Real_t =
        hourgam0[i00] * xd[0] + hourgam1[i00] * xd[1] +
        hourgam2[i00] * xd[2] + hourgam3[i00] * xd[3] +
        hourgam4[i00] * xd[4] + hourgam5[i00] * xd[5] +
        hourgam6[i00] * xd[6] + hourgam7[i00] * xd[7];

    let mut h01: Real_t =
        hourgam0[i01] * xd[0] + hourgam1[i01] * xd[1] +
        hourgam2[i01] * xd[2] + hourgam3[i01] * xd[3] +
        hourgam4[i01] * xd[4] + hourgam5[i01] * xd[5] +
        hourgam6[i01] * xd[6] + hourgam7[i01] * xd[7];

    let mut h02: Real_t =
        hourgam0[i02] * xd[0] + hourgam1[i02] * xd[1]+
        hourgam2[i02] * xd[2] + hourgam3[i02] * xd[3]+
        hourgam4[i02] * xd[4] + hourgam5[i02] * xd[5]+
        hourgam6[i02] * xd[6] + hourgam7[i02] * xd[7];

    let mut h03: Real_t =
        hourgam0[i03] * xd[0] + hourgam1[i03] * xd[1] +
        hourgam2[i03] * xd[2] + hourgam3[i03] * xd[3] +
        hourgam4[i03] * xd[4] + hourgam5[i03] * xd[5] +
        hourgam6[i03] * xd[6] + hourgam7[i03] * xd[7];

    hgfx[0] = coefficient *
        (hourgam0[i00] * h00 + hourgam0[i01] * h01 +
         hourgam0[i02] * h02 + hourgam0[i03] * h03);

    hgfx[1] = coefficient *
        (hourgam1[i00] * h00 + hourgam1[i01] * h01 +
         hourgam1[i02] * h02 + hourgam1[i03] * h03);

    hgfx[2] = coefficient *
        (hourgam2[i00] * h00 + hourgam2[i01] * h01 +
         hourgam2[i02] * h02 + hourgam2[i03] * h03);

    hgfx[3] = coefficient *
        (hourgam3[i00] * h00 + hourgam3[i01] * h01 +
         hourgam3[i02] * h02 + hourgam3[i03] * h03);

    hgfx[4] = coefficient *
        (hourgam4[i00] * h00 + hourgam4[i01] * h01 +
         hourgam4[i02] * h02 + hourgam4[i03] * h03);

    hgfx[5] = coefficient *
        (hourgam5[i00] * h00 + hourgam5[i01] * h01 +
         hourgam5[i02] * h02 + hourgam5[i03] * h03);

    hgfx[6] = coefficient *
        (hourgam6[i00] * h00 + hourgam6[i01] * h01 +
         hourgam6[i02] * h02 + hourgam6[i03] * h03);

    hgfx[7] = coefficient *
        (hourgam7[i00] * h00 + hourgam7[i01] * h01 +
         hourgam7[i02] * h02 + hourgam7[i03] * h03);

    h00 =
        hourgam0[i00] * yd[0] + hourgam1[i00] * yd[1] +
        hourgam2[i00] * yd[2] + hourgam3[i00] * yd[3] +
        hourgam4[i00] * yd[4] + hourgam5[i00] * yd[5] +
        hourgam6[i00] * yd[6] + hourgam7[i00] * yd[7];

    h01 =
        hourgam0[i01] * yd[0] + hourgam1[i01] * yd[1] +
        hourgam2[i01] * yd[2] + hourgam3[i01] * yd[3] +
        hourgam4[i01] * yd[4] + hourgam5[i01] * yd[5] +
        hourgam6[i01] * yd[6] + hourgam7[i01] * yd[7];

    h02 =
        hourgam0[i02] * yd[0] + hourgam1[i02] * yd[1]+
        hourgam2[i02] * yd[2] + hourgam3[i02] * yd[3]+
        hourgam4[i02] * yd[4] + hourgam5[i02] * yd[5]+
        hourgam6[i02] * yd[6] + hourgam7[i02] * yd[7];

    h03 =
        hourgam0[i03] * yd[0] + hourgam1[i03] * yd[1] +
        hourgam2[i03] * yd[2] + hourgam3[i03] * yd[3] +
        hourgam4[i03] * yd[4] + hourgam5[i03] * yd[5] +
        hourgam6[i03] * yd[6] + hourgam7[i03] * yd[7];


    hgfy[0] = coefficient *
        (hourgam0[i00] * h00 + hourgam0[i01] * h01 +
         hourgam0[i02] * h02 + hourgam0[i03] * h03);

    hgfy[1] = coefficient *
        (hourgam1[i00] * h00 + hourgam1[i01] * h01 +
         hourgam1[i02] * h02 + hourgam1[i03] * h03);

    hgfy[2] = coefficient *
        (hourgam2[i00] * h00 + hourgam2[i01] * h01 +
         hourgam2[i02] * h02 + hourgam2[i03] * h03);

    hgfy[3] = coefficient *
        (hourgam3[i00] * h00 + hourgam3[i01] * h01 +
         hourgam3[i02] * h02 + hourgam3[i03] * h03);

    hgfy[4] = coefficient *
        (hourgam4[i00] * h00 + hourgam4[i01] * h01 +
         hourgam4[i02] * h02 + hourgam4[i03] * h03);

    hgfy[5] = coefficient *
        (hourgam5[i00] * h00 + hourgam5[i01] * h01 +
         hourgam5[i02] * h02 + hourgam5[i03] * h03);

    hgfy[6] = coefficient *
        (hourgam6[i00] * h00 + hourgam6[i01] * h01 +
         hourgam6[i02] * h02 + hourgam6[i03] * h03);

    hgfy[7] = coefficient *
        (hourgam7[i00] * h00 + hourgam7[i01] * h01 +
         hourgam7[i02] * h02 + hourgam7[i03] * h03);

    h00 =
        hourgam0[i00] * zd[0] + hourgam1[i00] * zd[1] +
        hourgam2[i00] * zd[2] + hourgam3[i00] * zd[3] +
        hourgam4[i00] * zd[4] + hourgam5[i00] * zd[5] +
        hourgam6[i00] * zd[6] + hourgam7[i00] * zd[7];

    h01 =
        hourgam0[i01] * zd[0] + hourgam1[i01] * zd[1] +
        hourgam2[i01] * zd[2] + hourgam3[i01] * zd[3] +
        hourgam4[i01] * zd[4] + hourgam5[i01] * zd[5] +
        hourgam6[i01] * zd[6] + hourgam7[i01] * zd[7];

    h02 =
        hourgam0[i02] * zd[0] + hourgam1[i02] * zd[1]+
        hourgam2[i02] * zd[2] + hourgam3[i02] * zd[3]+
        hourgam4[i02] * zd[4] + hourgam5[i02] * zd[5]+
        hourgam6[i02] * zd[6] + hourgam7[i02] * zd[7];

    h03 =
        hourgam0[i03] * zd[0] + hourgam1[i03] * zd[1] +
        hourgam2[i03] * zd[2] + hourgam3[i03] * zd[3] +
        hourgam4[i03] * zd[4] + hourgam5[i03] * zd[5] +
        hourgam6[i03] * zd[6] + hourgam7[i03] * zd[7];


    hgfz[0] = coefficient *
        (hourgam0[i00] * h00 + hourgam0[i01] * h01 +
         hourgam0[i02] * h02 + hourgam0[i03] * h03);

    hgfz[1] = coefficient *
        (hourgam1[i00] * h00 + hourgam1[i01] * h01 +
         hourgam1[i02] * h02 + hourgam1[i03] * h03);

    hgfz[2] = coefficient *
        (hourgam2[i00] * h00 + hourgam2[i01] * h01 +
         hourgam2[i02] * h02 + hourgam2[i03] * h03);

    hgfz[3] = coefficient *
        (hourgam3[i00] * h00 + hourgam3[i01] * h01 +
         hourgam3[i02] * h02 + hourgam3[i03] * h03);

    hgfz[4] = coefficient *
        (hourgam4[i00] * h00 + hourgam4[i01] * h01 +
         hourgam4[i02] * h02 + hourgam4[i03] * h03);

    hgfz[5] = coefficient *
        (hourgam5[i00] * h00 + hourgam5[i01] * h01 +
         hourgam5[i02] * h02 + hourgam5[i03] * h03);

    hgfz[6] = coefficient *
        (hourgam6[i00] * h00 + hourgam6[i01] * h01 +
         hourgam6[i02] * h02 + hourgam6[i03] * h03);

    hgfz[7] = coefficient *
        (hourgam7[i00] * h00 + hourgam7[i01] * h01 +
         hourgam7[i02] * h02 + hourgam7[i03] * h03);
}

#[allow(dead_code)]
#[inline(always)]
pub fn sum4(a: Real_t, b: Real_t, c: Real_t, d: Real_t) -> Real_t {
    (a + b + c + d)
}

pub fn original_program_main(hgcoef_param : Real_t, ss4o3_param : Real_t, 
                qlc_monoq_param : Real_t, qqc_monoq_param : Real_t, qqc_param : Real_t) {
    let mut m = Mesh::new(10, hgcoef_param, ss4o3_param,
qlc_monoq_param, qqc_monoq_param, qqc_param);
    // let mut it = 1;
    while m.time < m.stoptime {
        // let iter_start_time = time::precise_time_s();
        m.time_increment();
        m.lagrange_leap_frog();
        // println!("Iteration {} complete, took {} seconds", it, time::precise_time_s() - iter_start_time);
        println!("time = {:.6e},\n\t dt={:.6e},\n\t e={:.6e}", m.time, m.deltatime, m.e[0]);
        // it += 1;
    }
    m.PrintAbsDiff();
}

pub const XI_M       : Int_t = 0x003;
pub const XI_M_SYMM  : Int_t = 0x001;
pub const XI_M_FREE  : Int_t = 0x002;

pub const XI_P       : Int_t = 0x00c;
pub const XI_P_SYMM  : Int_t = 0x004;
pub const XI_P_FREE  : Int_t = 0x008;

pub const ETA_M      : Int_t = 0x030;
pub const ETA_M_SYMM : Int_t = 0x010;
pub const ETA_M_FREE : Int_t = 0x020;

pub const ETA_P      : Int_t = 0x0c0;
pub const ETA_P_SYMM : Int_t = 0x040;
pub const ETA_P_FREE : Int_t = 0x080;

pub const ZETA_M     : Int_t = 0x300;
pub const ZETA_M_SYMM: Int_t = 0x100;
pub const ZETA_M_FREE: Int_t = 0x200;

pub const ZETA_P     : Int_t = 0xc00;
pub const ZETA_P_SYMM: Int_t = 0x400;
pub const ZETA_P_FREE: Int_t = 0x800;

#[allow(non_snake_case,dead_code)]
pub struct Mesh {
    /* Node-centered */

    x: Vec<Real_t>,  /* coordinates */
    y: Vec<Real_t>,
    z: Vec<Real_t>,

    xd: Vec<Real_t>, /* velocities */
    yd: Vec<Real_t>,
    zd: Vec<Real_t>,

    xdd: Vec<Real_t>, /* accelerations */
    ydd: Vec<Real_t>,
    zdd: Vec<Real_t>,

    fx: Vec<Real_t>,  /* forces */
    fy: Vec<Real_t>,
    fz: Vec<Real_t>,

    nodalMass: Vec<Real_t>,  /* mass */

    symmX: Vec<Index_t>,  /* symmetry plane nodesets */
    symmY: Vec<Index_t>,
    symmZ: Vec<Index_t>,

    /* Element-centered */

    matElemlist: Vec<Index_t>,  /* material indexset */
    nodelist: Vec<Etn_t>,     /* elem_to_node connectivity */

    lxim: Vec<Index_t>,  /* element connectivity across each face */
    lxip: Vec<Index_t>,
    letam: Vec<Index_t>,
    letap: Vec<Index_t>,
    lzetam: Vec<Index_t>,
    lzetap: Vec<Index_t>,

    elemBC: Vec<Int_t>,  /* symmetry/free-surface flags for each elem face */

    dxx: Vec<Real_t>,  /* principal strains -- temporary */
    dyy: Vec<Real_t>,
    dzz: Vec<Real_t>,

    dvdx: Vec<Real_t>,
    dvdy: Vec<Real_t>,
    dvdz: Vec<Real_t>,

    x8n: Vec<Real_t>,
    y8n: Vec<Real_t>,
    z8n: Vec<Real_t>,

    delv_xi: Vec<Real_t>,    /* velocity gradient -- temporary */
    delv_eta: Vec<Real_t>,
    delv_zeta: Vec<Real_t>,

    delx_xi: Vec<Real_t>,    /* coordinate gradient -- temporary */
    delx_eta: Vec<Real_t>,
    delx_zeta: Vec<Real_t>,

    pub e: Vec<Real_t>,   /* energy */

    p: Vec<Real_t>,   /* pressure */
    q: Vec<Real_t>,   /* q */
    ql: Vec<Real_t>,  /* linear term for q */
    qq: Vec<Real_t>,  /* quadratic term for q */

    v: Vec<Real_t>,     /* relative volume */
    volo: Vec<Real_t>,  /* reference volume */
    vnew: Vec<Real_t>,  /* new relative volume -- temporary */
    delv: Vec<Real_t>,  /* vnew - v */
    vdov: Vec<Real_t>,  /* volume derivative over volume */

    arealg: Vec<Real_t>,  /* characteristic length of an element */

    ss: Vec<Real_t>,      /* "sound speed" */

    elemMass: Vec<Real_t>,  /* mass */

    /* Parameters */

    dtfixed: Real_t,           /* fixed time increment */
    pub time: Real_t,              /* current time */
    pub deltatime: Real_t,         /* variable time increment */
    deltatimemultlb: Real_t,
    deltatimemultub: Real_t,
    pub stoptime: Real_t,          /* end time for simulation */

    u_cut: Real_t,             /* velocity tolerance */
    hgcoef: Real_t,            /* hourglass control */
    qstop: Real_t,             /* excessive q indicator */
    monoq_max_slope: Real_t,
    monoq_limiter_mult: Real_t,
    e_cut: Real_t,             /* energy tolerance */
    p_cut: Real_t,             /* pressure tolerance */
    ss4o3: Real_t,
    q_cut: Real_t,             /* q tolerance */
    v_cut: Real_t,             /* relative volume tolerance */
    qlc_monoq: Real_t,         /* linear term coef for q */
    qqc_monoq: Real_t,         /* quadratic term coef for q */
    qqc: Real_t,
    eosvmax: Option<Real_t>,
    eosvmin: Option<Real_t>,
    pmin: Real_t,              /* pressure floor */
    emin: Real_t,              /* energy floor */
    dvovmax: Real_t,           /* maximum allowable volume change */
    refdens: Real_t,           /* reference density */

    dtcourant: Real_t,         /* courant constraint */
    dthydro: Real_t,           /* volume change constraint */
    dtmax: Real_t,             /* maximum allowable time increment */

    cycle: Int_t,             /* iteration count for simulation */

    sizeX: Index_t,           /* X,Y,Z extent of this block */
    sizeY: Index_t,
    sizeZ: Index_t,

    numElem: Index_t,         /* Elements/Nodes in this domain */
    numNode: Index_t,
}

impl Mesh {
    fn initialize_nodal_coordinates(&mut self, edgeElems: Index_t, edgeNodes: Index_t) {
        let mut nidx = 0 ;
        let mut tz  = 0. as Real_t ;
        for plane in 0..edgeNodes {
            let mut ty = 0 as Real_t;
            for row in 0..edgeNodes {
                let mut tx = 0 as Real_t;
                for col in 0..edgeNodes {
                    self.x[nidx] = tx ;
                    self.y[nidx] = ty ;
                    self.z[nidx] = tz ;
                    nidx += 1 ;
                    // tx += ds ; /* may accumulate roundoff... */
                    tx = (1.125 as Real_t)*((col+1) as Real_t)/(edgeElems as Real_t) ;
                }
                // ty += ds ;  /* may accumulate roundoff... */
                ty = (1.125 as Real_t)*((row+1) as Real_t)/(edgeElems as Real_t) ;
            }
            // tz += ds ;  /* may accumulate roundoff... */
            tz = (1.125 as Real_t)*((plane+1) as Real_t)/(edgeElems as Real_t) ;
        }
    }

    fn embed_hexehedral_elements_in_nodal_point_lattice(&mut self, 
                                                        edgeElems: Index_t,
                                                        edgeNodes: Index_t) {
        let mut nidx = 0;
        let mut zidx = 0;
        for _ /*plane*/ in 0..edgeElems {
            for _ /*row*/ in 0..edgeElems {
                for _ /*col*/ in 0..edgeElems {
                    self.nodelist[zidx][0] = nidx                                       ;
                    self.nodelist[zidx][1] = nidx                                   + 1 ;
                    self.nodelist[zidx][2] = nidx                       + edgeNodes + 1 ;
                    self.nodelist[zidx][3] = nidx                       + edgeNodes     ;
                    self.nodelist[zidx][4] = nidx + edgeNodes*edgeNodes                 ;
                    self.nodelist[zidx][5] = nidx + edgeNodes*edgeNodes             + 1 ;
                    self.nodelist[zidx][6] = nidx + edgeNodes*edgeNodes + edgeNodes + 1 ;
                    self.nodelist[zidx][7] = nidx + edgeNodes*edgeNodes + edgeNodes     ;
                    nidx += 1;
                    zidx += 1;
                }
                nidx += 1;
            }
            nidx += edgeNodes;
        }
    }

    fn initialize_field_data(&mut self, meshElems: Index_t) {
        let mut x_local: [Real_t; 8] = [0.0; 8];
        let mut y_local: [Real_t; 8] = [0.0; 8];
        let mut z_local: [Real_t; 8] = [0.0; 8];
        for i in 0..meshElems {
            for (lnode, gnode) in self.nodelist[i].iter().enumerate() {
                x_local[lnode] = self.x[*gnode];
                y_local[lnode] = self.y[*gnode];
                z_local[lnode] = self.z[*gnode];
            }
            // volume calculations
            let volume = CalcElemVolume(&x_local, &y_local, &z_local);
            self.volo[i] = volume ;
            self.elemMass[i] = volume ;
            for idx in 0..8  {
                self.nodalMass[self.nodelist[i][idx]] += volume /(8.0 as Real_t) ;
            }
        }
    }

    fn set_up_symmetry_nodesets(&mut self, edgeNodes: Index_t) {
        let mut nidx = 0 ;
        for i in 0..edgeNodes {
            let planeInc = i*edgeNodes*edgeNodes ;
            let rowInc   = i*edgeNodes ;
            for j in 0..edgeNodes {
                self.symmX[nidx] = planeInc + j*edgeNodes ;
                self.symmY[nidx] = planeInc + j ;
                self.symmZ[nidx] = rowInc   + j ;
                nidx += 1;
            }
        }
    }

    fn set_up_connectivity(&mut self, meshElems: Index_t, edgeElems: Index_t) {
        self.lxim[0] = 0 ;
        for i in 1..meshElems {
            self.lxim[i]   = i-1 ;
            self.lxip[i-1] = i ;
        }
        self.lxip[meshElems-1] = meshElems-1 ;

        for i in 0..edgeElems {
            self.letam[i] = i ; 
            self.letap[meshElems-edgeElems+i] = meshElems-edgeElems+i ;
        }
        for i in edgeElems..meshElems {
            self.letam[i] = i-edgeElems ;
            self.letap[i-edgeElems] = i ;
        }

        for i in 0..(edgeElems*edgeElems) {
            self.lzetam[i] = i ;
            self.lzetap[meshElems-edgeElems*edgeElems+i] = meshElems-edgeElems*edgeElems+i ;
        }
        for i in (edgeElems*edgeElems)..meshElems {
            self.lzetam[i] = i - edgeElems*edgeElems ;
            self.lzetap[i-edgeElems*edgeElems] = i ;
        }
    }

    fn set_up_boundaries(&mut self, meshElems: Index_t, edgeElems: Index_t) {
        for i in 0..edgeElems {
            let planeInc = i*edgeElems*edgeElems ;
            let rowInc   = i*edgeElems ;
            for j in 0..edgeElems {
                self.elemBC[planeInc+j*edgeElems] |= XI_M_SYMM ;
                self.elemBC[planeInc+j*edgeElems+edgeElems-1] |= XI_P_FREE ;
                self.elemBC[planeInc+j] |= ETA_M_SYMM ;
                self.elemBC[planeInc+j+edgeElems*edgeElems-edgeElems] |= ETA_P_FREE ;
                self.elemBC[rowInc+j] |= ZETA_M_SYMM ;
                self.elemBC[rowInc+j+meshElems-edgeElems*edgeElems] |= ZETA_P_FREE ;
            }
        }
    }

    pub fn new (edgeElems: Index_t, hgcoef_param : Real_t, ss4o3_param : Real_t, qlc_monoq_param : Real_t, qqc_monoq_param : Real_t, qqc_param : Real_t) -> Box<Mesh> {
        let edgeNodes = edgeElems + 1;
        let numElem: Index_t = edgeElems*edgeElems*edgeElems;
        let numNode: Index_t = edgeNodes*edgeNodes*edgeNodes;
        let mut m = Box::new(Mesh {
            sizeX: edgeElems,
            sizeY: edgeElems,
            sizeZ: edgeElems,

            numElem: numElem,
            numNode: numNode,

            dtfixed: -1.0e-7,
            deltatime: 1.0e-7,
            deltatimemultlb:1.1 ,
            deltatimemultub:1.2 ,
            stoptime:1.0e-2 ,
            dtcourant:1.0e+20 ,
            dthydro:1.0e+20 ,
            dtmax:1.0e-2 ,
            time:0.0 ,
            cycle: 0 ,

            e_cut:1.0e-7 ,
            p_cut:1.0e-7 ,
            q_cut:1.0e-7 ,
            u_cut:1.0e-7 ,
            v_cut:1.0e-10 ,

            hgcoef:hgcoef_param ,
            ss4o3:ss4o3_param ,

            qstop:1.0e+12 ,
            monoq_max_slope:1.0 ,
            monoq_limiter_mult:2.0 ,
            qlc_monoq:qlc_monoq_param,
            qqc_monoq:qqc_monoq_param ,
            qqc:qqc_param ,

            pmin:0.0 ,
            emin:-1.0e+15 ,

            dvovmax:0.1 ,

            eosvmax: Some(1.0e+9) ,
            eosvmin: Some(1.0e-9) ,

            refdens:1.0 ,

            // Node Persistent
            x: vec![0.0; numNode],
            y: vec![0.0; numNode],
            z: vec![0.0; numNode],
            xd: vec![0.0; numNode],
            yd: vec![0.0; numNode],
            zd: vec![0.0; numNode],
            xdd: vec![0.0; numNode],
            ydd: vec![0.0; numNode],
            zdd: vec![0.0; numNode],
            fx: vec![0.0; numNode],
            fy: vec![0.0; numNode],
            fz: vec![0.0; numNode],
            nodalMass: vec![0.0; numNode],

            // Nodesets
            symmX: vec![0; edgeNodes * edgeNodes],
            symmY: vec![0; edgeNodes * edgeNodes],
            symmZ: vec![0; edgeNodes * edgeNodes],

            // ElemPersistent
            matElemlist: Vec::from_iter(0..numElem),
            nodelist: vec![[0; 8]; numElem],
            lxim: vec![0; numElem],
            lxip: vec![0; numElem],
            letam: vec![0; numElem],
            letap: vec![0; numElem],
            lzetam: vec![0; numElem],
            lzetap: vec![0; numElem],
            elemBC: vec![0; numElem],
            e: vec![0.0; numElem],
            p: vec![0.0; numElem],
            q: vec![0.0; numElem],
            ql: vec![0.0; numElem],
            qq: vec![0.0; numElem],
            v: vec![1.0; numElem],
            volo: vec![0.0; numElem],
            vnew: vec![0.0; numElem],
            delv: vec![0.0; numElem],
            vdov: vec![0.0; numElem],
            arealg: vec![0.0; numElem],
            ss: vec![0.0; numElem],
            elemMass: vec![0.0; numElem],

            // ElemTemporary
            dxx: vec![0.0; numElem],
            dyy: vec![0.0; numElem],
            dzz: vec![0.0; numElem],

            //teporaries, added here to hoist initialization
            dvdx: vec![0.0; numElem * 8],
            dvdy: vec![0.0; numElem * 8],
            dvdz: vec![0.0; numElem * 8],

            x8n: vec![0.0; numElem * 8],
            y8n: vec![0.0; numElem * 8],
            z8n: vec![0.0; numElem * 8],

            delv_xi: vec![0.0; numElem],
            delv_eta: vec![0.0; numElem],
            delv_zeta: vec![0.0; numElem],
            delx_xi: vec![0.0; numElem],
            delx_eta: vec![0.0; numElem],
            delx_zeta: vec![0.0; numElem]
        });
        m.initialize_nodal_coordinates(edgeElems, edgeNodes);
        m.embed_hexehedral_elements_in_nodal_point_lattice(edgeElems, edgeNodes);
        m.initialize_field_data(numElem);
        /* deposit energy */
        m.e[0] = 3.948746e+7;
        m.set_up_symmetry_nodesets(edgeNodes);
        m.set_up_connectivity(numElem, edgeElems);
        m.set_up_boundaries(numElem, edgeElems);
        m
    }

    pub fn time_increment(&mut self) {

        if ((self.dtfixed <= 0.0) && (self.cycle != 0)) {
            /* This will require a reduction in parallel */
            let mut newdt = 1.0e+20 ;
            if (self.dtcourant < newdt) {
                newdt = self.dtcourant / (2.0 as Real_t) ;
            }
            if (self.dthydro < newdt) {
                newdt = self.dthydro * (2.0 as Real_t) / (3.0 as Real_t) ;
            }

            let ratio = newdt / self.deltatime ;
            if (ratio >= 1.0) {
                if (ratio < self.deltatimemultlb) {
                    newdt = self.deltatime ;
                }
                else if (ratio > self.deltatimemultub) {
                    newdt = self.deltatime*self.deltatimemultub ;
                }
            }

            if (newdt > self.dtmax) {
                newdt = self.dtmax ;
            }
            self.deltatime = newdt ;
        }

        let mut targetdt = self.stoptime - self.time ;
        /* TRY TO PREVENT VERY SMALL SCALING ON THE NEXT CYCLE */
        if ((targetdt > self.deltatime) &&
            (targetdt < (4.0 * self.deltatime / 3.0)) ) {
            targetdt = 2.0 * self.deltatime / 3.0 ;
        }

        if (targetdt < self.deltatime) {
            self.deltatime = targetdt ;
        }

        self.time += self.deltatime ;

        self.cycle += 1;
    }

#[inline]
    fn InitStressTermsForElems(&self,
                               numElem: Index_t, 
                               sigxx: &mut [Real_t],
                               sigyy: &mut [Real_t],
                               sigzz: &mut [Real_t])
    {
        //
        // pull in the stresses appropriate to the hydro integration
        //
        for i in 0..numElem {
            sigxx[i] =  - self.p[i] - self.q[i] ;
            sigyy[i] =  - self.p[i] - self.q[i] ;
            sigzz[i] =  - self.p[i] - self.q[i] ;
        }
    }

#[inline]
    fn IntegrateStressForElems( &mut self,
                                sigxx: &mut [Real_t], sigyy: &mut [Real_t], sigzz: &mut [Real_t],
                                determ: &mut [Real_t])
    {
        let mut B: B_t = [[ 0.0 ; 8]; 3];// shape function derivatives
        let mut x_local  = NODE_EMPTY ;
        let mut y_local  = NODE_EMPTY ;
        let mut z_local  = NODE_EMPTY ;
        let mut fx_local = NODE_EMPTY ;
        let mut fy_local = NODE_EMPTY ;
        let mut fz_local = NODE_EMPTY ;

        // loop over all elements
        for (k, elemNodes) in self.nodelist.iter().enumerate() {
            // get nodal coordinates from global arrays and copy into local arrays.
            for (lnode, gnode) in elemNodes.iter().enumerate() {
                x_local[lnode] = self.x[*gnode];
                y_local[lnode] = self.y[*gnode];
                z_local[lnode] = self.z[*gnode];
            }

            /* Volume calculation involves extra work for numerical consistency. */
            numerical_kernel1(&x_local, &y_local, &z_local,
                                             &mut B, &mut determ[k]);

            CalcElemNodeNormals( &mut B,
                                 &x_local, &y_local, &z_local );

            SumElemStressesToNodeForces( &B, sigxx[k], sigyy[k], sigzz[k],
                                         &mut fx_local, &mut fy_local, &mut fz_local ) ;

            // copy nodal force contributions to global force arrray.
            for (lnode, gnode) in elemNodes.iter().enumerate() {
                self.fx[*gnode] += fx_local[lnode];
                self.fy[*gnode] += fy_local[lnode];
                self.fz[*gnode] += fz_local[lnode];
            }
        }
    }

#[inline]
    fn CollectDomainNodesToElemNodes(&self,
                                     elem_to_node: Etn_t,
                                     elemX: &mut [Real_t; 8],
                                     elemY: &mut [Real_t; 8],
                                     elemZ: &mut [Real_t; 8])
    {
        for (i, etn) in elem_to_node.iter().enumerate() {
            elemX[i] = self.x[*etn];
            elemY[i] = self.y[*etn];
            elemZ[i] = self.z[*etn];
        }
    }

#[inline]
    fn CalcFBHourglassForceForElems(&mut self,
                                    determ: &[Real_t],
                                    hourg: Real_t)
    {
        /*************************************************
         *
         *     FUNCTION: Calculates the Flanagan-Belytschko anti-hourglass
         *               force.
         *
         *************************************************/

        let numElem: Index_t = self.numElem ;

        let mut hgfx = [ 0.0 as Real_t; 8]; 
        let mut hgfy = [ 0.0 as Real_t; 8]; 
        let mut hgfz = [ 0.0 as Real_t; 8]; 

        let gamma: [[Real_t; 8]; 4] = [
            [ 1.0,  1.0, -1.0, -1.0, -1.0, -1.0, 1.0,  1.0],
            [ 1.0, -1.0, -1.0,  1.0, -1.0,  1.0, 1.0, -1.0],
            [ 1.0, -1.0,  1.0, -1.0,  1.0, -1.0, 1.0, -1.0],
            [-1.0,  1.0, -1.0,  1.0,  1.0, -1.0, 1.0, -1.0] ];

        let mut hourgam0 = [ 0.0 as Real_t; 4];
        let mut hourgam1 = [ 0.0 as Real_t; 4];
        let mut hourgam2 = [ 0.0 as Real_t; 4];
        let mut hourgam3 = [ 0.0 as Real_t; 4];
        let mut hourgam4 = [ 0.0 as Real_t; 4];
        let mut hourgam5 = [ 0.0 as Real_t; 4];
        let mut hourgam6 = [ 0.0 as Real_t; 4];
        let mut hourgam7 = [ 0.0 as Real_t; 4];
        let mut xd1      = [ 0.0 as Real_t; 8];
        let mut yd1      = [ 0.0 as Real_t; 8];
        let mut zd1      = [ 0.0 as Real_t; 8];

        /*************************************************/
        /*    compute the hourglass modes */


        for i2 in 0..numElem {
            let elem_to_node = self.nodelist[i2];
            let i3: Index_t = 8*i2;
            let volinv: Real_t = (1.0 as Real_t)/determ[i2];
            for i1 in 0..4 {

                let hourmodx: Real_t =
                    self.x8n[i3] * gamma[i1][0] + self.x8n[i3+1] * gamma[i1][1] +
                    self.x8n[i3+2] * gamma[i1][2] + self.x8n[i3+3] * gamma[i1][3] +
                    self.x8n[i3+4] * gamma[i1][4] + self.x8n[i3+5] * gamma[i1][5] +
                    self.x8n[i3+6] * gamma[i1][6] + self.x8n[i3+7] * gamma[i1][7];

                let hourmody: Real_t =
                    self.y8n[i3] * gamma[i1][0] + self.y8n[i3+1] * gamma[i1][1] +
                    self.y8n[i3+2] * gamma[i1][2] + self.y8n[i3+3] * gamma[i1][3] +
                    self.y8n[i3+4] * gamma[i1][4] + self.y8n[i3+5] * gamma[i1][5] +
                    self.y8n[i3+6] * gamma[i1][6] + self.y8n[i3+7] * gamma[i1][7];

                let hourmodz: Real_t =
                    self.z8n[i3] * gamma[i1][0] + self.z8n[i3+1] * gamma[i1][1] +
                    self.z8n[i3+2] * gamma[i1][2] + self.z8n[i3+3] * gamma[i1][3] +
                    self.z8n[i3+4] * gamma[i1][4] + self.z8n[i3+5] * gamma[i1][5] +
                    self.z8n[i3+6] * gamma[i1][6] + self.z8n[i3+7] * gamma[i1][7];

                hourgam0[i1] = gamma[i1][0] -  volinv*(self.dvdx[i3  ] * hourmodx +
                                                       self.dvdy[i3  ] * hourmody +
                                                       self.dvdz[i3  ] * hourmodz );

                hourgam1[i1] = gamma[i1][1] -  volinv*(self.dvdx[i3+1] * hourmodx +
                                                       self.dvdy[i3+1] * hourmody +
                                                       self.dvdz[i3+1] * hourmodz );

                hourgam2[i1] = gamma[i1][2] -  volinv*(self.dvdx[i3+2] * hourmodx +
                                                       self.dvdy[i3+2] * hourmody +
                                                       self.dvdz[i3+2] * hourmodz );

                hourgam3[i1] = gamma[i1][3] -  volinv*(self.dvdx[i3+3] * hourmodx +
                                                       self.dvdy[i3+3] * hourmody +
                                                       self.dvdz[i3+3] * hourmodz );

                hourgam4[i1] = gamma[i1][4] -  volinv*(self.dvdx[i3+4] * hourmodx +
                                                       self.dvdy[i3+4] * hourmody +
                                                       self.dvdz[i3+4] * hourmodz );

                hourgam5[i1] = gamma[i1][5] -  volinv*(self.dvdx[i3+5] * hourmodx +
                                                       self.dvdy[i3+5] * hourmody +
                                                       self.dvdz[i3+5] * hourmodz );

                hourgam6[i1] = gamma[i1][6] -  volinv*(self.dvdx[i3+6] * hourmodx +
                                                       self.dvdy[i3+6] * hourmody +
                                                       self.dvdz[i3+6] * hourmodz );

                hourgam7[i1] = gamma[i1][7] -  volinv*(self.dvdx[i3+7] * hourmodx +
                                                       self.dvdy[i3+7] * hourmody +
                                                       self.dvdz[i3+7] * hourmodz );

            }

            /* compute forces */
            /* store forces into h arrays (force arrays) */

            let ss1=self.ss[i2];
            let mass1=self.elemMass[i2];
            let volume13=(determ[i2].cbrt());

            for (i, etn) in elem_to_node.iter().enumerate() {
                xd1[i] = self.xd[*etn];
                yd1[i] = self.yd[*etn];
                zd1[i] = self.zd[*etn];
            }

            let coefficient = - hourg * (0.01 as Real_t) * ss1 * mass1 / volume13;

            CalcElemFBHourglassForce(&xd1,&yd1,&zd1,
                                     &hourgam0,&hourgam1,&hourgam2,&hourgam3,
                                     &hourgam4,&hourgam5,&hourgam6,&hourgam7,
                                     coefficient, &mut hgfx, &mut hgfy, &mut hgfz);

            for (i, etn) in elem_to_node.iter().enumerate() {
                self.fx[*etn] += hgfx[i];
                self.fy[*etn] += hgfy[i];
                self.fz[*etn] += hgfz[i];
            }
        }
    }

fn CalcHourglassControlForElems(&mut self, determ: &mut [Real_t], hgcoef: Real_t)
    {
        let mut  x1 = [ 0.0 as Real_t; 8];
        let mut  y1 = [ 0.0 as Real_t; 8];
        let mut  z1 = [ 0.0 as Real_t; 8];
        let mut pfx = [ 0.0 as Real_t; 8];
        let mut pfy = [ 0.0 as Real_t; 8];
        let mut pfz = [ 0.0 as Real_t; 8];

        /* start loop over elements */
       /*for (i, elem_to_node, volo, v, determ_i in Zip::new((0..self.numElem,
                          self.nodelist.iter(),
                          self.volo.iter(),
                          self.v.iter(),
                          determ.iter_mut()))*/
	for i in 0..self.numElem
	 {
	    let volo = self.volo[i];
	    let elem_to_node = self.nodelist[i];
	    let v = self.v[i];
    							self.CollectDomainNodesToElemNodes(elem_to_node,
                                               &mut x1,
                                               &mut y1,
                                               &mut z1);

            CalcElemVolumeDerivative(&mut pfx,
                                     &mut pfy,
                                     &mut pfz, 
                                     &mut x1, 
                                     &mut y1, 
                                     &mut z1);

            /* load into temporary storage for FB Hour Glass control */
            for ii in 0..8 {
                let jj=8*i+ii;

                self.dvdx[jj] = pfx[ii];
                self.dvdy[jj] = pfy[ii];
                self.dvdz[jj] = pfz[ii];
                self.x8n[jj]  = x1[ii];
                self.y8n[jj]  = y1[ii];
                self.z8n[jj]  = z1[ii];
            }

            determ[i] = volo * v;

            /* Do a check for negative volumes */
            if v <= 0.0 {
                assert!(false, "I found a negative value in your volume: v[{}]={}", i, self.v[i]) ;
            }
        }

        if ( hgcoef > 0.0 ) {
            self.CalcFBHourglassForceForElems(determ,
                                              hgcoef) ;
        }
    }
#[inline]
    fn CalcVolumeForceForElems(&mut self)
    {
        let numElem: Index_t = self.numElem ;
        if (numElem != 0) {
            let hgcoef: Real_t = self.hgcoef ;
            let mut sigxx : Vec<Real_t> = vec![0.0; numElem] ;
            let mut sigyy : Vec<Real_t> = vec![0.0; numElem] ;
            let mut sigzz : Vec<Real_t> = vec![0.0; numElem] ;
            let mut determ : Vec<Real_t> = vec![0.0; numElem] ;

            /* Sum contributions to total stress tensor */
            self.InitStressTermsForElems(numElem,
                                         &mut sigxx[..], 
                                         &mut sigyy[..], 
                                         &mut sigzz[..]);

            // call elemlib stress integration loop to produce nodal forces from
            // material stresses.
            self.IntegrateStressForElems( &mut sigxx[..], 
                                          &mut sigyy[..], 
                                          &mut sigzz[..], 
                                          &mut determ[..]) ;

            // check for negative element volume
            for k in 0..numElem {
                if (determ[k] <= (0.0 as Real_t)) {
                    assert!(false, "I found a negative value in your determ") ;
                }
            }

            self.CalcHourglassControlForElems(&mut determ[..], hgcoef) ;
        }
    }

#[inline]
    fn CalcForceForNodes(&mut self)
    {
        for i in 0..self.numNode {
            self.fx[i] = (0.0 as Real_t) ;
            self.fy[i] = (0.0 as Real_t) ;
            self.fz[i] = (0.0 as Real_t) ;
        }

        /* Calcforce calls partial, force, hourq */
        self.CalcVolumeForceForElems() ;

        /* Calculate Nodal Forces at domain boundaries */
        /* problem->commSBN->Transfer(CommSBN::forces); */

    }

#[inline]
    fn CalcAccelerationForNodes(&mut self)
    {
        for i in 0..self.numNode {
            self.xdd[i] = self.fx[i] / self.nodalMass[i];
            self.ydd[i] = self.fy[i] / self.nodalMass[i];
            self.zdd[i] = self.fz[i] / self.nodalMass[i];
        }
    }

#[inline]
    fn ApplyAccelerationBoundaryConditionsForNodes(&mut self)
    {
        let numNodeBC: Index_t = (self.sizeX+1)*(self.sizeX+1) ;
        for i in 0..numNodeBC {
            self.xdd[self.symmX[i]] = (0.0 as Real_t) ;
            self.ydd[self.symmY[i]] = (0.0 as Real_t) ;
            self.zdd[self.symmZ[i]] = (0.0 as Real_t) ;
        }
    }

#[inline]
    fn CalcVelocityForDim(d_in: &mut [Real_t],
                          dd_in: &[Real_t],
                          dt: Real_t,
                          u_cut: Real_t) {
        for (d, dd) in d_in.iter_mut().zip(dd_in) {
            let mut dtmp = *d + dd * dt ;
            if( (dtmp).abs() < u_cut ) {
                dtmp = (0.0 as Real_t);
            }
            *d = dtmp ;
        }
    }

#[inline]
    fn CalcVelocityForNodes(&mut self, dt: Real_t, u_cut: Real_t)
    {
        Mesh::CalcVelocityForDim(&mut self.xd, &self.xdd, dt, u_cut);
        Mesh::CalcVelocityForDim(&mut self.yd, &self.ydd, dt, u_cut);
        Mesh::CalcVelocityForDim(&mut self.zd, &self.zdd, dt, u_cut);
    }

#[inline]
    fn CalcPositionForNodes(&mut self, dt: Real_t )
    {
        let numNode: Index_t = self.numNode ;

        for i in 0..numNode {
            self.x[i] += self.xd[i] * dt ;
            self.y[i] += self.yd[i] * dt ;
            self.z[i] += self.zd[i] * dt ;
        }
    }

#[inline]
    fn LagrangeNodal(&mut self)
    {
        let delt: Real_t = self.deltatime ;
        let u_cut: Real_t = self.u_cut ;

        /* time of boundary condition evaluation is beginning of step for force and
         * acceleration boundary conditions. */
        self.CalcForceForNodes();

        self.CalcAccelerationForNodes();

        self.ApplyAccelerationBoundaryConditionsForNodes();

        self.CalcVelocityForNodes( delt, u_cut ) ;

        self.CalcPositionForNodes( delt );
    }

#[inline]
    fn CalcKinematicsForElems( &mut self, numElem: Index_t , dt: Real_t )
    {
        let mut B: B_t = [NODE_EMPTY; 3]; /* shape function derivatives */
        let mut  x_local = NODE_EMPTY ;
        let mut  y_local = NODE_EMPTY ;
        let mut  z_local = NODE_EMPTY ;
        let mut xd_local = NODE_EMPTY ;
        let mut yd_local = NODE_EMPTY ;
        let mut zd_local = NODE_EMPTY ;
        let mut detJ = (0.0 as Real_t) ;

        // loop over all elements
        for k in 0..numElem {
            let elem_to_node = self.nodelist[k] ;

            // get nodal coordinates from global arrays and copy into local arrays.
            for lnode in 0..8 {
                let gnode: Index_t = elem_to_node[lnode];
                x_local[lnode] = self.x[gnode];
                y_local[lnode] = self.y[gnode];
                z_local[lnode] = self.z[gnode];
            }

            // volume calculations
            let volume = CalcElemVolume(&x_local, &y_local, &z_local );
            let relativeVolume = volume / self.volo[k] ;
            self.vnew[k] = relativeVolume ;
            self.delv[k] = relativeVolume - self.v[k] ;

            // See if any volumes are negative, and take appropriate action.
            if (self.vnew[k] <= 0.0 as Real_t)
            {
                println!("volo: {}, volume: {}", self.volo[k], volume);
                assert!(false, "I found a negative value in your volume: vnew[{}]={}", k, self.vnew[k]) ;
            }

            // set characteristic length
            self.arealg[k] = CalcElemCharacteristicLength(&mut x_local,
                                                          &mut y_local,
                                                          &mut z_local,
                                                          volume);

            // get nodal velocities from global array and copy into local arrays.
            for lnode in 0..8 {
                let gnode: Index_t = elem_to_node[lnode];
                xd_local[lnode] = self.xd[gnode];
                yd_local[lnode] = self.yd[gnode];
                zd_local[lnode] = self.zd[gnode];
            }

            let dt2: Real_t = (0.5 as Real_t) * dt;
            for j in 0..8 {
                x_local[j] -= dt2 * xd_local[j];
                y_local[j] -= dt2 * yd_local[j];
                z_local[j] -= dt2 * zd_local[j];
            }

            numerical_kernel1( &x_local,
                                              &y_local,
                                              &z_local,
                                              &mut B, &mut detJ );

            let (x, y, z, _, _, _) = 
                CalcElemVelocityGrandient( &xd_local,
                                       &yd_local,
                                       &zd_local,
                                       &B,
                                       detJ);

            // put velocity gradient quantities into their global arrays.
            self.dxx[k] = x;
            self.dyy[k] = y;
            self.dzz[k] = z;
        }
    }

#[inline]
    fn CalcLagrangeElements(&mut self, deltatime: Real_t )
    {
        let numElem: Index_t = self.numElem ;
        if (numElem > 0) {
            // set element connectivity array as a single dimension array. It is
            // assumed that the array will be of length numelems*numnodesperelem.

            self.CalcKinematicsForElems(numElem, deltatime) ;

            // element loop to do some stuff not included in the elemlib function.
            for k in 0..numElem {
                // calc strain rate and apply as constraint (only done in FB element)
                let vdov: Real_t = self.dxx[k] + self.dyy[k] + self.dzz[k] ;
                let vdovthird: Real_t = vdov/(3.0 as Real_t) ;

                // make the rate of deformation tensor deviatoric
                self.vdov[k] = vdov ;
                self.dxx[k] -= vdovthird ;
                self.dyy[k] -= vdovthird ;
                self.dzz[k] -= vdovthird ;

                // See if any volumes are negative, and take appropriate action.
                if (self.vnew[k] <= 0.0 as Real_t)
                {
                    assert!(false, "I found a negative value in your volume: vnew[{}]={}", k, self.vnew[k]) ;
                }
            }
        }
    }


#[inline]
    fn CalcMonotonicQGradientsForElems(&mut self)
    {
        let numElem: Index_t = self.numElem ;
        let ptiny: Real_t = (1.0e-36 as Real_t) ;

        for i in 0..numElem {
            let elem_to_node = self.nodelist[i];
            let n0: Index_t = elem_to_node[0] ;
            let n1: Index_t = elem_to_node[1] ;
            let n2: Index_t = elem_to_node[2] ;
            let n3: Index_t = elem_to_node[3] ;
            let n4: Index_t = elem_to_node[4] ;
            let n5: Index_t = elem_to_node[5] ;
            let n6: Index_t = elem_to_node[6] ;
            let n7: Index_t = elem_to_node[7] ;

            let x0: Real_t = self.x[n0] ;
            let x1: Real_t = self.x[n1] ;
            let x2: Real_t = self.x[n2] ;
            let x3: Real_t = self.x[n3] ;
            let x4: Real_t = self.x[n4] ;
            let x5: Real_t = self.x[n5] ;
            let x6: Real_t = self.x[n6] ;
            let x7: Real_t = self.x[n7] ;

            let y0: Real_t = self.y[n0] ;
            let y1: Real_t = self.y[n1] ;
            let y2: Real_t = self.y[n2] ;
            let y3: Real_t = self.y[n3] ;
            let y4: Real_t = self.y[n4] ;
            let y5: Real_t = self.y[n5] ;
            let y6: Real_t = self.y[n6] ;
            let y7: Real_t = self.y[n7] ;

            let z0: Real_t = self.z[n0] ;
            let z1: Real_t = self.z[n1] ;
            let z2: Real_t = self.z[n2] ;
            let z3: Real_t = self.z[n3] ;
            let z4: Real_t = self.z[n4] ;
            let z5: Real_t = self.z[n5] ;
            let z6: Real_t = self.z[n6] ;
            let z7: Real_t = self.z[n7] ;

            let xv0: Real_t = self.xd[n0] ;
            let xv1: Real_t = self.xd[n1] ;
            let xv2: Real_t = self.xd[n2] ;
            let xv3: Real_t = self.xd[n3] ;
            let xv4: Real_t = self.xd[n4] ;
            let xv5: Real_t = self.xd[n5] ;
            let xv6: Real_t = self.xd[n6] ;
            let xv7: Real_t = self.xd[n7] ;

            let yv0: Real_t = self.yd[n0] ;
            let yv1: Real_t = self.yd[n1] ;
            let yv2: Real_t = self.yd[n2] ;
            let yv3: Real_t = self.yd[n3] ;
            let yv4: Real_t = self.yd[n4] ;
            let yv5: Real_t = self.yd[n5] ;
            let yv6: Real_t = self.yd[n6] ;
            let yv7: Real_t = self.yd[n7] ;

            let zv0: Real_t = self.zd[n0] ;
            let zv1: Real_t = self.zd[n1] ;
            let zv2: Real_t = self.zd[n2] ;
            let zv3: Real_t = self.zd[n3] ;
            let zv4: Real_t = self.zd[n4] ;
            let zv5: Real_t = self.zd[n5] ;
            let zv6: Real_t = self.zd[n6] ;
            let zv7: Real_t = self.zd[n7] ;

            let vol: Real_t = self.volo[i]*self.vnew[i] ;
            let norm: Real_t = (1.0 as Real_t) / ( vol + ptiny ) ;

            let dxj: Real_t = (-0.25 as Real_t)*(sum4(x0,x1,x5,x4) - sum4(x3,x2,x6,x7)) ;
            let dyj: Real_t = (-0.25 as Real_t)*(sum4(y0,y1,y5,y4) - sum4(y3,y2,y6,y7)) ;
            let dzj: Real_t = (-0.25 as Real_t)*(sum4(z0,z1,z5,z4) - sum4(z3,z2,z6,z7)) ;

            let dxi: Real_t = ( 0.25 as Real_t)*(sum4(x1,x2,x6,x5) - sum4(x0,x3,x7,x4)) ;
            let dyi: Real_t = ( 0.25 as Real_t)*(sum4(y1,y2,y6,y5) - sum4(y0,y3,y7,y4)) ;
            let dzi: Real_t = ( 0.25 as Real_t)*(sum4(z1,z2,z6,z5) - sum4(z0,z3,z7,z4)) ;

            let dxk: Real_t = ( 0.25 as Real_t)*(sum4(x4,x5,x6,x7) - sum4(x0,x1,x2,x3)) ;
            let dyk: Real_t = ( 0.25 as Real_t)*(sum4(y4,y5,y6,y7) - sum4(y0,y1,y2,y3)) ;
            let dzk: Real_t = ( 0.25 as Real_t)*(sum4(z4,z5,z6,z7) - sum4(z0,z1,z2,z3)) ;

            /* find delvk and delxk ( i cross j ) */

            let mut ax = dyi*dzj - dzi*dyj ;
            let mut ay = dzi*dxj - dxi*dzj ;
            let mut az = dxi*dyj - dyi*dxj ;

            self.delx_zeta[i] = vol /(ax*ax + ay*ay + az*az + ptiny).sqrt() ;

            ax *= norm ;
            ay *= norm ;
            az *= norm ;

            let mut dxv = (0.25 as Real_t)*(sum4(xv4,xv5,xv6,xv7) - sum4(xv0,xv1,xv2,xv3)) ;
            let mut dyv = (0.25 as Real_t)*(sum4(yv4,yv5,yv6,yv7) - sum4(yv0,yv1,yv2,yv3)) ;
            let mut dzv = (0.25 as Real_t)*(sum4(zv4,zv5,zv6,zv7) - sum4(zv0,zv1,zv2,zv3)) ;

            self.delv_zeta[i] = ax*dxv + ay*dyv + az*dzv ;

            /* find delxi and delvi ( j cross k ) */

            ax = dyj*dzk - dzj*dyk ;
            ay = dzj*dxk - dxj*dzk ;
            az = dxj*dyk - dyj*dxk ;

            self.delx_xi[i] = vol / (ax*ax + ay*ay + az*az + ptiny).sqrt() ;

            ax *= norm ;
            ay *= norm ;
            az *= norm ;

            dxv = (0.25 as Real_t)*(sum4(xv1,xv2,xv6,xv5) - sum4(xv0,xv3,xv7,xv4)) ;
            dyv = (0.25 as Real_t)*(sum4(yv1,yv2,yv6,yv5) - sum4(yv0,yv3,yv7,yv4)) ;
            dzv = (0.25 as Real_t)*(sum4(zv1,zv2,zv6,zv5) - sum4(zv0,zv3,zv7,zv4)) ;

            self.delv_xi[i] = ax*dxv + ay*dyv + az*dzv ;

            /* find delxj and delvj ( k cross i ) */

            ax = dyk*dzi - dzk*dyi ;
            ay = dzk*dxi - dxk*dzi ;
            az = dxk*dyi - dyk*dxi ;

            self.delx_eta[i] = vol /(ax*ax + ay*ay + az*az + ptiny).sqrt() ;

            ax *= norm ;
            ay *= norm ;
            az *= norm ;

            dxv = (-0.25 as Real_t)*(sum4(xv0,xv1,xv5,xv4) - sum4(xv3,xv2,xv6,xv7)) ;
            dyv = (-0.25 as Real_t)*(sum4(yv0,yv1,yv5,yv4) - sum4(yv3,yv2,yv6,yv7)) ;
            dzv = (-0.25 as Real_t)*(sum4(zv0,zv1,zv5,zv4) - sum4(zv3,zv2,zv6,zv7)) ;

            self.delv_eta[i] = ax*dxv + ay*dyv + az*dzv ;
        }
    }

#[inline]
    fn CalcMonotonicQRegionForElems(// parameters
        &mut self,
        qlc_monoq: Real_t,
        qqc_monoq: Real_t,
        monoq_limiter_mult: Real_t,
        monoq_max_slope: Real_t,
        ptiny: Real_t)
    {
        for ie in self.matElemlist.iter() {
            let i = *ie;
            let bcMask: Int_t = self.elemBC[i] ;

            /*  phixi     */
            let mut norm: Real_t = (1. as Real_t) / ( self.delv_xi[i] + ptiny ) ;

            let mut delvm = match (bcMask & XI_M) {
                0 =>         self.delv_xi[self.lxim[i]],
                XI_M_SYMM => self.delv_xi[i],
                XI_M_FREE => (0.0 as Real_t),
                _ =>        /* ERROR */  {
                    process::exit(17);
                },
            };
            let mut delvp = match (bcMask & XI_P) {
                0 =>         self.delv_xi[self.lxip[i]],
                XI_P_SYMM => self.delv_xi[i],
                XI_P_FREE => (0.0 as Real_t),
                _ =>        /* ERROR */  {
                    process::exit(17);
                },
            };

            delvm = delvm * norm ;
            delvp = delvp * norm ;

            let mut phixi = (0.5 as Real_t) * ( delvm + delvp ) ;

            delvm *= monoq_limiter_mult ;
            delvp *= monoq_limiter_mult ;

            if ( delvm < phixi ) { phixi = delvm }
            if ( delvp < phixi ) { phixi = delvp }
            if ( phixi < (0. as Real_t)) { phixi = (0. as Real_t) }
            if ( phixi > monoq_max_slope) { phixi = monoq_max_slope }


            /*  phieta     */
            norm = (1. as Real_t) / ( self.delv_eta[i] + ptiny ) ;

            delvm = match (bcMask & ETA_M) {
                0=>          self.delv_eta[self.letam[i]],
                ETA_M_SYMM=> self.delv_eta[i],
                ETA_M_FREE=> (0.0 as Real_t),
                _=>         /* ERROR */ process::exit(17),
            };
            delvp = match (bcMask & ETA_P) {
                0=>          self.delv_eta[self.letap[i]],
                ETA_P_SYMM=> self.delv_eta[i],
                ETA_P_FREE=> (0.0 as Real_t),
                _=>         /* ERROR */ process::exit(17),
            };

            delvm = delvm * norm ;
            delvp = delvp * norm ;

            let mut phieta = (0.5 as Real_t) * ( delvm + delvp ) ;

            delvm *= monoq_limiter_mult ;
            delvp *= monoq_limiter_mult ;

            if ( delvm  < phieta ) {phieta = delvm ;}
            if ( delvp  < phieta ) {phieta = delvp ;}
            if ( phieta < (0. as Real_t)) {phieta = (0. as Real_t) ;}
            if ( phieta > monoq_max_slope)  {phieta = monoq_max_slope;}

            /*  phizeta     */
            norm = (1. as Real_t) / ( self.delv_zeta[i] + ptiny ) ;

            delvm = match (bcMask & ZETA_M) {
                0=>          self.delv_zeta[self.lzetam[i]],
                ZETA_M_SYMM=> self.delv_zeta[i],
                ZETA_M_FREE=> (0.0 as Real_t),
                _=>         /* ERROR */ process::exit(17),
            };
            delvp = match (bcMask & ZETA_P) {
                0=>          self.delv_zeta[self.lzetap[i]],
                ZETA_P_SYMM=> self.delv_zeta[i],
                ZETA_P_FREE=> (0.0 as Real_t),
                _=>         /* ERROR */ process::exit(17),
            };

            delvm = delvm * norm ;
            delvp = delvp * norm ;

            let mut phizeta = (0.5 as Real_t) * ( delvm + delvp ) ;

            delvm *= monoq_limiter_mult ;
            delvp *= monoq_limiter_mult ;

            if ( delvm   < phizeta ) {phizeta = delvm ;}
            if ( delvp   < phizeta ) {phizeta = delvp ;}
            if ( phizeta < (0. as Real_t)) {phizeta = (0. as Real_t);}
            if ( phizeta > monoq_max_slope  ) {phizeta = monoq_max_slope;}

            /* Remove length scale */

            let mut qlin = 0.0 as Real_t;
            let mut qquad = 0.0 as Real_t;
            if  self.vdov[i] <= 0.0   {
                let mut delvxxi: Real_t = self.delv_xi[i]   * self.delx_xi[i]   ;
                let mut delvxeta: Real_t = self.delv_eta[i]  * self.delx_eta[i]  ;
                let mut delvxzeta: Real_t = self.delv_zeta[i] * self.delx_zeta[i] ;

                if ( delvxxi   > (0. as Real_t) ) {delvxxi   = (0. as Real_t) ;}
                if ( delvxeta  > (0. as Real_t) ) {delvxeta  = (0. as Real_t) ;}
                if ( delvxzeta > (0. as Real_t) ) {delvxzeta = (0. as Real_t) ;}

                let rho: Real_t = self.elemMass[i] / (self.volo[i] * self.vnew[i]) ;

                qlin = -qlc_monoq * rho *
                    (  delvxxi   * ((1.0 as Real_t) - phixi) +
                       delvxeta  * ((1.0 as Real_t) - phieta) +
                       delvxzeta * ((1.0 as Real_t) - phizeta)  ) ;

                qquad = qqc_monoq * rho *
                    (  delvxxi*delvxxi     * ((1.0 as Real_t) - phixi*phixi) +
                       delvxeta*delvxeta   * ((1.0 as Real_t) - phieta*phieta) +
                       delvxzeta*delvxzeta * ((1.0 as Real_t) - phizeta*phizeta)  ) ;
            }

            self.qq[i] = qquad;
            self.ql[i] = qlin;
        }
    }

#[inline]
    fn CalcMonotonicQForElems(&mut self)
    {  
        //
        // initialize parameters
        // 
        let ptiny: Real_t = (1.0e-36 as Real_t) ;
        let monoq_max_slope: Real_t = self.monoq_max_slope ;
        let monoq_limiter_mult: Real_t = self.monoq_limiter_mult ;

        //
        // calculate the monotonic q for pure regions
        //
        if (self.numElem > 0) {
            let qlc_monoq: Real_t = self.qlc_monoq;
            let qqc_monoq: Real_t = self.qqc_monoq;
            self.CalcMonotonicQRegionForElems(// parameters
                qlc_monoq,
                qqc_monoq,
                monoq_limiter_mult,
                monoq_max_slope,
                ptiny);
        }
    }

#[inline]
    fn CalcQForElems(&mut self)
    {
        let qstop: Real_t = self.qstop ;
        let numElem: Index_t = self.numElem ;

        //
        // MONOTONIC Q option
        //

        /* Calculate velocity gradients */
        self.CalcMonotonicQGradientsForElems() ;

        /* Transfer veloctiy gradients in the first order elements */
        /* problem->commElements->Transfer(CommElements::monoQ) ; */
        self.CalcMonotonicQForElems() ;

        /* Don't allow excessive artificial viscosity */
        if (numElem != 0) {
            let mut idx: Option<Index_t> = None; 
            for i in 0..numElem {
                if ( self.q[i] > qstop ) {
                    idx = Some(i) ;
                    break ;
                }
            }

            if let Some(_) = idx {
                process::exit(QSTOP_ERROR) ;
            }
        }
    }

#[inline]
    fn CalcSoundSpeedForElems(&mut self,
                              vnewc: &[Real_t], rho0: Real_t, enewc: &[Real_t],
                              pnewc: &[Real_t], pbvc: &[Real_t],
                              bvc: &[Real_t], nz: Index_t )
    {
        for i in 0..nz {
            let iz: Index_t = self.matElemlist[i];
            let mut ssTmp: Real_t = (pbvc[i] * enewc[i] + vnewc[i] * vnewc[i] *
                                     bvc[i] * pnewc[i]) / rho0;
            if (ssTmp <= (0.111111e-36 as Real_t)) {
                ssTmp = (0.111111e-36 as Real_t);
            }
            self.ss[iz] =(ssTmp.sqrt());
        }
    }

#[inline]
    fn EvalEOSForElems(&mut self, vnewc: &mut [Real_t], length: Index_t )
    {
        let e_cut: Real_t = self.e_cut;
        let p_cut: Real_t = self.p_cut;
        let q_cut: Real_t = self.q_cut;

        let pmin: Real_t = self.pmin ;
        let emin: Real_t = self.emin ;
        let rho0: Real_t = self.refdens ;

        let mut e_old = vec![0 as Real_t; (length)] ;
        let mut delvc = vec![0 as Real_t; (length)] ;
        let mut p_old = vec![0 as Real_t; (length)] ;
        let mut q_old = vec![0 as Real_t; (length)] ;
        let mut compression = vec![0 as Real_t; (length)] ;
        let mut compHalfStep = vec![0 as Real_t; (length)] ;
        let mut qq = vec![0 as Real_t; (length)] ;
        let mut ql = vec![0 as Real_t; (length)] ;
        let mut work = vec![0 as Real_t; (length)] ;
        let mut p_new = vec![0 as Real_t; (length)] ;
        let mut e_new = vec![0 as Real_t; (length)] ;
        let mut q_new = vec![0 as Real_t; (length)] ;
        let mut bvc = vec![0 as Real_t; (length)] ;
        let mut pbvc = vec![0 as Real_t; (length)] ;

        /* compress data, minimal set */
        for i in 0..length {
            let zidx: Index_t = self.matElemlist[i] ;
            e_old[i] = self.e[zidx] ;
            delvc[i] = self.delv[zidx] ;
            p_old[i] = self.p[zidx] ;
            q_old[i] = self.q[zidx] ;
        }

        for i in 0..length {
            compression[i] = (1. as Real_t) / vnewc[i] - (1. as Real_t);
            let vchalf = vnewc[i] - delvc[i] * (0.5 as Real_t);
            compHalfStep[i] = (1. as Real_t) / vchalf - (1. as Real_t);
        }

        /* Check for v > eosvmax or v < eosvmin */
        if let Some(eosvmin) = self.eosvmin {
            for i in 0..length {
                if (vnewc[i] <= eosvmin) { /* impossible due to calling func? */
                    compHalfStep[i] = compression[i] ;
                }
            }
        }

        if let Some(eosvmax) = self.eosvmax {
            for i in 0..length {
                if (vnewc[i] >= eosvmax) { /* impossible due to calling func? */
                    p_old[i]        = (0. as Real_t) ;
                    compression[i]  = (0. as Real_t) ;
                    compHalfStep[i] = (0. as Real_t) ;
                }
            }
        }

        for i in 0..length {
            let zidx: Index_t = self.matElemlist[i] ;
            qq[i] = self.qq[zidx] ;
            ql[i] = self.ql[zidx] ;
        }

        CalcEnergyForElems(&mut p_new[..], &mut e_new[..], &mut q_new[..],
                           &mut bvc[..],   &mut pbvc[..],
                           &mut p_old[..], &mut e_old[..], &mut q_old[..], 
                           &mut compression[..], &mut compHalfStep[..],
                           &mut vnewc[..], &mut work[..],  &mut delvc[..],
                           pmin,
                           p_cut, e_cut, q_cut, emin,
                           &qq[..],    &ql[..],    rho0,  self.eosvmax,     length);


        for i in 0..length {
            let zidx: Index_t = self.matElemlist[i] ;
            self.p[zidx] = p_new[i] ;
        }

        for i in 0..length {
            let zidx: Index_t = self.matElemlist[i] ;
            self.e[zidx] = e_new[i] ;
        }

        for i in 0..length {
            let zidx: Index_t = self.matElemlist[i] ;
            self.q[zidx] = q_new[i] ;
        }

        self.CalcSoundSpeedForElems(vnewc, rho0, &e_new[..], &mut p_new[..],
                                    &pbvc[..], &bvc[..], length) ;
    }

#[inline]
    fn ApplyMaterialPropertiesForElems(&mut self)
    {
        let length: Index_t = self.numElem ;

        if (length != 0) {
            /* Expose all of the variables needed for material evaluation */
            let mut vnewc = vec![0 as Real_t; (length)] ;

            for i in 0..length {
                let zn: Index_t = self.matElemlist[i] ;
                vnewc[i] = self.vnew[zn] ;
            }

            if let Some(eosvmin) = self.eosvmin {
                for i in 0..length {
                    if (vnewc[i] < eosvmin) {
                        vnewc[i] = eosvmin ;
                    }
                }
            }

            if let Some(eosvmax) = self.eosvmax {
                for i in 0..length {
                    if (vnewc[i] > eosvmax) {
                        vnewc[i] = eosvmax ;
                    }
                }
            }

            for i in 0..length {
                let zn: Index_t = self.matElemlist[i] ;
                let mut vc = self.v[zn] ;
                if let Some(eosvmin) = self.eosvmin {
                    if (vc < eosvmin) {
                        vc = eosvmin ;
                    }
                }
                if let Some(eosvmax) = self.eosvmax {
                    if (vc > eosvmax) {
                        vc = eosvmax ;
                    }
                }
                if (vc <= 0.) {
                    assert!(false, "I found a negative value in your volume: v[{}]={}", i, vc) ;
                }
            }

            self.EvalEOSForElems(&mut vnewc[..], length);
        }
    }

#[inline]
    fn UpdateVolumesForElems(&mut self)
    {
        let numElem: Index_t = self.numElem;
        if (numElem != 0) {
            let v_cut: Real_t = self.v_cut;

            for i in 0..numElem {
                let mut tmpV = self.vnew[i] ;

                if ( (tmpV - (1.0 as Real_t)).abs() < v_cut ) {
                    tmpV = (1.0 as Real_t) ;
                }
                self.v[i] = tmpV ;
                if (self.v[i] <= 0.) {
                    assert!(false, "I found a negative value in your volume: v[{}]={}", i, self.v[i]) ;
                }
            }
        }
    }

#[inline]
    fn LagrangeElements(&mut self)
    {
        let deltatime: Real_t = self.deltatime ;

        self.CalcLagrangeElements(deltatime) ;

        /* Calculate Q.  (Monotonic q option requires communication) */
        self.CalcQForElems() ;

        self.ApplyMaterialPropertiesForElems() ;

        self.UpdateVolumesForElems() ;
    }

#[inline]
    fn CalcCourantConstraintForElems(&mut self)
    {
        let mut dtcourant: Real_t = (1.0e+20 as Real_t) ;
        let mut courant_elem: Option<Index_t> = None ;
        let qqc: Real_t = self.qqc ;
        let length: Index_t = self.numElem ;

        let qqc2: Real_t = (64.0 as Real_t) * qqc * qqc ;

        for i in 0..length {
            let indx: Index_t = self.matElemlist[i] ;

            let mut dtf: Real_t = self.ss[indx] * self.ss[indx] ;

            if ( self.vdov[indx] < (0. as Real_t) ) {
                dtf = dtf
                    + qqc2 * self.arealg[indx] * self.arealg[indx]
                    * self.vdov[indx] * self.vdov[indx] ;
            }

            dtf = dtf.sqrt() ;

            dtf = self.arealg[indx] / dtf ;

            /* determine minimum timestep with its corresponding elem */
            if (self.vdov[indx] != (0. as Real_t)) {
                if ( dtf < dtcourant ) {
                    dtcourant = dtf ;
                    courant_elem = Some(indx) ;
                }
            }
        }

        /* Don't try to register a time constraint if none of the elements
         * were active */
        if let Some(_) = courant_elem {
            self.dtcourant = dtcourant ;
        }
    }

#[inline]
    fn CalcHydroConstraintForElems(&mut self)
    {
        let mut dthydro: Real_t = (1.0e+20 as Real_t) ;
        let mut hydro_elem: Option<Index_t> = None ;
        let dvovmax: Real_t = self.dvovmax ;
        let length: Index_t = self.numElem ;

        for i in 0..length {
            let indx: Index_t = self.matElemlist[i] ;

            if (self.vdov[indx] != (0. as Real_t)) {
                let dtdvov: Real_t = dvovmax / ((self.vdov[indx]).abs()+(1.0e-20 as Real_t)) ;
                if ( dthydro > dtdvov ) {
                    dthydro = dtdvov ;
                    hydro_elem = Some(indx) ;
                }
            }
        }

        if let Some(_) = hydro_elem {
            self.dthydro = dthydro ;
        }
    }

#[inline]
    fn CalcTimeConstraintsForElems(&mut self) {
        /* evaluate time constraint */
        self.CalcCourantConstraintForElems() ;

        /* check hydro constraint */
        self.CalcHydroConstraintForElems() ;
    }

#[inline]
    pub fn lagrange_leap_frog(&mut self)
    {
        /* calculate nodal forces, accelerations, velocities, positions, with
         * applied boundary conditions and slide surface considerations */
        self.LagrangeNodal();

        /* calculate element quantities (i.e. velocity gradient & q), and update
         * material states */
        self.LagrangeElements();

        self.CalcTimeConstraintsForElems();

        // LagrangeRelease() ;  Creation/destruction of temps may be important to capture 
    }

    pub fn PrintAbsDiff(&self)
    {
        let edgeElems = self.sizeX;
        let mut   MaxAbsDiff = 0.0 as Real_t;
        let mut TotalAbsDiff = 0.0 as Real_t;
        let mut   MaxRelDiff = 0.0 as Real_t;

        for j in 0..edgeElems {
            for k in j+1..edgeElems {
                let AbsDiff = (self.e[j*edgeElems+k] - self.e[k*edgeElems+j]).abs();
                TotalAbsDiff  += AbsDiff;

                if (MaxAbsDiff <AbsDiff) {MaxAbsDiff = AbsDiff;}

                let RelDiff = AbsDiff / self.e[k*edgeElems+j];

                if (MaxRelDiff <RelDiff) { MaxRelDiff = RelDiff;}
            }
        }

        println!("   Testing Plane 0 of Energy Array:");
        println!("        MaxAbsDiff   = {:12.6e}",   MaxAbsDiff   );
        println!("        TotalAbsDiff = {:12.6e}",   TotalAbsDiff );
        println!("        MaxRelDiff   = {:12.6e}\n", MaxRelDiff   );
    }

}
