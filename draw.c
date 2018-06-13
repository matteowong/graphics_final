#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "ml6.h"
#include "display.h"
#include "draw.h"
#include "matrix.h"
#include "math.h"
#include "gmath.h"

/*======== void scanline_convert() ==========
  Inputs: struct matrix *points
          int i
          screen s
          zbuffer zb
  Returns:

  Fills in polygon i by drawing consecutive horizontal (or vertical) lines.

  Color should be set differently for each polygon.
  ====================*/
void scanline_convert( struct matrix *points, int i, screen s, zbuffer zb, color c) {

  int top, mid, bot, y;
  int distance0, distance1, distance2;
  double x0, x1, y0, y1, y2, dx0, dx1, z0, z1, dz0, dz1;
  int flip = 0;

  z0 = z1 = dz0 = dz1 = 0;

  y0 = points->m[1][i];
  y1 = points->m[1][i+1];
  y2 = points->m[1][i+2];

  /* color c; */
  /* c.red = (23 * (i/3))%255; */
  /* c.green = (109 * (i/3))%255; */
  /* c.blue = (c.blue+(227 * (i/3)))%255; */

  //find bot, mid, top
  if ( y0 <= y1 && y0 <= y2) {
    bot = i;
    if (y1 <= y2) {
      mid = i+1;
      top = i+2;
    }
    else {
      mid = i+2;
      top = i+1;
    }
  }//end y0 bottom
  else if (y1 <= y0 && y1 <= y2) {
    bot = i+1;
    if (y0 <= y2) {
      mid = i;
      top = i+2;
    }
    else {
      mid = i+2;
      top = i;
    }
  }//end y1 bottom
  else {
    bot = i+2;
    if (y0 <= y1) {
      mid = i;
      top = i+1;
    }
    else {
      mid = i+1;
      top = i;
    }
  }//end y2 bottom
  //printf("ybot: %0.2f, ymid: %0.2f, ytop: %0.2f\n", (points->m[1][bot]),(points->m[1][mid]), (points->m[1][top]));
  /* printf("bot: (%0.2f, %0.2f, %0.2f) mid: (%0.2f, %0.2f, %0.2f) top: (%0.2f, %0.2f, %0.2f)\n", */

  x0 = points->m[0][bot];
  x1 = points->m[0][bot];
  z0 = points->m[2][bot];
  z1 = points->m[2][bot];
  y = (int)(points->m[1][bot]);

  distance0 = (int)(points->m[1][top]) - y;
  distance1 = (int)(points->m[1][mid]) - y;
  distance2 = (int)(points->m[1][top]) - (int)(points->m[1][mid]);

  //printf("distance0: %d distance1: %d distance2: %d\n", distance0, distance1, distance2);
  dx0 = distance0 > 0 ? (points->m[0][top]-points->m[0][bot])/distance0 : 0;
  dx1 = distance1 > 0 ? (points->m[0][mid]-points->m[0][bot])/distance1 : 0;
  dz0 = distance0 > 0 ? (points->m[2][top]-points->m[2][bot])/distance0 : 0;
  dz1 = distance1 > 0 ? (points->m[2][mid]-points->m[2][bot])/distance1 : 0;

  while ( y <= (int)points->m[1][top] ) {
    //printf("\tx0: %0.2f x1: %0.2f y: %d\n", x0, x1, y);
    draw_line(x0, y, z0, x1, y, z1, s, zb, c);

    x0+= dx0;
    x1+= dx1;
    z0+= dz0;
    z1+= dz1;
    y++;

    if ( !flip && y >= (int)(points->m[1][mid]) ) {
      flip = 1;
      dx1 = distance2 > 0 ? (points->m[0][top]-points->m[0][mid])/distance2 : 0;
      dz1 = distance2 > 0 ? (points->m[2][top]-points->m[2][mid])/distance2 : 0;
      x1 = points->m[0][mid];
      z1 = points->m[2][mid];
    }//end flip code
  }//end scanline loop
}

