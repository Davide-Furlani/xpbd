/**
 * @file
 * @brief Contains the class Cloth.
 * @author Davide Furlani
 * @version 0.1
 * @date January, 2023
 * @copyright 2023 Davide Furlani
 */

#pragma once
#include <vector>
#include "node/node.h"
#include "constraints/constraint.h"
#include "state/state.h"
#include "display/shader.h"
#include "display/camera.h"

namespace cloth{
class Cloth
{
public:
    // physics
    float node_mass = 1.0f;
    float node_thickness;
    std::vector<Node> nodes;
    float self_friction = 0.2f;
    std::vector<Constraint> s_cs;
    std::vector<Constraint> b_cs;
    int pin1_index;
    int pin2_index;
    
    // rendering
    
    /**
     * lista degli indici di tutti i nodi duplicati per ogni triangolo per poi mandarli al rendering
     */
    std::vector<int> verts;
    
    unsigned VAO, VBO;
    Shader shader {"resources/Shaders/ClothVS.glsl", "resources/Shaders/ClothFS.glsl"};
    unsigned int texture;
    
    GLuint ssbo_verts;
    Shader compute_predict {"resources/gpu_kernels/next_predict_pos.comp"};
    
    
    //temporaneo
    int rows;
    int columns;
    
    struct triangle_struct{
        int a,b,c;
    };
    std::vector<triangle_struct> up_left_tris;
    std::vector<triangle_struct> low_right_tris;
    std::vector<triangle_struct> all_tris;
    // fine temporaneo
    
    Cloth(int rows, int columns, float size, float thickness, render::State& s);
    
    void generate_stretch_constraints(render::State& s);
    void generate_bend_constraints(render::State& s);
    
    void pin1(int index);
    void pin2(int index);
    void unpin1();
    void unpin2();
    
    void proces_input(GLFWwindow *window);
    
    void generate_verts();
    std::vector<float> get_GL_tris();
    
    void compute_normals();
    void render(render::Camera& c);
    void free_resources();
    
    void simulate_XPBD (render::State& s);
    void XPBD_predict(float t, glm::vec3 g, float max_velocity);
    void XPBD_solve_constraints(float t);
    void XPBD_update_velocity(float t);
    void XPBD_solve_stretching(float t);
    void XPBD_solve_bending(float t);
    
    void GPU_predict(float t, glm::vec3 g, float max_velocity);

    void TMP_solve_ground_collisions();
    
    void updateHashGrid(render::State& s);
    void queryAll(render::State& s, float max_travel_distance);
    void HG_solve_collisions();

};

}