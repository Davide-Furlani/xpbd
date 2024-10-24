/**
 * @file
 * @brief Contains the class Cloth.
 * @author Davide Furlani
 * @version 0.1
 * @date September, 2024
 * @copyright 2024 Davide Furlani
 */

#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <filesystem>
#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <learnopengl/shader.h>
#include <learnopengl/model.h>
#include "node.h"
#include "constraint.h"
#include "state.h"
#include "hashgrid.h"
#include "camera.h"
#include "display.h"




namespace cloth {
    class Cloth {
    public:
        // physics
        float node_mass = 0.01f;
        float node_thickness = 0.01f;
        float stretching_compliance = 0.0f;
        float bending_compliance = 0.03f;
        float self_friction = 0.2f;

        std::map<int, std::vector<int>> same_verts;

        std::vector<Node> nodes;
        std::vector<Constraint> constraints;
        std::vector<glm::ivec3> tris;

        Model model;

        Shader shader;
        unsigned int texture;


        Cloth(std::filesystem::path &model_path, std::pair<char const*, char const*> &shader_paths, std::filesystem::path &texture_p, render::State &state): model{Model(model_path)}, shader{Shader(shader_paths.first, shader_paths.second)}{

            if(this->model.meshes.size() > 1){
                std::cerr << "Cloth require a model with a single mesh, you tried to load a model with " << model.meshes.size() << " meshes" << std::endl;
                return;
            }
            
            //imposto le shader
            this->texture = render::load_textures(texture_p);
            this->shader.use();
            glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)state.scr_width / (float)state.scr_height, 0.1f, 100.0f);
            this->shader.setMat4("uniProjMatrix", projection);
            this->shader.setInt("uniTex", 0);
            this->shader.setVec3("uniLightPos", glm::vec3(1.0, 1.0, 4.0));
            this->shader.setVec3("uniLightColor", glm::vec3(1.0, 1.0, 1.0));


            // creazione dei nodi
            for (int i=0; i<model.meshes[0].vertices.size(); i++) {

                int original_vert = find_same_vert(i, model.meshes[0]);
                // se il vertice è unico rispetto a quelli già analizzati
                if (original_vert == -1) {
                    // preparo la lista per i successivi vertici
                    this->same_verts[nodes.size()] = {i};
                    // creo il nodo
                    glm::vec3 pos = model.meshes[0].vertices[i].Position;
                    glm::vec3 vel{0.0};
                    glm::vec3 normal{0.0, 0.0, 1.0};
                    glm::vec2 uv_c{0.0, 0.0};
                    nodes.emplace_back(pos, node_thickness, node_mass, vel, normal, uv_c);
                } else { // se ho già trovato un vertice con la stessa posizione
                    this->same_verts[original_vert].push_back(i); // lo aggiungo alla lista corretta
                }
            }

            // creazione dei stretching constraints
            // todo! rimuovere i constraint duplicati
            for(int first_v_of_tri=0; first_v_of_tri < model.meshes[0].indices.size(); first_v_of_tri+=3){

                int n_a = find_corresponding_node(model.meshes[0].indices[first_v_of_tri]);
                int n_b = find_corresponding_node(model.meshes[0].indices[first_v_of_tri+1]);
                int n_c = find_corresponding_node(model.meshes[0].indices[first_v_of_tri+2]);

                if (n_a == -1 || n_b == -1 || n_c == -1)
                    break;

                this->constraints.emplace_back(nodes, n_a, n_b, this->stretching_compliance);
                this->constraints.emplace_back(nodes, n_b, n_c, this->stretching_compliance);
                this->constraints.emplace_back(nodes, n_c, n_a, this->stretching_compliance);

                this->tris.emplace_back(n_a, n_b, n_c);
            }

            // todo! creazione dei bending constraints

