#pragma once

#include "animation/model_animation.h"

// -------------------------------- Sphere BVH structure --------------------------------
struct SphereBVH
{
    string name;
    Vec3 center;
    float radius;
    vector<Triangle*> triangles_list;

    bool hasParents = false;
    Vec3* center_p1 = nullptr;
    Vec3* center_p2 = nullptr;

    SphereBVH(string n, Vec3 c, float r)
    {
        name = n;
        center = c,
                radius = r;
    }



    // Check se il triangolo � nella sfera 
    bool checkInSphere(Triangle t) {
        double dist = 0;
        if ((center - t.v1->Position).length() <= radius) {
            if ((center - t.v2->Position).length() <= radius) {
                if ((center - t.v3->Position).length() <= radius)
                    return true;
            }
        }
        return false;
    }

    // Stampa dei triangoli appartenenti alla sfera 
    void showTriangles() {
        for (int i = 0; i < triangles_list.size(); i++)

            printf("[t%i]: v1:(%f, %f, %f), v2:(%f, %f, %f), v3: (%f, %f, %f)\n", i,
                   triangles_list[i]->v1->Position.x, triangles_list[i]->v1->Position.y, triangles_list[i]->v1->Position.z,
                   triangles_list[i]->v2->Position.x, triangles_list[i]->v2->Position.y, triangles_list[i]->v2->Position.z,
                   triangles_list[i]->v3->Position.x, triangles_list[i]->v3->Position.y, triangles_list[i]->v3->Position.z);
    }
};
