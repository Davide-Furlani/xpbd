/**
* @file
* @brief Contains the implementation of State class.
* @author Davide Furlani
* @version 0.1
* @date January, 2023
* @copyright 2023 Davide Furlani
*/
#include "state.h"
#include <GLFW/glfw3.h>

namespace render {

    
    
    State::State(const unsigned int w, const unsigned int h){
        
        scr_width = w;
        scr_height = h;
        
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