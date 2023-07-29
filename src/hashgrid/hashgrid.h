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
#include <cmath>
#include "node/node.h"
#include "glm.hpp"

namespace hashgrid {
//    template <size_t N>
    class HashGrid {
    public:
        float spacing;
        int num_cells;
        std::vector<std::vector<cloth::Node*>> grid;
        
        HashGrid(float spacing, int num_cells);
        
        int hashCoords(int x, int y, int z);
        int intCoord(float c);
        int hashIndex(glm::vec3 p);
        
        void update_grid();

    };
}