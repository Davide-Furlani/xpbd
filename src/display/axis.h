// temporaneo, per visualizzare gli assi


#pragma once

#include <glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include "display/display.h"
#include "display/shader.h"

using namespace render;

class Axis{
    
public:
    
    unsigned int XaVAO, YaVAO, ZaVAO, XaVBO, YaVBO, ZaVBO;
    
    Shader XaShader {"resources/Shaders/Xaxis.vert", "resources/Shaders/Xaxis.frag"};
    Shader YaShader {"resources/Shaders/Yaxis.vert", "resources/Shaders/Yaxis.frag"};
    Shader ZaShader {"resources/Shaders/Zaxis.vert", "resources/Shaders/Zaxis.frag"};
    
    Axis(unsigned scr_width, unsigned scr_height){
        float Xaxis_v[] = {-1.0f, -0.01f, -0.01f,  100.0f, -0.01f, -0.01f,  100.0f,  0.01f, -0.01f,  100.0f,  0.01f, -0.01f, -1.0f,  0.01f, -0.01f, -1.0f, -0.01f, -0.01f, -1.0f, -0.01f,  0.01f,  100.0f, -0.01f,  0.01f,  100.0f,  0.01f,  0.01f,  100.0f,  0.01f,  0.01f, -1.0f,  0.01f,  0.01f, -1.0f, -0.01f,  0.01f, -1.0f,  0.01f,  0.01f, -1.0f,  0.01f, -0.01f, -1.0f, -0.01f, -0.01f, -1.0f, -0.01f, -0.01f, -1.0f, -0.01f,  0.01f, -1.0f,  0.01f,  0.01f,  100.0f,  0.01f,  0.01f,  100.0f,  0.01f, -0.01f,  100.0f, -0.01f, -0.01f,  100.0f, -0.01f, -0.01f,  100.0f, -0.01f,  0.01f,  100.0f,  0.01f,  0.01f, -1.0f, -0.01f, -0.01f,  100.0f, -0.01f, -0.01f,  100.0f, -0.01f,  0.01f,  100.0f, -0.01f,  0.01f, -1.0f, -0.01f,  0.01f, -1.0f, -0.01f, -0.01f, -1.0f,  0.01f, -0.01f,  100.0f,  0.01f, -0.01f,  100.0f,  0.01f,  0.01f,  100.0f,  0.01f,  0.01f, -1.0f,  0.01f,  0.01f, -1.0f,  0.01f, -0.01f};
        float Yaxis_v[] = {-0.01f, -1.0f, -0.01f,  0.01f, -1.0f, -0.01f,  0.01f,  100.0f, -0.01f,  0.01f,  100.0f, -0.01f, -0.01f,  100.0f, -0.01f, -0.01f, -1.0f, -0.01f, -0.01f, -1.0f,  0.01f,  0.01f, -1.0f,  0.01f,  0.01f,  100.0f,  0.01f,  0.01f,  100.0f,  0.01f, -0.01f,  100.0f,  0.01f, -0.01f, -1.0f,  0.01f, -0.01f,  100.0f,  0.01f, -0.01f,  100.0f, -0.01f, -0.01f, -1.0f, -0.01f, -0.01f, -1.0f, -0.01f, -0.01f, -1.0f,  0.01f, -0.01f,  100.0f,  0.01f,  0.01f,  100.0f,  0.01f,  0.01f,  100.0f, -0.01f,  0.01f, -1.0f, -0.01f,  0.01f, -1.0f, -0.01f,  0.01f, -1.0f,  0.01f,  0.01f,  100.0f,  0.01f, -0.01f, -1.0f, -0.01f,  0.01f, -1.0f, -0.01f,  0.01f, -1.0f,  0.01f,  0.01f, -1.0f,  0.01f, -0.01f, -1.0f,  0.01f, -0.01f, -1.0f, -0.01f, -0.01f,  100.0f, -0.01f,  0.01f,  100.0f, -0.01f,  0.01f,  100.0f,  0.01f,  0.01f,  100.0f,  0.01f, -0.01f,  100.0f,  0.01f, -0.01f,  100.0f, -0.01f};
        float Zaxis_v[] = {-0.01f, -0.01f, -1.0f,  0.01f, -0.01f, -1.0f,  0.01f,  0.01f, -1.0f,  0.01f,  0.01f, -1.0f, -0.01f,  0.01f, -1.0f, -0.01f, -0.01f, -1.0f, -0.01f, -0.01f,  100.0f,  0.01f, -0.01f,  100.0f,  0.01f,  0.01f,  100.0f,  0.01f,  0.01f,  100.0f, -0.01f,  0.01f,  100.0f, -0.01f, -0.01f,  100.0f, -0.01f,  0.01f,  100.0f, -0.01f,  0.01f, -1.0f, -0.01f, -0.01f, -1.0f, -0.01f, -0.01f, -1.0f, -0.01f, -0.01f,  100.0f, -0.01f,  0.01f,  100.0f,  0.01f,  0.01f,  100.0f,  0.01f,  0.01f, -1.0f,  0.01f, -0.01f, -1.0f,  0.01f, -0.01f, -1.0f,  0.01f, -0.01f,  100.0f,  0.01f,  0.01f,  100.0f, -0.01f, -0.01f, -1.0f,  0.01f, -0.01f, -1.0f,  0.01f, -0.01f,  100.0f,  0.01f, -0.01f,  100.0f, -0.01f, -0.01f,  100.0f, -0.01f, -0.01f, -1.0f, -0.01f,  0.01f, -1.0f,  0.01f,  0.01f, -1.0f,  0.01f,  0.01f,  100.0f,  0.01f,  0.01f,  100.0f, -0.01f,  0.01f,  100.0f, -0.01f,  0.01f, -1.0f};
        
        glGenVertexArrays(1, &XaVAO);
        glGenBuffers(1, &XaVBO);
        glBindVertexArray(XaVAO);
        glBindBuffer(GL_ARRAY_BUFFER, XaVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Xaxis_v), Xaxis_v, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glGenVertexArrays(1, &YaVAO);
        glGenBuffers(1, &YaVBO);
        glBindVertexArray(YaVAO);
        glBindBuffer(GL_ARRAY_BUFFER, YaVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Yaxis_v), Yaxis_v, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glGenVertexArrays(1, &ZaVAO);
        glGenBuffers(1, &ZaVBO);
        glBindVertexArray(ZaVAO);
        glBindBuffer(GL_ARRAY_BUFFER, ZaVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Zaxis_v), Zaxis_v, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        mat4 projection = perspective(glm::radians(45.0f), static_cast<float>(scr_width) / static_cast<float>(scr_height), 0.1f, 100.0f);
        XaShader.use();
        XaShader.setMat4("uniProjMatrix", projection);
        YaShader.use();
        YaShader.setMat4("uniProjMatrix", projection);
        ZaShader.use();
        ZaShader.setMat4("uniProjMatrix", projection);
    }

    void render(Camera& c){

        mat4 view = lookAt(c.pos, c.pos + c.front_v, c.up_v);
        
        XaShader.use();
        XaShader.setMat4("uniViewMatrix", view);
        glBindVertexArray(XaVAO);
        mat4 model = mat4(1.0f);
        XaShader.setMat4("uniModelMatrix", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);


        YaShader.use();
        YaShader.setMat4("uniViewMatrix", view);
        glBindVertexArray(YaVAO);
        model = mat4(1.0f);
        YaShader.setMat4("uniModelMatrix", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);


        ZaShader.use();
        ZaShader.setMat4("uniViewMatrix", view);
        glBindVertexArray(ZaVAO);
        model = mat4(1.0f);
        ZaShader.setMat4("uniModelMatrix", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    
    void free(){
        glDeleteVertexArrays(1, &XaVAO);
        glDeleteBuffers(1, &XaVBO);
        glDeleteVertexArrays(1, &YaVAO);
        glDeleteBuffers(1, &YaVBO);
        glDeleteVertexArrays(1, &ZaVAO);
        glDeleteBuffers(1, &ZaVBO);
        XaShader.destroy();
        YaShader.destroy();
        ZaShader.destroy();
    }
    
};

