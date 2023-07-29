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
#include "node/node.h"

namespace cloth{
struct Constraint
{   
    /**
     * Node pair on which a stretching constraint is set
    */
    std::pair<int, int> nodes;
    /**
     * Distance between nodes at rest
    */
    float rest_dist;
    float compliance;
    //bool obliquo;
    
    Constraint(std::vector<Node>& nodes, int node1, int node2, float compliance);
};
}