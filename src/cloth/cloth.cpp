/**
 * @file
 * @brief Contains the implementation of class Cloth.
 * @author Davide Furlani
 * @version 0.1
 * @date January, 2023
 * @copyright 2023 Davide Furlani
 */
#include <vector>
#include <filesystem>
#include <cmath>
#include <algorithm>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include "node/node.h"
#include "cloth.h"
#include "constraints/constraint.h"
#include "display/display.h"
#include "state/state.h"
#include "omp.h"

#define N_SSBO 0
#define CC_SSBO 1
#define JC_SSBO 2

#define COLORING 0
#define JACOBI 1
#define HYBRID 2

int solve_type = JACOBI;

namespace cloth {
    
    Cloth::Cloth(int rows, int columns, float size, float thickness, render::State& s){
        Cloth::rows = rows;
        Cloth::columns = columns;
        
        node_thickness = thickness;
        pin1_index = 0;
        pin2_index = columns-1;
        
        float z_constant = 3.0;
        vec3 vel {0.0};
        vec3 normal {0.0, 0.0, 1.0};
        
        size = 1/size;
        for(int i=0; i<rows; ++i){
            for(int j=0; j<columns; ++j){
                vec3 pos {static_cast<float>(j)/(size*static_cast<float>(columns-1)), static_cast<float>(i)/(size*static_cast<float>(columns-1)), z_constant};
                vec2 uv_c {static_cast<float>(j)/static_cast<float>(columns-1), static_cast<float>(i)/static_cast<float>(rows-1)};
                nodes.emplace_back(Node(pos, node_thickness, node_mass, vel, normal, uv_c));
            }
        }

        pin1(pin1_index);
        pin2(pin2_index);

        generate_verts();

        generate_constraints(s);
        
        find_max_node_cardinality();
        
        // dati per rendering

        std::vector<float> cloth_verts_data = get_GL_tris();
        
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, cloth_verts_data.size() * sizeof(float), &cloth_verts_data.front(), GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6*sizeof(float)));
        glEnableVertexAttribArray(2);


        std::filesystem::path texture_p {"resources/Textures/tex1.jpg"};
        texture = render::load_textures(texture_p);

        this->cloth_shader.use();

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)s.scr_width / (float)s.scr_height, 0.1f, 100.0f);
        this->cloth_shader.setMat4("uniProjMatrix", projection);

        this->cloth_shader.setInt("uniTex", 0);
        this->cloth_shader.setVec3("uniLightPos", glm::vec3(0.0, 0.0, 1.0));
        this->cloth_shader.setVec3("uniLightColor", glm::vec3(1.0, 1.0, 1.0));
        
        // buffer per compute shaders
        // nodi
        glGenBuffers(1, &(this->ssbo_nodes));
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->ssbo_nodes);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Node)*this->nodes.size(), this->nodes.data(), GL_DYNAMIC_READ);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, N_SSBO, this->ssbo_nodes);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        
        // coloring constraint
        glGenBuffers(1, &(this->ssbo_cconstrs));
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->ssbo_cconstrs);
        std::vector<Constraint> sets_unrolled = {};
        for(auto set : this->constr_sets){
            for(auto c : set){
                sets_unrolled.push_back(c);
            }
        }
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Constraint)*sets_unrolled.size(), sets_unrolled.data(), GL_STATIC_READ);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, CC_SSBO, this->ssbo_cconstrs);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        
//        std::cout << this->constr_sets.size() << ", " << sets_unrolled.size() << std::endl;

        // jacobi constraints
        glGenBuffers(1, &(this->ssbo_jconstrs));
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->ssbo_jconstrs);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Constraint)*this->constraints.size(), this->constraints.data(), GL_STATIC_READ);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, JC_SSBO, this->ssbo_jconstrs);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        
        
    }
    
    void Cloth::generate_constraints(render::State& s){
        generate_stretch_constraints(s);
        generate_bend_constraints(s);
        constraints_coloring();
    }
    
    void Cloth::generate_stretch_constraints(render::State& s) {

        for (auto t : up_left_tris){
            this->constraints.emplace_back(nodes, t.a, t.b, this->stretching_compliance);
//            this->constraints.emplace_back(nodes, t.b, t.c, s.stretching_compliance); // questo mi crea i constraints oblicui e mi rompe tutto
            this->constraints.emplace_back(nodes, t.a, t.c, this->stretching_compliance);
        }

        for(int i=columns-1; i<columns*(rows-1); i+=columns){
            this->constraints.emplace_back(nodes, i, i+columns, this->stretching_compliance);
        }
        for(int i=columns*(rows-1); i<(rows*columns)-1; ++i){
            this->constraints.emplace_back(nodes, i, i+1, this->stretching_compliance);
        }
    }
    
    void Cloth::generate_bend_constraints(render::State& s) {

        for(int i=0; i<rows-1; ++i){
            for(int j=0; j<columns-1; ++j){
                this->constraints.emplace_back(nodes, i*columns+j, (i*columns+j)+(columns+1), this->bending_compliance);
            }
        }
        for(int i=1; i<rows; ++i){
            for(int j=0; j<columns-2; ++j){
                this->constraints.emplace_back(nodes, i*columns+j, (i*columns+j)-(columns-2), this->bending_compliance);
            }
        }
        for(int i=0; i<rows-2; ++i){
            for(int j=1; j<columns; ++j){
                this->constraints.emplace_back(nodes, i*columns+j, (i*columns+j)+(2*columns-1), this->bending_compliance);
            }
        }
    }
    
    void Cloth::constraints_coloring(){
                
        std::vector<bool> node_marks(this->nodes.size(), false);
        std::vector<bool> constr_marks(this->constraints.size(), false);
        
        
        while(std::find(constr_marks.begin(), constr_marks.end(), false) != constr_marks.end()) {
            std::vector<Constraint> set;
            for (int i = 0; i < node_marks.size(); i++) {
                node_marks.at(i) = false;
            }
            for (int i=0; i<this->constraints.size(); i++) {
                if(constr_marks.at(i) == true)
                    continue;
                if (node_marks.at(this->constraints.at(i).a_node) == false && node_marks.at(this->constraints.at(i).b_node) == false){
                    set.push_back(this->constraints.at(i));
                    constr_marks.at(i) = true;
                    node_marks.at(this->constraints.at(i).a_node) = true;
                    node_marks.at(this->constraints.at(i).b_node) = true;
                }
            }
            this->constr_sets.push_back(set);
        }        
    }

    void Cloth::find_max_node_cardinality(){

        std::vector<int> node_cardinalities(this->nodes.size());
        
        for(auto &c : this->constraints) {
            node_cardinalities.at(c.a_node)++;
            node_cardinalities.at(c.b_node)++;
        }

        this->max_node_cardinality = 0;
        for(auto n : node_cardinalities){
            if (n > this->max_node_cardinality)
                this->max_node_cardinality = n;
        }
                
    }
    
    void Cloth::pin1(int index) {
        nodes.at(pin1_index).m = std::numeric_limits<float>::infinity();
        nodes.at(pin1_index).w = 0.0;
        pin1_index = index;
    }
    void Cloth::pin2(int index) {
        nodes.at(pin2_index).m = std::numeric_limits<float>::infinity();
        nodes.at(pin2_index).w = 0.0;
        pin2_index = index;
    }
    void Cloth::unpin1() {
        nodes.at(pin1_index).m = node_mass;
        nodes.at(pin1_index).w = 1.0/node_mass;
    }
    void Cloth::unpin2() {
        nodes.at(pin2_index).m = 1.0;
        nodes.at(pin2_index).w = 1.0;
    }    

    void Cloth::proces_input(GLFWwindow *window){
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
            unpin1();
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
            unpin2();
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS){
                solve_type = COLORING;
                std::cout << "Coloring" << std::endl;
        }
        if(glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS){
                solve_type = JACOBI;
                std::cout << "Jacobi" << std::endl;
        }
        if(glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS){
            solve_type = HYBRID;
            std::cout << "Hybrid" << std::endl;
        }
    }

    void Cloth::generate_verts() {
            
        //up_left
        for(int i=0; i<rows-1; ++i){
            for(int j=0; j<columns-1; ++j){
                int v1 = (i * columns) + j;
                int v2 = v1 + columns;
                int v3 = v1 + 1;
                up_left_tris.emplace_back(triangle_struct{v1, v2, v3});
            }
        }
        
        //low_right
        for(int i=1; i<rows; ++i){
            for(int j=0; j<columns-1; ++j){
                int v1 = (i * columns) + j;
                int v2 = v1 + 1;
                int v3 = v2 - columns;
                low_right_tris.emplace_back(triangle_struct{v1, v2, v3});
            }
        }
        
        all_tris.insert(all_tris.end(), up_left_tris.begin(), up_left_tris.end());
        all_tris.insert(all_tris.end(), low_right_tris.begin(), low_right_tris.end());
        
        for(auto t : up_left_tris){
            verts.emplace_back(t.a);
            verts.emplace_back(t.b);
            verts.emplace_back(t.c);
        }
        for(auto t : low_right_tris){
            verts.emplace_back(t.a);
            verts.emplace_back(t.b);
            verts.emplace_back(t.c);
        }
    }
    
    std::vector<float> Cloth::get_GL_tris() {
        std::vector<float> v_array {};
          
        for(auto i : verts){
            Node& n = nodes[i];
            v_array.emplace_back(n.pos.x);
            v_array.emplace_back(n.pos.y);
            v_array.emplace_back(n.pos.z);
            v_array.emplace_back(n.n.x);
            v_array.emplace_back(n.n.y);
            v_array.emplace_back(n.n.z);
            v_array.emplace_back(n.uv_c.x);
            v_array.emplace_back(n.uv_c.y);

        }
        return v_array;
    }
    
    void Cloth::compute_normals() {
        /** Reset nodes' normal **/
        vec3 normal(0.0, 0.0, 0.0);
        for (auto& n : nodes) {
            n.n = normal;
        }
        
        for(auto t : up_left_tris){
            Node& n1 = nodes[t.a];
            Node& n2 = nodes[t.b];
            Node& n3 = nodes[t.c];
            normal = cross(n2.pos - n1.pos, n3.pos - n1.pos);
            n1.n += normal;
            n2.n += normal;
            n3.n += normal;
        }
        for(auto t : low_right_tris){
            Node& n1 = nodes[t.a];
            Node& n2 = nodes[t.b];
            Node& n3 = nodes[t.c];
            normal = cross(n2.pos - n1.pos, n3.pos - n1.pos);
            n1.n += normal;
            n2.n += normal;
            n3.n += normal;
        }
        
        for (auto& n : nodes) {
            n.n = normalize(n.n);
        }
    }
    
    void Cloth::render(render::Camera& c) {
        
        compute_normals();
        std::vector<float> cloth_verts_data = get_GL_tris();
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, cloth_verts_data.size() * sizeof(float), &cloth_verts_data.front(), GL_DYNAMIC_DRAW);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        this->cloth_shader.use();
        mat4 view = lookAt(c.pos, c.pos + c.front_v, c.up_v);
        this->cloth_shader.setMat4("uniViewMatrix", view);
        glBindVertexArray(VAO);

        mat4 model = mat4(1.0f);
        this->cloth_shader.setMat4("uniModelMatrix", model);
        glDrawArrays(GL_TRIANGLES, 0, cloth_verts_data.size()/8.0);
        
    }
    
    void Cloth::free_resources() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &ssbo_nodes);
        glDeleteBuffers(1, &ssbo_cconstrs);
        glDeleteBuffers(1, &ssbo_jconstrs);
        this->cloth_shader.destroy();
        this->compute_predict.destroy();
        this->TMP_compute_ground_collisions.destroy();
        this->compute_solve_coloring_constraints.destroy();
        this->compute_solve_jacobi_constraints.destroy();
        this->compute_jacobi_add_correction.destroy();
        this->compute_update_velocities.destroy();
    }
    
    void Cloth::simulate_XPBD(render::State& s, hashgrid::HashGrid& grid) {
        if(s.sym_type == CPU)
            CPU_SIM(s, grid);
        if(s.sym_type == GPU)
            GPU_SIM(s, grid);
    }

    void Cloth::CPU_SIM(render::State& s, hashgrid::HashGrid& grid){

        float time_step = s.simulation_step_time/s.iteration_per_frame;
        float max_velocity = (0.5f * node_thickness) / time_step; // da tweakkare, più piccolo = meno possibili collisioni = simulazione più veloce
        float max_travel_distance = max_velocity * s.simulation_step_time;
        updateHashGrid(grid);
        queryAll(grid, max_travel_distance);

        for(int i=0; i< s.iteration_per_frame; ++i){
            XPBD_predict(time_step, s.gravity, max_velocity);
            XPBD_solve_ground_collisions();
            XPBD_solve_constraints(time_step);
            HG_solve_collisions();
            XPBD_update_velocity(time_step);
        }
    }
    
    void Cloth::GPU_SIM(render::State& s, hashgrid::HashGrid& grid){

        float time_step = s.simulation_step_time/s.iteration_per_frame;
        float max_velocity = (0.5f * node_thickness) / time_step; // da tweakkare, più piccolo = meno possibili collisioni = simulazione più veloce
        float max_travel_distance = max_velocity * s.simulation_step_time;
        updateHashGrid(grid);
        queryAll(grid, max_travel_distance);
        GPU_send_data();
        for(int i=0; i< s.iteration_per_frame; ++i){
            GPU_XPBD_predict(time_step, s.gravity, max_velocity);
            GPU_solve_ground_collisions();
            if(solve_type == COLORING) {
                GPU_XPBD_solve_constraints_coloring(time_step, 0);
            }else if(solve_type == JACOBI) {
                GPU_XPBD_solve_constraints_jacobi(time_step, 0);
                GPU_XPBD_add_jacobi_correction();
            }else if (solve_type == HYBRID) {
                GPU_XPBD_solve_constraints_coloring(time_step, 4);
                GPU_XPBD_solve_constraints_jacobi(time_step, 4);
                GPU_XPBD_add_jacobi_correction();
            }
            GPU_retrieve_data();
            HG_solve_collisions(); // TODO buttarlo in GPU -----------------------------------------------------
            GPU_send_data();
            GPU_XPBD_update_velocity(time_step);
        }
        GPU_retrieve_data();
    }
    
    void Cloth::XPBD_predict(float t, glm::vec3 g, float max_velocity){
        /** Nodes **/
        for (auto& n : nodes) {
            if (n.w == 0.0)
                continue;

            n.vel += g * t;
            float tmp_vel = sqrt(n.vel.x*n.vel.x + n.vel.y*n.vel.y + n.vel.z*n.vel.z); //forzo la velocità massima
            if(tmp_vel>max_velocity)                                                   //
                n.vel *= max_velocity/tmp_vel;                                         //
            n.prev_pos = n.pos;
            n.pos += n.vel * t;
        }
    }

    void Cloth::XPBD_solve_ground_collisions() {
        for (auto& n : nodes) {
            if (n.w == 0.0)
                continue;
            if (n.pos.z < 0.5*n.thickness) {
                float damping = 0.02;
                vec3 diff = n.pos - n.prev_pos;
                n.pos += diff * -damping;
                n.pos.z = 0.5f * n.thickness;
            }
        }
    }
    
    void Cloth::XPBD_solve_constraints(float time_step){
        
        
        for (auto& c : this->constraints) {
            float alpha = c.compliance / time_step / time_step;
            
            if (this->nodes[c.a_node].w + this->nodes[c.b_node].w == 0.0)
                continue;
            
            vec3 distance = this->nodes[c.a_node].pos - this->nodes[c.b_node].pos;
            float abs_distance = sqrtf(powf(distance.x,2) + powf(distance.y,2) + powf(distance.z,2));

            if (abs_distance == 0.0)
                continue;
            distance = distance * (1 / abs_distance);

            float rest_len = c.rest_dist;
            float error = abs_distance - rest_len;
            float correction = -error / (this->nodes[c.a_node].w + nodes[c.b_node].w + alpha);

            float first_corr = correction * this->nodes[c.a_node].w;
            float second_corr = -correction * this->nodes[c.b_node].w;

            this->nodes[c.a_node].pos += distance * first_corr;
            this->nodes[c.b_node].pos += distance * second_corr;
        }
    }
    
    void Cloth::XPBD_update_velocity(float t) {
        /** Nodes **/
        for (auto& n : nodes) {
            if (n.w == 0.0)
                continue;
            n.vel = (n.pos - n.prev_pos) * 1.0f/t;
        }
    }

    void Cloth::GPU_send_data(){
        //nodes
        glNamedBufferSubData(this->ssbo_nodes, 0, sizeof(Node)*this->nodes.size(), this->nodes.data());

    }

    void Cloth::GPU_retrieve_data(){
        //nodes
        glGetNamedBufferSubData(this->ssbo_nodes, 0, sizeof(Node)*this->nodes.size(), this->nodes.data());
    }
    
    void Cloth::GPU_XPBD_predict(float time_step, glm::vec3 gravity, float max_velocity) {
        
        this->compute_predict.setVec3("gravity", gravity);
        this->compute_predict.setFloat("time_step", time_step);
        this->compute_predict.setFloat("max_velocity", max_velocity);
        
        this->compute_predict.use();
        glDispatchCompute(std::ceil(this->nodes.size()/32), 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }
    
    void Cloth::GPU_solve_ground_collisions() {

        this->TMP_compute_ground_collisions.use();
        glDispatchCompute(std::ceil(this->nodes.size()/32), 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    }
    
     void Cloth::GPU_XPBD_solve_constraints_coloring(float time_step, int passes){
        
        this->compute_solve_coloring_constraints.setFloat("time_step", time_step);

        int set_start = 0;
        int set_end = 0;
        
        if(passes == 0)
            passes = (int)constr_sets.size();
        
        for(int i=0; i<passes; i++){
            set_start = set_end;
            set_end += (int)constr_sets[i].size();
            this->compute_solve_coloring_constraints.setInt("set_start", set_start);
            this->compute_solve_coloring_constraints.setInt("set_end", set_end);
            
            this->compute_solve_coloring_constraints.use();
            glDispatchCompute(std::ceil(constr_sets[i].size()/32), 1, 1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        }
        
    }

    void Cloth::GPU_XPBD_solve_constraints_jacobi(float time_step, int coloring_passes){

        this->compute_solve_jacobi_constraints.setFloat("time_step", time_step);
        
        int first_constr = 0;
        for(int i=0; i<coloring_passes; i++){
            first_constr += (int)constr_sets[i].size();
        }
        this->compute_solve_jacobi_constraints.setInt("first_constr", first_constr);

        this->compute_solve_jacobi_constraints.use();
        glDispatchCompute(std::ceil((this->constraints.size()-first_constr)/32), 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    }

    void Cloth::GPU_XPBD_add_jacobi_correction(){
        float magic_number = 0.3;
        this->compute_jacobi_add_correction.setFloat("magic_number", magic_number);
        
        this->compute_jacobi_add_correction.use();
        glDispatchCompute(std::ceil(this->nodes.size()/32), 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }
    
    void Cloth::GPU_XPBD_update_velocity (float time_step){
        this->compute_update_velocities.setFloat("time_step", time_step);
        
        this->compute_update_velocities.use();
        glDispatchCompute(std::ceil(this->nodes.size()/32), 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }
    
    void Cloth::updateHashGrid(hashgrid::HashGrid& grid){
                
        for(auto& cell : grid.cells){
            cell = 0;
        }

        for(auto& n: this->nodes){
            grid.cells[grid.hashIndex(n.pos)]++; //incremento il contatore di particelle della cella in cui sta il nodo
        }

        for(int i=1; i<grid.cells.size(); i++){
            grid.cells[i] += grid.cells[i-1];  // riempio tutte le celle della hashgrid
        }

        for(int i=0; i<this->nodes.size(); i++){

            int hash_index = grid.hashIndex(this->nodes[i].pos);

            grid.cells[hash_index]--;                            // sistemo i puntatori delle celle
            grid.nodes_indexes[grid.cells[hash_index]] = i;   // metto le particelle nel loro array
        }


    }
    
    void Cloth::queryAll(hashgrid::HashGrid& grid, float max_travel_distance) {
        float max_dist_2 = max_travel_distance * max_travel_distance;
        #pragma omp parallel for
        for(int node=0; node<this->nodes.size(); node++){
            int min_x = grid.intCoord(this->nodes[node].pos.x - max_travel_distance);
            int max_x = grid.intCoord(this->nodes[node].pos.x + max_travel_distance);
            int min_y = grid.intCoord(this->nodes[node].pos.y - max_travel_distance);
            int max_y = grid.intCoord(this->nodes[node].pos.y + max_travel_distance);
            int min_z = grid.intCoord(this->nodes[node].pos.z - max_travel_distance);
            int max_z = grid.intCoord(this->nodes[node].pos.z + max_travel_distance);

            this->nodes[node].neighbours.clear();

            
            for(int x=min_x; x<max_x; ++x)              // qui scorro tutte le celle in cui posso trovare particelle
                for(int y=min_y; y<max_y; ++y)          // con cui la particella in esame potrebbe collidere
                    for(int z=min_z; z<max_z; ++z){     // la complessità è cubica nel raggio di possibile collisione (max_travel_distance)
                        int hashcoor = grid.hashCoords(x, y, z);
                        for (int i=grid.cells[hashcoor]; i< grid.cells[hashcoor+1]; i++){
                            if (i == node)
                                continue;
                            vec3 dist = this->nodes[node].pos - this->nodes[grid.nodes_indexes[i]].pos;
                            float dist_2 = (powf(dist.x,2) + powf(dist.y,2) + powf(dist.z,2));
                            if (dist_2 <=max_dist_2)
                                this->nodes[node].neighbours.push_back(grid.nodes_indexes[i]);
                        }
                    }
        }
    }
    void Cloth::HG_solve_collisions(){
        float thickness_2 = this->node_thickness * this->node_thickness;

        #pragma omp parallel for
        for(auto& n: nodes) {
            if (n.w == 0.0)
                continue;
            for(int n_idx: n.neighbours){
                
                if(this->nodes[n_idx].w == 0.0) {
                    continue;
                }

                vec3 diff = this->nodes[n_idx].pos - n.pos;
                float dist_2 = diff.x*diff.x + diff.y*diff.y + diff.z*diff.z;

                if(dist_2 > thickness_2 || dist_2 == 0.0) {
                    continue;
                }

                vec3 nat_dist = n.nat_pos - this->nodes[n_idx].nat_pos;
                float nat_dist_2 = nat_dist.x*nat_dist.x + nat_dist.y*nat_dist.y + nat_dist.z*nat_dist.z;
                if(dist_2 > nat_dist_2) {
                    continue;
                }


                float min_dist = n.thickness;
                if(nat_dist_2 < thickness_2)
                    min_dist = sqrtf(nat_dist_2);

                float dist = sqrtf(dist_2);
                vec3 correction = diff * ((min_dist-dist)/dist);
                n.pos += correction * (-0.5f);
                this->nodes[n_idx].pos += correction * (0.5f);

                vec3 n1_vel = n.pos - n.prev_pos;
                vec3 n2_vel = this->nodes[n_idx].pos - this->nodes[n_idx].prev_pos;

                vec3 av_vel = (n1_vel + n2_vel) * 0.5f;

                n1_vel = av_vel - n1_vel;
                n2_vel = av_vel - n2_vel;

                n.pos += n1_vel * this->self_friction;
                this->nodes[n_idx].pos += n2_vel * this->self_friction;
            }
        }
    }
    
}

