/*======== void add_polygon() ==========
  Inputs:   struct matrix *surfaces
  double x0
  double y0
  double z0
  double x1
  double y1
  double z1
  double x2
  double y2
  double z2
  Returns:
  Adds the vertices (x0, y0, z0), (x1, y1, z1)
  and (x2, y2, z2) to the polygon matrix. They
  define a single triangle surface.
  ====================*/
void add_polygon( struct matrix *polygons,
                  double x0, double y0, double z0,
                  double x1, double y1, double z1,
                  double x2, double y2, double z2 ) {

  add_point(polygons, x0, y0, z0);
  add_point(polygons, x1, y1, z1);
  add_point(polygons, x2, y2, z2);
}

/*======== void draw_polygons() ==========
  Inputs:   struct matrix *polygons
  screen s
  color c
  Returns:
  Goes through polygons 3 points at a time, drawing
  lines connecting each points to create bounding
  triangles
  ====================*/
void draw_polygons(struct matrix *polygons, screen s, zbuffer zb,
                   double *view, double light[2][3], color ambient,
                   double *areflect,
                   double *dreflect,
                   double *sreflect) {
  if ( polygons->lastcol < 3 ) {
    printf("Need at least 3 points to draw a polygon!\n");
    return;
  }

  print_matrix(polygons);
  
  int point;
  double *normal;

  for (point=0; point < polygons->lastcol-2; point+=3) {

    normal = calculate_normal(polygons, point);

    if ( dot_product(normal, view) > 0 ) {

      color c = get_lighting(normal, view, ambient, light, areflect, dreflect, sreflect);
      //color c;
      //c.red=c.green=c.blue=0;

      scanline_convert(polygons, point, s, zb, c);

      draw_line( polygons->m[0][point],
                 polygons->m[1][point],
                 polygons->m[2][point],
                 polygons->m[0][point+1],
                 polygons->m[1][point+1],
                 polygons->m[2][point+1],
                 s, zb, c);
      draw_line( polygons->m[0][point+2],
                 polygons->m[1][point+2],
                 polygons->m[2][point+2],
                 polygons->m[0][point+1],
                 polygons->m[1][point+1],
                 polygons->m[2][point+1],
                 s, zb, c);
      draw_line( polygons->m[0][point],
                 polygons->m[1][point],
                 polygons->m[2][point],
                 polygons->m[0][point+2],
                 polygons->m[1][point+2],
                 polygons->m[2][point+2],
                 s, zb, c);
    }
  }
}

/*======== void add_box() ==========
  Inputs:   struct matrix * edges
  double x
  double y
  double z
  double width
  double height
  double depth
  Returns:

  add the points for a rectagular prism whose
  upper-left corner is (x, y, z) with width,
  height and depth dimensions.
  ====================*/
void add_box( struct matrix * polygons,
              double x, double y, double z,
              double width, double height, double depth ) {

  double x1, y1, z1;
  x1 = x+width;
  y1 = y-height;
  z1 = z-depth;

  //front
  add_polygon(polygons, x, y, z, x1, y1, z, x1, y, z);
  add_polygon(polygons, x, y, z, x, y1, z, x1, y1, z);

  //back
  add_polygon(polygons, x1, y, z1, x, y1, z1, x, y, z1);
  add_polygon(polygons, x1, y, z1, x1, y1, z1, x, y1, z1);

  //right side
  add_polygon(polygons, x1, y, z, x1, y1, z1, x1, y, z1);
  add_polygon(polygons, x1, y, z, x1, y1, z, x1, y1, z1);
  //left side
  add_polygon(polygons, x, y, z1, x, y1, z, x, y, z);
  add_polygon(polygons, x, y, z1, x, y1, z1, x, y1, z);

  //top
  add_polygon(polygons, x, y, z1, x1, y, z, x1, y, z1);
  add_polygon(polygons, x, y, z1, x, y, z, x1, y, z);
  //bottom
  add_polygon(polygons, x, y1, z, x1, y1, z1, x1, y1, z);
  add_polygon(polygons, x, y1, z, x, y1, z1, x1, y1, z1);
}//end add_box

