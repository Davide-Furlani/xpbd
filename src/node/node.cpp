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

Node::Node(vec3 position, float mass, vec3 velocity, vec3 normal, vec2 uv_coordinates){
    pos       = position;
    prev_pos  = position;
    m         = mass;
    w         = 1.0 / mass;
    vel       = velocity;
    prev_vel  = velocity;
    n         = normal;
    uv_c      = uv_coordinates;
}


float Node::distance(Node& node){
    
    return sqrt(powf((pos.x - node.pos.x), 2) +
                powf((pos.y - node.pos.y), 2) +
                powf((pos.z - node.pos.z), 2));
    
    
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
