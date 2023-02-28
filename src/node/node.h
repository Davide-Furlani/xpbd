/**
 * @file
 * @brief Contains the struct Node used for define Cloth.
 * @author Davide Furlani
 * @version 0.1
 * @date January, 2023
 * @copyright 2023 Davide Furlani
 */

#pragma once
#include <glm.hpp>
#include <ostream>

namespace cloth{
using namespace glm;
/**
 * @class Node
 * @brief This class represents a node of a cloth.
 */
struct Node
{
public:
    /**
     * Position of the node in 3d space
    */
    vec3 pos;
    /**
     * Previous position of the node in 3d space
    */
    vec3 prev_pos;
    /**
     * Mass of the node
    */
    float m;
    /**
     * Inverse of mass (to make formulas more readable)
    */
    float w;
    /**
     * Velocity of the node (speed and direction of movement)
    */
    vec3 vel;
    /**
     * Previous velocity
    */
    vec3 prev_vel;
    /**
     * Normal of the node (for rendering purposes)
    */
    vec3 n;
    /**
     * UV coordinates (for rendering purposes)
    */
    vec2 uv_c;

    /**
     * Constructor of node
     * @param position
     * @param mass
     * @param velocity
     * @param normal
     * @param uv_coordinates
     */
    Node(vec3 position, 
        float mass, 
        vec3 velocity, 
        vec3 normal,
        vec2 uv_coordinates);
    
    /**
     * Calculate distance value between two nodes
     * @param node1
     * @param node2
     * @returns distance
     */
    float distance(Node& node);

    /**
     * Stream operator
     * @param os output stream
     * @param node node to send to stream
     * @return stream
     */
    friend std::ostream& operator<<(std::ostream& os, const Node& node);
};
}