/*======== void add_sphere() ==========
  Inputs:   struct matrix * points
  double cx
  double cy
  double cz
  double r
  double step
  Returns:

  adds all the points for a sphere with center
  (cx, cy, cz) and radius r.

  should call generate_sphere to create the
  necessary points
  ====================*/
void add_sphere( struct matrix * edges,
                 double cx, double cy, double cz,
                 double r, int step ) {

  struct matrix *points = generate_sphere(cx, cy, cz, r, step);

  int p0, p1, p2, p3, lat, longt;
  int latStop, longStop, latStart, longStart;
  latStart = 0;
  latStop = step;
  longStart = 0;
  longStop = step;

  step++;
  for ( lat = latStart; lat < latStop; lat++ ) {
    for ( longt = longStart; longt < longStop; longt++ ) {

      p0 = lat * (step) + longt;
      p1 = p0+1;
      p2 = (p1+step) % (step * (step-1));
      p3 = (p0+step) % (step * (step-1));

      //printf("p0: %d\tp1: %d\tp2: %d\tp3: %d\n", p0, p1, p2, p3);
      if (longt < step - 2)
        add_polygon( edges, points->m[0][p0],
                     points->m[1][p0],
                     points->m[2][p0],
                     points->m[0][p1],
                     points->m[1][p1],
                     points->m[2][p1],
                     points->m[0][p2],
                     points->m[1][p2],
                     points->m[2][p2]);
      if (longt > 0 )
        add_polygon( edges, points->m[0][p0],
                     points->m[1][p0],
                     points->m[2][p0],
                     points->m[0][p2],
                     points->m[1][p2],
                     points->m[2][p2],
                     points->m[0][p3],
                     points->m[1][p3],
                     points->m[2][p3]);
    }
  }
  free_matrix(points);
}

/*======== void generate_sphere() ==========
  Inputs:   double cx
  double cy
  double cz
  double r
  int step
  Returns: Generates all the points along the surface
  of a sphere with center (cx, cy, cz) and
  radius r.
  Returns a matrix of those points
  ====================*/
struct matrix * generate_sphere(double cx, double cy, double cz,
                                double r, int step ) {

  struct matrix *points = new_matrix(4, step * step);
  int circle, rotation, rot_start, rot_stop, circ_start, circ_stop;
  double x, y, z, rot, circ;

  rot_start = 0;
  rot_stop = step;
  circ_start = 0;
  circ_stop = step;

  for (rotation = rot_start; rotation < rot_stop; rotation++) {
    rot = (double)rotation / step;

    for(circle = circ_start; circle <= circ_stop; circle++){
      circ = (double)circle / step;

      x = r * cos(M_PI * circ) + cx;
      y = r * sin(M_PI * circ) *
        cos(2*M_PI * rot) + cy;
      z = r * sin(M_PI * circ) *
        sin(2*M_PI * rot) + cz;

      /* printf("rotation: %d\tcircle: %d\n", rotation, circle); */
      /* printf("rot: %lf\tcirc: %lf\n", rot, circ); */
      /* printf("sphere point: (%0.2f, %0.2f, %0.2f)\n\n", x, y, z); */
      add_point(points, x, y, z);
    }
  }

  return points;
}

/*======== void add_torus() ==========
  Inputs:   struct matrix * points
  double cx
  double cy
  double cz
  double r1
  double r2
  double step
  Returns:

  adds all the points required to make a torus
  with center (cx, cy, cz) and radii r1 and r2.

  should call generate_torus to create the
  necessary points
  ====================*/
