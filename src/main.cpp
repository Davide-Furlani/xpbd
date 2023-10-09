#include <iostream>

#include <glad.h>
#include <GLFW/glfw3.h>
#include "cloth/cloth.h"
#include "display/display.h"
#include "state/state.h"
#include "display/camera.h"
#include "display/axis.h"
#include "display/floor.h"
#include "hashgrid/hashgrid.h"

constexpr unsigned int NUM_FRAME_MEAN = 600;

constexpr unsigned int SCR_WIDTH = 750;
constexpr unsigned int SCR_HEIGHT = 450;

constexpr unsigned int CLOTH_WIDTH  = 64;
constexpr unsigned int CLOTH_HEIGHT = 64;
constexpr float CLOTH_SIZE = 2.0;
constexpr float PARTICLE_THICKNESS = CLOTH_SIZE/CLOTH_WIDTH *0.67;
constexpr float GRID_CELL_SIZE = 2*PARTICLE_THICKNESS;

using namespace glm;
using namespace render;
using namespace cloth;

int main(){

    hashgrid::HashGrid grid {GRID_CELL_SIZE, CLOTH_WIDTH*CLOTH_HEIGHT, CLOTH_WIDTH*CLOTH_HEIGHT};
    render::State state {SCR_WIDTH, SCR_HEIGHT, GPU, NO_HASHGRID};
    GLFWwindow* window = getWindow(SCR_WIDTH, SCR_HEIGHT);
    
    set_GL_parameters();
    
    const GLubyte* vendor = glGetString(GL_VENDOR); // Returns the vendor
    const GLubyte* renderer = glGetString(GL_RENDERER); // Returns a hint to the model

    std::cout << vendor << std::endl;
    std::cout << renderer << std::endl;

    cloth::Cloth cloth {CLOTH_HEIGHT, CLOTH_WIDTH, CLOTH_SIZE, PARTICLE_THICKNESS, state};
    cloth.GPU_send_data();
    
//    render::Camera camera {glm::vec3(3.0, 2.0, 1.3), 
//                           glm::vec3(-1.0, -1.0, -0.3),
//                           glm::vec3(0.0, 0.0, 1.0)};
//                           
    render::Camera camera {glm::vec3(1.0, 4., 1.85), 
                           glm::vec3(0.0, -1.0, 0.0),
                           glm::vec3(0.0, 0.0, 1.0)};
    
    Axis axis {SCR_WIDTH, SCR_HEIGHT};
    Floor floor {SCR_WIDTH, SCR_HEIGHT};
    
//    float benchmark_time[NUM_FRAME_MEAN];
//    int mean_index = 0;
   
    
    // main render loop
    while(!glfwWindowShouldClose(window)){
        state.update(window);
        
//        std::cout << "frame time: " << state.delta_time << " - (" << 1.0/state.delta_time << " FPS)" << std::endl;
        
//        benchmark_time[mean_index] = (float)state.delta_time;
//        mean_index++;
//        mean_index = mean_index%NUM_FRAME_MEAN;
//        if(mean_index == NUM_FRAME_MEAN)
//            break;
        
        processInput(window, state, camera);
        
        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
//        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // per vedere le linee dei triangoli

//        cloth.proces_input(window);
    
//        if(state.sym_type == GPU){
//            cloth.GPU_retrieve_data();
            cloth.proces_input(window);
//            cloth.GPU_send_data();
//        }
        cloth.simulate_XPBD(state, grid);
        
        cloth.render(camera);
//        axis.render(camera);
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

//    float avg_sum = 0.0f;
//    for(int i=0; i<NUM_FRAME_MEAN; i++){
//        avg_sum += benchmark_time[i];
//    }
//    float avg_time = avg_sum/NUM_FRAME_MEAN;
//    std::cout << "media: " << avg_time << std::endl;

    cloth.free_resources();
    axis.free();
    floor.free();
    glfwTerminate();

    return 0;
}
