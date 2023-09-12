#pragma once

#include <glm.hpp>

#include "animation/model_animation.h"
#include "node/node.h"
#include "bvh/BVH.h"
#include "constraints/constraint.h"
#include "bvh/Box.h"


#define COLORING 0
#define JACOBI 1
#define HYBRID 2


#define N_SSBO 8
#define CC_SSBO 9
#define JC_SSBO 10
#define HF_SSBO 11
#define HV_SSBO 12

bool first_iteration = true;
int solve_type = JACOBI;

using namespace cloth;

class ClothModel {
public:
    Model *modelCloth;
    Model *associatedHuman;
    std::vector<Node> nodes;
    std::vector<Constraint> springs;
    std::vector<std::vector<Constraint>> constr_sets;
    std::vector<Node *> faces;
    Box *box;

    glm::vec3 translation;
    glm::vec3 scaling;

    vector<glm::vec3> constraint;
    vector<int> cloth_pos;
    vector<int> human_pos;

    // ---------- render data ----------
    glm::vec3 *vboPos; // Position
    glm::vec2 *vboTex; // Texture
    glm::vec3 *vboNor; // Normal

    Shader cloth_shader{"resources/Shaders/ClothVS.glsl", "resources/Shaders/ClothFS.glsl"};
    GLuint vaoID;
    GLuint vboIDs[3];
    GLuint texID;

    GLint aPtrPos;
    GLint aPtrTex;
    GLint aPtrNor;
    

    GLuint ssbo_nodes;
    GLuint ssbo_cconstrs;
    GLuint ssbo_jconstrs;
    GLuint ssbo_model_faces;
    GLuint ssbo_model_vertexes;

    Shader compute_predict {"resources/gpu_kernels/next_predict_pos.comp"};
    Shader TMP_compute_ground_collisions {"resources/gpu_kernels/ground_collisions.comp"};
    Shader compute_solve_coloring_constraints {"resources/gpu_kernels/solve_coloring_constraints.comp"};
    Shader compute_solve_jacobi_constraints {"resources/gpu_kernels/solve_jacobi_constraints.comp"};
    Shader compute_jacobi_add_correction {"resources/gpu_kernels/jacobi_add_correction.comp"};
    Shader compute_update_velocities {"resources/gpu_kernels/update_velocities.comp"};
    Shader compute_external_collisions{"resources/gpu_kernels/external_collisions.comp"};

    const float stretching_compliance = 0.0f;
    const float bending_compliance = 0.03f;
    
    float node_thickness;

    ClothModel(Model *m, Model *h, glm::vec3 t, glm::vec3 s, float thickness) {
        modelCloth = m;
        associatedHuman = h;
        translation = t;
        scaling = s;
        node_thickness = thickness;
        box = new Box();
        initClothModel(thickness);
        
        init_gpu_data();
    }


    // -------------------------------- Init cloth --------------------------------
    void initClothModel(float thickness) {
        //Delete duplicate Vertex
        format_model();
    
        //Structures reconstruction
        vector<Vertex> v_supporto = modelCloth->meshes[0].vertices;
        vector<unsigned int> indices = modelCloth->meshes[0].indices;

        //Update coordinates
        vector<Vertex> v = update_nodes_coordinate(v_supporto);


        // ---------- Nodes ----------
//        printf("\nInit cloth with %u nodes\n", v.size());
        for (int j = 0; j < v.size(); j++) {
            nodes.emplace_back(v[j].Position, thickness, 0.01f, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 0.0), glm::vec2(0.0));
            //printf("\t[%indices] (%f, %f, %f) - (%f, %f)\n", j, node->position.x, node->position.y, node->position.z);
        }

        // ---------- Stretching constraint without duplication ----------
//        printf("\nStructural springs:\n");
        vector<int> edges;
        for (int j = 0; j < indices.size() - 3; j = j + 3) {
            edges.push_back(indices[j]);
            edges.push_back(indices[j + 1]);
            edges.push_back(indices[j + 1]);
            edges.push_back(indices[j + 2]);
            edges.push_back(indices[j + 2]);
            edges.push_back(indices[j]);
        }

        for (int j = 0; j < edges.size() - 2; j = j + 2) {
            int count = 0;
            for (int z = j + 2; z < edges.size() - 2; z = z + 2) {
                if (edges[j] == edges[z] || edges[j] == edges[z + 1]) {
                    if (edges[j + 1] == edges[z] || edges[j + 1] == edges[z + 1])
                        count++;
                }
            }
            if (count == 0) {
                springs.emplace_back(nodes, edges[j], edges[j + 1], stretching_compliance);
                //printf("\t[%indices] (Node %indices) - (Node %indices)\n", springs.size(), edges[j], edges[j + 1]);
            }
        }

        // ---------- Bending springs ----------
//        printf("\nBending springs:\n");
        for (int j = 0; j < indices.size() - 3; j = j + 3)
            bool result = add_bending_springs(indices, j);
//        printf("\nInit springs with %indices springs\n", springs.size());

        // ---------- Faces ----------
        for (int j = 0; j < indices.size(); j++)
            faces.push_back(&nodes[indices[j]]);