void add_torus( struct matrix * edges,
                double cx, double cy, double cz,
                double r1, double r2, int step ) {

  struct matrix *points = generate_torus(cx, cy, cz, r1, r2, step);

  int p0, p1, p2, p3, lat, longt;
  int latStop, longStop, latStart, longStart;
  latStart = 0;
  latStop = step;
  longStart = 0;
  longStop = step;

  //printf("points: %d\n", points->lastcol);

  for ( lat = latStart; lat < latStop; lat++ ) {
    for ( longt = longStart; longt < longStop; longt++ ) {
      p0 = lat * step + longt;
      if (longt == step - 1)
        p1 = p0 - longt;
      else
        p1 = p0 + 1;
      p2 = (p1 + step) % (step * step);
      p3 = (p0 + step) % (step * step);

      //printf("p0: %d\tp1: %d\tp2: %d\tp3: %d\n", p0, p1, p2, p3);
      add_polygon( edges, points->m[0][p0],
                   points->m[1][p0],
                   points->m[2][p0],
                   points->m[0][p3],
                   points->m[1][p3],
                   points->m[2][p3],
                   points->m[0][p2],
                   points->m[1][p2],
                   points->m[2][p2]);
      add_polygon( edges, points->m[0][p0],
                   points->m[1][p0],
                   points->m[2][p0],
                   points->m[0][p2],
                   points->m[1][p2],
                   points->m[2][p2],
                   points->m[0][p1],
                   points->m[1][p1],
                   points->m[2][p1]);
    }
  }
  free_matrix(points);
}
/*======== void generate_torus() ==========
  Inputs:   struct matrix * points
  double cx
  double cy
  double cz
  double r
  int step
  Returns: Generates all the points along the surface
  of a torus with center (cx, cy, cz) and
  radii r1 and r2.
  Returns a matrix of those points
  ====================*/
struct matrix * generate_torus( double cx, double cy, double cz,
                                double r1, double r2, int step ) {

  struct matrix *points = new_matrix(4, step * step);
  int circle, rotation, rot_start, rot_stop, circ_start, circ_stop;
  double x, y, z, rot, circ;

  rot_start = 0;
  rot_stop = step;
  circ_start = 0;
  circ_stop = step;

  for (rotation = rot_start; rotation < rot_stop; rotation++) {
    rot = (double)rotation / step;

    for(circle = circ_start; circle < circ_stop; circle++){
      circ = (double)circle / step;

      x = cos(2*M_PI * rot) *
        (r1 * cos(2*M_PI * circ) + r2) + cx;
      y = r1 * sin(2*M_PI * circ) + cy;
      z = -1*sin(2*M_PI * rot) *
        (r1 * cos(2*M_PI * circ) + r2) + cz;

      //printf("rotation: %d\tcircle: %d\n", rotation, circle);
      //printf("torus point: (%0.2f, %0.2f, %0.2f)\n", x, y, z);
      add_point(points, x, y, z);
    }
  }
  return points;
}

/*======== void add_circle() ==========
  Inputs:   struct matrix * points
  double cx
  double cy
  double r
  double step
  Returns:

  Adds the circle at (cx, cy) with radius r to edges
  ====================*/
void add_circle( struct matrix * edges,
                 double cx, double cy, double cz,
                 double r, int step ) {
  double x0, y0, x1, y1, t;
  int i;
  x0 = r + cx;
  y0 = cy;

  for (i=1; i<=step; i++) {
    t = (double)i/step;
    x1 = r * cos(2 * M_PI * t) + cx;
    y1 = r * sin(2 * M_PI * t) + cy;

    add_edge(edges, x0, y0, cz, x1, y1, cz);
    x0 = x1;
    y0 = y1;
  }
}


/*======== void add_curve() ==========
  Inputs:   struct matrix *points
  double x0
  double y0
  double x1
  double y1
  double x2
  double y2
  double x3
  double y3
  double step
  int type
  Returns:

  Adds the curve bounded by the 4 points passsed as parameters
  of type specified in type (see matrix.h for curve type constants)
  to the matrix points
  ====================*/
