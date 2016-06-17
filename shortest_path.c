/* 
---------------------------------------------------------------------------------------------------------
Solution to the 1st Project of Fundamental Algorithms I0600
City College of New York, CUNY 

AUTHOR : ANKAN KUMAR GIRI

Email : agiri000@citymail.cuny.edu / ankangiri@gmail.com
---------------------------------------------------------------------------------------------------------

INSTRUCTIONS:
=============

compiles with command line "gcc shortest_path.c -lX11 -lm -L/usr/X11R6/lib -o shortest_path.out"
run using "./shortestpath.out inputfilename.extension" as command line argument.
inputfilename.extension is a file containing triangle shaped obstacles in the following format :
T (x1,y1) (x2,y2) (x3,y3)
T (x4,y4) (x5,y5) (x6,y6)
and so on................
Each point in a triangle should be within the minimum and maximum range to be displayed full as 
a triangle.
Minimum x value : 0
Minimum y value : 0
Maximum x value : Your Max Screen Widthness - 300
Maximum y value : Your Max Screen Height - 200
User can left mouse click to select the start point and the target point and click the right 
mouse button to exit.
Note : No points can be selected inside a triangle or a region bounded by three triangles or 
outside the bounding rectangular box.
------------------------------------------------------------------------------------------------------------
*/

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>

Display *display_ptr;
Screen *screen_ptr;
int screen_num;
char *display_name = NULL;
unsigned int display_width, display_height;

Window win;
int border_width;
unsigned int win_width, win_height;
int win_x, win_y;

XWMHints *wm_hints;
XClassHint *class_hints;
XSizeHints *size_hints;
XTextProperty win_name, icon_name;
char *win_name_string = "Example Window";
char *icon_name_string = "Icon for Example Window";

XEvent report;

GC gc_red, gc_black, gc_blue, gc_white;
unsigned long valuemask = 0;
XGCValues gc_red_values, gc_black_values, gc_blue_values, gc_white_values;
Colormap color_map;
XColor tmp_color1, tmp_color2;

struct Triangle
{
   int x1;
   int y1;
   int x2;
   int y2;
   int x3;
   int y3;
   int point_1_inside_triangle;
   int point_2_inside_triangle;
   int point_3_inside_triangle;
} t[100];
struct Pixel
{
  int name;
  int x;
  int y;
} pixel[302];
int  pixel_count;
int  triangle_count;
int  cost[302][302];
int  x_center,y_center;
int  x_gap,x_max,x_min;
int  y_gap,y_max,y_min;
int  x_start,y_start,x_end,y_end;
int  rec_width,rec_height;

int get_x_min(int x1,int x2,int x3)
{
  return (x1 > x2 ? (x2 > x3 ? x3 : x2) : (x1 > x3 ? x3 : x1)) ;        
}

int get_y_min(int y1,int y2,int y3)
{
  return (y1 > y2 ? (y2 > y3 ? y3 : y2) : (y1 > y3 ? y3 : y1)) ;
}

int get_x_max(int x1,int x2,int x3)
{
  return (x1 < x2 ? (x2 < x3 ? x3 : x2) : (x1 < x3 ? x3 : x1)) ;
}

int get_y_max(int y1,int y2,int y3)
{
  return (y1 < y2 ? (y2 < y3 ? y3 : y2) : (y1 < y3 ? y3 : y1)) ;
}

int64_t orientation(int a_x, int a_y, int b_x, int b_y, int c_x, int c_y)
{
  int64_t ori = (a_x * b_y) + (b_x * c_y) + (c_x * a_y) - (a_y * b_x) - (b_y * c_x) - (c_y * a_x);
  return ori;
}

