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
#include "hashgrid//hashgrid.h"
#include "display/shader.h"
#include "display/camera.h"

namespace cloth{
class Cloth
{
public:
    // physics
    float node_mass = 0.01f;
    float node_thickness;
    float stretching_compliance = 0.0f;
    float bending_compliance = 0.000f;
    std::vector<Node> nodes;
    float self_friction = 0.2f;
    std::vector<Constraint> constraints;
    std::vector<std::vector<Constraint>> constr_sets;
    int pin1_index;
    int pin2_index;
    int max_node_cardinality;
    
    // rendering
    
    /**
     * lista degli indici di tutti i nodi duplicati per ogni triangolo per poi mandarli al rendering
     */
    std::vector<int> verts;
    
    unsigned VAO, VBO;
    Shader cloth_shader {"resources/Shaders/ClothVS.glsl", "resources/Shaders/ClothFS.glsl"};
    unsigned int texture;
    
    GLuint ssbo_nodes;
    GLuint ssbo_cconstrs;
    GLuint ssbo_jconstrs;
//    GLuint ssbo_grid_cells;
//    GLuint ssbo_grid_nodes;
//    GLuint ssbo_grid_neighbours;
    Shader compute_predict {"resources/gpu_kernels/next_predict_pos.comp"};
    Shader TMP_compute_ground_collisions {"resources/gpu_kernels/ground_collisions.comp"};
    Shader compute_solve_coloring_constraints {"resources/gpu_kernels/solve_coloring_constraints.comp"};
    Shader compute_solve_jacobi_constraints {"resources/gpu_kernels/solve_jacobi_constraints.comp"};
    Shader compute_jacobi_add_correction {"resources/gpu_kernels/jacobi_add_correction.comp"};
//    Shader compute_HG_collisions {""};
    Shader compute_update_velocities {"resources/gpu_kernels/update_velocities.comp"};
    
    
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
    
    
    void generate_constraints(render::State& s);
    void generate_stretch_constraints(render::State& s);
    void generate_bend_constraints(render::State& s);
    void constraints_coloring();
    void find_max_node_cardinality();
    
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
    
    void simulate_XPBD (render::State& s, hashgrid::HashGrid& grid);
    void XPBD_predict(float t, glm::vec3 g, float max_velocity);
    void XPBD_solve_ground_collisions();
    void XPBD_solve_constraints(float t);
    void XPBD_update_velocity(float t);

    void GPU_send_data();
    void GPU_retrieve_data();
    
    void GPU_XPBD_predict(float t, glm::vec3 g, float max_velocity);
    void GPU_solve_ground_collisions();
    void GPU_XPBD_solve_constraints_coloring(float t, int passes);
    void GPU_XPBD_solve_constraints_jacobi(float t, int coloring_passes);
    void GPU_XPBD_add_jacobi_correction();
    void GPU_XPBD_update_velocity (float time_step);
    
    void updateHashGrid(hashgrid::HashGrid& grid);
    void queryAll(hashgrid::HashGrid& grid, float max_travel_distance);
    void HG_solve_collisions();
  private:
    void CPU_SIM(render::State& s, hashgrid::HashGrid& grid);
    void GPU_SIM(render::State& s, hashgrid::HashGrid& grid);
    
};

}