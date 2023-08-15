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
#include <vector>

namespace cloth{

Node::Node(vec3 position, float thickness, float mass, vec3 velocity, vec3 normal, vec2 uv_coordinates){
    this->pos       = position;
    this->prev_pos  = position;
    this->nat_pos   = position;
    this->corr      = vec3(0.0, 0.0, 0.0);
    this->thickness = thickness;
    this->m         = mass;
    this->w         = 1.0f / mass;
    this->vel       = velocity;
    this->prev_vel  = velocity;
    this->n         = normal;
    this->uv_c      = uv_coordinates;
//    this->neighbours= std::vector<Node*>();
}


float Node::distance(Node& node){
    
    float x = pos.x - node.pos.x;
    float y = pos.y - node.pos.y;
    float z = pos.z - node.pos.z;
    
    return sqrt(x*x + y*y + z*z);
}
}
