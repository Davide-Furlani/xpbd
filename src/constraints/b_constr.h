/**
 * @file
 * @brief Contains the struct BendConstraint.
 * @author Davide Furlani
 * @version 0.1
 * @date January, 2023
 * @copyright 2023 Davide Furlani
 */

#pragma once
#include <utility>
#include "node/node.h"

namespace cloth{
struct BendConstraint
{   
    /**
     * Node pair on which a bending constraint is set
    */
    std::pair<Node&, Node&> nodes;
    /**
     * Distance between nodes at rest
    */
    float rest_dist;
    float compliance = 0.03;
    

    BendConstraint(Node& node1, Node& node2);
    BendConstraint(Node& node1, Node& node2, float compliance);
    BendConstraint(Node& node1, Node& node2, float compliance, float rest_distance);

    friend std::ostream& operator<<(std::ostream& os, const BendConstraint& b);
};
}