void add_curve( struct matrix *edges,
                double x0, double y0,
                double x1, double y1,
                double x2, double y2,
                double x3, double y3,
                int step, int type ) {

  double t, x, y;
  struct matrix *xcoefs;
  struct matrix *ycoefs;
  int i;

  xcoefs = generate_curve_coefs(x0, x1, x2, x3, type);
  ycoefs = generate_curve_coefs(y0, y1, y2, y3, type);

  /* print_matrix(xcoefs); */
  /* printf("\n"); */
  /* print_matrix(ycoefs); */

  for (i=1; i<=step; i++) {

    t = (double)i/step;
    x = xcoefs->m[0][0] *t*t*t + xcoefs->m[1][0] *t*t+
      xcoefs->m[2][0] *t + xcoefs->m[3][0];
    y = ycoefs->m[0][0] *t*t*t + ycoefs->m[1][0] *t*t+
      ycoefs->m[2][0] *t + ycoefs->m[3][0];

    add_edge(edges, x0, y0, 0, x, y, 0);
    x0 = x;
    y0 = y;
  }

  free_matrix(xcoefs);
  free_matrix(ycoefs);
}


/*======== void add_point() ==========
  Inputs:   struct matrix * points
  int x
  int y
  int z
  Returns:
  adds point (x, y, z) to points and increment points.lastcol
  if points is full, should call grow on points
  ====================*/
void add_point( struct matrix * points, double x, double y, double z) {

  if ( points->lastcol == points->cols )
    grow_matrix( points, points->lastcol + 100 );

  points->m[0][ points->lastcol ] = x;
  points->m[1][ points->lastcol ] = y;
  points->m[2][ points->lastcol ] = z;
  points->m[3][ points->lastcol ] = 1;
  points->lastcol++;
} //end add_point

/*======== void add_edge() ==========
  Inputs:   struct matrix * points
  int x0, int y0, int z0, int x1, int y1, int z1
  Returns:
  add the line connecting (x0, y0, z0) to (x1, y1, z1) to points
  should use add_point
  ====================*/
void add_edge( struct matrix * points,
               double x0, double y0, double z0,
               double x1, double y1, double z1) {
  add_point( points, x0, y0, z0 );
  add_point( points, x1, y1, z1 );
}

/*======== void draw_lines() ==========
  Inputs:   struct matrix * points
  screen s
  color c
  Returns:
  Go through points 2 at a time and call draw_line to add that line
  to the screen
  ====================*/
void draw_lines( struct matrix * points, screen s, zbuffer zb, color c) {

  if ( points->lastcol < 2 ) {
    printf("Need at least 2 points to draw a line!\n");
    return;
  }
  int point;
  for (point=0; point < points->lastcol-1; point+=2)
    draw_line( points->m[0][point],
               points->m[1][point],
               points->m[2][point],
               points->m[0][point+1],
               points->m[1][point+1],
               points->m[2][point+1],
               s, zb, c);
}// end draw_lines




