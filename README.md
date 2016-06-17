# Shortest-Path XLIB Project

The aim of this project is to create a program that takes a set of triangle obstacles as
input, as well as a start and target point, and shows the shortest path between them. 

It provides a visual feedback by drawing the points, obstacles, and the path on the screen
using the xlib interface.

####The program takes one command-line argument, a file name.

The following lines give the set of triangle obstacles in the format:

>T (20,100) (55,63) (30,50)

The point coordinates are integers and can be easily parsed using scanf. There
are less than 1000 obstacles.

* As first stage, the input is read, a bounding box is found (10% added space around
the boundary is given), window is opened, the obstacles are displayed in a it. 

* Two left mouseclick events are fetched, which give the start and target point of the shortest path. 

* A graph is constructed; the vertices are the two points given by the mouseclicks, and all
vertices of obstacles. 

Any pair (p, q) of these points forms an edge of the graph if the line segment pq is not intersected by any of the obstacles; 
if (p, q) is an edge, then its length is the euclidean distance of p and q. 

A shortest-path algorithm is run on this graph, and resulting path is displayed on the screen. 

* This is repeated as long as the user inputs point pairs with left mouseclicks.
* When a right mouseclick is received program is terminated.
* The triangles (and path) are redrawn whenever an ‘expose’ event is fetched.
