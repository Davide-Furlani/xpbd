#pragma once

#include "Vectors.h"

struct Vertex
{
public:
    Vec3 position;
    Vec3 normal;
    
    Vertex() {}
    Vertex(Vec3 pos)
    {
        position = pos;
    }
    ~Vertex() {}
};

class Node
{
public:
    double  mass;           // In this project it will always be 1
    double  w;              // the inverse of mass
    bool    isFixed;        // Use to pin the cloth
    Vec2    texCoord;       // Texture coord
    Vec3    normal;         // For smoothly shading
	Vec3	position;
    Vec3    prev_pos;
    Vec3    velocity;
    Vec3    force;
	Vec3	acceleration;

public:
    Node(void) {
        mass = 1.0;
        w = 1.0/mass;
        isFixed = false;
        velocity.setZeroVec();
        force.setZeroVec();
        acceleration.setZeroVec();
    }
	Node(Vec3 pos)
    {
        mass = 1.0;
        w = 1.0/mass;
        isFixed = false;
        position = pos;
        velocity.setZeroVec();
        force.setZeroVec();
        acceleration.setZeroVec();
    }
	~Node(void) {}

	void addForce(Vec3 f)
	{
        force += f;
	}

	void integrate(double timeStep) // Only non-fixed nodes take integration
	{
		if (!isFixed) // Verlet integration
		{
            acceleration = force/mass;
            velocity += acceleration*timeStep;
            position += velocity*timeStep;
        }
        force.setZeroVec();
	}
};
