/**
 * @file
 * @brief Contains the implementation of class HashGrid.
 * @author Davide Furlani
 * @version 0.1
 * @date February, 2023
 * @copyright 2023 Davide Furlani
 */

#include "hashgrid.h"
#include <cmath>
#include "glm.hpp"

using namespace hashgrid;

HashGrid::HashGrid(float spacing, int num_cells, int num_particles) {
    this->spacing = spacing;
    this->num_cells = num_cells;
    this->cells = std::vector<int>(num_cells+1);
    this->nodes_indexes = std::vector<int>(num_particles);
}
/**
 * date le coordinate INTERE mappate sulla dimensione della hashmap ritorna l'indice della griglia dove è collocato
 * l'oggetto da cercare
 * @param x 
 * @param y 
 * @param z 
 * @return ritorna l'indice della griglia dove è collocato l'oggetto da cercare
 */
int HashGrid::hashCoords(int x, int y, int z) const {
    int h = (x * 92837111) ^ (y * 689287499) ^ (z * 283923481);
    return std::abs(h) % num_cells;
}

int HashGrid::intCoord(float c) const {
    return std::floor(c / spacing);
}

int HashGrid::hashIndex(glm::vec3 p) const {
    return hashCoords(
            intCoord(p.x),
            intCoord(p.y),
            intCoord(p.z));
}

