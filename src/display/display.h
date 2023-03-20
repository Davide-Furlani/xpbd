/**
* @file
* @brief Contains the definition of some useful functions for display and render.
* @author Davide Furlani
* @version 0.1
* @date January, 2023
* @copyright 2023 Davide Furlani
*/

#pragma once
#include <glad.h>
#include <GLFW/glfw3.h>
#include <filesystem>
#include "state/state.h"
#include "display/camera.h"
#include "cloth/cloth.h"
#include "display/shader.h"

namespace render{
    
    struct vertex_buffer{
        unsigned int VAO;
        unsigned int VBO;
    };

    void framebuffer_size_callback(GLFWwindow* window, int width, int height);

    void process_camera_translation(GLFWwindow* window, State& state, Camera& camera);

    void process_camera_rotation(GLFWwindow* window, State& state, Camera& camera);

    void process_camera_movement(GLFWwindow* window, State& state, Camera& camera);

    bool should_close(GLFWwindow* window);

    void processInput(GLFWwindow* window, State& state, Camera& camera);
    
    GLFWwindow* getWindow(int width, int height);
    
    void set_GL_parameters();

    unsigned int load_textures(std::filesystem::path& texture);

}