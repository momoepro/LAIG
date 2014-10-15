//
//  SceneGraph.h
//  CGFExample
//
//  Created by Cláudio Monteiro on 12/10/14.
//  Copyright (c) 2014 me. All rights reserved.
//

#ifndef _SceneGraph_h_
#define _SceneGraph_h_

#include <stdio.h>
#include "primitives.h"
#include <vector>

class Node{
public:
    Node(char *n);
    void addTransform(vector<float> t);
    void addPrimitive(Primitive p);
    void addDescendant(char *d);
	void setAppearance(char *a);
	std::vector<Primitive> getPrimitives();
	std::vector<vector<float> > getTransforms();
    ~Node();
private:
    char *name, *appearance;
    std::vector<vector<float> > transforms;
    std::vector<Primitive> primitives;
    std::vector<char *> descendants;
};

class SceneGraph{
public:
	SceneGraph();
    SceneGraph(char *r);
    void addNode(Node n);
	std::vector<Node> getNodes();
    ~SceneGraph();
private:
    char * root;
    std::vector<Node> nodes;
};

#endif /* defined(__CGFExample__SceneGraph__) */