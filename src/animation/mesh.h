#ifndef MESH_H
#define MESH_H

#include <glad.h> // holds all OpenGL type declarations

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>


#include <display/shader.h>

#include <string>
#include <vector>

using namespace std;

#define MAX_BONE_INFLUENCE 4

struct Vertex {
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;
    // tangent
    glm::vec3 Tangent;
    // bitangent
    glm::vec3 Bitangent;
    //bone indexes which will influence this vertex
    int m_BoneIDs[MAX_BONE_INFLUENCE] = { -1, -1, -1, -1 };
    //weights from each bone
    float m_Weights[MAX_BONE_INFLUENCE];

    Vertex() {}
    Vertex(glm::vec3 pos)
    {
        Position = pos;
    }
    ~Vertex() {}
};

class Triangle {
public:
    Vertex* v1;
    Vertex* v2;
    Vertex* v3;
    bool isInsideSphere;

    Triangle(Vertex* v1_0, Vertex* v2_0, Vertex* v3_0) {
        v1 = v1_0;
        v2 = v2_0;
        v3 = v3_0;
        isInsideSphere = false;
    }

    float area() {
        glm::vec3 e1 = v1->Position - v2->Position;
        glm::vec3 e2 = v1->Position - v3->Position;
        glm::vec3 cr = glm::cross(e1, e2);

        return (cr.length() / 2);
    }

    glm::vec3 centroid() {
        return  glm::vec3((v1->Position.x + v2->Position.x + v3->Position.x) / 3, (v1->Position.y + v2->Position.y + v3->Position.y) / 3, (v1->Position.z + v2->Position.z + v3->Position.z) / 3);
    }

    glm::vec3 normal() {
        glm::vec3 e1 = v2->Position - v1->Position;
        glm::vec3 e2 = v3->Position - v1->Position;
        
        glm::vec3 n = glm::cross(e1, e2);
        glm::normalize(n);

        if (n.x == -0.0) { n.x = 0.0; }
        if (n.y == -0.0) { n.y = 0.0; }
        if (n.z == -0.0) { n.z = 0.0; }

        return n;
    }

    //Data una particella del vestito calcola la posizione del vertice pi� vicino
    glm::vec3 nearest_vertex(glm::vec3* particle_position) {
        float distance = (*particle_position - v1->Position).length();
        glm::vec3 result = v1->Position;

        float new_distance = (*particle_position - v2->Position).length();
        if (new_distance < distance)
            result = v2->Position;

        new_distance = (*particle_position - v3->Position).length();
        if (new_distance < distance)
            result = v3->Position;

        return result;
    }

    void print_triangle(int pos) {
        printf("------------- TRIANGLE %i -------------\n", pos);
        printf("Vertex a: (%f, %f, %f)\n", v1->Position.x, v1->Position.y, v1->Position.z);
        printf("Vertex b: (%f, %f, %f)\n", v2->Position.x, v2->Position.y, v2->Position.z);
        printf("Vertex c: (%f, %f, %f)\n", v3->Position.x, v3->Position.y, v3->Position.z);
        printf("\n");
    }
};

struct Texture {
    unsigned int id;
    string type;
    string path;
};

class Mesh {
public:
    // mesh Data
    vector<Vertex>       vertices;
    vector<unsigned int> indices;
    vector<Triangle>     triangles;
    vector<Texture>      textures;
    unsigned int         textureID;
    unsigned int VAO;


    // constructor
    Mesh(vector<Vertex>&& vertices, vector<unsigned int> indices, vector<Triangle> triangles, vector<Texture> textures)
    {
        this->vertices = std::move(vertices);
        this->indices = indices;
        this->triangles = triangles;
        this->textures = textures;

        // now that we have all the required data, set the vertex buffers and its attribute pointers.
        setupMesh();
    }


    // render the mesh
    void Draw(Shader& shader)
    {
        // bind appropriate texture
        glUniform1i(glGetUniformLocation(shader.ID, "texture_diffuse"), 0);
        //glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // draw mesh
        glBindVertexArray(VAO);
        glDrawElements( GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // always good practice to set everything back to defaults once configured.
        glActiveTexture(GL_TEXTURE0);
    }


private:
    // render data 
    unsigned int VBO, EBO;

    // initializes all the buffer objects/arrays
    void setupMesh()
    {
        // load texture image
        int texW, texH, colorChannels;
        unsigned char* data = stbi_load("resources/Textures/pink.png", &texW, &texH, &colorChannels, 0);

        // create texture object and bind it
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // set texture wrapping/filtering options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // generate texture from image data
        if (data) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texW, texH, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else {
            std::cout << "Failed to load texture image." << std::endl;
        }
        // free image data
        stbi_image_free(data);

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        // load data into vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // set the vertex attribute pointers
        // vertex Positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        // vertex texture coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
        // vertex tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
        // vertex bitangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
        // ids
        glEnableVertexAttribArray(5);
        glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));

        // weights
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));
        glBindVertexArray(0);
    }
};
#endif