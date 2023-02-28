#include <iostream>
#include <filesystem>

#include <glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <sys/time.h>
#include "cloth/cloth.h"
#include "display/display.h"
#include "state/state.h"
#include "display/camera.h"
#include "display/axis.h"

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

using namespace glm;
using namespace render;
using namespace cloth;

int main(){


    render::State state {SCR_WIDTH, SCR_HEIGHT};
    GLFWwindow* window = getWindow(SCR_WIDTH, SCR_HEIGHT);
    
    set_GL_parameters();

    cloth::Cloth cloth {60, 60, 1.0, state};
    
    render::Camera camera {glm::vec3(0.0, 3.0, 2.0),
                           glm::vec3(0.0, -1.0, -1.0),
                           glm::vec3(0.0, 0.0, 1.0)};
    
    Axis axis {SCR_WIDTH, SCR_HEIGHT};
    
    // main render loop
    while(!glfwWindowShouldClose(window)){

        state.update(window);
        //std::cout << state.delta_time << std::endl;
        
        processInput(window, state, camera);
        
        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // per vedere le linee dei triangoli
        
        cloth.simulate_XPBD(state);
        
        cloth.render(camera);
        axis.render(camera);

        glfwSwapBuffers(window);
        //glFlush();
        glfwPollEvents();
        //std::cout << state.delta_time << std::endl;
    }


    cloth.free_resources();
    axis.free();
    glfwTerminate();

    return 0;
}