void draw_line(int x0, int y0, double z0,
               int x1, int y1, double z1,
               screen s, zbuffer zb, color c) {


  int x, y, d, A, B;
  int dy_east, dy_northeast, dx_east, dx_northeast, d_east, d_northeast;
  int loop_start, loop_end;
  double distance;
  double z, dz;

  //swap points if going right -> left
  int xt, yt;
  if (x0 > x1) {
    xt = x0;
    yt = y0;
    z = z0;
    x0 = x1;
    y0 = y1;
    z0 = z1;
    x1 = xt;
    y1 = yt;
    z1 = z;
  }

  x = x0;
  y = y0;
  A = 2 * (y1 - y0);
  B = -2 * (x1 - x0);
  int wide = 0;
  int tall = 0;
  //octants 1 and 8
  if ( abs(x1 - x0) >= abs(y1 - y0) ) { //octant 1/8
    wide = 1;
    loop_start = x;
    loop_end = x1;
    dx_east = dx_northeast = 1;
    dy_east = 0;
    d_east = A;
    distance = x1 - x;
    if ( A > 0 ) { //octant 1
      d = A + B/2;
      dy_northeast = 1;
      d_northeast = A + B;
    }
    else { //octant 8
      d = A - B/2;
      dy_northeast = -1;
      d_northeast = A - B;
    }
  }//end octant 1/8
  else { //octant 2/7
    tall = 1;
    dx_east = 0;
    dx_northeast = 1;
    distance = abs(y1 - y);
    if ( A > 0 ) {     //octant 2
      d = A/2 + B;
      dy_east = dy_northeast = 1;
      d_northeast = A + B;
      d_east = B;
      loop_start = y;
      loop_end = y1;
    }
    else {     //octant 7
      d = A/2 - B;
      dy_east = dy_northeast = -1;
      d_northeast = A - B;
      d_east = -1 * B;
      loop_start = y1;
      loop_end = y;
    }
  }

  z = z0;
  dz = (z1 - z0) / distance;
  //printf("\t(%d, %d) -> (%d, %d)\tdistance: %0.2f\tdz: %0.2f\tz: %0.2f\n", x0, y0, x1, y1, distance, dz, z);

  while ( loop_start < loop_end ) {

    plot( s, zb, c, x, y, z );
    if ( (wide && ((A > 0 && d > 0) ||
                   (A < 0 && d < 0)))
         ||
         (tall && ((A > 0 && d < 0 ) ||
                   (A < 0 && d > 0) ))) {
      y+= dy_northeast;
      d+= d_northeast;
      x+= dx_northeast;
    }
    else {
      x+= dx_east;
      y+= dy_east;
      d+= d_east;
    }
    z+= dz;
    loop_start++;
  } //end drawing loop
  plot( s, zb, c, x1, y1, z );
} //end draw_line

struct matrix * generate_cylinder(double cx, double cy, double cz, double r, double h0, int step) {

  int h=(int)h0;
  struct matrix * points = new_matrix(4, step*h);
  double x,height,z;
  int rot;
  height=0;

  int heightstep=1;
  //if (h0>99)
  //heightstep=5;
  while (height<=h) {
    printf("height: %lf\n",height);
    //printf("height: %lf\n",height);
    for (rot=0;rot<step;rot++) {
      double t = (double)rot/step;
      x=r*cos(2*M_PI*t)+cx;
      z=r*sin(2*M_PI*t)+cz;//problem is here
      printf("inner rot height: %lf\n",height);
      add_point(points,x,height+cy,z);
      //printf("x: %lf, y: %lf, z: %lf\n",x,height,z);

    }
    height+=heightstep;
  }
  return points;

}

void add_cylinder(struct matrix * edges, double cx, double cy, double cz, double r, double h, int step) {
  struct matrix * points=generate_cylinder(cx,cy,cz,r,h,step);
  printf("points\n");
  print_matrix(points);
  int p0, p1, p2, p3;
  int height, j;

  int heightstep=1;
  //if (h>99)
  //heightstep=5;


  //add circles at ends
  
  double x0=r+cx;
  double z0=cz;
  double x1,z1;
  int rot;
  for (rot=1;rot<=step;rot++) {
      double t = (double)rot/step;
      x1=r*cos(2*M_PI*t)+cx;
      z1=r*sin(2*M_PI*t)+cz;

      add_polygon(edges,x0,cy,z0,x1,cy,z1,cx,cy,cz);
      add_polygon(edges,x1,cy+h,z1,x0,cy+h,z0,cx,cy+h,cz);
      x0=x1;
      z0=z1;

  }
  
  for (height=0;height<h;height++) {
    for (j=0;j<step;j++) {
      
      p0=step*height+j;
      p2=step*(height+1)+j;
      if (j==step-1) {
	p1=step*height;
	p3=step*(height+1);
      } else {
	p1=p0+1;
	p3=p2+1;
      }
      //printf("p0: %d, p1: %d, p2: %d, p3: %d\n",p0,p1,p2,p3);
	add_polygon( edges, points->m[0][p0],
                   points->m[1][p0],
                   points->m[2][p0],
                   points->m[0][p3],
                   points->m[1][p3],
                   points->m[2][p3],
                   points->m[0][p1],
                   points->m[1][p1],
                   points->m[2][p1]);
	add_polygon( edges, points->m[0][p0],
                   points->m[1][p0],
                   points->m[2][p0],
                   points->m[0][p2],
                   points->m[1][p2],
                   points->m[2][p2],
                   points->m[0][p3],
                   points->m[1][p3],
                   points->m[2][p3]);
      
      
    }

  }
  
  /*
  int i=0;
  while (i<points->lastcol){
    add_edge(edges,points->m[0][i],points->m[1][i],points->m[2][i],points->m[0][i],points->m[1][i]+1,points->m[2][i]);
    i++;
    }*/

  free(points);
}

