/**
* @file
* @brief Contains the definition of State class.
* @author Davide Furlani
* @version 0.1
* @date January, 2023
* @copyright 2023 Davide Furlani
*/

#pragma once
#include <glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include "hashgrid/hashgrid.h"

#define CPU 0
#define GPU 1

namespace render {
    struct State {
    public:
        
        //simulation
        int sym_type;
        
        //physics
        int iteration_per_frame = 30;
        float simulation_step_time = 1.0f/60.0f;
        glm::vec3 gravity {0.0, -9.81, 0.0};
        
        
        //rendering
        unsigned scr_width;
        unsigned scr_height;
        
        double start_time = 0.0;
        double current_time_from_start = 0.0;
        
        double last_frame_time = 0.0;
        double current_frame_time = 0.0;
        double delta_time = 0.0;
        
        double last_mouseX_pos = 0.0;
        double current_mouseX_pos = 0.0;
        double delta_mouseX_pos = 0.0;

        double last_mouseY_pos = 0.0;
        double current_mouseY_pos = 0.0;
        double delta_mouseY_pos = 0.0;


        void update(GLFWwindow* window);
        
        State(unsigned int w, unsigned int h, int sim_type);
        
    };
}