            pin1();
            pin2();
        }

        int find_same_vert(int i, Mesh &mesh) {

            for (auto &[node_index, same_verts_list]: this->same_verts) {
                if (mesh.vertices[i].Position.x == mesh.vertices[same_verts_list[0]].Position.x &&  //si usa il primo elemento della lista perchè è quello che per primo ha identificato un vertice diverso da quelli precedentemente controllati
                        mesh.vertices[i].Position.y == mesh.vertices[same_verts_list[0]].Position.y &&
                        mesh.vertices[i].Position.z == mesh.vertices[same_verts_list[0]].Position.z) {
                    return node_index;
                }
            }
            return -1;
        }

        int find_corresponding_node (int i){
            for(auto &pair: this->same_verts){
                for(auto vert : pair.second){
                    if(vert == i) {
                        return pair.first;
                    }
                }
            }
            std::cout << "vertex index of vertex not found in existing vertices" << std::endl;
            return -1;
        }

        void render(render::Camera& c){
            sync_nodes_positions();
            this->model.Draw(shader, &c.pos, &c.front_v, &c.up_v);
        }

        void sync_nodes_positions(){
            for(int i=0; i< this->nodes.size(); ++i){
                for (auto v: this->same_verts[i]){
                    this->model.meshes[0].vertices[v].Position = this->nodes[i].pos;
                }
            }

//            for(int i=0; i<this->model.meshes[0].indices.size(); i+=3){
//                auto &p1 = this->model.meshes[0].vertices[this->model.meshes[0].indices[i]];
//                auto &p2 = this->model.meshes[0].vertices[this->model.meshes[0].indices[i+1]];
//                auto &p3 = this->model.meshes[0].vertices[this->model.meshes[0].indices[i+2]];
//
//                glm::vec3 U = p2.Position - p1.Position;
//                glm::vec3 V = p3.Position - p1.Position;
//
//                glm::vec3 N = glm::cross(U, V);
//                p1.Normal = N;
//                p2.Normal = N;
//                p3.Normal = N;
//            }

            this->model.meshes[0].setupMesh();
        }

        void translate (glm::vec3 translation){
            for(auto &n: this->nodes){
                n.pos = n.pos + translation;
            }
        }
        void rotate (float angle, glm::vec3 axis){
            for(auto &n: this->nodes){
                axis = glm::normalize(axis);
                glm::mat4 rotMat = glm::rotate(glm::mat4(1.0), glm::radians(angle), axis);
                n.pos = glm::vec3(rotMat * glm::vec4(n.pos, 1.0));
            }
        }

        void pin1() {
            this->nodes[10].m = std::numeric_limits<float>::infinity();
            this->nodes[10].w = 0.0f;
        }
        void pin2() {
            this->nodes[20].m = std::numeric_limits<float>::infinity();
            this->nodes[20].w = 0.0f;
        }
        void unpin1() {
            this->nodes[10].m = node_mass;
            this->nodes[10].w = 1.0f/node_mass;
        }
        void unpin2() {
            this->nodes[20].m = node_mass;
            this->nodes[20].w = 1.0f/node_mass;
        }


        void simulate_XPBD(render::State& s, hashgrid::HashGrid& grid) {
            if(s.sim_type == CPU)
                CPU_SIM(s, grid);
            if(s.sim_type == GPU)
                GPU_SIM(s, grid);
        }

        void CPU_SIM(render::State& s, hashgrid::HashGrid& grid){
            float time_step = s.simulation_step_time/s.iteration_per_frame;
            float max_velocity = (0.5f * node_thickness) / time_step; // da tweakkare, più piccolo = meno possibili collisioni = simulazione più veloce
            float max_travel_distance = max_velocity * s.simulation_step_time;
            if(s.hashgrid_sim == HASHGRID) {
                updateHashGrid(grid);
                queryAll(grid, max_travel_distance);
            }

            for(int i=0; i< s.iteration_per_frame; ++i){
                CPU_XPBD_predict(time_step, s.gravity, max_velocity);
                CPU_XPBD_solve_ground_collisions();
                CPU_XPBD_solve_constraints(time_step);
                if(s.hashgrid_sim == HASHGRID)
                    HG_solve_collisions();
                CPU_XPBD_update_velocity(time_step);
            }
        }

        void CPU_XPBD_predict(float t, glm::vec3 g, float max_velocity){
            /** Nodes **/
            for (int i=0; i<this->nodes.size(); ++i) {
                auto &n = this->nodes[i];
                if (n.w == 0.0)
                    continue;

                n.vel += g * t;

                if(i==0){
                    std::cout << n.vel.x << ", " << n.vel.y << ", " << n.vel.z << ", " << std::endl;
                }

                float tmp_vel = sqrt(n.vel.x*n.vel.x + n.vel.y*n.vel.y + n.vel.z*n.vel.z); //forzo la velocità massima
                if(tmp_vel>max_velocity)                                                   //
                    n.vel *= max_velocity/tmp_vel;                                         //
                n.prev_pos = n.pos;
                n.pos += n.vel * t;
            }
        }

        void CPU_XPBD_solve_ground_collisions() {
            for (auto& n : nodes) {
                if (n.w == 0.0)
                    continue;
                if (n.pos.z < 0.5*n.thickness) {
                    float damping = 0.02;
                    glm::vec3 diff = n.pos - n.prev_pos;
                    n.pos += diff * -damping;
                    n.pos.z = 0.5f * n.thickness;
                }
            }
        }

        void CPU_XPBD_solve_constraints(float time_step){


            for (auto& c : this->constraints) {
                float alpha = c.compliance / time_step / time_step;

                if (this->nodes[c.a_node].w + this->nodes[c.b_node].w == 0.0)
                    continue;

                glm::vec3 distance = this->nodes[c.a_node].pos - this->nodes[c.b_node].pos;
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

        void CPU_XPBD_update_velocity(float t) {
            /** Nodes **/
            for (auto& n : nodes) {
                if (n.w == 0.0)
                    continue;
                n.vel = (n.pos - n.prev_pos) * 1.0f/t;
            }
        }




        void GPU_SIM(render::State& s, hashgrid::HashGrid& grid){

        }


        void updateHashGrid(hashgrid::HashGrid& grid){
#pragma omp parallel for
            for(auto& cell : grid.cells){
                cell = 0;
            }
#pragma omp parallel for
            for(auto& n: this->nodes){
                grid.cells[grid.hashIndex(n.pos)]++; //incremento il contatore di particelle della cella in cui sta il nodo
            }
//#pragma omp parallel for
            for(int i=1; i<grid.cells.size(); i++){
                grid.cells[i] += grid.cells[i-1];  // riempio tutte le celle della hashgrid
            }
#pragma omp parallel for
            for(int i=0; i<this->nodes.size(); i++){

                int hash_index = grid.hashIndex(this->nodes[i].pos);

                grid.cells[hash_index]--;                            // sistemo i puntatori delle celle
                grid.nodes_indexes[grid.cells[hash_index]] = i;   // metto le particelle nel loro array
            }


        }

        void queryAll(hashgrid::HashGrid& grid, float max_travel_distance) {
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


                for(int x=min_x; x<=max_x; ++x)              // qui scorro tutte le celle in cui posso trovare particelle
                    for(int y=min_y; y<=max_y; ++y)          // con cui la particella in esame potrebbe collidere
                        for(int z=min_z; z<=max_z; ++z){     // la complessità è cubica nel raggio di possibile collisione (max_travel_distance)
                            int hashcoor = grid.hashCoords(x, y, z);
                            for (int i=grid.cells[hashcoor]; i< grid.cells[hashcoor+1]; i++){
                                if (i == node)
                                    continue;
                                glm::vec3 dist = this->nodes[node].pos - this->nodes[grid.nodes_indexes[i]].pos;
                                float dist_2 = (powf(dist.x,2) + powf(dist.y,2) + powf(dist.z,2));
                                if (dist_2 <=max_dist_2)
                                    this->nodes[node].neighbours.push_back(grid.nodes_indexes[i]);
                            }
                        }
            }
        }

        void HG_solve_collisions(){
            float thickness_2 = this->node_thickness * this->node_thickness;

#pragma omp parallel for
            for(auto& n: nodes) {
                if (n.w == 0.0)
                    continue;
                for(int n_idx: n.neighbours){

                    if(this->nodes[n_idx].w == 0.0) {
                        continue;
                    }

                    glm::vec3 diff = this->nodes[n_idx].pos - n.pos;
                    float dist_2 = diff.x*diff.x + diff.y*diff.y + diff.z*diff.z;

                    if(dist_2 > thickness_2 || dist_2 == 0.0) {
                        continue;
                    }

                    glm::vec3 nat_dist = n.nat_pos - this->nodes[n_idx].nat_pos;
                    float nat_dist_2 = nat_dist.x*nat_dist.x + nat_dist.y*nat_dist.y + nat_dist.z*nat_dist.z;
                    if(dist_2 > nat_dist_2) {
                        continue;
                    }

                    float min_dist = n.thickness;
                    if(nat_dist_2 < thickness_2)
                        min_dist = sqrtf(nat_dist_2);

                    float dist = sqrtf(dist_2);
                    glm::vec3 correction = diff * ((min_dist-dist)/dist);
                    n.pos += correction * (-0.5f);
                    this->nodes[n_idx].pos += correction * (0.5f);

                    glm::vec3 n1_vel = n.pos - n.prev_pos;
                    glm::vec3 n2_vel = this->nodes[n_idx].pos - this->nodes[n_idx].prev_pos;

                    glm::vec3 av_vel = (n1_vel + n2_vel) * 0.5f;

                    n1_vel = av_vel - n1_vel;
                    n2_vel = av_vel - n2_vel;

                    n.pos += n1_vel * this->self_friction;
                    this->nodes[n_idx].pos += n2_vel * this->self_friction;
                }
            }
        }

/*        void generate_constraints(){
//            generate_stretch_constraints();
//            generate_bend_constraints();
//            constraints_coloring();
//        }
//
//        void generate_stretch_constraints(){
//
//            for (auto &mesh : model.meshes) {
//                for (int i = 0; i < mesh.indices.size(); i += 3) {
//
//                    this->constraints.emplace_back(nodes, i, i+1, this->stretching_compliance);
//                    this->constraints.emplace_back(nodes, i+1, i+2, this->stretching_compliance);
//                    this->constraints.emplace_back(nodes, i+2, i, this->stretching_compliance);
//                }
//            }
//        }
//
//        void generate_bend_constraints(){
//
//            for(int i=0; i<rows-1; ++i){
//                for(int j=0; j<columns-1; ++j){
//                    this->constraints.emplace_back(nodes, i*columns+j, (i*columns+j)+(columns+1), this->bending_compliance);
//                }
//            }
//            for(int i=1; i<rows; ++i){
//                for(int j=0; j<columns-2; ++j){
//                    this->constraints.emplace_back(nodes, i*columns+j, (i*columns+j)-(columns-2), this->bending_compliance);
//                }
//            }
//            for(int i=0; i<rows-2; ++i){
//                for(int j=1; j<columns; ++j){
//                    this->constraints.emplace_back(nodes, i*columns+j, (i*columns+j)+(2*columns-1), this->bending_compliance);
//                }
//            }
//        }
//
//        void constraints_coloring(){
//
//            std::vector<bool> node_marks(this->nodes.size(), false);
//            std::vector<bool> constr_marks(this->constraints.size(), false);
//
//
//            while(std::find(constr_marks.begin(), constr_marks.end(), false) != constr_marks.end()) {
//                std::vector<Constraint> set;
//                for (int i = 0; i < node_marks.size(); i++) {
//                    node_marks[i] = false;
//                }
//                for (int i=0; i<this->constraints.size(); i++) {
//                    if(constr_marks[i] == true)
//                        continue;
//                    if (node_marks[this->constraints[i].a_node] == false && node_marks[this->constraints[i].b_node] == false){
//                        set.push_back(this->constraints[i]);
//                        constr_marks[i] = true;
//                        node_marks[this->constraints[i].a_node] = true;
//                        node_marks[this->constraints[i].b_node] = true;
//                    }
//                }
//                this->constr_sets.push_back(set);
//            }
//        }
//
//        void find_max_node_cardinality(){
//
//            std::vector<int> node_cardinalities(this->nodes.size());
//
//            for(auto &c : this->constraints) {
//                node_cardinalities[c.a_node]++;
//                node_cardinalities[c.b_node]++;
//            }
//
//            this->max_node_cardinality = 0;
//            for(auto n : node_cardinalities){
//                if (n > this->max_node_cardinality)
//                    this->max_node_cardinality = n;
//            }
//
//        }
*/

    };
}