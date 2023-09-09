/**
* @file
* @brief Contains the implementation for the floor object with which the cloth will collide.
* @author Davide Furlani
* @version 0.1
* @date March, 2023
* @copyright 2023 Davide Furlani
*/

#include <glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include "display/display.h"
#include "display/shader.h"

using namespace render;

class Floor{

public:

    unsigned int fVAO, fVBO;

    Shader FloorShader {"resources/Shaders/floor.vert", "resources/Shaders/floor.frag"};

    Floor(unsigned scr_width, unsigned scr_height){
        float Xaxis_v[] = { -10.0f, -8.02f, -10.0f,      10.0f, -8.02f, -10.0f,       10.0f,  -8.0f, -10.0f,      10.0f,  -8.0f, -10.0f,      -10.0f,  -8.0f, -10.0f,     -10.0f, -8.02f, -10.0f,      -10.0f, -8.02f,   10.0f,      10.0f, -8.02f,   10.0f,       10.0f,  -8.0f,   10.0f,      10.0f,  -8.0f,   10.0f,      -10.0f,  -8.0f,   10.0f,     -10.0f, -8.02f,   10.0f,      -10.0f,  -8.0f,   10.0f,     -10.0f,  -8.0f, -10.0f,      -10.0f, -8.02f, -10.0f,     -10.0f, -8.02f, -10.0f,      -10.0f, -8.02f,   10.0f,     -10.0f,  -8.0f,   10.0f,       10.0f,  -8.0f,   10.0f,      10.0f,  -8.0f, -10.0f,       10.0f, -8.02f, -10.0f,      10.0f, -8.02f, -10.0f,       10.0f, -8.02f,   10.0f,      10.0f,  -8.0f,   10.0f,      -10.0f, -8.02f, -10.0f,      10.0f, -8.02f, -10.0f,       10.0f, -8.02f,   10.0f,      10.0f, -8.02f,   10.0f,      -10.0f, -8.02f,   10.0f,     -10.0f, -8.02f, -10.0f,      -10.0f,  -8.0f, -10.0f,     10.0f,  -8.0f, -10.0f,     10.0f,  -8.0f,   10.0f,     10.0f,  -8.0f,   10.0f,      -10.0f,  -8.0f,   10.0f,     -10.0f,  -8.0f, -10.0f};
        
        glGenVertexArrays(1, &fVAO);
        glGenBuffers(1, &fVBO);
        glBindVertexArray(fVAO);
        glBindBuffer(GL_ARRAY_BUFFER, fVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Xaxis_v), Xaxis_v, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        mat4 projection = perspective(glm::radians(45.0f), static_cast<float>(scr_width) / static_cast<float>(scr_height), 0.1f, 100.0f);
        FloorShader.use();
        FloorShader.setMat4("uniProjMatrix", projection);
    }

    void render(Camera& c){

        mat4 view = lookAt(c.pos, c.pos + c.front_v, c.up_v);

        FloorShader.use();
        FloorShader.setMat4("uniViewMatrix", view);
        glBindVertexArray(fVAO);
        mat4 model = mat4(1.0f);
        FloorShader.setMat4("uniModelMatrix", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    void free(){
        glDeleteVertexArrays(1, &fVAO);
        glDeleteBuffers(1, &fVBO);
        FloorShader.destroy();
    }

};

