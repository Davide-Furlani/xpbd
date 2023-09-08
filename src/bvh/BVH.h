#pragma once

#include <iostream>


#include "SphereBVH.h"
#include "DisplayRigid.h"
#include "animation/gerarchy.h"
#include "display/camera.h"
#include "state/state.h"


class BVH
{
public:
    Model* model;
    std::map<string, glm::vec3>* map_bone;
    vector<SphereBVH*> bounding_spheres_bones;

    vector<Ball> bones;
    vector<BallRender> bonesRender;

    BVH(Model* m, std::map<string, glm::vec3>* m_b, render::Camera &cam, render::State &s)
    {
        model = m;
        map_bone = m_b;
        initBVH(cam, s);
    }
    ~BVH()
    {
    }

    //Funzione che inizializza la struttura BVH
    void initBVH(render::Camera &cam, render::State &s) {
        vector<Pair> AtoT_gerarchy = create_AtoT_gerarchy();
        int root_pos = 0;
        for (auto it = map_bone->begin(); it != map_bone->end(); ++it) {
            std::string boneName = it->first;
            glm::vec3 bone = (*map_bone)[boneName];
            bounding_spheres_bones.push_back(new SphereBVH(boneName, glm::vec3(bone.x, bone.y, bone.z), 0.5));
            if (boneName == "Root")
                root_pos = bounding_spheres_bones.size()-1;
        }
        int size_bSphere = bounding_spheres_bones.size();

        for (int i = 0; i < AtoT_gerarchy.size(); i++) {
            std::string boneName = AtoT_gerarchy[i].bone1 + "_" + AtoT_gerarchy[i].bone2;
            bounding_spheres_bones.push_back(new SphereBVH(boneName, glm::vec3(0.0, 0.0, 0.0), 0.5));
            bounding_spheres_bones[bounding_spheres_bones.size() - 1]->hasParents = true;

            for (int j = 0; j < size_bSphere; j++) {
                if (AtoT_gerarchy[i].bone1 == bounding_spheres_bones[j]->name)
                    bounding_spheres_bones[bounding_spheres_bones.size() - 1]->center_p1 = &(bounding_spheres_bones[j]->center);
                if (AtoT_gerarchy[i].bone2 == bounding_spheres_bones[j]->name)
                    bounding_spheres_bones[bounding_spheres_bones.size() - 1]->center_p2 = &(bounding_spheres_bones[j]->center);
            }

            glm::vec3 c1 = *bounding_spheres_bones[bounding_spheres_bones.size() - 1]->center_p1;
            glm::vec3 c2 = *bounding_spheres_bones[bounding_spheres_bones.size() - 1]->center_p2;
            bounding_spheres_bones[bounding_spheres_bones.size() - 1]->center = glm::vec3((c1.x + c2.x) / 2, (c1.y + c2.y) / 2, (c1.z + c2.z) / 2);
        }

        //Aumento man mano le sfere
        bool iter = false;
        while (iter == false) {
            for (int i = 0; i < bounding_spheres_bones.size(); i++) {
                if (i != root_pos) {
                    for (int j = 0; j < model->meshes[0].triangles.size(); j++) {
                        if (model->meshes[0].triangles[j].isInsideSphere == false) {
                            bool result = bounding_spheres_bones[i]->checkInSphere(model->meshes[0].triangles[j]);
                            if (result == true) {
                                model->meshes[0].triangles[j].isInsideSphere = true;
                                bounding_spheres_bones[i]->triangles_list.push_back(&model->meshes[0].triangles[j]);
                            }
                        }
                    }
                }
            }
            int franco = std::count_if(model->meshes[0].triangles.begin(), model->meshes[0].triangles.end(),[](Triangle t){return t.isInsideSphere;});
            float curr_radius = bounding_spheres_bones[0]->radius;
            iter = check();
            if (iter == false) {
                for (int j = 0; j < bounding_spheres_bones.size(); j++) {
                    if (j != root_pos)
                        bounding_spheres_bones[j]->radius += 0.5;
                }
            }
            if (iter == true) {
                int sum = 0;
                for (auto &bv: bounding_spheres_bones){
                    bv->radius += 0.1;
                    sum += bv->triangles_list.size();
                }
            }
        }

        for (int i = 0; i < bounding_spheres_bones.size(); i++) {
            if (i == root_pos) {
                bones.push_back(Ball(bounding_spheres_bones[i]->center, bounding_spheres_bones[i]->radius, glm::vec4(1.0f, 0.0f, 0.0f, 0.3f)));
            }
            else {
                bones.push_back(Ball(bounding_spheres_bones[i]->center, bounding_spheres_bones[i]->radius, glm::vec4(1.0f, 1.0f, 0.0f, 0.3f)));
            }
            bonesRender.push_back(BallRender(&bones[i], cam, s));
        }
    }

