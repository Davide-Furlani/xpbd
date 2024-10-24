#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <learnopengl/model.h>
#include <learnopengl/shader.h>
//#include "headers/square_cloth.h"
#include "headers/cloth.h"
#include "headers/display.h"
#include "headers/state.h"
#include "headers/camera.h"
#include "headers/axis.h"
#include "headers/floor.h"
#include "headers/hashgrid.h"


constexpr unsigned int NUM_FRAME_MEAN = 600;

constexpr unsigned int SCR_WIDTH = 750;
constexpr unsigned int SCR_HEIGHT = 450;

constexpr unsigned int CLOTH_WIDTH  = 24;
constexpr unsigned int CLOTH_HEIGHT = 24;
constexpr float CLOTH_SIZE = 2.0;
constexpr float PARTICLE_THICKNESS = CLOTH_SIZE/CLOTH_WIDTH *0.67;
constexpr float GRID_CELL_SIZE = 2*PARTICLE_THICKNESS;


using namespace glm;
using namespace render;
using namespace cloth;


// start of the simulator
int main(){

    hashgrid::HashGrid grid {GRID_CELL_SIZE, CLOTH_WIDTH*CLOTH_HEIGHT, CLOTH_WIDTH*CLOTH_HEIGHT};
    render::State state {SCR_WIDTH, SCR_HEIGHT, GPU, NO_HASHGRID};
    GLFWwindow* window = getWindow(SCR_WIDTH, SCR_HEIGHT);
    
    set_GL_parameters();
    
    const GLubyte* vendor = glGetString(GL_VENDOR); // Returns the vendor
    const GLubyte* renderer = glGetString(GL_RENDERER); // Returns a hint to the model

    std::cout << vendor << std::endl;
    std::cout << renderer << std::endl;

    //cloth::SquareCloth cloth {CLOTH_HEIGHT, CLOTH_WIDTH, CLOTH_SIZE, PARTICLE_THICKNESS, state};
    //cloth.GPU_send_data();

    std::filesystem::path cloth_path = "resources/Meshes/cube.stl";

    cloth::Cloth cloth = Cloth{cloth_path};

    
//    render::Camera camera {glm::vec3(3.0, 2.0, 1.3), 
//                           glm::vec3(-1.0, -1.0, -0.3),
//                           glm::vec3(0.0, 0.0, 1.0)};
//                           
    render::Camera camera {glm::vec3(1.0, 4., 1.85), 
                           glm::vec3(0.0, -1.0, 0.0),
                           glm::vec3(0.0, 0.0, 1.0)};
    
    Axis axis {SCR_WIDTH, SCR_HEIGHT};
    Floor floor {SCR_WIDTH, SCR_HEIGHT};
//    Model owl{"resources/Meshes/owl_light.stl"};
//    Shader owl_shader{"resources/Shaders/owl.vert", "resources/Shaders/owl.frag"};
//
//
//    for(auto& v: owl.meshes[0].vertices) {
//        v.Position.x *= 0.1f;
//        v.Position.y *= 0.1f;
//        v.Position.z *= 0.1f;
//    }
//    owl.meshes[0].setupMesh();
//
//    mat4 projection = perspective(glm::radians(45.0f), static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT), 0.1f, 100.0f);
//    owl_shader.use();
//    owl_shader.setMat4("uniProjMatrix", projection);





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


    
//        if(state.sim_type == GPU){
//            cloth.GPU_retrieve_data();
//            cloth.proces_input(window);
//            cloth.GPU_send_data();
//        }else{ // CPU
//            cloth.proces_input(window);
//        }
//
//        cloth.simulate_XPBD(state, grid);
//
//        cloth.render(camera);
        axis.render(camera);
        floor.render(camera);
//        owl.Draw(owl_shader, &camera.pos, &camera.front_v, &camera.up_v);

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

//    cloth.free_resources();
    axis.free();
    floor.free();
    glfwTerminate();

    return 0;
}
