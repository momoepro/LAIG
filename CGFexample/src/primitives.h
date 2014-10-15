#ifndef _primitives_h_
#define _primitives_h_

#include <stdio.h>
#include "CGFapplication.h"

class Rectangle{
public:
	Rectangle();
	Rectangle(float xi, float yi, float xf, float yf);
	float getFirstX();
	float getFirstY();
	float getLastX();
	float getLastY();
	~Rectangle();
private:
	float x1,x2,y1,y2;
};

class Triangle{
public:
	Triangle();
	Triangle(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3);
	float getFirstX();
	float getFirstY();
	float getFirstZ();
	float getMiddleX();
	float getMiddleY();
	float getMiddleZ();
	float getLastX();
	float getLastY();
	float getLastZ();
	~Triangle();
private:
	float p1x, p1y, p1z, p2x, p2y, p2z, p3x, p3y, p3z;
};

class Cylinder{
public:
	Cylinder();
	Cylinder(float b, float t, float h, int sl, int st);
	float getBase();
	float getTop();
	float getHeight();
	int getSlices();
	int getStacks();
	~Cylinder();
private:
	float base, top, height;
	int slices, stacks;
};

class Sphere{
public:
	Sphere();
	Sphere(float r, int sl, int st);
	float getRadius();
	int getSlices();
	int getStacks();
	~Sphere();
private:
	float radius;
	int slices, stacks;
};

class Torus{
public:
	Torus();
    Torus(float i, float o, int sl, int l);
	float getInner();
	float getOutter();
	int getSlices();
	int getLoops();
    ~Torus();
private:
	float inner, outter;
	int slices, loops;
};

class Primitive{
public:
    Primitive(Rectangle r);
	Primitive(Triangle t);
	Primitive(Cylinder c);
	Primitive(Sphere s);
	Primitive(Torus t);
	Rectangle getRectangle();
	Triangle getTriangle();
	Cylinder getCylinder();
	Sphere getSphere();
	Torus getTorus();
	char *getType();
    ~Primitive();
private:
	Rectangle rectangle;
	Triangle triangle;
	Cylinder cylinder;
	Sphere sphere;
	Torus torus;
	char *type;
};

#endif