/**
 * @file
 * @brief Contains the implementation of class Node.
 * @author Davide Furlani
 * @version 0.1
 * @date January, 2023
 * @copyright 2023 Davide Furlani
 */

#include "node/node.h"
#include <cmath>

namespace cloth{

Node::Node(vec3 position, float thickness, float mass, vec3 velocity, vec3 normal, vec2 uv_coordinates){
    this->pos       = position;
    this->prev_pos  = position;
    this->thickness = thickness;
    this->m         = mass;
    this->w         = 1.0 / mass;
    this->vel       = velocity;
    this->prev_vel  = velocity;
    this->n         = normal;
    this->uv_c      = uv_coordinates;
}


float Node::distance(Node& node){
    
    float x = pos.x - node.pos.x;
    float y = pos.y - node.pos.y;
    float z = pos.z - node.pos.z;
    
    return sqrt(x*x + y*y + z*z);
}

std::ostream& operator<<(std::ostream& os, const Node& node) {
    os  << "[physics]" << std::endl
        << "  position: \t  [" << node.prev_pos.x << ", " << node.prev_pos.y << ", " << node.prev_pos.z << "] -> ["
        << node.pos.x << ", " << node.pos.y << ", " << node.pos.z << "]" << std::endl
        << "  mass: \t  " << node.m << "\t(inverse mass: " << node.w << ")" << std::endl
        << "  velocity: \t  [" << node.prev_vel.x << ", " << node.prev_vel.y << ", " << node.prev_vel.z << "] -> ["
        << node.vel.x << ", " << node.vel.y << ", " << node.vel.z << "]" << std::endl
        << "[rendering]" << std::endl
        << "  normals: \t  [" << node.n.x << ", " << node.n.y << ", " << node.n.z << "]" << std::endl
        << "  uv coordinates: [" << node.uv_c.x << ", " << node.uv_c.y << "]";
    return os;
}
}
