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
#include "constraints/s_constr.h"
#include "constraints/b_constr.h"
#include "state/state.h"
#include "display/shader.h"
#include "display/camera.h"

namespace cloth{
class Cloth
{
public:
    // physics
    std::vector<Node> nodes;
    //float damping = 0.9999;
    std::vector<StretchConstraint> s_cs;
    std::vector<BendConstraint> b_cs;
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
    
    void pin1(int index);
    void pin2(int index);
    
    void proces_input(GLFWwindow *window);
    void unpin1();
    void unpin2();
    
    void generate_verts();
    
    std::vector<float> get_GL_tris();
    
    void generate_stretch_constraints();
    void generate_bend_constraints();
    
    void compute_normals();
    void render(render::Camera& c);

    
    void TMP_solve_ground_collisions();
    void simulate_XPBD (render::State& s);
    void XPBD_predict(float t, glm::vec3 g);
    void XPBD_solve_constraints(float t, render::State& s);
    void XPBD_update_velocity(float t);
    void XPBD_solve_stretching(float t, render::State& s);
    void XPBD_solve_bending(float t);

    void free_resources();
};

}