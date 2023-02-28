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

namespace render {
    struct State {
    public:
        
        unsigned scr_width;
        unsigned scr_height;
        glm::vec3 gravity {0.0, 0.0, -9.81};

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
        
        State(const unsigned int w, const unsigned int h);
        
    };
}