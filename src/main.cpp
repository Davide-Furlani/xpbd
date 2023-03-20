#include <iostream>

#include <glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include "cloth/cloth.h"
#include "display/display.h"
#include "state/state.h"
#include "display/camera.h"
#include "display/axis.h"
#include "display/floor.h"

const unsigned int SCR_WIDTH = 700;
const unsigned int SCR_HEIGHT = 500;

using namespace glm;
using namespace render;
using namespace cloth;

int main(){


    render::State state {SCR_WIDTH, SCR_HEIGHT};
    GLFWwindow* window = getWindow(SCR_WIDTH, SCR_HEIGHT);
    
    set_GL_parameters();

    cloth::Cloth cloth {120, 40, 1.0, 0.01, state};
    
    render::Camera camera {glm::vec3(4.0, 3.0, 1.0), 
                           glm::vec3(-1.0, -1.0, -0.3),
                           glm::vec3(0.0, 0.0, 1.0)};
    
    Axis axis {SCR_WIDTH, SCR_HEIGHT};
    Floor floor {SCR_WIDTH, SCR_HEIGHT};
    
    // main render loop
    while(!glfwWindowShouldClose(window)){

        state.update(window);
        //std::cout << state.delta_time << std::endl;
        
        processInput(window, state, camera);
        
        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // per vedere le linee dei triangoli

        cloth.proces_input(window);
        cloth.simulate_XPBD(state);
        
        cloth.render(camera);
        axis.render(camera);
        floor.render(camera);

        glfwSwapBuffers(window);
//        glFlush(); // no framerate max
        glfwPollEvents();
        std::cout.precision(1);
        std::cout << std::fixed << 1/state.delta_time << " \t";
        std::cout.precision(3);
        std::cout << std::fixed << state.delta_time << "\t";
        std::cout << cloth.all_tris.size() << std::endl;
    }


    cloth.free_resources();
    axis.free();
    floor.free();
    glfwTerminate();

    return 0;
}