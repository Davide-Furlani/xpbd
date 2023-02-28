/**
 * @file
 * @brief Contains the implementation of class StretchConstraint.
 * @author Davide Furlani
 * @version 0.1
 * @date January, 2023
 * @copyright 2023 Davide Furlani
 */

#include "constraints/s_constr.h"

namespace cloth{
StretchConstraint::StretchConstraint(Node& node1, Node& node2) : nodes(node1,node2){
    rest_dist = node1.distance(node2);
}

StretchConstraint::StretchConstraint(Node& node1, Node& node2, float compliance) : nodes(node1,node2){
    rest_dist = node1.distance(node2);
    StretchConstraint::compliance = compliance;
}

StretchConstraint::StretchConstraint(Node& node1, Node& node2, float compliance, float rest_distance) : nodes(node1,node2) {
    StretchConstraint::rest_dist = rest_distance;
    StretchConstraint::compliance = compliance;
}

std::ostream& operator<<(std::ostream& os, const StretchConstraint& s) {
    os << "Stretch:  node <- " << s.rest_dist << " -> node";
    return os;
}
}