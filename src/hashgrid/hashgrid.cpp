/**
 * @file
 * @brief Contains the implementation of class HashGrid.
 * @author Davide Furlani
 * @version 0.1
 * @date February, 2023
 * @copyright 2023 Davide Furlani
 */

#include "hashgrid.h"
#include <vector>
#include <variant>
#include <cmath>
#include "glm.hpp"

using namespace hashgrid;

HashGrid::HashGrid(float sp, int nc) {
    spacing = sp;
    num_cells = nc;
    grid = std::vector<std::vector<std::variant<cloth::Node*, mesh::Triangle*>>>(num_cells);
}
/**
 * date le coordinate INTERE mappate sulla dimensione della hashmap ritorna l'indice della griglia dove è collocato
 * l'oggetto da cercare
 * @param x 
 * @param y 
 * @param z 
 * @return ritorna l'indice della griglia dove è collocato l'oggetto da cercare
 */
int HashGrid::hashCoords(int x, int y, int z) {
    int h = (x * 92837111) ^ (y * 689287499) ^ (z * 283923481);
    return std::abs(h) % num_cells;
}

void HashGrid::update_grid() {

}

std::vector<std::variant<cloth::Node *, mesh::Triangle *>> &HashGrid::get_neighbours(cloth::Node &n) {
    
}
