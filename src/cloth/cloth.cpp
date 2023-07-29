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
#include "compute_shader.h"
#include "constraints/constraint.h"
#include "display/display.h"
#include "state/state.h"


namespace cloth {
    
    Cloth::Cloth(int rows, int columns, float size, float thickness, render::State& s){
        Cloth::rows = rows;
        Cloth::columns = columns;
        
        node_thickness = thickness;
        pin1_index = 0;
        pin2_index = columns-1;
        
        float z_constant = 2.0;
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

        generate_stretch_constraints(s);
        
        generate_bend_constraints(s);
        
        // robe per rendering

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

        shader.use();

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)s.scr_width / (float)s.scr_height, 0.1f, 100.0f);
        shader.setMat4("uniProjMatrix", projection);

        shader.setInt("uniTex", 0);
        shader.setVec3("uniLightPos", glm::vec3(0.0, 0.0, 1.0));
        shader.setVec3("uniLightColor", glm::vec3(1.0, 1.0, 1.0));
        
        
        glGenBuffers(1, &(this->ssbo_verts));
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->ssbo_verts);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Node)*this->nodes.size(), this->nodes.data(), GL_DYNAMIC_READ);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, this->ssbo_verts);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        
    }
    
    void Cloth::generate_stretch_constraints(render::State& s) {

        if(up_left_tris.empty())
            generate_verts();

        for (auto t : up_left_tris){
            s_cs.emplace_back(nodes, t.a, t.b, s.stretching_compliance);
//                s_cs.emplace_back(nodes.at(t.b), nodes.at(t.c)); // questo mi crea i constraints oblicui e mi rompe tutto
            s_cs.emplace_back(nodes, t.a, t.c, s.stretching_compliance);
        }

        for(int i=columns-1; i<columns*(rows-1); i+=columns){
            s_cs.emplace_back(nodes, i, i+columns, s.stretching_compliance);
        }
        for(int i=columns*(rows-1); i<(rows*columns)-1; ++i){
            s_cs.emplace_back(nodes, i, i+1, s.stretching_compliance);
        }
    }
    void Cloth::generate_bend_constraints(render::State& s) {

        if(all_tris.empty())
            generate_verts();

        for(int i=0; i<rows-1; ++i){
            for(int j=0; j<columns-1; ++j){
                b_cs.emplace_back(nodes, i*columns+j, (i*columns+j)+(columns+1), s.bending_compliance);
            }
        }
        for(int i=1; i<rows; ++i){
            for(int j=0; j<columns-2; ++j){
                b_cs.emplace_back(nodes, i*columns+j, (i*columns+j)-(columns-2), s.bending_compliance);
            }
        }
        for(int i=0; i<rows-2; ++i){
            for(int j=1; j<columns; ++j){
                b_cs.emplace_back(nodes, i*columns+j, (i*columns+j)+(2*columns-1), s.bending_compliance);
            }
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
        
        if(verts.empty())
            generate_verts();
            
            
        for(auto i : verts){
            Node& n = nodes.at(i);
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
            Node& n1 = nodes.at(t.a);
            Node& n2 = nodes.at(t.b);
            Node& n3 = nodes.at(t.c);
            normal = cross(n2.pos - n1.pos, n3.pos - n1.pos);
            n1.n += normal;
            n2.n += normal;
            n3.n += normal;
        }
        for(auto t : low_right_tris){
            Node& n1 = nodes.at(t.a);
            Node& n2 = nodes.at(t.b);
            Node& n3 = nodes.at(t.c);
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
        shader.use();
        mat4 view = lookAt(c.pos, c.pos + c.front_v, c.up_v);
        shader.setMat4("uniViewMatrix", view);
        glBindVertexArray(VAO);

        mat4 model = mat4(1.0f);
        shader.setMat4("uniModelMatrix", model);
        glDrawArrays(GL_TRIANGLES, 0, cloth_verts_data.size()/8.0);
        
    }
    void Cloth::free_resources() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &ssbo_verts);
        shader.destroy();
        compute_predict.destroy();
    }
    
    void Cloth::simulate_XPBD(render::State& s) {
        float timestep = s.simulation_step_time/s.iteration_per_frame;
        float max_velocity = (0.5f * node_thickness) / timestep; // da tweakkare, più piccolo = meno possibili collisioni = simulazione più veloce
        float max_travel_distance = max_velocity * s.simulation_step_time;
//        updateHashGrid(s);
//        queryAll(s, max_travel_distance);
        
        for(int i=0; i< s.iteration_per_frame; ++i){
//            XPBD_predict(timestep, s.gravity, max_velocity);
            GPU_predict(timestep, s.gravity, max_velocity);
            TMP_solve_ground_collisions();
            XPBD_solve_constraints(timestep);
            HG_solve_collisions();
            XPBD_update_velocity(timestep);
        }
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
    void Cloth::XPBD_solve_constraints(float t){
        XPBD_solve_stretching(t);
        XPBD_solve_bending(t);
    }
    void Cloth::XPBD_solve_stretching(float timeStep) {
        
        for (auto& s_c : s_cs) {
            float alpha = s_c.compliance / timeStep / timeStep;
            if (nodes.at(s_c.nodes.first).w + nodes.at(s_c.nodes.second).w == 0.0)
                continue;
            vec3 distance = nodes.at(s_c.nodes.first).pos - nodes.at(s_c.nodes.second).pos;
            float abs_distance = sqrtf(powf(distance.x,2) + powf(distance.y,2) + powf(distance.z,2));
            
            if (abs_distance == 0.0)
                continue;
            distance = distance * (1 / abs_distance);

            float rest_len = s_c.rest_dist;
            float error = abs_distance - rest_len;
            float correction = -error / (nodes.at(s_c.nodes.first).w + nodes.at(s_c.nodes.second).w + alpha);
            
            float first_corr = correction * nodes.at(s_c.nodes.first).w;
            float second_corr = -correction * nodes.at(s_c.nodes.second).w;

            nodes.at(s_c.nodes.first).pos += distance * first_corr;
            nodes.at(s_c.nodes.second).pos += distance * second_corr;
        }
    }
    void Cloth::XPBD_solve_bending(float timeStep) {
        
        for (auto& b_c : b_cs) {
            float alpha = b_c.compliance / timeStep / timeStep;
            if (nodes.at(b_c.nodes.first).w + nodes.at(b_c.nodes.second).w == 0.0)
                continue;
            vec3 distance = nodes.at(b_c.nodes.first).pos - nodes.at(b_c.nodes.second).pos;
            float abs_distance = sqrtf(powf(distance.x,2) + powf(distance.y,2) + powf(distance.z,2));
            if (abs_distance == 0.0)
                continue;
            distance *= 1 / abs_distance;

            float rest_len = b_c.rest_dist;
            float error = abs_distance - rest_len;
            float correction = -error / ((nodes.at(b_c.nodes.first).w + nodes.at(b_c.nodes.second).w) + alpha);

            nodes.at(b_c.nodes.first).pos += distance * correction * nodes.at(b_c.nodes.first).w;
            nodes.at(b_c.nodes.second).pos += distance * -correction * nodes.at(b_c.nodes.second).w;
        }
    }
    void Cloth::XPBD_update_velocity(float t){
        /** Nodes **/
        for (auto& n : nodes) {
            if (n.w == 0.0)
                continue;
            n.vel = (n.pos - n.prev_pos) * 1.0f/t;
        }
    }
    
    void Cloth::GPU_predict(float t, glm::vec3 g, float max_velocity) {
    
        this->compute_predict.setVec3("g", g);
        this->compute_predict.setFloat("t", t);
        this->compute_predict.setFloat("max_velocity", max_velocity);
        
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->ssbo_verts);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Node)*this->nodes.size(), this->nodes.data(), GL_DYNAMIC_READ);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, this->ssbo_verts);
        this->compute_predict.use();

        glDispatchCompute(this->nodes.size(), 1, 1); //TODO aggiungere un groupsize multiplo di 32
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        
        Node* nodes_predicted_position = (Node*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
        
        memcpy(nodes.data(), nodes_predicted_position, sizeof(Node) * nodes.size());
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        
    }

    void Cloth::TMP_solve_ground_collisions() {
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
    
    void Cloth::updateHashGrid(render::State& s){
        for(auto& n: nodes){
            s.grid.grid.at(s.grid.hashIndex(n.pos)).push_back(&n); // popolo la hashgrid coi puntatori ai nodi
        }
    }
    void Cloth::queryAll(render::State &s, float max_travel_distance) {
        float max_dist_2 = max_travel_distance * max_travel_distance;
        for(auto& node: nodes){
            int min_x = s.grid.intCoord(node.pos.x - max_travel_distance);
            int max_x = s.grid.intCoord(node.pos.x + max_travel_distance);
            int min_y = s.grid.intCoord(node.pos.y - max_travel_distance);
            int max_y = s.grid.intCoord(node.pos.y + max_travel_distance);
            int min_z = s.grid.intCoord(node.pos.z - max_travel_distance);
            int max_z = s.grid.intCoord(node.pos.z + max_travel_distance);

            node.neighbours.clear();

            for(int x=min_x; x<max_x; ++x)              // qui scorro tutte le celle in cui posso trovare particelle
                for(int y=min_y; y<max_y; ++y)          // con cui la particella in esame potrebbe collidere
                    for(int z=min_z; z<max_z; ++z){     // la complessità è cubica nel raggio di possibile collisione (max_travel_distance)
                        int hashcoor = s.grid.hashCoords(x, y, z);
                        for (auto n : s.grid.grid.at(hashcoor)){
                            if(!n)
                                continue;
                            if (n->pos == node.pos)
                                continue;
                            vec3 dist = node.pos - n->pos;
                            float dist_2 = (powf(dist.x,2) + powf(dist.y,2) + powf(dist.z,2));
                            if (dist_2 <=max_dist_2)
                                node.neighbours.push_back(n);
                        }
                    }
        }
    }
    void Cloth::HG_solve_collisions(){
        float thickness_2 = this->node_thickness * this->node_thickness;
        for(auto& n: nodes) {
            if (n.w == 0.0)
                continue;
            for(auto neighbour_node: n.neighbours){
                if(!neighbour_node) {
                    continue;
                }
                if(neighbour_node->w == 0.0) {
                    continue;
                }
                
                vec3 diff = neighbour_node->pos - n.pos;
                float dist_2 = diff.x*diff.x + diff.y*diff.y + diff.z*diff.z;
                
                if(dist_2 > n.thickness * n.thickness || dist_2 == 0.0) {
                    continue;
                }
                
                vec3 nat_diff = n.nat_pos - neighbour_node->nat_pos;
                float nat_dist_2 = nat_diff.x*nat_diff.x + nat_diff.y*nat_diff.y + nat_diff.z*nat_diff.z;
                if(dist_2 >= nat_dist_2) {
                    continue;
                }
                
                float min_dist = sqrtf(thickness_2);
                if(nat_dist_2 < thickness_2)
                    min_dist = sqrtf(min_dist);

                float dist = sqrtf(dist_2);
                vec3 correction = diff * ((min_dist-dist)/dist);
                n.pos += correction * (-0.5f);
                neighbour_node->pos += correction * (0.5f);

                vec3 n1_vel = n.pos - n.prev_pos;
                vec3 n2_vel = neighbour_node->pos - neighbour_node->prev_pos;

                vec3 av_vel = (n1_vel + n2_vel) * 0.5f;

                n1_vel = av_vel - n1_vel;
                n2_vel = av_vel - n2_vel;

                n.pos += n1_vel * this->self_friction;
                neighbour_node->pos += n2_vel * this->self_friction;
            }
        }
    }
    
}

























