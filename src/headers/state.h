/**
* @file
* @brief Contains the definition of State class.
* @author Davide Furlani
* @version 0.1
* @date January, 2023
* @copyright 2023 Davide Furlani
*/

#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "hashgrid.h"

#define CPU 0
#define GPU 1
#define HASHGRID 1
#define NO_HASHGRID 0

#define N_SSBO 0
#define CC_SSBO 1
#define JC_SSBO 2

#define COLORING 0
#define JACOBI 1
#define HYBRID 2

namespace render {
    struct State {
    public:
        
        //simulation
        int sim_type;
        int hashgrid_sim;
        
        //physics
        int iteration_per_frame = 30;
        float simulation_step_time = 1.0f/60.0f;
        glm::vec3 gravity {0.0, 0.0, -9.81};
        
        
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


        State(unsigned int w, unsigned int h, int sim_type, int hashgrid_sim){
            this->sim_type = sim_type;
            this->hashgrid_sim = hashgrid_sim;

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

        void update(GLFWwindow* window){

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

    };
}