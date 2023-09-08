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

#include "animation/animator.h"

#include "bvh/BVH.h"


constexpr unsigned int SCR_WIDTH = 750;
constexpr unsigned int SCR_HEIGHT = 450;

constexpr unsigned int CLOTH_WIDTH  = 32;
constexpr unsigned int CLOTH_HEIGHT = 32;
constexpr float CLOTH_SIZE = 2.0;
constexpr float PARTICLE_THICKNESS = CLOTH_SIZE/CLOTH_WIDTH;
constexpr float GRID_CELL_SIZE = 2*PARTICLE_THICKNESS;

using namespace glm;
using namespace render;
using namespace cloth;

int main(){

    hashgrid::HashGrid grid {GRID_CELL_SIZE, CLOTH_WIDTH*CLOTH_HEIGHT, CLOTH_WIDTH*CLOTH_HEIGHT};
    render::State state {SCR_WIDTH, SCR_HEIGHT, GPU};
    GLFWwindow* window = getWindow(SCR_WIDTH, SCR_HEIGHT);
    
    set_GL_parameters();
    
    const GLubyte* vendor = glGetString(GL_VENDOR); // Returns the vendor
    const GLubyte* renderer = glGetString(GL_RENDERER); // Returns a hint to the model

    std::cout << vendor << std::endl;
    std::cout << renderer << std::endl;

    cloth::Cloth cloth {CLOTH_HEIGHT, CLOTH_WIDTH, CLOTH_SIZE, PARTICLE_THICKNESS, state};
    cloth.GPU_send_data();
    
    render::Camera camera {glm::vec3(12.0, 3.0, 4.0), 
                           glm::vec3(-1.0, -0.3, 0.03),
                           glm::vec3(0.0, 0.0, 1.0)};
    
    Axis axis {SCR_WIDTH, SCR_HEIGHT};
    Floor floor {SCR_WIDTH, SCR_HEIGHT};

    // -------------------------------- Load model with Assimp --------------------------------
    Shader human_shader("resources/Shaders/anim_model.vert", "resources/Shaders/anim_model.frag");  //Build and compile shaders
    Model human("resources/meshes/Woman/WomanDanza3.fbx"); //Load Model
    Animation animation("resources/meshes/Woman/WomanDanza3.fbx", &human); //Animation
    Animator animator(&animation); //Animator

    BVH bvh{&human, &animation.bonePositions, camera, state}; //Create BVH structure
    
    // main render loop
    while(!glfwWindowShouldClose(window)){
        state.update(window);
//        std::cout << "frame time: " << state.delta_time << " - (" << 1.0/state.delta_time << " FPS)" << std::endl;
        
        processInput(window, state, camera);
        
        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // per vedere le linee dei triangoli

        cloth.proces_input(window);
//        if(state.current_time_from_start > 5){
//            cloth.unpin2();
//        }
//        if(state.current_time_from_start > 5){
//            cloth.unpin1();
//        }
    
//        if(state.sym_type == GPU){
//            cloth.GPU_retrieve_data();
//            cloth.GPU_send_data();
//        }
        animator.UpdateAnimation(state.delta_time); //Update animation
        bvh.modify(animator.GetBonePositions(), human.GetBoneCount()); //Update BVH structure
        
        human_shader.use();

        //View and projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)state.scr_width / (float)state.scr_height, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        human_shader.setMat4("projection", projection);
        human_shader.setMat4("view", view);

        //Compute animation
        auto transforms = animator.GetFinalBoneMatrices();
        for (int i = 0; i < transforms.size(); ++i)
            human_shader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

        //Render the loaded model
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        human_shader.setMat4("model", modelMatrix);
        human.Draw(human_shader);
        
        cloth.simulate_XPBD(state, grid);

        cloth.render(camera);
        axis.render(camera);
        floor.render(camera);
        
        bvh.flush(camera);

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
