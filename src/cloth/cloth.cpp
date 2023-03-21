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
#include "constraints/s_constr.h"
#include "constraints/b_constr.h"
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
        
//        std::cout << "creo i nodi" << std::endl;
        size = 1/size;
        for(int i=0; i<rows; ++i){
            for(int j=0; j<columns; ++j){
                vec3 pos {static_cast<float>(j)/(size*static_cast<float>(columns-1)), static_cast<float>(i)/(size*static_cast<float>(columns-1)), z_constant};
                vec2 uv_c {static_cast<float>(j)/static_cast<float>(columns-1), static_cast<float>(i)/static_cast<float>(rows-1)};
                nodes.emplace_back(Node(pos, node_thickness, node_mass, vel, normal, uv_c));
            }
        }

//        std::cout << "pin" << std::endl;
        pin1(pin1_index);
        pin2(pin2_index);

//        std::cout << "genero i vertici" << std::endl;
        generate_verts();

//        std::cout << "genero gli s_constr" << std::endl;
        generate_stretch_constraints();
        
//        std::cout << "genero gli s_constr" << std::endl;
        generate_bend_constraints();
        
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

    void Cloth::proces_input(GLFWwindow *window){
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
            unpin1();
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
            unpin2();
    }
    void Cloth::unpin1() {
        nodes.at(pin1_index).m = node_mass;
        nodes.at(pin1_index).w = 1.0/node_mass;
    }
    void Cloth::unpin2() {
        nodes.at(pin2_index).m = 1.0;
        nodes.at(pin2_index).w = 1.0;
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

    void Cloth::generate_stretch_constraints() {
        
        if(up_left_tris.empty())
            generate_verts();
        
        for (auto t : up_left_tris){
                s_cs.emplace_back(nodes.at(t.a), nodes.at(t.b), false);
                s_cs.emplace_back(nodes.at(t.b), nodes.at(t.c), true); // questo mi crea i constraints oblicui e mi rompe tutto
                s_cs.emplace_back(nodes.at(t.a), nodes.at(t.c), false);
        }
        
        for(int i=columns-1; i<columns*(rows-1); i+=columns){
                s_cs.emplace_back(nodes.at(i), nodes.at(i+columns), false);
        }
        for(int i=columns*(rows-1); i<(rows*columns)-1; ++i){
                s_cs.emplace_back(nodes.at(i), nodes.at(i+1), false);
        }
    }
    void Cloth::generate_bend_constraints() {
        
        if(all_tris.empty())
            generate_verts();
        
//        std::vector<std::pair<int, int>> edges {};
//        
//        int num_tris = low_right_tris.size() + up_left_tris.size();
//        for (int i=0; i< num_tris; ++i){
//            int id1 = all_tris.at(i).a;
//            int id2 = all_tris.at(i).b;
//            edges.emplace_back(min(id1, id2), max(id1, id2));
//            id1 = all_tris.at(i).b;
//            id2 = all_tris.at(i).c;
//            edges.emplace_back(min(id1, id2), max(id1, id2));
//            id1 = all_tris.at(i).a;
//            id2 = all_tris.at(i).c;
//            edges.emplace_back(min(id1, id2), max(id1, id2));
//        }
//
//        std::sort(edges.begin(), edges.end());
//        std
//        
//        int nr = 0;
//        while(nr < edges.size()){
//            auto e0 = edges.at(nr);
//            nr++;
//            if(nr<edges.size()){
//                auto e1 = edges.at(nr);
//                if(e0.first == e1.first && e0.second == e1.second){
//                    
//                }
//                nr++;
//            }
//        }
//              TODO

        for(int i=0; i<rows-1; ++i){
            for(int j=0; j<columns-1; ++j){
                b_cs.emplace_back(nodes.at(i*columns+j), nodes.at((i*columns+j)+8));
            }
        }
        for(int i=1; i<rows; ++i){
            for(int j=0; j<columns-2; ++j){
                b_cs.emplace_back(nodes.at(i*columns+j), nodes.at((i*columns+j)-5));
            }
        }
        for(int i=0; i<rows-2; ++i){
            for(int j=1; j<columns; ++j){
                b_cs.emplace_back(nodes.at(i*columns+j), nodes.at((i*columns+j)+13));
            }
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
        shader.destroy();
    }
    
    void Cloth::simulate_XPBD(render::State& s) {
        
        float timestep = s.simulation_step_time/s.iteration_per_frame; // frame indipendent, la velocità della simulazione è come se fosse costante a 60 frame al secondo, se non riesce a generare 60 frame al secondo la simulazione sembra rallentata
        float max_velocity = (0.5f * node_thickness) / timestep; // da provare a modificare, più piccolo = meno nodi collidenti = sim più veloce
        float max_travel_distance = max_velocity * s.simulation_step_time;
        updateHashGrid(s);
        queryAll(s, max_travel_distance);
        
        for(int i=0; i< s.iteration_per_frame; ++i){
            XPBD_predict(timestep, s.gravity, max_velocity);
            TMP_solve_ground_collisions();
            XPBD_solve_constraints(timestep, s);
            XPBD_update_velocity(timestep);
        }
    }
    
    void Cloth::updateHashGrid(render::State& s){
        for(auto& n: nodes){
            s.grid.grid.at(s.grid.hashIndex(n.pos)).push_back(&n); // popolo la hashgrid coi puntatori ai nodi
        }
    }

    
    
    
    
    void Cloth::queryAll(render::State &s, float max_travel_distance) {
        for(auto& n: nodes){
            
        }
    }
    
    void Cloth::XPBD_predict(float t, glm::vec3 g, float max_velocity){
        /** Nodes **/
        for (auto& n : nodes) {
            if (n.w == 0.0)
                continue;
                
            n.vel += g * t;
            float tmp_vel = sqrt(n.vel.x*n.vel.x + n.vel.y*n.vel.y + n.vel.z*n.vel.z); //forzo la veloità massima
            if(tmp_vel>max_velocity)                                                   //
                n.vel *= max_velocity/tmp_vel;                                         //
            n.prev_pos = n.pos;
            n.pos += n.vel * t;
        }
    }
    void Cloth::TMP_solve_ground_collisions() {
        for (auto& n : nodes) {
            if (n.w == 0.0)
                continue;
            if (n.pos.z < 0.5*n.thickness) {
                float damping = 0.0002;
                vec3 diff = n.pos - n.prev_pos;
                n.pos += diff * -damping;
                n.pos.z = 0.5f * n.thickness;
            }
        }
    }
    void Cloth::XPBD_solve_constraints(float t, render::State& s){
        XPBD_solve_stretching(t, s);
        XPBD_solve_bending(t);
        
    }
    void Cloth::XPBD_solve_stretching(float timeStep, render::State& s) {
        
        for (auto& s_c : s_cs) {
            float alpha = s_c.compliance / timeStep / timeStep;
            if (s_c.nodes.first.w + s_c.nodes.second.w == 0.0)
                continue;
            vec3 distance = s_c.nodes.first.pos - s_c.nodes.second.pos;
            float abs_distance = static_cast<float>(sqrt(pow(distance.x,2) + pow(distance.y,2) + pow(distance.z,2)));
            
            if (abs_distance == 0.0)
                continue;
            distance = distance * (1 / abs_distance);

            float rest_len = s_c.rest_dist;
            float error = abs_distance - rest_len;
            float correction = -error / (s_c.nodes.first.w + s_c.nodes.second.w + alpha);
            
            float first_corr = correction * s_c.nodes.first.w;
            float second_corr = -correction * s_c.nodes.second.w;
            
            s_c.nodes.first.pos += distance * first_corr;
            s_c.nodes.second.pos += distance * second_corr;
        }
    }
    void Cloth::XPBD_solve_bending(float timeStep) {
        
        for (auto& b_c : b_cs) {
            float alpha = b_c.compliance / timeStep / timeStep;
            if (b_c.nodes.first.w + b_c.nodes.second.w == 0.0)
                continue;
            vec3 distance = b_c.nodes.first.pos - b_c.nodes.second.pos;
            float abs_distance = static_cast<float>(sqrt(pow(distance.x,2) + pow(distance.y,2) + pow(distance.z,2)));
            if (abs_distance == 0.0)
                continue;
            distance *= 1 / abs_distance;

            float rest_len = b_c.rest_dist;
            float error = abs_distance - rest_len;
            float correction = -error / ((b_c.nodes.first.w + b_c.nodes.second.w) + alpha);

            b_c.nodes.first.pos += distance * correction * b_c.nodes.first.w;
            b_c.nodes.second.pos += distance * -correction * b_c.nodes.second.w;
        }
    }
    void Cloth::XPBD_update_velocity(float t){
        /** Nodes **/
        for (auto& n : nodes) {
            if (n.w == 0.0)
                continue;
            n.vel = (n.pos - n.prev_pos) * static_cast<float>(1.0)/t;
            //n.vel *= damping;
        }
    }

}