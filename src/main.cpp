#include <iostream>

#include <glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include "cloth/cloth.h" // TODO commenta
#include "cloth/ClothModel.h"
#include "display/display.h"
#include "state/state.h"
#include "display/camera.h"
#include "display/axis.h"
#include "display/floor.h"
#include "hashgrid/hashgrid.h"

#include "animation/animator.h"

#include "bvh/BVH.h"


constexpr unsigned int SCR_WIDTH = 1080;
constexpr unsigned int SCR_HEIGHT = 720;

constexpr unsigned int CLOTH_WIDTH  = 24;
constexpr unsigned int CLOTH_HEIGHT = 24;
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


    
    render::Camera camera {glm::vec3(0.0, -3.0, 12.0), 
                           glm::vec3(0.0, 0.04, -1),
                           glm::vec3(0.0, 1.0, 0.0)};
    
    Axis axis {SCR_WIDTH, SCR_HEIGHT};
    Floor floor {SCR_WIDTH, SCR_HEIGHT};

    // -------------------------------- Load model with Assimp --------------------------------
    Shader human_shader("resources/Shaders/anim_model.vert", "resources/Shaders/anim_model.frag");  //Build and compile shaders
    Model human("resources/meshes/Woman/WomanDanza3.fbx"); //Load Model
    Animation animation("resources/meshes/Woman/WomanDanza3.fbx", &human); //Animation
    Animator animator(&animation); //Animator



    Model model_cloth("resources/meshes/Top/Top3.obj"); //Load model
    ClothModel cloth_top(&model_cloth, &human, glm::vec3(0.09f, -3.4f, -0.7f), glm::vec3(6.76f, 6.9f, 6.2f), 0.1f, 8, 9, 10); //Convert model into cloth
    cloth_top.init_render_data(camera, state);
//    cloth.GPU_send_data();

//    Model model_cloth("resources/meshes/Tshirt/TshirtTposeBig.obj");
//    ClothModel cloth_top(&model_cloth, &human, glm::vec3(0.0f, -1.5f, 0.7f), glm::vec3(0.044f, 0.034f, 0.042f), 0.2f); //Convert model into cloth
//    cloth_top.init_render_data(camera, state);


    // -------------------------------- Cloth --------------------------------    
   
//    Model model_cloth2(FileSystem::getPath(getClothPath())); //Load model
//    ClothModel cloth2(&model_cloth2, &human, getTranslation(), getScale()); //Convert model into cloth
//    ClothRenderModel clothRender2(&cloth2); //Render cloth
//    ClothSpringRender clothSpringRender2(&cloth2); //Render springs

    BVH bvh{&human, &animation.bonePositions, camera, state}; //Create BVH structure

//-------------------------------------------------------------------------------------------------
    GLuint ssbo;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec3) * human.meshes[0].vertices.size(), nullptr, GL_DYNAMIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    std::vector<glm::vec3> gpuResult;
    gpuResult.resize(human.meshes[0].vertices.size());
    
//-------------------------------------------------------------------------------------------------

    
    // main render loop
    while(!glfwWindowShouldClose(window)){
        state.update(window);
        std::cout << "frame time: " << state.delta_time << " - (" << 1.0/state.delta_time << " FPS)" << std::endl;
        
        processInput(window, state, camera);
        
        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // per vedere le linee dei triangoli

        cloth_top.proces_input(window);

    
//        if(state.sym_type == GPU){
//            cloth.GPU_retrieve_data();
//            cloth.GPU_send_data();
//        }
//        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
            animator.UpdateAnimation(state.simulation_step_time); //Update animation
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
        

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, ssbo);
        human.Draw(human_shader);

        //Save data from GPU (Linear blend skinning)
        glm::vec3* output = (glm::vec3*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
        memcpy(gpuResult.data(), output, sizeof(glm::vec3) * gpuResult.capacity());
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        //Update coordinates
        for (int j = 0; j < gpuResult.size(); j++)
            human.meshes[0].vertices[j].Position = gpuResult[j];

//        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        cloth_top.simulate_XPBD(state, grid, bvh);
//        }
        
        

        cloth_top.flush(camera, state);
        axis.render(camera);
        floor.render(camera);
        
       // bvh.flush(camera);

        glfwSwapBuffers(window);
//        glFlush(); // no framerate max
        glfwPollEvents();
        
//        std::cout.precision(1);
//        std::cout << std::fixed << 1/state.delta_time << " \t";
//        std::cout.precision(3);
//        std::cout << std::fixed << state.delta_time << "\t";
//        std::cout << cloth.all_tris.size() << "\t" << cloth.nodes.size() << "\n";

    }


    cloth_top.destroy();
    axis.free();
    floor.free();
    glfwTerminate();

    return 0;
}
