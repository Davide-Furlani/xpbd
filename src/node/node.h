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
#include <vector>

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
     * Thickness of the particle
     */
    float thickness;
    /**
     * Previous position of the node in 3d space
    */
    vec3 prev_pos;
    /**
     * Mass of the node
    */
    float m;
    /**
     * Initial position to calculate the natural distance at rest pos
     */
    vec3 nat_pos;
    /**
     * Inverse of mass (to make formulas more readable)
    */
    float w;
    /**
     * Velocity of the node (speed and direction of movement)
    */
    vec3 vel;
private: float padding_vel_w = 0.0;
public:
    /**
     * Previous velocity
    */
    vec3 prev_vel;
private: float padding_prev_vel_w = 0.0;
public:
    /**
     * Normal of the node (for rendering purposes)
    */
    vec3 n;
private: float padding_n_w = 0.0;
public:
    /**
     * Correction vector for Jacobi solve
     */
     vec3 corr;
private: float padding_corr_w = 0.0;
public:
    /**
     * UV coordinates (for rendering purposes)
    */
    vec2 uv_c;
    /**
     * vector of pointers that contains the neighbour nodes
     */
    std::vector<int> neighbours;

    /**
     * Constructor of node
     * @param position
     * @param mass
     * @param velocity
     * @param normal
     * @param uv_coordinates
     */
    Node(vec3 position, 
        float thickness,
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
};
}
