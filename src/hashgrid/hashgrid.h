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
#include <variant>
#include "node/node.h"
#include "triangle/triangle.h"
#include "glm.hpp"

namespace hashgrid {
    
    class HashGrid {
    public:
        float spacing;
        int num_cells;
        std::vector<std::vector<std::variant<cloth::Node*, mesh::Triangle*>>> grid;
        
        HashGrid(float spacing, int num_cells);
        
        int hashCoords(int x, int y, int z);
        
        void update_grid();

        std::vector<std::variant<cloth::Node*, mesh::Triangle*>>& get_neighbours(cloth::Node& n);
    };
}