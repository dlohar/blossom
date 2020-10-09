use std::f64;
 
const _EPS: f64 = 0.00001;
const _MIN: f64 = f64::MIN_POSITIVE;
const _MAX: f64 = f64::MAX;
 
#[derive(Debug)]
#[derive(Clone)]
struct Point {
    x: f64,
    y: f64,
}

#[derive(Debug)]
#[derive(Clone)]
struct Edge {
    pt1: Point,
    pt2: Point,
}
 
impl Edge {
    fn new(pt1: (f64, f64), pt2: (f64, f64)) -> Edge {
        Edge {
            pt1: Point { x: pt1.0, y: pt1.1 },
            pt2: Point { x: pt2.0, y: pt2.1 },
        }
    }
}
 
struct Polygon {
    edges: Vec<Edge>, // Polygon has to be created with counter-clockwise coordinates
}
 
fn pt_in_polygon(pt: &Point, poly: &Polygon) -> bool {
    let count = poly.edges
        .iter()
        .filter(|edge| ray_intersect_seg(pt, edge))
        .count();
 
    count % 2 == 1
}

fn numerical_kernel1(a_x : f64, a_y : f64, b_x : f64, b_y : f64) -> f64 {
	(b_y - a_y) / (b_x - a_x)
}

fn numerical_kernel2(a_x : f64, a_y : f64, b_x : f64, b_y : f64) -> f64 {
	(b_y - a_y) / (b_x - a_x)
}
 
fn ray_intersect_seg(p: &Point, edge: &Edge) -> bool {
    let mut pt = p.clone();
    let (mut a, mut b): (&Point, &Point) = (&edge.pt1, &edge.pt2);
    if a.y > b.y {
        std::mem::swap(&mut a, &mut b);
    }
    if pt.y == a.y || pt.y == b.y {
        pt.y += _EPS;
    }

    println!("{:?}", edge);

    if (pt.y > b.y || pt.y < a.y) || pt.x > a.x.max(b.x) {
        false
    } else if pt.x < a.x.min(b.x) {
        true
    } else {
        let m_red = if (a.x - b.x).abs() > _MIN {
	    numerical_kernel1(a.x, a.y, b.x, b.y)
        } else {
            _MAX
        };
        let m_blue = if (a.x - pt.x).abs() > _MIN {
	    numerical_kernel2(a.x, a.y, pt.x, pt.y)
        } else {
            _MAX
        };
        m_blue >= m_red
    }
}
 
pub fn original_program_main(x : f64, y : f64) {
    let p = |x, y| Point { x, y };
    let testpoints = [ p(x, y)];
    let poly_square = Polygon {
        edges: vec![
            Edge::new((0.0, 0.0), (10.0, 0.0)),
            Edge::new((10.0, 0.0), (10.0, 10.0)),
            Edge::new((10.0, 10.0), (0.0, 10.0)),
            Edge::new((0.0, 10.0), (0.0, 0.0)),
        ],
    };
    let poly_square_hole = Polygon {
        edges: vec![
            Edge::new((0.0, 0.0), (10.0, 0.0)),
            Edge::new((10.0, 0.0), (10.0, 10.0)),
            Edge::new((10.0, 10.0), (0.0, 10.0)),
            Edge::new((0.0, 10.0), (0.0, 0.0)),
            Edge::new((2.5, 2.5), (7.5, 2.5)),
            Edge::new((7.5, 2.5), (7.5, 7.5)),
            Edge::new((7.5, 7.5), (2.5, 7.5)),
            Edge::new((2.5, 7.5), (2.5, 2.5)),
        ],
    };
    let poly_strange = Polygon {
        edges: vec![
            Edge::new((0.0, 0.0), (2.5, 2.5)),
            Edge::new((2.5, 2.5), (0.0, 10.0)),
            Edge::new((0.0, 10.0), (2.5, 7.5)),
            Edge::new((2.5, 7.5), (7.5, 7.5)),
            Edge::new((7.5, 7.5), (10.0, 10.0)),
            Edge::new((10.0, 10.0), (10.0, 0.0)),
            Edge::new((10.0, 0.0), (2.5, 2.5)),
        ],
    };
    let poly_hexagon = Polygon {
        edges: vec![
            Edge::new((3.0, 0.0), (7.0, 0.0)),
            Edge::new((7.0, 0.0), (10.0, 5.0)),
            Edge::new((10.0, 5.0), (7.0, 10.0)),
            Edge::new((7.0, 10.0), (3.0, 10.0)),
            Edge::new((3.0, 10.0), (0.0, 5.0)),
            Edge::new((0.0, 5.0), (3.0, 0.0)),
        ],
    };
    for pt in &testpoints {
        pt_in_polygon(pt, &poly_square);
    }
    for pt in &testpoints {
        pt_in_polygon(pt, &poly_square_hole);
    }
    for pt in &testpoints {
        pt_in_polygon(pt, &poly_strange);
    }
    for pt in &testpoints {
       pt_in_polygon(pt, &poly_hexagon);
    }
}
