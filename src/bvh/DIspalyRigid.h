#pragma once
#include <iostream>
#include "Rigid.h"
#include "GeometryData.h"
#include "Program.h"
#include "camera.h"

//Definizione camera
Camera cam;

//Definizione luce
struct Light
{
    glm::vec3 pos = glm::vec3(-5.0f, 7.0f, 6.0f);
    glm::vec3 color = glm::vec3(0.7f, 0.7f, 1.0f);
};
Light sun;



struct RigidRender // Single color & Lighting
{
    std::vector<Vertex*> faces;
    int vertexCount; // Number of nodes in faces

    glm::vec4 uniRigidColor;

    glm::vec3* vboPos; // Position
    glm::vec3* vboNor; // Normal

    GLuint programID;
    GLuint vaoID;
    GLuint vboIDs[2];

    GLint aPtrPos;
    GLint aPtrNor;

    // Render any rigid body only with it's faces, color and modelVector
    void init(std::vector<Vertex*> f, glm::vec4 c, glm::vec3 modelVec)
    {
        faces = f;
        vertexCount = (int)(faces.size());
        if (vertexCount <= 0) {
            std::cout << "ERROR::RigidRender : No vertex exists." << std::endl;
            exit(-1);
        }

        uniRigidColor = c;

        vboPos = new glm::vec3[vertexCount];
        vboNor = new glm::vec3[vertexCount];
        for (int i = 0; i < vertexCount; i++) {
            Vertex* v = faces[i];
            vboPos[i] = glm::vec3(v->Position.x, v->Position.y, v->Position.z);
            vboNor[i] = glm::vec3(v->Normal.x, v->Normal.y, v->Normal.z);
        }

        //Build render program
        Program program("Shaders/RigidVS.glsl", "Shaders/RigidFS.glsl");
        programID = program.ID;
        std::cout << "Rigid Program ID: " << programID << std::endl;

        // Generate ID of VAO and VBOs
        glGenVertexArrays(1, &vaoID);
        glGenBuffers(2, vboIDs);

        // Attribute pointers of VAO
        aPtrPos = 0;
        aPtrNor = 1;
        // Bind VAO
        glBindVertexArray(vaoID);

        // Position buffer
        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]);
        glVertexAttribPointer(aPtrPos, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(glm::vec3), vboPos, GL_DYNAMIC_DRAW);
        // Normal buffer
        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]);
        glVertexAttribPointer(aPtrNor, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(glm::vec3), vboNor, GL_DYNAMIC_DRAW);

        // Enable it's attribute pointers since they were set well
        glEnableVertexAttribArray(aPtrPos);
        glEnableVertexAttribArray(aPtrNor);

        //Set uniform
        glUseProgram(programID); // Active shader before set uniform
        // Set color
        glUniform4fv(glGetUniformLocation(programID, "uniRigidColor"), 1, &uniRigidColor[0]);

        //Projection matrix : The frustum that camera observes
        // Since projection matrix rarely changes, set it outside the rendering loop for only onec time
        glUniformMatrix4fv(glGetUniformLocation(programID, "uniProjMatrix"), 1, GL_FALSE, &cam.uniProjMatrix[0][0]);

        //Model Matrix : Put rigid into the world
        glm::mat4 uniModelMatrix = glm::mat4(1.0f);
        uniModelMatrix = glm::translate(uniModelMatrix, modelVec);
        glUniformMatrix4fv(glGetUniformLocation(programID, "uniModelMatrix"), 1, GL_FALSE, &uniModelMatrix[0][0]);

        //Light
        glUniform3fv(glGetUniformLocation(programID, "uniLightPos"), 1, &(sun.pos[0]));
        glUniform3fv(glGetUniformLocation(programID, "uniLightColor"), 1, &(sun.color[0]));

        // Cleanup
        glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbined VBO
        glBindVertexArray(0); // Unbined VAO
    }

    void destroy()
    {
        delete[] vboPos;
        delete[] vboNor;

        if (vaoID)
        {
            glDeleteVertexArrays(1, &vaoID);
            glDeleteBuffers(2, vboIDs);
            vaoID = 0;
        }
        if (programID)
        {
            glDeleteProgram(programID);
            programID = 0;
        }
    }

    void flush()
    {
        for (int i = 0; i < vertexCount; i++) {
            Vertex* v = faces[i];
            vboPos[i] = glm::vec3(v->Position.x, v->Position.y, v->Position.z);
            vboNor[i] = glm::vec3(v->Normal.x, v->Normal.y, v->Normal.z);
        }

        glUseProgram(programID);

        glBindVertexArray(vaoID);
        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertexCount * sizeof(glm::vec3), vboPos);
        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertexCount * sizeof(glm::vec3), vboNor);

        //View Matrix : The camera
        cam.uniViewMatrix = glm::lookAt(cam.Position, cam.Position + cam.Front, cam.Up);
        glUniformMatrix4fv(glGetUniformLocation(programID, "uniViewMatrix"), 1, GL_FALSE, &cam.uniViewMatrix[0][0]);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        //Draw
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);

        // End flushing
        glDisable(GL_BLEND);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glUseProgram(0);
    }
};


struct BallRender
{
    Ball* ball;
    RigidRender render;

    BallRender(Ball* b)
    {
        ball = b;
        render.init(ball->sphere->faces, ball->color, glm::vec3(ball->center.x, ball->center.y, ball->center.z));
    }

    void flush() { render.flush(); }
};


struct GroundRender
{
    Ground* ground;
    RigidRender render;

    GroundRender(Ground* g)
    {
        ground = g;
        render.init(ground->faces, ground->color, glm::vec3(ground->position.x, ground->position.y, ground->position.z));
    }

    void flush() { render.flush(); }
};