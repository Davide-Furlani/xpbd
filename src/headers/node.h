/**
 * @file
 * @brief Contains the struct Node used for define Cloth.
 * @author Davide Furlani
 * @version 0.1
 * @date January, 2023
 * @copyright 2023 Davide Furlani
 */

#pragma once
#include <glm/glm.hpp>
#include <ostream>
#include <vector>

namespace cloth{

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
    glm::vec3 pos;
    /**
     * Thickness of the particle
     */
    float thickness;
    /**
     * Previous position of the node in 3d space
    */
    glm::vec3 prev_pos;
    /**
     * Mass of the node
    */
    float m;
    /**
     * Initial position to calculate the natural distance at rest pos
     */
    glm::vec3 nat_pos;
    /**
     * Inverse of mass (to make formulas more readable)
    */
    float w;
    /**
     * Velocity of the node (speed and direction of movement)
    */
    glm::vec3 vel;
private: float padding_vel_w = 0.0;
public:
    /**
     * Previous velocity
    */
    glm::vec3 prev_vel;
private: float padding_prev_vel_w = 0.0;
public:
    /**
     * Normal of the node (for rendering purposes)
    */
    glm::vec3 n;
private: float padding_n_w = 0.0;
public:
    /**
     * Correction vector for Jacobi solve
     */
    glm::vec3 corr;
private: float padding_corr_w = 0.0;
public:
    /**
     * UV coordinates (for rendering purposes)
    */
    glm::vec2 uv_c;
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
    Node(glm::vec3 position,
        float thickness,
        float mass,
         glm::vec3 velocity,
         glm::vec3 normal,
         glm::vec2 uv_coordinates){
            this->pos       = position;
            this->prev_pos  = position;
            this->nat_pos   = position;
            this->corr      = glm::vec3(0.0, 0.0, 0.0);
            this->thickness = thickness;
            this->m         = mass;
            this->w         = 1.0f / mass;
            this->vel       = velocity;
            this->prev_vel  = velocity;
            this->n         = normal;
            this->uv_c      = uv_coordinates;
    //    this->neighbours= std::vector<Node*>();
        }
    
    /**
     * Calculate distance value between two nodes
     * @param node1
     * @param node2
     * @returns distance
     */
    float distance(Node& node){

        float x = pos.x - node.pos.x;
        float y = pos.y - node.pos.y;
        float z = pos.z - node.pos.z;

        return sqrt(x*x + y*y + z*z);
    }
};
}
