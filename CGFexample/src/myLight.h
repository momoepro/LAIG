//
//  myLight.h
//  CGFExample
//
//  Created by Cláudio Monteiro on 12/10/14.
//  Copyright (c) 2014 me. All rights reserved.
//

#ifndef _myLight_h_
#define _myLight_h_

#include <stdio.h>
#include "CGFapplication.h"

class myLight{
public:
    myLight(unsigned int id, float *pos, char *n, bool e, bool m);
    myLight(unsigned int id, float *pos, float* dir, char *n, bool e, bool m);
	unsigned int getID();
    char *getName();
	float *getPosition();
	float *getDirection();
	void setAmbient(float *amb);
	void setDiffuse(float *dif);
	void setSpecular(float *spec);
	void setAngle(float a);
	float *getAmbient();
	float *getDiffuse();
	float *getSpecular();
	float getAngle();
    bool isEnabled();
    bool isMarked();
    ~myLight();
    
private:
    char *name;
	float position[3], direction[3], ambient[4], diffuse[4], specular[4], angle;
    bool enabled, marked;
	unsigned int id;
};

#endif /* defined(__CGFExample__myLight__) */
