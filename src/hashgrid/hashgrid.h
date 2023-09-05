/**
 * @file
 * @brief Contains the implementation of class HashGrid.
 * @author Davide Furlani
 * @version 0.1
 * @date February, 2023
 * @copyright 2023 Davide Furlani
 */

#pragma once
#include <vector>
#include <array>
#include <variant>
#include <cmath>
#include "node/node.h"
#include "triangle/triangle.h"
#include "glm.hpp"

namespace hashgrid {
//    template <size_t N>
    class HashGrid {
    public:
        float spacing;
        int num_cells;
//        std::array<std::vector<std::variant<cloth::Node*, mesh::Triangle*>>, N> grid;
        std::vector<std::vector<std::variant<cloth::Node*, mesh::Triangle*>>> grid;
        
        HashGrid(float spacing, int num_cells);
        
        int hashCoords(int x, int y, int z);
        int intCoord(float c);
        int hashIndex(glm::vec3 p);

    };
}