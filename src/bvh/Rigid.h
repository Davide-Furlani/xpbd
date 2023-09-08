#pragma once
#define _USE_MATH_DEFINES
#include <math.h>

#include <cmath>
#include <vector>

struct Ground
{
    Vec3 position;
    int width, height;
    glm::vec4 color;
    const double friction = 0.9;
    
    std::vector<Vertex*> vertexes;
    std::vector<Vertex*> faces;
    
    Ground(Vec3 pos, Vec2 size, glm::vec4 c) {
        position = pos;
        width = size.x;
        height = size.y;
        color = c;
        
        init();
    }
    ~Ground()
    {
        for (int i = 0; i < vertexes.size(); i++) { delete vertexes[i]; }
        vertexes.clear();
        faces.clear();
    }
    
    void init()
    {
        vertexes.push_back(new Vertex(Vec3(0.0, 0.0, 0.0)));
        vertexes.push_back(new Vertex(Vec3(width, 0.0, 0.0)));
        vertexes.push_back(new Vertex(Vec3(0.0, 0.0, -height)));
        vertexes.push_back(new Vertex(Vec3(width, 0.0, -height)));
        
        for (int i = 0; i < vertexes.size(); i ++) {
            vertexes[i]->Normal = Vec3(0.0, 1.0, 0.0); // It's not neccessery to normalize here
            
            // Debug info
            printf("Ground[%d]: (%f, %f, %f) - (%f, %f, %f)\n", i, vertexes[i]->Position.x, vertexes[i]->Position.y, vertexes[i]->Position.z, vertexes[i]->Normal.x, vertexes[i]->Normal.y, vertexes[i]->Normal.z);
        }
        
        faces.push_back(vertexes[0]);
        faces.push_back(vertexes[1]);
        faces.push_back(vertexes[2]);
        faces.push_back(vertexes[1]);
        faces.push_back(vertexes[2]);
        faces.push_back(vertexes[3]);
    }
};

class Sphere
{
public:
    const int meridianNum = 11;  //24
    const int parallelNum = 23;  //250
    
    float radius;
    
    std::vector<Vertex*> vertexes;
    std::vector<Vertex*> faces;
    
    Sphere(float r)
    {
        radius = r;
        init();
    }
    ~Sphere()
    {
        for (int i = 0; i < vertexes.size(); i++) { delete vertexes[i]; }
        vertexes.clear();
        faces.clear();
    }
    
    Vertex* getTop() { return vertexes[0]; }
    Vertex* getVertex(int x, int y)
    {
        if (x < 0 || x >= parallelNum || y < 0 || y >= meridianNum) {
            printf("Vertex Index Out of Range.\n");
            exit(-1);
        } else {
            return vertexes[1+x*meridianNum+y];
        }
    }
    Vertex* getBottom() { return vertexes[vertexes.size()-1]; }
    
    Vec3 computeFaceNormal(Vertex* v1, Vertex* v2, Vertex* v3)
    {
        return Vec3::cross(v2->Position - v1->Position, v3->Position - v1->Position);
    }
    
    void computeSphereNormal()
    {
        Vec3 normal(0.0, 0.0, 0.0);
        for (int i = 0; i < vertexes.size(); i ++) {
            vertexes[i]->Normal = normal;
        }
        
        // The normal of all faces of the first and last cycle should be calculated specially!
        for (int i = 0; i < faces.size()/3; i ++) {
            Vertex* v1 = faces[i*3+0];
            Vertex* v2 = faces[i*3+1];
            Vertex* v3 = faces[i*3+2];
            
            normal = computeFaceNormal(v1, v3, v2);
            v1->Normal += normal;
            v2->Normal += normal;
            v3->Normal += normal;
        }
        
        for (int i = 0; i < vertexes.size(); i ++) {
            vertexes[i]->Normal.normalize();
        }
    }
    