void add_cone(struct matrix * edges, double cx, double cy, double cz, double radius, double h0, int step) {

  double x0=radius+cx;
  double z0=cz;
  double x1,z1;
  int rot;
  for (rot=1;rot<=step;rot++) {
      double t = (double)rot/step;
      x1=radius*cos(2*M_PI*t)+cx;
      z1=radius*sin(2*M_PI*t)+cz;

      //add_edge(edges,x0,cy,z0,x1,cy,z1);
      add_polygon(edges,x0,cy,z0,x1,cy,z1,cx,cy,cz);
      //add_polygon(edges,x1,cy+h,z1,x0,cy+h,z0,cx,cy+h,cz);
      x0=x1;
      z0=z1;

  }

  struct matrix * points=generate_cone(cx,cy,cz,radius,h0,step);

  int p0, p1, p2, p3;
  int height, j;

  int heightstep=1;
  //if (h>99)
  //heightstep=5;


  //add circles at ends
  
  
  for (height=0;height<h0;height++) {
    for (j=0;j<step;j++) {
      
      p0=step*height+j;
      p2=step*(height+1)+j;
      if (j==step-1) {
	p1=step*height;
	p3=step*(height+1);
      } else {
	p1=p0+1;
	p3=p2+1;
      }
      //printf("p0: %d, p1: %d, p2: %d, p3: %d\n",p0,p1,p2,p3);
	add_polygon( edges, points->m[0][p0],
                   points->m[1][p0],
                   points->m[2][p0],
                   points->m[0][p3],
                   points->m[1][p3],
                   points->m[2][p3],
                   points->m[0][p1],
                   points->m[1][p1],
                   points->m[2][p1]);
	add_polygon( edges, points->m[0][p0],
                   points->m[1][p0],
                   points->m[2][p0],
                   points->m[0][p2],
                   points->m[1][p2],
                   points->m[2][p2],
                   points->m[0][p3],
                   points->m[1][p3],
                   points->m[2][p3]);
      
      
    }

  }

  /*
  int i=0;
  while (i<points->lastcol){
    add_edge(edges,points->m[0][i],points->m[1][i],points->m[2][i],points->m[0][i],points->m[1][i]+1,points->m[2][i]);
    i++;
    }*/

  free(points);


}
struct matrix * generate_cone(double cx, double cy, double cz, double radius, double h0, int step) {

  int h=(int)h0;
  struct matrix * points = new_matrix(4, step*h);
  double x,height,z;
  int rot;
  height=0;

  int heightstep=1;
  double r;
  //if (h0>99)
  //heightstep=5;
  while (height<=h) {
    //printf("height: %lf\n",height);
    r=radius*((double)(h0-height)/h0);
    //printf("height: %lf\n",height);
    for (rot=0;rot<step;rot++) {
      double t = (double)rot/step;
      x=r*cos(2*M_PI*t)+cx;
      z=r*sin(2*M_PI*t)+cz;//problem is here
      printf("inner rot height: %lf\n",height);
      add_point(points,x,height+cy,z);
      //printf("x: %lf, y: %lf, z: %lf\n",x,height,z);

    }
    height+=heightstep;
  }
  return points;

}




