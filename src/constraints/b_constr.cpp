/**
 * @file
 * @brief Contains the implementation of class BendConstraint.
 * @author Davide Furlani
 * @version 0.1
 * @date January, 2023
 * @copyright 2023 Davide Furlani
 */

#include "constraints/b_constr.h"

namespace cloth{

BendConstraint::BendConstraint(Node& node1, Node& node2) : nodes(node1,node2){
    rest_dist = node1.distance(node2);
}

BendConstraint::BendConstraint(Node& node1, Node& node2, float compliance) : nodes(node1,node2){
    rest_dist = node1.distance(node2);
    BendConstraint::compliance = compliance;
}

BendConstraint::BendConstraint(Node& node1, Node& node2, float compliance, float rest_distance) : nodes(node1,node2) {
    BendConstraint::rest_dist = rest_distance;
    BendConstraint::compliance = compliance;
}
std::ostream& operator<<(std::ostream& os, const BendConstraint& b) {
        os << "Bend:     node <- " << b.rest_dist << " -> node";
        
        return os;
}
}