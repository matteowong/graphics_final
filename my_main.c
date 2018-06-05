/*========== my_main.c ==========

  This is the only file you need to modify in order
  to get a working mdl project (for now).

  my_main.c will serve as the interpreter for mdl.
  When an mdl script goes through a lexer and parser,
  the resulting operations will be in the array op[].

  Your job is to go through each entry in op and perform
  the required action from the list below:

  push: push a new origin matrix onto the origin stack
  pop: remove the top matrix on the origin stack

  move/scale/rotate: create a transformation matrix
  based on the provided values, then
  multiply the current top of the
  origins stack by it.

  box/sphere/torus: create a solid object based on the
  provided values. Store that in a
  temporary matrix, multiply it by the
  current top of the origins stack, then
  call draw_polygons.

  line: create a line based on the provided values. Stores
  that in a temporary matrix, multiply it by the
  current top of the origins stack, then call draw_lines.

  save: call save_extension with the provided filename

  display: view the image live
  =========================*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "parser.h"
#include "symtab.h"
#include "y.tab.h"

#include "matrix.h"
#include "ml6.h"
#include "display.h"
#include "draw.h"
#include "stack.h"
#include "gmath.h"


/*======== void first_pass() ==========
  Inputs:
  Returns:

  Checks the op array for any animation commands
  (frames, basename, vary)

  Should set num_frames and basename if the frames
  or basename commands are present

  If vary is found, but frames is not, the entire
  program should exit.

  If frames is found, but basename is not, set name
  to some default value, and print out a message
  with the name being used.
  ====================*/

int num_frames;
char name[128];

void first_pass() {
  //in order to use name and num_frames throughout
  //they must be extern variables
  //int num_frames;
  num_frames=1;
  //char name[128];
  strcpy(name,"generic");
  //name=(char *)NULL;

  int varied=0;
  
  int i;
  for (i=0;i<lastop;i++) {

    if (op[i].opcode==FRAMES) {
      num_frames=op[i].op.frames.num_frames;
    } else if (op[i].opcode==BASENAME) {
      strcpy(name,op[i].op.basename.p->name);
    } else if (!varied && op[i].opcode==VARY) {
      varied=1;
    }
    
  }
  if (num_frames==1 && varied) {
    printf("Vary commands without specified total frames.\n");
    exit(0);
  }
  if (num_frames!=1 && !strcmp(name,"generic")) {
    
    printf("No basename found, set to [generic]\n");
  }
  printf("finished first_pass()\n");

}

/*======== struct vary_node ** second_pass() ==========
  Inputs:
  Returns: An array of vary_node linked lists

  In order to set the knobs for animation, we need to keep
  a seaprate value for each knob for each frame. We can do
  this by using an array of linked lists. Each array index
  will correspond to a frame (eg. knobs[0] would be the first
  frame, knobs[2] would be the 3rd frame and so on).

  Each index should contain a linked list of vary_nodes, each
  node contains a knob name, a value, and a pointer to the
  next node.

  Go through the opcode array, and when you find vary, go
  from knobs[0] to knobs[frames-1] and add (or modify) the
  vary_node corresponding to the given knob with the
  appropirate value.
  ====================*/