        // ---------- Box on Cloth for collisions ----------
        compute_box_on_cloth();

        // ---------- Compute constraint ----------
//        if (meshType == SKIRT) {
//            cloth_pos = getSkirtConstrint();
//            human_pos = getSkirtHumanConstraint();
//            pin_skirt();
//            constraint = compute_constraint_skirt();
//        }

        constraints_coloring();
        

    }


    void init_avatar_model_gpu_data(Mesh &mesh){
        // model faces
        glGenBuffers(1, &(this->ssbo_model_faces));
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->ssbo_model_faces);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int)*mesh.indices.size(), mesh.indices.data(), GL_STATIC_READ);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, HF_SSBO, this->ssbo_model_faces);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        
//        // model verteces
//        glGenBuffers(1, &(this->ssbo_model_vertexes));
//        glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->ssbo_model_vertexes);
//        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float)*3*mesh.vertices.size(), nullptr, GL_STATIC_READ);
//        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, HF_SSBO, this->ssbo_model_vertexes);
//        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
    
    void init_gpu_data(){
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
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Constraint)*this->springs.size(), this->springs.data(), GL_STATIC_READ);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, JC_SSBO, this->ssbo_jconstrs);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
    
    
    // -------------------------------- Init render data --------------------------------
    void init_render_data(render::Camera &cam, render::State &s) {
        int nodeCount = (int) (this->faces.size());
        if (nodeCount <= 0) {
            std::cout << "ERROR::ClothRender : No node exists." << std::endl;
            exit(-1);
        }

        vboPos = new glm::vec3[nodeCount];
        vboTex = new glm::vec2[nodeCount];
        vboNor = new glm::vec3[nodeCount];
        for (int i = 0; i < nodeCount; i++) {
            Node *n = this->faces[i];
            vboPos[i] = glm::vec3(n->pos.x, n->pos.y, n->pos.z);
            vboTex[i] = glm::vec2(n->uv_c.x, n->uv_c.y); // Texture coord will only be set here
            vboNor[i] = glm::vec3(n->n.x, n->n.y, n->n.z);
        }

        // Generate ID of VAO and VBOs
        glGenVertexArrays(1, &vaoID);
        glGenBuffers(3, vboIDs);

        // Attribute pointers of VAO
        aPtrPos = 0;
        aPtrTex = 1;
        aPtrNor = 2;
        // Bind VAO
        glBindVertexArray(vaoID);

        // Position buffer
        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]);
        glVertexAttribPointer(aPtrPos, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
        glBufferData(GL_ARRAY_BUFFER, nodeCount * sizeof(glm::vec3), vboPos, GL_DYNAMIC_DRAW);
        // Texture buffer
        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]);
        glVertexAttribPointer(aPtrTex, 2, GL_FLOAT, GL_FALSE, 0, (void *) 0);
        glBufferData(GL_ARRAY_BUFFER, nodeCount * sizeof(glm::vec2), vboTex, GL_DYNAMIC_DRAW);
        // Normal buffer
        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[2]);
        glVertexAttribPointer(aPtrNor, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
        glBufferData(GL_ARRAY_BUFFER, nodeCount * sizeof(glm::vec3), vboNor, GL_DYNAMIC_DRAW);

        // Enable it's attribute pointers since they were set well
        glEnableVertexAttribArray(aPtrPos);
        glEnableVertexAttribArray(aPtrTex);
        glEnableVertexAttribArray(aPtrNor);

        //Load texture
        // Assign texture ID and gengeration
        glGenTextures(1, &texID);
        glBindTexture(GL_TEXTURE_2D, texID);
        // Set the texture wrapping parameters (for 2D tex: S, T)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // Set texture filtering parameters (Minify, Magnify)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        //Load image and configure texture
        stbi_set_flip_vertically_on_load(true);
        int texW, texH, colorChannels; // After loading the image, stb_image will fill them
        unsigned char *data = stbi_load("resources/Textures/tex1.jpg", &texW, &texH, &colorChannels, 0);

        if (data) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texW, texH, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            // Automatically generate all the required mipmaps for the currently bound texture.
            cout << "Texture loaded correctly" << endl;
            glGenerateMipmap(GL_TEXTURE_2D);
        } else {
            cout << "Failed to load texture" << endl;
        }
        // Always free image memory
        stbi_image_free(data);



        //Set uniform
        cloth_shader.use(); // Active shader before set uniform
        // Set texture sampler
        this->cloth_shader.setInt("uniTex", 0);

        //Projection matrix : The frustum that camera observes
        // Since projection matrix rarely changes, set it outside the rendering loop for only onec time
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float) s.scr_width / (float) s.scr_height, 0.1f, 100.0f);
        cloth_shader.setMat4("uniProjMatrix", projection);

        //Model Matrix : Put rigid into the world
        glm::mat4 uniModelMatrix = glm::mat4(1.0f);
        // = glm::translate(uniModelMatrix, glm::vec3(5.0, -8.0f, 7.0f)); //Translate
        //uniModelMatrix = glm::scale(uniModelMatrix, glm::vec3(10.0f, 10.0f, 10.0f)); //Scale
        //uniModelMatrix = glm::rotate(uniModelMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); //Rotate
        cloth_shader.setMat4("uniModelMatrix", uniModelMatrix);

        //Light
        cloth_shader.setVec3("uniLightPos", glm::vec3(-5.0f, 7.0f, 6.0f));
        cloth_shader.setVec3("uniLightColor", glm::vec3(0.7f, 0.7f, 1.0f));

        // Cleanup
        glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbined VBO
        glBindVertexArray(0); // Unbined VAO

    }

    void destroy() {
        cloth_shader.destroy();
        glDeleteBuffers(1, &ssbo_nodes);
        glDeleteBuffers(1, &ssbo_cconstrs);
        glDeleteBuffers(1, &ssbo_jconstrs);
        glDeleteBuffers(1, &ssbo_model_faces);
        glDeleteBuffers(1, &ssbo_model_vertexes);
        this->cloth_shader.destroy();
        this->compute_predict.destroy();
        this->TMP_compute_ground_collisions.destroy();
        this->compute_solve_coloring_constraints.destroy();
        this->compute_solve_jacobi_constraints.destroy();
        this->compute_jacobi_add_correction.destroy();
        this->compute_update_velocities.destroy();
        this->compute_external_collisions.destroy();
    }

    void flush(render::Camera &cam, render::State &state) {
        // Update all the positions of nodes
        int nodeCount = this->faces.size();
//        printf("------%zu\n", this->nodes.size());

        for (int i = 0; i < nodeCount; i++) { // Tex coordinate dose not change
            Node *n = this->faces[i];
            vboPos[i] = glm::vec3(n->pos.x, n->pos.y, n->pos.z);
            vboNor[i] = glm::vec3(n->n.x, n->n.y, n->n.z);
        }
        computeNormal();
        cloth_shader.use();

        glBindVertexArray(vaoID);

        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, nodeCount * sizeof(glm::vec3), vboPos);
        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, nodeCount * sizeof(glm::vec2), vboTex);
        glBindBuffer(GL_ARRAY_BUFFER, vboIDs[2]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, nodeCount * sizeof(glm::vec3), vboNor);

        //Bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texID);

        //View Matrix : The camera
        mat4 view = lookAt(cam.pos, cam.pos + cam.front_v, cam.up_v);
        this->cloth_shader.setMat4("uniViewMatrix", view);


        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        //Draw

        glDrawArrays(GL_TRIANGLES, 0, nodeCount);
        
        // End flushing
        glDisable(GL_BLEND);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glUseProgram(0);
    }

    // -------------------------------- Format model --------------------------------
    void format_model() {
        vector<Vertex> initial_vertices = modelCloth->meshes[0].vertices;
        vector<Vertex> compact_vertices;
        vector<unsigned int> initial_indices = modelCloth->meshes[0].indices;

        for (int i = 0; i < initial_vertices.size(); i++) {
            vector<unsigned int> position;
            if (!isnan(initial_vertices[i].Position.x)) {
                compact_vertices.push_back(initial_vertices[i]);
                position.push_back(i);
                for (int j = (i + 1); j < initial_vertices.size(); j++) {
                    if (initial_vertices[i].Position.x == initial_vertices[j].Position.x &&
                        initial_vertices[i].Position.y == initial_vertices[j].Position.y &&
                        initial_vertices[i].Position.z == initial_vertices[j].Position.z) {
                        position.push_back(j);
                        initial_vertices[j].Position.x = NAN;
                    }
                }
                for (int z = 0; z < position.size(); z++) {
                    for (int y = 0; y < initial_indices.size(); y++) {
                        if (initial_indices[y] == position[z])
                            initial_indices[y] = compact_vertices.size() - 1;
                    }
                }
            }
        }

        modelCloth->meshes[0].vertices = compact_vertices;
        modelCloth->meshes[0].indices = initial_indices;
    }

    // -------------------------------- Update vertices --------------------------------
    vector<Vertex> update_nodes_coordinate(vector<Vertex> v) {
        vector<Vertex> result = v;

        for (int j = 0; j < result.size(); j++) {
            //Scaling
            result[j].Position.x *= scaling.x;
            result[j].Position.y *= scaling.y;
            result[j].Position.z *= scaling.z;
            result[j].TexCoords.x *= scaling.x;
            result[j].TexCoords.y *= scaling.y;
        }

        for (int j = 0; j < result.size(); j++) {
            //Translation
            result[j].Position.x += translation.x;
            result[j].Position.y += translation.y;
            result[j].Position.z += translation.z;
            result[j].TexCoords.x += translation.x;
            result[j].TexCoords.y += translation.y;
        }

        return result;
    }

    // -------------------------------- Add bending springs --------------------------------
    bool add_bending_springs(vector<unsigned int> indices, int pos) {
        vector<unsigned int> t1;
        t1.push_back(indices[pos]);
        t1.push_back(indices[pos + 1]);
        t1.push_back(indices[pos + 2]);

        for (int j = pos + 3; j < indices.size() - 6; j = j + 3) {
            vector<unsigned int> t2;
            t2.push_back(indices[j]);
            t2.push_back(indices[j + 1]);
            t2.push_back(indices[j + 2]);

            vector<unsigned int> result = single_check(t1, t2);

            if (result.size() > 1) {
                //springs.push_back(new Constraint(*nodes[indices[pos + result[0]]], *nodes[indices[j + result[1]]], bending_compliance));
                springs.emplace_back(nodes, indices[pos + result[0]], indices[j + result[1]], bending_compliance);
//                printf("\t[%i] (Node %i) - (Node %i)\n", springs.size(), indices[pos + result[0]], indices[j + result[1]]);
                return true;
            }
        }
        return false;
    }

    // -------------------------------- Single check --------------------------------
    vector<unsigned int> single_check(vector<unsigned int> t1, vector<unsigned int> t2) {
        vector<unsigned int> result;
        int check = 0;
        for (int j = 0; j < t1.size(); j++) {
            for (int z = 0; z < t2.size(); z++) {
                if (t1[j] == t2[z]) {
                    t1[j] = 0;
                    t2[z] = 0;
                    check++;
                }
            }
        }

        if (check >= 2) {
            for (int j = 0; j < t1.size(); j++) {
                if (t1[j] != 0) {
                    result.push_back(j);
                    break;
                }
            }
            for (int j = 0; j < t2.size(); j++) {
                if (t2[j] != 0) {
                    result.push_back(j);
                    break;
                }
            }
        }

        return result;
    }

    // -------------------------------- Compute constraint --------------------------------
