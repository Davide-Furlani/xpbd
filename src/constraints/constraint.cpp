/**
 * @file
 * @brief Contains the implementation of class StretchConstraint.
 * @author Davide Furlani
 * @version 0.1
 * @date January, 2023
 * @copyright 2023 Davide Furlani
 */

#include "constraints/constraint.h"

namespace cloth{
Constraint::Constraint(std::vector<Node>& nodes, int node1, int node2, float compliance){
    this->a_node = node1;
    this->b_node = node2;
    this->rest_dist = nodes.at(node1).distance(nodes.at(node2));
    this->compliance = compliance;
}
}