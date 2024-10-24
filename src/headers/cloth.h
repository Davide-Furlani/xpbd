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
#include "node.h"
#include "constraint.h"
#include "state.h"
#include "hashgrid.h"
#include <learnopengl/shader.h>
#include "camera.h"
#include "display.h"




namespace cloth {
    class Cloth {
    public:
        // physics
        float node_mass = 0.01f;
        float node_thickness;
        float stretching_compliance = 0.0f;
        float bending_compliance = 0.03f;
        std::vector<Node> nodes;
        float self_friction = 0.2f;

        std::map<unsigned int, std::vector<unsigned int>> same_verts;

        Model model;

        std::vector<Constraint> constraints;

        Cloth(std::filesystem::path &path): model{Model(path)} {

            for (auto &mesh: model.meshes) {
                for (unsigned int i=0; i<mesh.vertices.size(); i++) {

                    unsigned int original_vert = find_same_vert(same_verts, i, mesh);
                    if (original_vert == -1) {
                        same_verts[i] = {i};
                    } else {
                        same_verts[original_vert].push_back(i);
                    }

                    glm::vec3 pos = mesh.vertices[i].Position;
                    glm::vec3 vel{0.0};
                    glm::vec3 normal{0.0, 0.0, 1.0};
                    glm::vec2 uv_c{0.0, 0.0};
                    nodes.emplace_back(pos, node_thickness, node_mass, vel, normal, uv_c);
                }
                printf("giggi");
            }

            printf("giggi");
        }

        int find_same_vert(std::map<unsigned int, std::vector<unsigned int>> &same_verts, unsigned int i, Mesh &mesh) {

            for (auto &[original_vert, same_verts_list]: same_verts) {
                if (mesh.vertices[i].Position.x == mesh.vertices[original_vert].Position.x &&
                        mesh.vertices[i].Position.y == mesh.vertices[original_vert].Position.y &&
                        mesh.vertices[i].Position.z == mesh.vertices[original_vert].Position.z) {
                    return original_vert;
                }
            }
            return -1;
        }


//        void generate_constraints(){
//            generate_stretch_constraints();
//            generate_bend_constraints();
//            constraints_coloring();
//        }
//
//        void generate_stretch_constraints(){
//
//            for (auto &mesh : model.meshes) {
//                for (unsigned int i = 0; i < mesh.indices.size(); i += 3) {
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

    };
}