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
#include <glm/glm.hpp>

namespace hashgrid {
//    template <size_t N>
    class HashGrid {
    public:
        float spacing;
        int num_cells;
        std::vector<int> cells;
        std::vector<int> nodes_indexes;
        
        
        HashGrid(float spacing, int num_cells, int num_particles){
            this->spacing = spacing;
            this->num_cells = num_cells;
            this->cells = std::vector<int>(num_cells+1);
            this->nodes_indexes = std::vector<int>(num_particles);
        }
        
        [[nodiscard]] int hashCoords(int x, int y, int z) const {
            int h = (x * 92837111) ^ (y * 689287499) ^ (z * 283923481);
            return std::abs(h) % num_cells;
        }

        [[nodiscard]] int intCoord(float c) const {
            return std::floor(c / spacing);
        }

        [[nodiscard]] int hashIndex(glm::vec3 p) const {
            return hashCoords(
                    intCoord(p.x),
                    intCoord(p.y),
                    intCoord(p.z));
        }

    };
}