//    vector<glm::vec3> compute_constraint_skirt() {
//        vector<glm::vec3> constraint;
//        for (int i = 0; i < cloth_pos.size(); i++) {
//            glm::vec3 dir = nodes[cloth_pos[i]]->position - associatedHuman->meshes[0].vertices[human_pos[i]].Position;
//            dir.normalize();
//            constraint.push_back(dir);
//        }
//
//        return constraint;
//    }
//    void pin_skirt() {
//        for (int i = 0; i < cloth_pos.size(); i++) {
//            nodes[cloth_pos[i]]->mass = std::numeric_limits<double>::infinity();
//            nodes[cloth_pos[i]]->w = 0;
//        }
//    }

    void proces_input(GLFWwindow *window) {
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
            solve_type = COLORING;
            std::cout << "Coloring" << std::endl;
        }
        if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
            solve_type = JACOBI;
            std::cout << "Jacobi" << std::endl;
        }
        if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
            solve_type = HYBRID;
            std::cout << "Hybrid" << std::endl;
        }
    }

    void constraints_coloring() {

        std::vector<bool> node_marks(this->nodes.size(), false);
        std::vector<bool> constr_marks(this->springs.size(), false);


        while (std::find(constr_marks.begin(), constr_marks.end(), false) != constr_marks.end()) {
            std::vector<Constraint> set;
            for (int i = 0; i < node_marks.size(); i++) {
                node_marks.at(i) = false;
            }
            for (int i = 0; i < this->springs.size(); i++) {
                if (constr_marks.at(i) == true)
                    continue;
                if (node_marks.at(this->springs.at(i).a_node) == false &&
                    node_marks.at(this->springs.at(i).b_node) == false) {
                    set.push_back(this->springs.at(i));
                    constr_marks.at(i) = true;
                    node_marks.at(this->springs.at(i).a_node) = true;
                    node_marks.at(this->springs.at(i).b_node) = true;
                }
            }
            this->constr_sets.push_back(set);
        }
    }

    // -------------------------------- Compute normal --------------------------------
    void computeNormal() {
        /** Reset nodes' normal **/
        glm::vec3 normal(0.0, 0.0, 0.0);
        for (int i = 0; i < nodes.size(); i++) {
            nodes[i].n = normal;
        }
        /** Compute normal of each face **/
        for (int i = 0; i < faces.size() / 3; i++) { // 3 nodes in each face
            Node *n1 = faces[3 * i + 0];
            Node *n2 = faces[3 * i + 1];
            Node *n3 = faces[3 * i + 2];

            // Face normal
            normal = computeFaceNormal(n1, n2, n3);
            // Add all face normal
            n1->n += normal;
            n2->n += normal;
            n3->n += normal;
        }

        for (int i = 0; i < nodes.size(); i++) {
            nodes[i].n = glm::normalize(nodes[i].n);
        }
    }

    glm::vec3 computeFaceNormal(Node *n1, Node *n2, Node *n3) {
        return glm::cross(n2->pos - n1->pos, n3->pos - n1->pos);
    }


    // -------------------------------- Algoritmo --------------------------------

