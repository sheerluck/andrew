
m = [[1, 4, 3, 1, 3, 2],
     [3, 2, 1, 3, 2, 4],
     [2, 3, 3, 2, 3, 1]];



w = [[0, 0, 0, 0, 0, 0],
     //[0, 2, 3, 1, 2, 0],
     [0, 1, 2, 0, 1, 0],
     [0, 0, 0, 0, 0, 0]];



module sugar(x,y,h)
{
  for (i = [0: h-1]) {
    translate([2*x, -2*y, 2*i])
    color([1,1,1])
    cube(1.95);
  };   
};

module water(x,y,h)
{
  if (h>0) {
    for (i = [0: h-1]) {
      translate([2*x, -2*y, 2*m[y][x] + 2*i])
      color([0.1,0.1,0.7,0.3])
      cube(1.95);
    };   
  };
};

for ( y = [0:3]) {
    for ( x = [0:6]) {
        sugar(x,y, m[y][x]);
    };
};
for ( y = [0:3]) {
    for ( x = [0:6]) {
        water(x,y, w[y][x]);
    };
};    
