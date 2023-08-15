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
#include <cmath>
#include "glm.hpp"

namespace hashgrid {
//    template <size_t N>
    class HashGrid {
    public:
        float spacing;
        int num_cells;
        std::vector<int> cells;
        std::vector<int> nodes_indexes;
        
        
        HashGrid(float spacing, int num_cells, int num_particles);
        
        [[nodiscard]] int hashCoords(int x, int y, int z) const;
        [[nodiscard]] int intCoord(float c) const;
        [[nodiscard]] int hashIndex(glm::vec3 p) const;

    };
}