/*

int main() {


  color ambient;
  double light[2][3];
  double view[3];
  double areflect[3];
  double dreflect[3];
  double sreflect[3];

  ambient.red = 50;
  ambient.green = 50;
  ambient.blue = 50;

  light[LOCATION][0] = 0.5;
  light[LOCATION][1] = 0.75;
  light[LOCATION][2] = 1;

  light[COLOR][RED] = 0;
  light[COLOR][GREEN] = 255;
  light[COLOR][BLUE] = 255;

  view[0] = 0;
  view[1] = 0;
  view[2] = 1;

  areflect[RED] = 0.1;
  areflect[GREEN] = 0.1;
  areflect[BLUE] = 0.1;

  dreflect[RED] = 0.5;
  dreflect[GREEN] = 0.5;
  dreflect[BLUE] = 0.5;

  sreflect[RED] = 0.5;
  sreflect[GREEN] = 0.5;
  sreflect[BLUE] = 0.5;


  double n_0[3]={1, 1, 10};
  double n_1[3]={0, 0, 10};
  normalize(n_0);
  normalize(n_1);
  //printf("[draw line phong] normal0:\nx:%lf |y:%lf |z:%lf\n",n_0[0],n_0[1],n_0[2]);
  //printf("[draw line phong] normal1:\nx:%lf |y:%lf |z:%lf\n\n\n",n_1[0],n_1[1],n_1[2]);

  color c0,c1;
  c0.red=c1.red=5;
  c0.green=30;
  c0.blue=30;
  c1.green=130;
  c1.blue=130;
  
  screen s;
  zbuffer zb;
  clear_screen(s);
  clear_zbuffer(zb);

  //draw_line_gouraud(10, 100, 8, 300, 100, 99, s, zb, c0, c1);
  //draw_line_gouraud(10, 101, 8, 300, 101, 99, s, zb, c0, c1);
  //draw_line_gouraud(10, 102, 8, 300, 102, 99, s, zb, c0, c1);

  //draw_line_gouraud2(10, 200, 8, 300, 200, 99, s, zb, c0, c1);
  //draw_line_gouraud2(10, 201, 8, 300, 201, 99, s, zb, c0, c1);
  //draw_line_gouraud2(10, 202, 8, 300, 202, 99, s, zb, c0, c1);

  //draw_line_phong(10, 100, 8, 300, 100, 99, s, zb, n_0, n_1, view, light, ambient, areflect, dreflect, sreflect);
  //draw_line_phong(10, 101, 8, 300, 101, 99, s, zb, n_0, n_1, view, light, ambient, areflect, dreflect, sreflect);
  //draw_line_phong(10, 102, 8, 300, 102, 99, s, zb, n_0, n_1, view, light, ambient, areflect, dreflect, sreflect);


  struct matrix * polygons=new_matrix(4,1000);
  add_cylinder(polygons,0,-10,0,100,100,8);
  //print_matrix(polygons);
  struct matrix * move=make_translate(250,100,0);
  struct matrix * rot=make_rotX(-10);
  matrix_mult(rot,polygons);
  matrix_mult(move,polygons);
  
    
  //matrix_mult(transform,edges);
  //matrix_mult(transform2,edges);

  //print_matrix(edges);
  draw_polygons(polygons, s, zb, view, light, ambient, areflect, dreflect, sreflect);
  //draw_lines(edges, s, zb, c0);
  
  //edges->lastcol=0;
  //add_sphere(edges, 0, 0, 0, 50, 30);
  //draw_polygons(edges, s, zb, view, light, ambient, areflect, dreflect, sreflect);
  display(s);
  //save_ppm(s, "test.ppm");
  free(polygons);
  return 0;
}*/