    void init() // Initialize vertexes coord and slice faces
    {
        /** Compute vertex position **/
        double cycleInterval = radius*2.0 / (parallelNum+1);
        double radianInterval = 2.0*M_PI/meridianNum;
        
        
        Vec3 pos(0.0, radius, 0.0);
        vertexes.push_back(new Vertex(pos)); // Top vertex
        
        for (int i = 0; i < parallelNum; i ++) {
            pos.y -= cycleInterval;
            for (int j = 0; j < meridianNum; j ++) {
                double xzLen = radius * sqrt(1.0 - pow(pos.y/radius, 2));
                double xRadian = j * radianInterval;  // The length of projection line on X-Z pane
                
                pos.x = xzLen * sin(xRadian);
                pos.z = xzLen * cos(xRadian);
                vertexes.push_back(new Vertex(pos));
            }
        }
        pos = Vec3(0.0, -radius, 0.0);
        vertexes.push_back(new Vertex(pos)); // Bottom vertex
        
        /** Slice faces **/
        // Top cycle
        for (int i = 0; i < meridianNum; i ++) {
            faces.push_back(getVertex(0, i));                               //   *   //
            faces.push_back(getTop());                                      //  / \  //
            faces.push_back(getVertex(0, (i+1)%meridianNum));               // *---* //
        }
        // Middle cycles
        for (int i = 0; i < parallelNum-1; i ++) {
            for (int j = 0; j < meridianNum; j ++) {
                faces.push_back(getVertex(i, j));                           //  *--* //
                faces.push_back(getVertex(i, (j+1)%meridianNum));           //  | /  //
                faces.push_back(getVertex(i+1, j));                         //  *    //
                
                faces.push_back(getVertex(i+1, (j+1)%meridianNum));         //     * //
                faces.push_back(getVertex(i+1, j));                         //   / | //
                faces.push_back(getVertex(i, (j+1)%meridianNum));           //  *--* //
            }
        }
        // Bottom cycle
        for (int i = 0; i < meridianNum; i ++) {
            faces.push_back(getBottom());                                   // *---* //
            faces.push_back(getVertex(parallelNum-1, i));                   //  \ /  //
            faces.push_back(getVertex(parallelNum-1, (i+1)%meridianNum));   //   *   //
        }
        
        /** Set normals **/
        computeSphereNormal();
    }
};

struct Ball
{
    Vec3 center;
    float radius;
    glm::vec4 color;
    const double friction = 0.4;
    
    Sphere* sphere;
    
    Ball(Vec3 cen, float r, glm::vec4 c)
    {
        center = cen;
        radius = r;
        color = c;
        
        sphere = new Sphere(radius);
    }
    ~Ball() {}


    bool move(glm::vec3 m, bool direction) {
        float limitInf = -3.0;
        float limitSup = 2.0;

        if (center.z >= limitSup && direction == false) 
            direction = true;
        if (center.z < limitInf && direction == true)
            direction = false;

        if (direction == false) {
            for (int i = 0; i < sphere->vertexes.size(); i++) {
                //Vertex position
                sphere->vertexes[i]->Position.x += m.x;
                sphere->vertexes[i]->Position.y += m.y;
                sphere->vertexes[i]->Position.z += m.z;

                //Vertex normal
                sphere->vertexes[i]->Normal.x += m.x;
                sphere->vertexes[i]->Normal.y += m.y;
                sphere->vertexes[i]->Normal.z += m.z;
            }
            //Center
            center.x += m.x;
            center.y += m.y;
            center.z += m.z;
        }

        if (direction == true) {
            for (int i = 0; i < sphere->vertexes.size(); i++) {
                //Vertex position
                sphere->vertexes[i]->Position.x -= m.x;
                sphere->vertexes[i]->Position.y -= m.y;
                sphere->vertexes[i]->Position.z -= m.z;

                //Vertex normal
                sphere->vertexes[i]->Normal.x -= m.x;
                sphere->vertexes[i]->Normal.y -= m.y;
                sphere->vertexes[i]->Normal.z -= m.z;
            }
            //Center
            center.x -= m.x;
            center.y -= m.y;
            center.z -= m.z;
        }

        return direction;
    }

    bool scale(float size, bool scale) {
        float limitInf = 0.7f;
        float limitSup = 1.35f;

        if (radius >= limitSup && scale == false)
            scale = true;
        if (radius <= limitInf && scale == true)
            scale = false;

        if (scale == false) {
            for (int i = 0; i < sphere->vertexes.size(); i++) {
                sphere->vertexes[i]->Position.x *= size;
                sphere->vertexes[i]->Position.y *= size;
                sphere->vertexes[i]->Position.z *= size;

                sphere->vertexes[i]->Normal.x *= size;
                sphere->vertexes[i]->Normal.y *= size;
                sphere->vertexes[i]->Normal.z *= size;
            }
            radius *= size;
        }

        if (scale == true) {
            for (int i = 0; i < sphere->vertexes.size(); i++) {
                sphere->vertexes[i]->Position.x /= size;
                sphere->vertexes[i]->Position.y /= size;
                sphere->vertexes[i]->Position.z /= size;

                sphere->vertexes[i]->Normal.x /= size;
                sphere->vertexes[i]->Normal.y /= size;
                sphere->vertexes[i]->Normal.z /= size;
            }
            radius /= size;
        }

        return scale;
    }

    void move_sleeve() {
        center.x += 0.003;
        center.y += 0.005;
        for (int i = 0; i < sphere->vertexes.size(); i++) {
            sphere->vertexes[i]->Position.x += 0.003;
            sphere->vertexes[i]->Position.y += 0.005;
        }
    }
};