struct vary_node ** second_pass() {
  //struct vary_node * knobs[num_frames];//=(struct vary_node **) malloc(sizeof(struct vary_node *));//calloc please
  //printf("begin second pass\n");
  struct vary_node ** knobs=(struct vary_node **) calloc(sizeof(struct vary_node *), num_frames);
  int i=0;
  for (;i<lastop;i++) {

    if (op[i].opcode==VARY) {
      //printf("[second pass]: located VARY\n");
      //printf("start_frame: %f\nend_frame: %f\n",op[i].op.vary.start_frame, op[i].op.vary.end_frame);
      int curr_f;
      for (curr_f=0;curr_f<num_frames;curr_f++) {
	
	if (!knobs[curr_f]) {//if empty
	  //printf("[second_pass]: knobs[curr_f] found empty\n");
	  if (op[i].op.vary.start_frame<=op[i].op.vary.end_frame &&
	      op[i].op.vary.start_frame>=0 &&
	      op[i].op.vary.end_frame>0 &&
	      curr_f>=op[i].op.vary.start_frame &&
	      curr_f<=op[i].op.vary.end_frame) {
	    //printf("[second_pass]: valid condition\n");
	    //calculate value
	    double step=(op[i].op.vary.end_val-op[i].op.vary.start_val)/(op[i].op.vary.end_frame-op[i].op.vary.start_frame);
	    //printf("[second_pass]: step for calculating value [%s]: %f\n",op[i].op.vary.p->name,step);
	    //double mult=(curr_f-op[i].op.vary.start_frame)/(op[i].op.vary.end_frame-op[i].op.vary.start_frame);
	    
	    //printf("[second_pass]: multiplier for calculating value [%s]: %f\n",op[i].op.vary.p->name,mult);	
	    struct vary_node* set=(struct vary_node *)malloc(sizeof(struct vary_node));
	    strcpy(set->name,op[i].op.vary.p->name);
	    //set->value=step*mult*100;
	    set->value=step*(curr_f-op[i].op.vary.start_frame);
	    set->next=NULL;
	    //printf("[second_pass]: created set\n");
	    knobs[curr_f]=set;
	    //printf("[second_pass]: set knobs[curr_f]\n");
	    continue;
	  }
	
	}

	
	struct vary_node* check=knobs[curr_f];
	//loop through linked list for frame # curr_f
	while (check->next) {
	  //if name exists, update value
	  if (!strcmp(check->name, op[i].op.vary.p->name)) {
	    //check if frame is in the range && if valid range
	    if (op[i].op.vary.start_frame<=op[i].op.vary.end_frame &&
		op[i].op.vary.start_frame>=0 &&
		op[i].op.vary.end_frame>=0 &&
		curr_f>=op[i].op.vary.start_frame &&
		curr_f<=op[i].op.vary.end_frame) {

	      //calculate value
	      double step=(op[i].op.vary.end_val-op[i].op.vary.start_val)/(op[i].op.vary.end_frame-op[i].op.vary.start_frame);
	      //double mult=(curr_f-op[i].op.vary.start_frame)/(op[i].op.vary.end_frame-op[i].op.vary.start_frame);
	      check->value=step*(curr_f-op[i].op.vary.start_frame);
	      //check->value=step*mult*100;
	      break;//out of while loop
	    }

	  }
	  check=check->next;
	}//end while loop through linked list

	//if the knob did not already exist
	//printf("[second_pass]: knob [%s] did not exist\n", op[i].op.vary.p->name);
	if (op[i].op.vary.start_frame<=op[i].op.vary.end_frame &&
	    op[i].op.vary.start_frame>=0 &&
	    op[i].op.vary.end_frame>=0 &&
	    curr_f>=op[i].op.vary.start_frame &&
	    curr_f<=op[i].op.vary.end_frame) {

	  //calculate value
	  double step=(op[i].op.vary.end_val-op[i].op.vary.start_val)/(op[i].op.vary.end_frame-op[i].op.vary.start_frame);
	  //printf("[second_pass]: step for calculating value [%s]: %f\n",op[i].op.vary.p->name,step);
	  //double mult=(curr_f-op[i].op.vary.start_frame)/(op[i].op.vary.end_frame-op[i].op.vary.start_frame);
	  //printf("[second_pass]: multiplier for calculating value [%s]: %f\n",op[i].op.vary.p->name,mult);	
	  struct vary_node* set=(struct vary_node *)malloc(sizeof(struct vary_node));
	  strcpy(set->name,op[i].op.vary.p->name);
	  //set->value=step*mult*100;
	  set->value=op[i].op.vary.start_val+step*(curr_f-op[i].op.vary.start_frame);
	  set->next=NULL;
	  check->next=set;
	}
      }
    }
  }
  printf("finished second_pass\n");

  //print the whole thing
  int r=0;

  for (;r<num_frames;r++) {
    struct vary_node * temp=knobs[r];
    printf("frame %d\n",r);
    while (temp) {
      printf("knob [%s], value [%f]\n",temp->name, temp->value);
      temp=temp->next;
    }
  }
  
  return knobs;
}

/*======== void print_knobs() ==========
  Inputs:
  Returns:

  Goes through symtab and display all the knobs and their
  currnt values
  ====================*/
void print_knobs() {
  int i;

  printf( "ID\tNAME\t\tTYPE\t\tVALUE\n" );
  for ( i=0; i < lastsym; i++ ) {

    if ( symtab[i].type == SYM_VALUE ) {
      printf( "%d\t%s\t\t", i, symtab[i].name );

      printf( "SYM_VALUE\t");
      printf( "%6.2f\n", symtab[i].s.value);
    }
  }
}