//    void free_resources() {
//        glDeleteVertexArrays(1, &VAO);
//        glDeleteBuffers(1, &VBO);
//        glDeleteBuffers(1, &ssbo_nodes);
//        glDeleteBuffers(1, &ssbo_cconstrs);
//        glDeleteBuffers(1, &ssbo_jconstrs);
//        this->cloth_shader.destroy();
//        this->compute_predict.destroy();
//        this->TMP_compute_ground_collisions.destroy();
//        this->compute_solve_coloring_constraints.destroy();
//        this->compute_solve_jacobi_constraints.destroy();
//        this->compute_jacobi_add_correction.destroy();
//        this->compute_update_velocities.destroy();
//    }

    void simulate_XPBD(render::State &s, hashgrid::HashGrid &grid, BVH &bvh) {
        if (s.sym_type == CPU)
            CPU_SIM(s, grid, bvh);
        if(s.sym_type == GPU)
            GPU_SIM(s, grid, bvh);
    }

    void CPU_SIM(render::State &s, hashgrid::HashGrid &grid, BVH &bvh) {

        float time_step = s.simulation_step_time / s.iteration_per_frame;
        float max_velocity = (0.5f * this->node_thickness) /
                             time_step; // da tweakkare, più piccolo = meno possibili collisioni = simulazione più veloce
        float max_travel_distance = max_velocity * s.simulation_step_time;
        updateHashGrid(grid);
        queryAll(grid, max_travel_distance);

        for (int i = 0; i < s.iteration_per_frame; ++i) {
            XPBD_predict(time_step, s.gravity, max_velocity);
            XPBD_solve_ground_collisions();
            XPBD_solve_constraints(time_step);
            HG_solve_collisions();
            collisionResponse_ray_triangle(&bvh);
            XPBD_update_velocity(time_step);
        }
    }

    void GPU_SIM(render::State& s, hashgrid::HashGrid& grid, BVH &bvh){

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
            HG_solve_collisions();
            GPU_send_data();
            GPU_collisionResponse_ray_triangle(&bvh);
            GPU_XPBD_update_velocity(time_step);
        }
        GPU_retrieve_data();
    }

    void XPBD_predict(float t, glm::vec3 g, float max_velocity) {
        /** Nodes **/
        for (auto &n: nodes) {
            if (n.w == 0.0)
                continue;

            n.vel += g * t;
            float tmp_vel = sqrt(n.vel.x * n.vel.x + n.vel.y * n.vel.y + n.vel.z * n.vel.z); //forzo la velocità massima
            if (tmp_vel > max_velocity)                                                   //
                n.vel *= max_velocity / tmp_vel;                                         //
            n.prev_pos = n.pos;
            n.pos += n.vel * t;
        }
    }

    void XPBD_solve_ground_collisions() {
        for (auto &n: nodes) {
            if (n.w == 0.0)
                continue;
            if (n.pos.y < -8.0f) {
                float damping = 0.02;
                vec3 diff = n.pos - n.prev_pos;
                n.pos += diff * -damping;
                n.pos.y = -8.0 + 0.5f * n.thickness;
            }
        }
    }

    void XPBD_solve_constraints(float time_step) {

        for (auto &c: this->springs) {
            float alpha = c.compliance / time_step / time_step;

            if (this->nodes[c.a_node].w + this->nodes[c.b_node].w == 0.0)
                continue;

            vec3 distance = this->nodes[c.a_node].pos - this->nodes[c.b_node].pos;
            float abs_distance = sqrtf(powf(distance.x, 2) + powf(distance.y, 2) + powf(distance.z, 2));

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

    void XPBD_update_velocity(float t) {
        /** Nodes **/
        for (auto &n: nodes) {
            if (n.w == 0.0)
                continue;
            n.vel = (n.pos - n.prev_pos) * 1.0f / t;
        }
    }

    void GPU_send_data(){
        //nodes
        glNamedBufferSubData(this->ssbo_nodes, 0, sizeof(Node)*this->nodes.size(), this->nodes.data());

    }

    void GPU_retrieve_data(){
        //nodes
        glGetNamedBufferSubData(this->ssbo_nodes, 0, sizeof(Node)*this->nodes.size(), this->nodes.data());
    }

    void GPU_XPBD_predict(float time_step, glm::vec3 gravity, float max_velocity) {

        this->compute_predict.setVec3("gravity", gravity);
        this->compute_predict.setFloat("time_step", time_step);
        this->compute_predict.setFloat("max_velocity", max_velocity);

        this->compute_predict.use();
        glDispatchCompute(std::ceil(this->nodes.size()/32), 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }

    void GPU_solve_ground_collisions() {

        this->TMP_compute_ground_collisions.use();
        glDispatchCompute(std::ceil(this->nodes.size()/32), 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    }

    void GPU_XPBD_solve_constraints_coloring(float time_step, int passes){

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

    void GPU_XPBD_solve_constraints_jacobi(float time_step, int coloring_passes){

        this->compute_solve_jacobi_constraints.setFloat("time_step", time_step);

        int first_constr = 0;
        for(int i=0; i<coloring_passes; i++){
            first_constr += (int)constr_sets[i].size();
        }
        this->compute_solve_jacobi_constraints.setInt("first_constr", first_constr);

        this->compute_solve_jacobi_constraints.use();
        glDispatchCompute(std::ceil((this->springs.size()-first_constr)/32), 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    }

    void GPU_XPBD_add_jacobi_correction(){
        float magic_number = 0.3;
        this->compute_jacobi_add_correction.setFloat("magic_number", magic_number);

        this->compute_jacobi_add_correction.use();
        glDispatchCompute(std::ceil(this->nodes.size()/32), 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }

    void GPU_XPBD_update_velocity (float time_step){
        this->compute_update_velocities.setFloat("time_step", time_step);

        this->compute_update_velocities.use();
        glDispatchCompute(std::ceil(this->nodes.size()/32), 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }

    void updateHashGrid(hashgrid::HashGrid& grid){

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

                n.pos += n1_vel * 0.2f;
                this->nodes[n_idx].pos += n2_vel * 0.2f;
            }
        }
    }


    // -------------------------------- GET/SET positions --------------------------------
    glm::vec3 getWorldPos(Node *n) { return n->pos; }

    glm::vec3 getWorldPosOld(Node *n) { return n->prev_pos; }

    void setWorldPos(Node *n, glm::vec3 pos) { n->pos = pos; }

    void setWorldPosOld(Node *n, glm::vec3 pos) { n->prev_pos = pos; }


    /*
    COLLISION DETECTION
      0) se sono alla prima iterazione non verifico le collisioni
      1) itero su tutte le sfere posizionate sui giunti della struttura BVH
      2) verifico che le sfere siano abbastanza vicine da poter collidere, viceversa passo alla sfera successiva: ||C_bvh - C_cloth||^2 < R_bvh + R_cloth
      3) itero su tutti i nodi del cloth
           -> verifico che il nodo sia all'interno della sfera BVH con la quale il vestito � entrato in collisione, in caso contrario esco
              ||C_bvh - nodes[j].position||^2 < R_bvh
           -> per ogni nodo verifico la collisione con ogni triangolo della sfera BVH
           -> se trovo la collisione posso passare al nodo successivo
           -> se non trovo la collisione vado avanti
      4) nel caso in cui il booleano che indica la prima iterazione sia ancora a true lo metto a false
      5) aggiorno nodes[...].old_position con la posizione attuale del nodo, in modo tale che al prossimo ciclo la posizione del nodo sar� aggiornata in
         base al movimento del vestito causato dalle forze e si potr� creare nuovamente il segmento per l'intersezione
      6) ritorno la nuova posizione del centro della sfera del cloth e il relativo raggio modificato
     */
    
    void GPU_collisionResponse_ray_triangle(BVH *bvh) {
        this->compute_external_collisions.setFloat("num_faces", bvh[0].model->meshes[0].triangles.size());

        this->compute_external_collisions.use();
        glDispatchCompute(std::ceil(this->nodes.size()/32), 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }

    void collisionResponse_ray_triangle(BVH *bvh) {
        if (first_iteration == false) {
            for (int i = 0; i < bvh->bounding_spheres_bones.size(); i++) {
                //AABB - Sphere collision
                bool AABB_Sphere_collision = true;
                glm::vec3 box_sphere_distance =
                        glm::vec3(bvh->bounding_spheres_bones[i]->center.x, 
                                  bvh->bounding_spheres_bones[i]->center.y,
                                  bvh->bounding_spheres_bones[i]->center.z) - box->box_center;

                //Calcola la distanza tra il centro della sfera e il box lungo ogni asse
                float dist_x = std::max(box->up_left_front.x - bvh->bounding_spheres_bones[i]->center.x,
                                        bvh->bounding_spheres_bones[i]->center.x - box->up_right_front.x);
                float dist_y = std::max(box->down_left_front.y - bvh->bounding_spheres_bones[i]->center.y,
                                        bvh->bounding_spheres_bones[i]->center.y - box->up_left_front.y);
                float dist_z = std::max(box->down_left_back.z - bvh->bounding_spheres_bones[i]->center.z,
                                        bvh->bounding_spheres_bones[i]->center.z - box->down_left_front.z);

                //Verifica la collisione lungo ogni asse
                if (dist_x > bvh->bounding_spheres_bones[i]->radius + (box->up_right_front.x - box->up_left_front.x) * 0.5f) {
                    AABB_Sphere_collision = false;
                }
                if (dist_y > bvh->bounding_spheres_bones[i]->radius + (box->up_left_front.y - box->down_left_front.y) * 0.5f) {
                    AABB_Sphere_collision = false;
                }
                if (dist_z > bvh->bounding_spheres_bones[i]->radius + (box->down_left_front.z - box->down_left_back.z) * 0.5f) {
                    AABB_Sphere_collision = false;
                }

                if (AABB_Sphere_collision) {
                    glm::vec3 center_sphere = bvh->bounding_spheres_bones[i]->center;

                #pragma omp parallel for
                    for (int j = 0; j < nodes.size(); j++) {
                        if (glm::length(center_sphere - nodes[j].pos) < bvh->bounding_spheres_bones[i]->radius) {
                            if (bvh->bounding_spheres_bones[i]->triangles_list.size() > 0) {
                                
                                
                                
                                for (int t = 0; t < bvh->bounding_spheres_bones[i]->triangles_list.size(); t++) {
                                    glm::vec3 nearest_v = bvh->bounding_spheres_bones[i]->triangles_list[t]->nearest_vertex(
                                            &nodes[j].pos);

                                    if (glm::length(nearest_v - nodes[j].pos) < this->node_thickness) //La particella del vestito è abbastanza vicino da poter andare in collisione
                                    {
                                        glm::vec3 intersection_point;
                                        glm::vec3 particle_direction = nodes[j].pos - nodes[j].prev_pos;
                                        particle_direction = glm::normalize(particle_direction);

                                        bool check_intersection = ray_triangle_intersection(
                                                *bvh->bounding_spheres_bones[i]->triangles_list[t],
                                                nodes[j].prev_pos, particle_direction, &intersection_point);

                                        if (check_intersection == true) {
                                            glm::vec3 collision_direction_shift =
                                                    nodes[j].prev_pos - intersection_point;
                                            collision_direction_shift = glm::normalize(collision_direction_shift);

                                            if (glm::length(intersection_point - nodes[j].pos) < this->node_thickness) {
                                                setWorldPos(&nodes[j], intersection_point + collision_direction_shift * this->node_thickness);
                                                nodes[j].vel = nodes[j].vel * 0.5f;
                                                continue;
                                            }
                                        } else {
                                            if (glm::length(nearest_v - nodes[j].pos) < this->node_thickness) {
                                                glm::vec3 dir = nodes[j].pos - nearest_v;
                                                dir = glm::normalize(dir);
                                                setWorldPos(&nodes[j], nearest_v + dir * this->node_thickness);
                                                nodes[j].vel = nodes[j].vel * 0.5f;
                                                continue;
                                            }
                                        }
                                    }
                                } // END FOR ON BVH_SPHERE_TRIANGLE_LIST (z)
                                
                                
                                
                            }
                        } // END IF

                    } // END FOR ON NODES (j)

                } // END IF (check)

            } // END FOR ON BVH_SPHERE (i)

            /*for (int j = 0; j < nodes.size(); j++)
            {
                for (int t = 0; t < associatedHuman->meshes[0].triangles.size(); t++)
                {
                    glm::vec3 intersection_point;
                    glm::vec3 particle_direction = nodes[j]->position - nodes[j]->old_position;
                    particle_direction.normalize();

                    bool check_intersection = ray_triangle_intersection(associatedHuman->meshes[0].triangles[t], nodes[j]->old_position, particle_direction, &intersection_point);

                    if (check_intersection == true) {
                        glm::vec3 collision_direction_shift = nodes[j]->old_position - intersection_point;
                        collision_direction_shift.normalize();

                        if (glm::length(intersection_point - nodes[j]->position) < 0.1) {
                            setWorldPos(nodes[j], intersection_point + collision_direction_shift * 0.1);
                            nodes[j]->velocity = nodes[j]->velocity * 0.5;
                            continue;
                        }
                    }
                }
            }*/

        } // END IF (first_iteration)
        first_iteration = false;
//        if (meshType == SKIRT) {
//            for (int c = 0; c < cloth_pos.size(); c++)
//                setWorldPos(nodes[cloth_pos[c]],
//                            associatedHuman->meshes[0].vertices[human_pos[c]].Position + constraint[c] * 0.09);
//        }



        //for (int i = 0; i < nodes.size(); i++)
        //{
        // -------------------------------- Ground collision --------------------------------
        //if (nodes[i]->position.y < ground->position.y) {
        //nodes[i]->position.y = ground->position.y + 0.01;
        //nodes[i]->velocity = nodes[i]->velocity * ground->friction;
        //}

        // -------------------------------- Ball --------------------------------
        /*/glm::vec3 distVec = nodes[i]->position - ball->center;
        double distLen = glm::length(distVec);
        double safeDist = ball->radius * 1.1;
        if (distLen < safeDist) {
            distVec.normalize();
            nodes[i]->position = distVec * safeDist + ball->center;
            nodes[i]->velocity = nodes[i]->velocity * ball->friction;
        }*/
        //setWorldPosOld(nodes[i], getWorldPos(nodes[i])); // Aggiornamento delle coordinate dei nodi del cloth
        //}

        compute_box_on_cloth();
    }
    
    // -------------------------------- Ray Triangle Intersection --------------------------------
    bool ray_triangle_intersection(Triangle triangle, glm::vec3 origin, glm::vec3 direction, glm::vec3 *intersection_point) {
        //Find the intersecrion point on the plane (shifted plane to prevent penetration)
        glm::vec3 n = triangle.normal();

        double denominator = n.x * direction.x + n.y * direction.y + n.z * direction.z;
        if (abs(denominator) < 0.0001f)
            return false;

        glm::vec3 cen = triangle.centroid();
        double numerator = n.x * (cen - origin).x + n.y * (cen - origin).y + n.z * (cen - origin).z;
        double len = (numerator / denominator);

        //Projection point on the plane
        *intersection_point = origin + glm::vec3(len * direction.x, len * direction.y, len * direction.z);

        // ---------- Check if the point lies inside a triangle ----------
        //Metodo del prodotto misto
        glm::vec3 point = *intersection_point;
        glm::vec3 v1v2 = triangle.v1->Position - triangle.v2->Position;
        glm::vec3 v1v3 = triangle.v1->Position - triangle.v3->Position;
        glm::vec3 v1P = triangle.v1->Position - point;
        glm::vec3 v1v2_cr_v1v3 = glm::cross(v1v2, v1v3);

        if ((v1v2_cr_v1v3.x * v1P.x + v1v2_cr_v1v3.y * v1P.y + v1v2_cr_v1v3.z * v1P.z) >= 0)
            return true;

        //Metodo delle 3 aree
        /*if ((internalTriangle_area(point, triangle.v1->Position, triangle.v2->Position) +
            internalTriangle_area(point, triangle.v2->Position, triangle.v3->Position) +
            internalTriangle_area(point, triangle.v3->Position, triangle.v1->Position)) <= triangle.area()) {
            return true;
        }*/
        return false;
    }
    
    double internalTriangle_area(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3) {
        glm::vec3 e1 = v1 - v2;
        glm::vec3 e2 = v1 - v3;
        glm::vec3 cr = glm::cross(e1, e2);

        return (glm::length(cr) / 2);
    }

    // -------------------------------- Check if a point lies inside a model --------------------------------
    bool inside_model(glm::vec3 intersection_point, glm::vec3 old_pos, glm::vec3 cur_pos) {
        glm::vec3 dir1 = intersection_point - old_pos;
        glm::vec3 dir2 = intersection_point - cur_pos;
        dir1 = glm::normalize(dir1);
        dir2 = glm::normalize(dir2);

        //dot product = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z
        double dot_product = glm::dot(dir1, dir2);
        if (dot_product > 0.0f)
            return false; // I due vettori hanno direzione uguale
        printf("dentro\n");
        return true; // I due vettori hanno direzione opposta
    }
    
    void compute_box_on_cloth() {
        box->up_right_front = glm::vec3(nodes[0].pos.x, nodes[0].pos.y, nodes[0].pos.z);
        box->up_left_front = glm::vec3(nodes[0].pos.x, nodes[0].pos.y, nodes[0].pos.z);
        box->down_left_front = glm::vec3(nodes[0].pos.x, nodes[0].pos.y, nodes[0].pos.z);
        box->down_right_front = glm::vec3(nodes[0].pos.x, nodes[0].pos.y, nodes[0].pos.z);

        box->up_right_back = glm::vec3(nodes[0].pos.x, nodes[0].pos.y, nodes[0].pos.z);
        box->up_left_back = glm::vec3(nodes[0].pos.x, nodes[0].pos.y, nodes[0].pos.z);
        box->down_left_back = glm::vec3(nodes[0].pos.x, nodes[0].pos.y, nodes[0].pos.z);
        box->down_right_back = glm::vec3(nodes[0].pos.x, nodes[0].pos.y, nodes[0].pos.z);

        for (int i = 1; i < nodes.size(); i++) {
            if (nodes[i].pos.x < box->down_left_front.x) {
                box->up_left_front.x = nodes[i].pos.x;
                box->down_left_front.x = nodes[i].pos.x;
                box->up_left_back.x = nodes[i].pos.x;
                box->down_left_back.x = nodes[i].pos.x;
            }

            if (nodes[i].pos.y < box->down_left_front.y) {
                box->down_left_front.y = nodes[i].pos.y;
                box->down_right_front.y = nodes[i].pos.y;
                box->down_left_back.y = nodes[i].pos.y;
                box->down_right_back.y = nodes[i].pos.y;
            }

            if (nodes[i].pos.z < box->down_left_back.z) {
                box->down_left_back.z = nodes[i].pos.z;
                box->down_right_back.z = nodes[i].pos.z;
                box->up_left_back.z = nodes[i].pos.z;
                box->up_right_back.z = nodes[i].pos.z;
            }

            if (nodes[i].pos.x > box->up_right_front.x) {
                box->up_right_front.x = nodes[i].pos.x;
                box->down_right_front.x = nodes[i].pos.x;
                box->up_right_back.x = nodes[i].pos.x;
                box->down_right_back.x = nodes[i].pos.x;
            }

            if (nodes[i].pos.y > box->up_right_front.y) {
                box->up_left_front.y = nodes[i].pos.y;
                box->up_right_front.y = nodes[i].pos.y;
                box->up_left_back.y = nodes[i].pos.y;
                box->up_right_back.y = nodes[i].pos.y;
            }

            if (nodes[i].pos.z > box->down_left_front.z) {
                box->down_left_front.z = nodes[i].pos.z;
                box->down_right_front.z = nodes[i].pos.z;
                box->up_left_front.z = nodes[i].pos.z;
                box->up_right_front.z = nodes[i].pos.z;
            }
        }

        box->box_center = glm::vec3((box->up_left_front.x + box->up_right_front.x) / 2,
                                    (box->down_left_front.y + box->up_left_front.y) / 2,
                                    (box->down_left_back.z + box->down_left_front.z) / 2);
    }

};