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
#include "node/node.h"

namespace cloth{
struct StretchConstraint
{   
    /**
     * Node pair on which a stretching constraint is set
    */
    std::pair<Node&, Node&> nodes;
    /**
     * Distance between nodes at rest
    */
    float rest_dist;
    float compliance = 0.000005; // più piccolo di 0.0000005 comincia a rompersi
//    float compliance = 0.0;
    bool obliquo;
    
    StretchConstraint(Node& node1, Node& node2, bool ob);

    friend std::ostream& operator<<(std::ostream& os, const StretchConstraint& s);
};
}