/*======== void my_main() ==========
  Inputs:
  Returns:

  This is the main engine of the interpreter, it should
  handle most of the commadns in mdl.

  If frames is not present in the source (and therefore
  num_frames is 1, then process_knobs should be called.

  If frames is present, the enitre op array must be
  applied frames time. At the end of each frame iteration
  save the current screen to a file named the
  provided basename plus a numeric string such that the
  files will be listed in order, then clear the screen and
  reset any other data structures that need it.

  Important note: you cannot just name your files in
  regular sequence, like pic0, pic1, pic2, pic3... if that
  is done, then pic1, pic10, pic11... will come before pic2
  and so on. In order to keep things clear, add leading 0s
  to the numeric portion of the name. If you use sprintf,
  you can use "%0xd" for this purpose. It will add at most
  x 0s in front of a number, if needed, so if used correctly,
  and x = 4, you would get numbers like 0001, 0002, 0011,
  0487
  ====================*/
void my_main() {

  int i;
  struct matrix *tmp;
  struct stack *systems;
  screen t;
  zbuffer zb;
  color g;
  g.red = 0;
  g.green = 0;
  g.blue = 0;
  double step_3d = 20;
  double theta;
  double knob_value, xval, yval, zval;

  //Lighting values here for easy access
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

  systems = new_stack();
  tmp = new_matrix(4, 1000);
  clear_screen( t );
  clear_zbuffer(zb);

  //check if frames==1
  first_pass();
  printf("frames: %d\n",num_frames);
  if (num_frames>1) {
  
  struct vary_node** knobs=second_pass();
  int frame;
  for (frame=0;frame<num_frames;frame++) {

    //updates symbol table
    struct vary_node* temp=knobs[frame];
    while (temp) {
      if (!lookup_symbol(temp->name))
	add_symbol(temp->name, SYM_VALUE, &(temp->value));
      else
	set_value(lookup_symbol(temp->name),temp->value);
      temp=temp->next;
    }
    printf("\n[frame %d] printing knobs\n",frame);
    print_knobs();
    for (i=0;i<lastop;i++) {
      //printf("%d: ",i);
      switch (op[i].opcode)
	{
	case SPHERE:
	  /* printf("Sphere: %6.2f %6.2f %6.2f r=%6.2f", */
	  /* 	 op[i].op.sphere.d[0],op[i].op.sphere.d[1], */
	  /* 	 op[i].op.sphere.d[2], */
	  /* 	 op[i].op.sphere.r); */
	  if (op[i].op.sphere.constants != NULL)
	    {
	      //printf("\tconstants: %s",op[i].op.sphere.constants->name);
	    }
	  if (op[i].op.sphere.cs != NULL)
	    {
	      //printf("\tcs: %s",op[i].op.sphere.cs->name);
	    }
	  add_sphere(tmp, op[i].op.sphere.d[0],
		     op[i].op.sphere.d[1],
		     op[i].op.sphere.d[2],
		     op[i].op.sphere.r, step_3d);
	  matrix_mult( peek(systems), tmp );
	  draw_polygons(tmp, t, zb, view, light, ambient,
			areflect, dreflect, sreflect);
	  tmp->lastcol = 0;
	  break;
	case TORUS:
	  /* printf("Torus: %6.2f %6.2f %6.2f r0=%6.2f r1=%6.2f", */
	  /* 	 op[i].op.torus.d[0],op[i].op.torus.d[1], */
	  /* 	 op[i].op.torus.d[2], */
	  /* 	 op[i].op.torus.r0,op[i].op.torus.r1); */
	  if (op[i].op.torus.constants != NULL)
	    {
	      //printf("\tconstants: %s",op[i].op.torus.constants->name);
	    }
	  if (op[i].op.torus.cs != NULL)
	    {
	      //printf("\tcs: %s",op[i].op.torus.cs->name);
	    }
	  add_torus(tmp,
		    op[i].op.torus.d[0],
		    op[i].op.torus.d[1],
		    op[i].op.torus.d[2],
		    op[i].op.torus.r0,op[i].op.torus.r1, step_3d);
	  matrix_mult( peek(systems), tmp );
	  draw_polygons(tmp, t, zb, view, light, ambient,
			areflect, dreflect, sreflect);
	  tmp->lastcol = 0;
	  break;
	case BOX:
	  /* printf("Box: d0: %6.2f %6.2f %6.2f d1: %6.2f %6.2f %6.2f", */
	  /* 	 op[i].op.box.d0[0],op[i].op.box.d0[1], */
	  /* 	 op[i].op.box.d0[2], */
	  /* 	 op[i].op.box.d1[0],op[i].op.box.d1[1], */
	  /* 	 op[i].op.box.d1[2]); */
	  if (op[i].op.box.constants != NULL)
	    {
	      //printf("\tconstants: %s",op[i].op.box.constants->name);
	    }
	  if (op[i].op.box.cs != NULL)
	    {
	      //printf("\tcs: %s",op[i].op.box.cs->name);
	    }
	  add_box(tmp,
		  op[i].op.box.d0[0],op[i].op.box.d0[1],
		  op[i].op.box.d0[2],
		  op[i].op.box.d1[0],op[i].op.box.d1[1],
		  op[i].op.box.d1[2]);
	  matrix_mult( peek(systems), tmp );
	  draw_polygons(tmp, t, zb, view, light, ambient,
			areflect, dreflect, sreflect);
	  tmp->lastcol = 0;
	  break;
	case LINE:
	  /* printf("Line: from: %6.2f %6.2f %6.2f to: %6.2f %6.2f %6.2f",*/
	  /* 	 op[i].op.line.p0[0],op[i].op.line.p0[1], */
	  /* 	 op[i].op.line.p0[1], */
	  /* 	 op[i].op.line.p1[0],op[i].op.line.p1[1], */
	  /* 	 op[i].op.line.p1[1]); */
	  if (op[i].op.line.constants != NULL)
	    {
	      //printf("\n\tConstants: %s",op[i].op.line.constants->name);
	    }
	  if (op[i].op.line.cs0 != NULL)
	    {
	      //printf("\n\tCS0: %s",op[i].op.line.cs0->name);
	    }
	  if (op[i].op.line.cs1 != NULL)
	    {
	      //printf("\n\tCS1: %s",op[i].op.line.cs1->name);
	    }
	  add_edge(tmp,
		   op[i].op.line.p0[0],op[i].op.line.p0[1],
		   op[i].op.line.p0[2],
		   op[i].op.line.p1[0],op[i].op.line.p1[1],
		   op[i].op.line.p1[2]);
	  matrix_mult( peek(systems), tmp );
	  draw_lines(tmp, t, zb, g);
	  tmp->lastcol = 0;
	  break;
	case MOVE:
	  //multiply by lookup_symbol(op[i].op.move.p->name)->s.value
	  //need a way to check if their is a knob
	  xval = op[i].op.move.d[0];
	  yval = op[i].op.move.d[1];
	  zval = op[i].op.move.d[2];

	  printf("Move: %6.2f %6.2f %6.2f",
		 xval, yval, zval);
	  if (op[i].op.move.p != NULL)
	    {
	      printf("\tknob: %s",op[i].op.move.p->name);
	      xval*= lookup_symbol(op[i].op.move.p->name)->s.value;
	      yval*= lookup_symbol(op[i].op.move.p->name)->s.value;
	      zval*= lookup_symbol(op[i].op.move.p->name)->s.value;
	    }
	  printf("Move post-knob: %6.2f %6.2f %6.2f",
		 xval, yval, zval);
	  tmp = make_translate( xval, yval, zval );
	  matrix_mult(peek(systems), tmp);
	  copy_matrix(tmp, peek(systems));
	  tmp->lastcol = 0;
	  break;
	case SCALE:
	  xval = op[i].op.scale.d[0];
	  yval = op[i].op.scale.d[1];
	  zval = op[i].op.scale.d[2];
	  printf("Scale: %6.2f %6.2f %6.2f",
		 xval, yval, zval);
	  
	  if (op[i].op.scale.p != NULL)
	    {
	      printf("\tknob: %s",op[i].op.scale.p->name);
	      xval*= lookup_symbol(op[i].op.scale.p->name)->s.value;
	      yval*= lookup_symbol(op[i].op.scale.p->name)->s.value;
	      zval*= lookup_symbol(op[i].op.scale.p->name)->s.value;
	    }
	  printf("Scale post-knob: %6.2f %6.2f %6.2f",
		 xval, yval, zval);
	  tmp = make_scale( xval, yval, zval );
	  matrix_mult(peek(systems), tmp);
	  copy_matrix(tmp, peek(systems));
	  tmp->lastcol = 0;
	  break;
	case ROTATE:
	  xval = op[i].op.rotate.axis;
	  theta = op[i].op.rotate.degrees;

	  printf("Rotate: axis: %6.2f degrees: %6.2f",
		 xval, theta);
	  if (op[i].op.rotate.p != NULL)
	    {
	      printf("\tknob: %s",op[i].op.rotate.p->name);
	      theta*= lookup_symbol(op[i].op.rotate.p->name)->s.value;
	    }
	  printf("Rotate post-knob: axis: %6.2f degrees: %6.2f",
		 xval, theta);
	  theta*= (M_PI / 180);
	  if (op[i].op.rotate.axis == 0 )
	    tmp = make_rotX( theta );
	  else if (op[i].op.rotate.axis == 1 )
	    tmp = make_rotY( theta );
	  else
	    tmp = make_rotZ( theta );

	  matrix_mult(peek(systems), tmp);
	  copy_matrix(tmp, peek(systems));
	  tmp->lastcol = 0;
	  break;
	case PUSH:
	  //printf("Push");
	  push(systems);
	  break;
	case POP:
	  //printf("Pop");
	  pop(systems);
	  break;
	case SAVE:
	  //printf("Save: %s",op[i].op.save.p->name);
	  save_extension(t, op[i].op.save.p->name);
	  break;
	case DISPLAY:
	  //printf("Display");
	  display(t);
	  break;
	} //end opcode switch
      printf("\n");
    }//end operation loop
    //save screen t image as .png to directory [anim/]
    char fname[12+strlen(name)];//8=strlen("anim/")+3 for numbers+strlen(".png")

    sprintf(fname,"anim/%s%03d.png",name,frame);
    //display(t);
    
    save_extension(t,fname);
    clear_screen(t);
    clear_zbuffer(zb);
    systems=new_stack();
    tmp=new_matrix(4,1000);
  }//end for loop for all frames
  make_animation(name);
  }
  else {
  for (i=0;i<lastop;i++) {
    //printf("%d: ",i);
    switch (op[i].opcode)
      {
      case SPHERE:
        /* printf("Sphere: %6.2f %6.2f %6.2f r=%6.2f", */
        /* 	 op[i].op.sphere.d[0],op[i].op.sphere.d[1], */
        /* 	 op[i].op.sphere.d[2], */
        /* 	 op[i].op.sphere.r); */
        if (op[i].op.sphere.constants != NULL)
          {
            //printf("\tconstants: %s",op[i].op.sphere.constants->name);
          }
        if (op[i].op.sphere.cs != NULL)
          {
            //printf("\tcs: %s",op[i].op.sphere.cs->name);
          }
        add_sphere(tmp, op[i].op.sphere.d[0],
                   op[i].op.sphere.d[1],
                   op[i].op.sphere.d[2],
                   op[i].op.sphere.r, step_3d);
        matrix_mult( peek(systems), tmp );
        draw_polygons(tmp, t, zb, view, light, ambient,
                      areflect, dreflect, sreflect);
        tmp->lastcol = 0;
        break;
      case TORUS:
        /* printf("Torus: %6.2f %6.2f %6.2f r0=%6.2f r1=%6.2f", */
        /* 	 op[i].op.torus.d[0],op[i].op.torus.d[1], */
        /* 	 op[i].op.torus.d[2], */
        /* 	 op[i].op.torus.r0,op[i].op.torus.r1); */
        if (op[i].op.torus.constants != NULL)
          {
            //printf("\tconstants: %s",op[i].op.torus.constants->name);
          }
        if (op[i].op.torus.cs != NULL)
          {
            //printf("\tcs: %s",op[i].op.torus.cs->name);
          }
        add_torus(tmp,
                  op[i].op.torus.d[0],
                  op[i].op.torus.d[1],
                  op[i].op.torus.d[2],
                  op[i].op.torus.r0,op[i].op.torus.r1, step_3d);
        matrix_mult( peek(systems), tmp );
        draw_polygons(tmp, t, zb, view, light, ambient,
                      areflect, dreflect, sreflect);
        tmp->lastcol = 0;
        break;
      case BOX:
        /* printf("Box: d0: %6.2f %6.2f %6.2f d1: %6.2f %6.2f %6.2f", */
        /* 	 op[i].op.box.d0[0],op[i].op.box.d0[1], */
        /* 	 op[i].op.box.d0[2], */
        /* 	 op[i].op.box.d1[0],op[i].op.box.d1[1], */
        /* 	 op[i].op.box.d1[2]); */
        if (op[i].op.box.constants != NULL)
          {
            //printf("\tconstants: %s",op[i].op.box.constants->name);
          }
        if (op[i].op.box.cs != NULL)
          {
            //printf("\tcs: %s",op[i].op.box.cs->name);
          }
        add_box(tmp,
                op[i].op.box.d0[0],op[i].op.box.d0[1],
                op[i].op.box.d0[2],
                op[i].op.box.d1[0],op[i].op.box.d1[1],
                op[i].op.box.d1[2]);
        matrix_mult( peek(systems), tmp );
        draw_polygons(tmp, t, zb, view, light, ambient,
                      areflect, dreflect, sreflect);
        tmp->lastcol = 0;
        break;
      case LINE:
        /* printf("Line: from: %6.2f %6.2f %6.2f to: %6.2f %6.2f %6.2f",*/
        /* 	 op[i].op.line.p0[0],op[i].op.line.p0[1], */
        /* 	 op[i].op.line.p0[1], */
        /* 	 op[i].op.line.p1[0],op[i].op.line.p1[1], */
        /* 	 op[i].op.line.p1[1]); */
        if (op[i].op.line.constants != NULL)
          {
            //printf("\n\tConstants: %s",op[i].op.line.constants->name);
          }
        if (op[i].op.line.cs0 != NULL)
          {
            //printf("\n\tCS0: %s",op[i].op.line.cs0->name);
          }
        if (op[i].op.line.cs1 != NULL)
          {
            //printf("\n\tCS1: %s",op[i].op.line.cs1->name);
          }
        add_edge(tmp,
                 op[i].op.line.p0[0],op[i].op.line.p0[1],
                 op[i].op.line.p0[2],
                 op[i].op.line.p1[0],op[i].op.line.p1[1],
                 op[i].op.line.p1[2]);
        matrix_mult( peek(systems), tmp );
        draw_lines(tmp, t, zb, g);
        tmp->lastcol = 0;
        break;
      case MOVE:
        xval = op[i].op.move.d[0];
        yval = op[i].op.move.d[1];
        zval = op[i].op.move.d[2];
        printf("Move: %6.2f %6.2f %6.2f",
               xval, yval, zval);
        if (op[i].op.move.p != NULL)
          {
            printf("\tknob: %s",op[i].op.move.p->name);
          }
        tmp = make_translate( xval, yval, zval );
        matrix_mult(peek(systems), tmp);
        copy_matrix(tmp, peek(systems));
        tmp->lastcol = 0;
        break;
      case SCALE:
        xval = op[i].op.scale.d[0];
        yval = op[i].op.scale.d[1];
        zval = op[i].op.scale.d[2];
        printf("Scale: %6.2f %6.2f %6.2f",
               xval, yval, zval);
        if (op[i].op.scale.p != NULL)
          {
            printf("\tknob: %s",op[i].op.scale.p->name);
          }
        tmp = make_scale( xval, yval, zval );
        matrix_mult(peek(systems), tmp);
        copy_matrix(tmp, peek(systems));
        tmp->lastcol = 0;
        break;
      case ROTATE:
        xval = op[i].op.rotate.axis;
        theta = op[i].op.rotate.degrees;
        printf("Rotate: axis: %6.2f degrees: %6.2f",
               xval, theta);
        if (op[i].op.rotate.p != NULL)
          {
            printf("\tknob: %s",op[i].op.rotate.p->name);
          }
        theta*= (M_PI / 180);
        if (op[i].op.rotate.axis == 0 )
          tmp = make_rotX( theta );
        else if (op[i].op.rotate.axis == 1 )
          tmp = make_rotY( theta );
        else
          tmp = make_rotZ( theta );

        matrix_mult(peek(systems), tmp);
        copy_matrix(tmp, peek(systems));
        tmp->lastcol = 0;
        break;
      case PUSH:
        //printf("Push");
        push(systems);
        break;
      case POP:
        //printf("Pop");
        pop(systems);
        break;
      case SAVE:
        //printf("Save: %s",op[i].op.save.p->name);
        save_extension(t, op[i].op.save.p->name);
        break;
      case DISPLAY:
        //printf("Display");
        display(t);
        break;
      } //end opcode switch
    printf("\n");
  }//end operation loop
  }
}	
	