int point_in_triangle(int p_x, int p_y, int a_x, int a_y, int b_x, int b_y, int c_x, int c_y)
{
  int64_t ori_1,ori_2,ori_3,ori_4,ori_5,ori_6,ori_7,ori_8,ori_9;
  ori_1 = orientation(p_x,p_y,a_x,a_y,b_x,b_y);
  ori_2 = orientation(c_x,c_y,a_x,a_y,b_x,b_y);
  ori_3 = orientation(p_x,p_y,b_x,b_y,c_x,c_y);
  ori_4 = orientation(a_x,a_y,b_x,b_y,c_x,c_y);
  ori_5 = orientation(p_x,p_y,a_x,a_y,c_x,c_y);
  ori_6 = orientation(b_x,b_y,a_x,a_y,c_x,c_y);
  ori_7 = ori_1*ori_2;
  ori_8 = ori_3*ori_4;
  ori_9 = ori_5*ori_6;
  if(ori_7 > 0 && ori_8 > 0 && ori_9 > 0)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

void expose()
{
  XClearWindow(display_ptr,win);
  int i;
  for(i=0;i<triangle_count;i++)
  {
    XDrawLine(display_ptr, win, gc_black, t[i].x1, t[i].y1, t[i].x2, t[i].y2);
    XDrawLine(display_ptr, win, gc_black, t[i].x2, t[i].y2, t[i].x3, t[i].y3);
    XDrawLine(display_ptr, win, gc_black, t[i].x3, t[i].y3, t[i].x1, t[i].y1);
  }
  XDrawRectangle(display_ptr, win, gc_black, x_start, y_start, rec_width, rec_height ); 
}

int eculidian_dist(int x1, int y1, int x2, int y2)
{
  int a,b;
  a = pow(x1-x2,2);
  b = pow(y1-y2,2);
  return sqrt(a+b);
}

int intersects(int p_x, int p_y, int q_x, int q_y, int r_x, int r_y, int s_x, int s_y)
{
  int64_t ori_1,ori_2,ori_3,ori_4,ori_5,ori_6;
  int intersect = 0;
  ori_1 = orientation(p_x,p_y,q_x,q_y,r_x,r_y);
  ori_2 = orientation(p_x,p_y,q_x,q_y,s_x,s_y);
  ori_3 = orientation(r_x,r_y,s_x,s_y,p_x,p_y);
  ori_4 = orientation(r_x,r_y,s_x,s_y,q_x,q_y);
  ori_5 = ori_1*ori_2;
  ori_6 = ori_3*ori_4;
  if( ori_5 < 0 && ori_6 < 0)
  {
    intersect = 1; 
  }
  return intersect;
}

int can_see(int x1, int y1, int x2, int y2)
{
  int i=0;
  int flag = 1;
  for(i=0;i<triangle_count;i++)
  {
    if( intersects(x1,y1,x2,y2,t[i].x1,t[i].y1,t[i].x2,t[i].y2) == 1 )
    {
      flag = 0;
      break;
    }
    if( intersects(x1,y1,x2,y2,t[i].x2,t[i].y2,t[i].x3,t[i].y3) == 1) 
    {
      flag = 0;
      break;
    }
    if( intersects(x1,y1,x2,y2,t[i].x3,t[i].y3,t[i].x1,t[i].y1) == 1)
    {
      flag = 0;
      break;
    }
  }
  return flag;
}

void clear_cost()
{
  int i,j;
  for(i=0;i<302;i++)
  {
    for(j=0;j<302;j++)
      cost[i][j]=9999;
  }
}

int get_x(int n)
{
  int i;
  for(i=0;i<pixel_count;i++)
  {
    if(pixel[i].name == n)
      return pixel[i].x;
  }
}

int get_y(int n)
{
  int i;
  for(i=0;i<pixel_count;i++)
  {
    if(pixel[i].name == n)
      return pixel[i].y;
  }
}

void dijsktra(int source,int target)
{
    int dist[302], prev[302], finished[302], path[302];
    int next, x1, y1, x2, y2, min, start, i, d;
    int no_of_path = 0;
    for(i=0;i<pixel_count;i++)
    {
        dist[i] = 9999;
        prev[i] = -1;
        finished[i]=0;
    }
    start = source;
    finished[start]=1;
    dist[start] = 0;
    while(finished[target] ==0)
    {
      min = 9999;
      next = 0;
      for(i=0;i < pixel_count;i++)
      {
          d = dist[start] + cost[start][i];
          if(d < dist[i] && finished[i]==0)
           {
              dist[i] = d;
              prev[i] = start;
          }
          if(min > dist[i] && finished[i]==0)
          {
              min = dist[i];
              next = i;
          }
      }
      start = next;
      finished[start] = 1;
    }
    start = target;
    while(start != -1)
    {
        path[no_of_path] = start;    
        no_of_path++;    
        start = prev[start];
    }
    printf("Distance : %d\n" , dist[target]);
    for(i=0;i<no_of_path-1;i++)
    {
      x1 = get_x(path[i]);
      y1 = get_y(path[i]);
      x2 = get_x(path[i+1]);
      y2 = get_y(path[i+1]);
      XDrawLine(display_ptr, win, gc_white, x1, y1, x2, y2);
      XDrawLine(display_ptr, win, gc_blue, x1, y1, x2, y2);
    }
    for(i=1;i<no_of_path-1;i++)
    {
      x1 = get_x(path[i]);
      y1 = get_y(path[i]);
      XFillArc( display_ptr, win, gc_red, x1- win_height/200, y1 - win_height/200, win_height/100, win_height/100, 0, 360*64);
    }
}

int main(int argc, char **argv)
{
  FILE *fp;
  int x_1,y_1,x_2,y_2,x_3,y_3;
  char a;
  int i,j;
  int outside;
  int ButtonPressed = 0;
  int temp_x, temp_y;
  pixel_count = 0;
  for(i=0;i<302;i++)
  {              
    for(j=0;j<302;j++)  
    {          
      cost[i][j] = 9999; 
    } 
  }

  if( (display_ptr = XOpenDisplay(display_name)) == NULL )
    { printf("Could not open display. \n"); exit(-1);}
  printf("Connected to X server  %s\n", XDisplayName(display_name) );
  screen_num = DefaultScreen( display_ptr );
  screen_ptr = DefaultScreenOfDisplay( display_ptr );
  color_map  = XDefaultColormap( display_ptr, screen_num );
  display_width  = DisplayWidth( display_ptr, screen_num );
  display_height = DisplayHeight( display_ptr, screen_num );

  printf("Width %d, Height %d, Screen Number %d\n", 
           display_width, display_height, screen_num);
  border_width = 10;
  win_x = 0; win_y = 0;
  win_width = display_width;
  win_height = display_height; 
  
  win= XCreateSimpleWindow( display_ptr, RootWindow( display_ptr, screen_num),
                            win_x, win_y, win_width, win_height, border_width,
                            BlackPixel(display_ptr, screen_num),
                            WhitePixel(display_ptr, screen_num) );
  size_hints = XAllocSizeHints();
  wm_hints = XAllocWMHints();
  class_hints = XAllocClassHint();
  if( size_hints == NULL || wm_hints == NULL || class_hints == NULL )
    { printf("Error allocating memory for hints. \n"); exit(-1);}

  size_hints -> flags = PPosition | PSize | PMinSize  ;
  size_hints -> min_width = 60;
  size_hints -> min_height = 60;

  XStringListToTextProperty( &win_name_string,1,&win_name);
  XStringListToTextProperty( &icon_name_string,1,&icon_name);
  
  wm_hints -> flags = StateHint | InputHint ;
  wm_hints -> initial_state = NormalState;
  wm_hints -> input = False;

  class_hints -> res_name = "x_use_example";
  class_hints -> res_class = "examples";

  XSetWMProperties( display_ptr, win, &win_name, &icon_name, argv, argc,
                    size_hints, wm_hints, class_hints );

  XSelectInput( display_ptr, win, 
            ExposureMask | StructureNotifyMask | ButtonPressMask );
  
  XMapWindow( display_ptr, win );

  XFlush(display_ptr);
  gc_red = XCreateGC( display_ptr, win, valuemask, &gc_red_values);
  XSetLineAttributes( display_ptr, gc_red, 3, LineSolid, CapRound, JoinRound);
  if( XAllocNamedColor( display_ptr, color_map, "red", 
      &tmp_color1, &tmp_color2 ) == 0 )
    {printf("failed to get color red\n"); exit(-1);} 
  else
    XSetForeground( display_ptr, gc_red, tmp_color1.pixel );
  gc_black = XCreateGC( display_ptr, win, valuemask, &gc_black_values);
  XSetLineAttributes(display_ptr, gc_black, 3, LineSolid,CapRound, JoinRound);
  if( XAllocNamedColor( display_ptr, color_map, "black", 
      &tmp_color1, &tmp_color2 ) == 0 )
    {printf("failed to get color black\n"); exit(-1);} 
  else
    XSetForeground( display_ptr, gc_black, tmp_color1.pixel );
  gc_blue = XCreateGC( display_ptr, win, valuemask, &gc_blue_values);
  XSetLineAttributes(display_ptr, gc_blue, 3, LineSolid,CapRound, JoinRound);
  if( XAllocNamedColor( display_ptr, color_map, "blue", 
      &tmp_color1, &tmp_color2 ) == 0 )
    {printf("failed to get blue yellow\n"); exit(-1);} 
  else
    XSetForeground( display_ptr, gc_blue, tmp_color1.pixel );
  gc_white = XCreateGC( display_ptr, win, valuemask, &gc_white_values);
  XSetLineAttributes(display_ptr, gc_white, 3, LineSolid,CapRound, JoinRound);
  if( XAllocNamedColor( display_ptr, color_map, "white", 
      &tmp_color1, &tmp_color2 ) == 0 )
    {printf("failed to get color white\n"); exit(-1);} 
  else
    XSetForeground( display_ptr, gc_white, tmp_color1.pixel );

  if ( argc != 2 ) 
  {
      printf( "Usage: %s filename.extension \n", argv[0] );
      exit(-1);
  }
  fp = fopen(argv[1], "r");
  if (fp == NULL) 
  {
    fprintf(stderr, "Can't open file %s!\n", argv[1]);
    printf( "Usage: %s filename.extension or Check whether file is present or not\n", argv[0] );
    exit(-1);
  } 
  triangle_count = 0;
  while (fscanf(fp, "%c %c%d%c%d%c %c%d%c%d%c %c%d%c%d%c%c", &a,&a,&x_1,&a,&y_1,&a,&a,&x_2,&a,&y_2,&a,&a,&x_3,&a,&y_3,&a,&a) != EOF) 
  {
    if(get_x_min(x_1,x_2,x_3) >= 0 && get_y_min(y_1,y_2,y_3) >= 0 && get_x_max(x_1,x_2,x_3) <= display_width - 300 && get_y_max(y_1,y_2,y_3) <= display_height - 200)
    {
      t[triangle_count].x1 = x_1;
      t[triangle_count].x2 = x_2;
      t[triangle_count].x3 = x_3;
      t[triangle_count].y1 = y_1;
      t[triangle_count].y2 = y_2;
      t[triangle_count].y3 = y_3;
      triangle_count++;
    }   
  }
  
  x_min = t[0].x1;
  x_max = x_min;
  y_min = t[0].y1;
  y_max = y_min;
  for(i=0;i<triangle_count;i++)
  {
    if(x_min > t[i].x1)
      x_min = t[i].x1;
    if(x_min > t[i].x2)
      x_min = t[i].x2;
    if(x_min > t[i].x2)
      x_min = t[i].x3;
    if(y_min > t[i].y1)
      y_min = t[i].y1;
    if(y_min > t[i].y2)
      y_min = t[i].y2;
    if(y_min > t[i].y3)
      y_min = t[i].y3;
    if(x_max < t[i].x1)
      x_max = t[i].x1;
    if(x_max < t[i].x2)
      x_max = t[i].x2;
    if(x_max < t[i].x3)
      x_max = t[i].x3;
    if(y_max < t[i].y1)
      y_max = t[i].y1;
    if(y_max < t[i].y2)
      y_max = t[i].y2;
    if(y_max < t[i].y3)
      y_max = t[i].y3;        
  }
   
  x_center = win_width / 2;
  y_center = win_height / 2;
  x_gap = x_max - x_min;
  y_gap = y_max - y_min;
  x_start = x_center - x_gap/2 - 50;
  y_start = y_center - y_gap/2 - 50;
  x_end = x_start + x_gap + 100;
  y_end = y_start + y_gap + 100;
  rec_width = x_end - x_start;
  rec_height = y_end - y_start;

  for(i=0;i<triangle_count;i++)
  {
    t[i].x1 = t[i].x1 + x_start + 50 - x_min;
    t[i].x2 = t[i].x2 + x_start + 50 - x_min;
    t[i].x3 = t[i].x3 + x_start + 50 - x_min;
    t[i].y1 = t[i].y1 + y_start + 50 - y_min;
    t[i].y2 = t[i].y2 + y_start + 50 - y_min;
    t[i].y3 = t[i].y3 + y_start + 50 - y_min;
  }
  for(i=0;i<triangle_count;i++)
  {
    t[i].point_1_inside_triangle = 0;
    t[i].point_2_inside_triangle = 0;
    t[i].point_3_inside_triangle = 0;       
  }
  for(i=0;i<triangle_count;i++)
  {
    for(j=0;j<triangle_count;j++)
    {
      if(j!=i)
      {
        if(point_in_triangle(t[i].x1,t[i].y1,t[j].x1,t[j].y1,t[j].x2,t[j].y2,t[j].x3,t[j].y3)==1)
        {
          t[i].point_1_inside_triangle = 1;
        }
        if(point_in_triangle(t[i].x2,t[i].y2,t[j].x1,t[j].y1,t[j].x2,t[j].y2,t[j].x3,t[j].y3)==1)
        {
          t[i].point_2_inside_triangle = 1;
        }
        if(point_in_triangle(t[i].x3,t[i].y3,t[j].x1,t[j].y1,t[j].x2,t[j].y2,t[j].x3,t[j].y3)==1)
        {
          t[i].point_3_inside_triangle = 1;
        }
      }
    }
  }
  
  while(1)
    { XNextEvent( display_ptr, &report );
      switch( report.type )
	{
	case Expose:
	        for(i=0;i<triangle_count;i++)
          {
            XDrawLine(display_ptr, win, gc_black, t[i].x1, t[i].y1, t[i].x2, t[i].y2);
            XDrawLine(display_ptr, win, gc_black, t[i].x2, t[i].y2, t[i].x3, t[i].y3);
            XDrawLine(display_ptr, win, gc_black, t[i].x3, t[i].y3, t[i].x1, t[i].y1);
          }
          XDrawRectangle(display_ptr, win, gc_black, x_start, y_start, rec_width, rec_height ); 

          break;
        case ConfigureNotify:
          win_width = report.xconfigure.width;
          win_height = report.xconfigure.height;
          break;
        case ButtonPress:
          {  
            int x, y;
  	        x = report.xbutton.x;
            y = report.xbutton.y;
            if (report.xbutton.button == Button1 )
	          {
              outside = 0;
              if(x <= x_start || y <= y_start || x >= x_end || y >= y_end)
              {
                outside = 1;
              } 
              for(i=0;i<triangle_count;i++)
              {
                if(point_in_triangle(x,y,t[i].x1,t[i].y1,t[i].x2,t[i].y2,t[i].x3,t[i].y3)==1)
                {
                  outside = 1;
                  break;
                }
              }
              if(outside == 0)
              {
                ButtonPressed++;
                if(ButtonPressed == 1)
                { 
                  temp_x = x;
                  temp_y = y;
                  pixel_count = 0;
                  XFillArc( display_ptr, win, gc_red, x - win_height/150, y - win_height/150, win_height/100, win_height/100, 0, 360*64);
                  pixel[pixel_count].name = pixel_count;
                  pixel[pixel_count].x = x; 
                  pixel[pixel_count].y = y;
                  pixel_count++;
                }
                else if(ButtonPressed == 2)
                { 
                  if(temp_x == x && temp_y == y)
                  {
                    ButtonPressed = 1;
                    break;
                  }
                  XFillArc( display_ptr, win, gc_red, x - win_height/150, y - win_height/150, win_height/100, win_height/100, 0, 360*64);
                  for(i=0;i<triangle_count;i++)
                  {
                      if(t[i].point_1_inside_triangle != 1)
                      {
                        pixel[pixel_count].name = pixel_count;
                        pixel[pixel_count].x = t[i].x1; 
                        pixel[pixel_count].y = t[i].y1;
                        pixel_count++;
                      }
                      if(t[i].point_2_inside_triangle != 1)
                      {
                        pixel[pixel_count].name = pixel_count;
                        pixel[pixel_count].x = t[i].x2; 
                        pixel[pixel_count].y = t[i].y2;
                        pixel_count++;
                      }
                      if(t[i].point_3_inside_triangle != 1)
                      {
                        pixel[pixel_count].name = pixel_count;
                        pixel[pixel_count].x = t[i].x3; 
                        pixel[pixel_count].y = t[i].y3;
                        pixel_count++;
                      }
                  }
                  pixel[pixel_count].name = pixel_count;
                  pixel[pixel_count].x = x; 
                  pixel[pixel_count].y = y;
                  pixel_count++;
                  for(i=0;i<pixel_count-1;i++)
                  {         
                    for(j=i+1;j<pixel_count;j++)
                    {
                      if(can_see(pixel[i].x,pixel[i].y,pixel[j].x,pixel[j].y)==1)
                      {
                        cost[pixel[i].name][pixel[j].name] = cost[pixel[j].name][pixel[i].name] = eculidian_dist(pixel[i].x,pixel[i].y,pixel[j].x,pixel[j].y);
                      }
                    }
                  }
                  dijsktra(pixel[0].name, pixel[pixel_count-1].name);
                }
                else
                {
                  clear_cost();
                  expose();
                  ButtonPressed = 0;          
                }
              }
            }
            else
            {
              XFlush(display_ptr);
              XCloseDisplay(display_ptr);
              exit(0);
            }

          }
          break;
        default:
          break;
	    }

  }
  exit(0);
}