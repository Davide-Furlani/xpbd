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
StretchConstraint::StretchConstraint(Node& node1, Node& node2, bool ob) : nodes(node1,node2){
    rest_dist = node1.distance(node2);
    obliquo = ob;
}

std::ostream& operator<<(std::ostream& os, const StretchConstraint& s) {
    os << "Stretch:  node <- " << s.rest_dist << " -> node";
    return os;
}
}