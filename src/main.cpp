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
#include "hashgrid/hashgrid.h"

constexpr unsigned int SCR_WIDTH = 800;
constexpr unsigned int SCR_HEIGHT = 800;

constexpr unsigned int CLOTH_WIDTH = 60;
constexpr unsigned int CLOTH_HEIGHT = 100;
constexpr float PARTICLE_THICKNESS = 0.02f;
constexpr float GRID_CELL_SIZE = 0.04f;

using namespace glm;
using namespace render;
using namespace cloth;

int main(){

    hashgrid::HashGrid grid {GRID_CELL_SIZE, CLOTH_WIDTH*CLOTH_HEIGHT*5};
    render::State state {SCR_WIDTH, SCR_HEIGHT, grid};
    GLFWwindow* window = getWindow(SCR_WIDTH, SCR_HEIGHT);
    
    set_GL_parameters();

    cloth::Cloth cloth {CLOTH_HEIGHT, CLOTH_WIDTH, 1.2, PARTICLE_THICKNESS, state};
    
    render::Camera camera {glm::vec3(3.0, 2.0, 1.3), 
                           glm::vec3(-1.0, -1.0, -0.3),
                           glm::vec3(0.0, 0.0, 1.0)};
    
    Axis axis {SCR_WIDTH, SCR_HEIGHT};
    Floor floor {SCR_WIDTH, SCR_HEIGHT};
    
   
    
    // main render loop
    while(!glfwWindowShouldClose(window)){

        state.update(window);
        std::cout << "frame time: " << state.delta_time << " - (" << 1.0/state.delta_time << " FPS)" << std::endl;
        
        processInput(window, state, camera);
        
        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // per vedere le linee dei triangoli

        cloth.proces_input(window);
        if(state.current_time_from_start > 5){
            cloth.unpin2();
        }
        if(state.current_time_from_start > 5){
            cloth.unpin1();
        }
        
        cloth.simulate_XPBD(state);
        
        cloth.render(camera);
        axis.render(camera);
        floor.render(camera);

        glfwSwapBuffers(window);
//        glFlush(); // no framerate max
        glfwPollEvents();
//        std::cout.precision(1);
//        std::cout << std::fixed << 1/state.delta_time << " \t";
//        std::cout.precision(3);
//        std::cout << std::fixed << state.delta_time << "\t";
//        std::cout << cloth.all_tris.size() << "\t" << cloth.nodes.size() << "\n";
    }


    cloth.free_resources();
    axis.free();
    floor.free();
    glfwTerminate();

    return 0;
}
