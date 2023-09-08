/**
* @file
* @brief Contains the implementation of some useful functions for display and render.
* @author Davide Furlani
* @version 0.1
* @date January, 2023
* @copyright 2023 Davide Furlani
*/

#include "display.h"
#include <glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
//#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "state/state.h"
#include "display/camera.h"


namespace render {

    void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);
    }
    
    void process_camera_translation(GLFWwindow* window, State& state, Camera& camera){
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.forward(state);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.backwards(state);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.left(state);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.right(state);
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            camera.down(state);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            camera.up(state);
    }
    
    void process_camera_rotation(GLFWwindow* window, State& state, Camera& camera){
        camera.update_rotation(state);
    }
    
    void process_camera_movement(GLFWwindow* window, State& state, Camera& camera){
        process_camera_translation(window, state, camera);
        process_camera_rotation(window, state, camera);
    }
    
    bool should_close(GLFWwindow* window){
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        return false;
    }

    void processInput(GLFWwindow* window, State& state, Camera& camera) {
        should_close(window);
        process_camera_movement(window, state, camera);
    }
    
    GLFWwindow *getWindow(int width, int height) {
        //creo la finestra
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_REFRESH_RATE, GLFW_DONT_CARE);
//        glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE); // no framerate max

        //buffer di pixel 2x2 per anti aliasing
        glfwWindowHint(GLFW_SAMPLES, 4);

        GLFWwindow *window = glfwCreateWindow(width, height, "Finestra", NULL, NULL);
        if (window == NULL) {
            std::cout << ">Failed to create GLFW window" << std::endl;
            glfwTerminate();
            exit(-1);
        }
        glfwMakeContextCurrent(window);

        if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
            std::cout << ">Failed to initialize GLAD" << std::endl;
            exit(-1);
        }
        glViewport(0, 0, width, height);

        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

//        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // cursor settings
        
        return window;
    }

    void set_GL_parameters(){
        //enable depth test
        glEnable(GL_DEPTH_TEST);
        //enable anti aliasing
        glEnable(GL_MULTISAMPLE);
    }

    unsigned int load_textures(std::filesystem::path& texture_p){
        unsigned int texture;

        // container    
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int width, height, nrChannels;

        stbi_set_flip_vertically_on_load(true);

        unsigned char *data = stbi_load(&texture_p.string()[0], &width, &height, &nrChannels, 0);
        //unsigned char *data = stbi_load("resources/Textures/tex1.jpg", &width, &height, &nrChannels, 0);

        if (data){
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else{
            std::cout << "Failed to load texture" << std::endl;
        }
        stbi_image_free(data);
        
        return texture;
    }
}