    bool check() {
        int count = 0;
        for (int i = 0; i < model->meshes[0].triangles.size(); i++) {
            if (model->meshes[0].triangles[i].isInsideSphere == true)
                count++;
        }

        if (count == model->meshes[0].triangles.size())
            return true;
        return false;
    }

    // Funzione che modifica la posizione dei bone durante l'animazione
    void modify(std::map<std::string, glm::mat4> BonePositions, int boneCount) {
        int count = 0;

        for (auto it = BonePositions.begin(); it != BonePositions.end(); ++it) {
            std::string boneName = it->first;
            glm::vec3 newBonePos = BonePositions[boneName][3];

            glm::vec3 traslazione = glm::vec3(newBonePos.x, newBonePos.y, newBonePos.z) - bones[count].center;
            bones[count].center = glm::vec3(newBonePos.x, newBonePos.y, newBonePos.z); //Nuovo centro
            bounding_spheres_bones[count]->center = glm::vec3(newBonePos.x, newBonePos.y, newBonePos.z);

            for (int j = 0; j < bones[count].sphere->vertexes.size(); j++) {
                bones[count].sphere->vertexes[j]->Position += traslazione;
            }
            count++;
        }

        //printf("(%f, %f, %f)\n", bounding_spheres_bones[10]->center.x, bounding_spheres_bones[10]->center.y, bounding_spheres_bones[10]->center.z);
        for (int i = boneCount; i < bones.size(); i++) {
            if (bounding_spheres_bones[i]->hasParents == true) {
                glm::vec3 c1 = *bounding_spheres_bones[i]->center_p1;
                glm::vec3 c2 = *bounding_spheres_bones[i]->center_p2;
                glm::vec3 new_center = glm::vec3((c1.x + c2.x) / 2, (c1.y + c2.y) / 2, (c1.z + c2.z) / 2);

                glm::vec3 traslazione = glm::vec3(new_center.x, new_center.y, new_center.z) - bones[i].center;
                bones[i].center = glm::vec3(new_center.x, new_center.y, new_center.z); //Nuovo centro
                bounding_spheres_bones[i]->center = glm::vec3(new_center.x, new_center.y, new_center.z);

                for (int j = 0; j < bones[i].sphere->vertexes.size(); j++) {
                    bones[i].sphere->vertexes[j]->Position += traslazione;
                }
            }
        }
    }

    void flush(render::Camera &cam) {
        for (int i = 0; i < bones.size(); i++)
            bonesRender[i].flush(cam);
    }

    void showInfo() {
        printf("\n--------------------------------- BVH informations ---------------------------------\n");
        for (int i = 0; i < bounding_spheres_bones.size(); i++) {
            if(bounding_spheres_bones[i]->hasParents == false)
                printf("%i) [%s] with %li triangles associated at (%f, %f, %f) -> BONE\n", i+1, bounding_spheres_bones[i]->name.c_str(), bounding_spheres_bones[i]->triangles_list.size(), bounding_spheres_bones[i]->center.x, bounding_spheres_bones[i]->center.y, bounding_spheres_bones[i]->center.z);
            else
                printf("%i) [%s] with %li triangles associated at (%f, %f, %f) -> SUPPORT\n", i + 1, bounding_spheres_bones[i]->name.c_str(), bounding_spheres_bones[i]->triangles_list.size(), bounding_spheres_bones[i]->center.x, bounding_spheres_bones[i]->center.y, bounding_spheres_bones[i]->center.z);
        }
        printf("\n");
    }
};