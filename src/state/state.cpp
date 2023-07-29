/**
* @file
* @brief Contains thhashe implementation of State class.
* @author Davide Furlani
* @version 0.1
* @date January, 2023
* @copyright 2023 Davide Furlani
*/
#include "state.h"
#include <GLFW/glfw3.h>

#include <utility>
#include <vector>
#include "node/node.h"
#include "hashgrid/hashgrid.h"


using namespace hashgrid;
namespace render {

    
    
    State::State(unsigned int w, unsigned int h, HashGrid g) :grid(std::move(g)){
        
        grid_size = g.num_cells;
        
        scr_width = w;
        scr_height = h;
        
        start_time = glfwGetTime();
        current_time_from_start = 0.0;
        
        last_frame_time = glfwGetTime();
        current_frame_time = glfwGetTime();
        delta_time = 0.0;
        
        last_mouseX_pos = 0.0;
        current_mouseX_pos = 0.0;
        delta_mouseX_pos = 0.0;
        
        last_mouseY_pos = 0.0;
        current_mouseY_pos = 0.0;
        delta_mouseY_pos = 0.0;
    }

    void State::update(GLFWwindow *window) {
        
        grid.grid.clear();
        grid.grid.reserve(grid.num_cells);
        for(int i=0; i<grid_size; ++i)
            grid.grid.emplace_back(std::vector<cloth::Node*>());
        current_time_from_start = glfwGetTime() - start_time;
        
        last_frame_time = current_frame_time;
        current_frame_time = static_cast<float>(glfwGetTime());
        delta_time = current_frame_time - last_frame_time;

        last_mouseX_pos = current_mouseX_pos;
        last_mouseY_pos = current_mouseY_pos;

        glfwGetCursorPos(window, &current_mouseX_pos, &current_mouseY_pos);

        delta_mouseX_pos = current_mouseX_pos - last_mouseX_pos;
        delta_mouseY_pos = last_mouseY_pos - current_mouseY_pos;
    }

}