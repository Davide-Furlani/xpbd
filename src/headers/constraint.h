/**
 * @file
 * @brief Contains the struct StretchConstraint.
 * @author Davide Furlani
 * @version 0.1
 * @date January, 2023
 * @copyright 2023 Davide Furlani
 */

#pragma once
#include <utility>
#include <vector>
#include "node.h"

namespace cloth{
struct Constraint
{
    int a_node;
    int b_node;
    
    float rest_dist;
    float compliance;
    
    Constraint(std::vector<Node>& nodes, int node1, int node2, float compliance){
        this->a_node = node1;
        this->b_node = node2;
        this->rest_dist = nodes.at(node1).distance(nodes.at(node2));
        this->compliance = compliance;
    }
};
}