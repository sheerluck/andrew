// http://www.openscad.org/downloads.html

h180= 2.23606798;
h90 = 2.05817103;
h60 = 1.73205081;
h45 = 1.42663106;
h36 = 1.25226119;

a180  = 0.44721359;
ZL180 = 0.89442719; ZH180 = h180 - ZL180;
points_list180 = [
    [ 0,     0,    0     ],
    [ a180,  0,    ZL180 ],
    [-a180,  0,    ZH180 ],
    [ 0,     0,    h180  ]];

a_90  = 0.61803398;
ZL_90 = 0.78615138; ZH_90 = h90 - ZL_90;
points_list90 = [
    [ 0,     0,    0     ],
    [ a_90,  0,    ZL_90 ],
    [ 0,     a_90, ZH_90 ],
    [-a_90,  0,    ZL_90 ],
    [ 0,    -a_90, ZH_90 ],
    [ 0,     0,    h90   ]];

a_60  = 0.81649658; s32   = 0.86602540;
ZL_60 = 0.57735027; ZH_60 = h60 - ZL_60;
points_list60 = [
    [ 0,         0,        0     ],
    [ a_60,      0,        ZL_60 ],
    [ 0.5*a_60,  s32*a_60, ZH_60 ],
    [-0.5*a_60,  s32*a_60, ZL_60 ],
    [-a_60,      0,        ZH_60 ],
    [-0.5*a_60, -s32*a_60, ZL_60 ],
    [ 0.5*a_60, -s32*a_60, ZH_60 ],
    [ 0,     0,            h60   ]];

a_45  = 0.93185165; b_45  = 0.65891862;
ZL_45 = 0.36283949; ZH_45 = h45 - ZL_45;
points_list45 = [
    [ 0,     0,     0     ],
    [ a_45,  0,     ZL_45 ],
    [ b_45,  b_45,  ZH_45 ],
    [ 0,     a_45,  ZL_45 ],
    [-b_45,  b_45,  ZH_45 ],
    [-a_45,  0,     ZL_45 ],
    [-b_45, -b_45,  ZH_45 ],
    [ 0,    -a_45,  ZL_45 ],
    [ b_45, -b_45,  ZH_45 ],
    [ 0,     0,     h45   ]];

a_36  = 0.97392904;
s1 = 0.58778525; s2 = 0.95105652;
c1 = 0.80901699; c2 = 0.30901699;
ZL_36 = 0.22685287; ZH_36 = h36 - ZL_36;
points_list36 = [
    [ 0,        0,        0     ],
    [ a_36,     0,        ZL_36 ],
    [ c1*a_36,  s1*a_36,  ZH_36 ],
    [ c2*a_36,  s2*a_36,  ZL_36 ],
    [-c2*a_36,  s2*a_36,  ZH_36 ],
    [-c1*a_36,  s1*a_36,  ZL_36 ],
    [-a_36,     0,        ZH_36 ],
    [-c1*a_36, -s1*a_36,  ZL_36 ],
    [-c2*a_36, -s2*a_36,  ZH_36 ],
    [ c2*a_36, -s2*a_36,  ZL_36 ],
    [ c1*a_36, -s1*a_36,  ZH_36 ],
    [ 0,        0,        h36   ]];


module top36() {
  color([0.9,0.3,0.2])
    polyhedron( points=points_list36,
                faces=[[2,1,11], [3,2,11], [4,3,11], [5,4,11],
                       [6,5,11], [7,6,11], [8,7,11], [9,8,11],
                       [10,9,11],[1,10,11]] ); }                
module top45() {
  color([0.9,0.3,0.2])
    polyhedron( points=points_list45,
                faces=[[2,1,9], [3,2,9], [4,3,9], [5,4,9],
                       [6,5,9], [7,6,9], [8,7,9], [1,8,9]] ); }
module top60() {
  color([0.9,0.3,0.2])
    polyhedron( points=points_list60,
                faces=[[2,1,7], [3,2,7], [4,3,7], [5,4,7],
                       [6,5,7], [1,6,7]] ); }
module top90() {
  color([0.9,0.3,0.2])
    polyhedron( points=points_list90,
                faces=[[2,1,5], [3,2,5], [4,3,5], [1,4,5]] ); }

module top180() {
  color([0.9,0.3,0.2])
    polyhedron( points=points_list180, faces=[[2,1,3]] ); }

module bottom36() {
  color([0.2,0.3,0.9])
    polyhedron( points=points_list36,
                faces=[[1,2,0], [2,3,0], [3,4,0], [4,5,0],
                       [5,6,0], [6,7,0], [7,8,0], [8,9,0],
                       [9,10,0],[10,1,0]] ); }
module bottom45() {
  color([0.2,0.3,0.9])
    polyhedron( points=points_list45,
                faces=[[1,2,0], [2,3,0], [3,4,0], [4,5,0],
                       [5,6,0], [6,7,0], [7,8,0], [8,1,0]] ); }
module bottom60() {
  color([0.2,0.3,0.9])
    polyhedron( points=points_list60,
                faces=[[1,2,0], [2,3,0], [3,4,0], [4,5,0],
                       [5,6,0], [6,1,0]] ); }
module bottom90() {
  color([0.2,0.3,0.9])
    polyhedron( points=points_list90,
                faces=[[1,2,0], [2,3,0], [3,4,0], [4,1,0]] ); }

module bottom180() {
  color([0.2,0.3,0.9])
    polyhedron( points=points_list180, faces=[[1,2,0]]); }

module thing36() {   top36();   bottom36();    }
module thing45() {   top45();   bottom45();    }
module thing60() {   top60();   bottom60();    }
module thing90() {   top90();   bottom90();    }
module thing180(){   top180();  bottom180();   }

translate([-4, 0, 0]) thing36();
translate([-2, 0, 0]) thing45();
translate([ 0, 0, 0]) thing60();
translate([ 2, 0, 0]) thing90();
translate([ 4, 0, 0]) thing180();
translate([ 0, 0, -0.5]) cube([1,1,1], center=true);

