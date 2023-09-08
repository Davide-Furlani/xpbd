#pragma once

#include <vector>
#include <map>
#include <glm.hpp>
#include <gtc/type_ptr.hpp>
#include <assimp/scene.h>
#include "bone.h"
#include <functional>
#include "animdata.h"
#include "model_animation.h"

struct AssimpNodeData
{
    glm::mat4 transformation;
    std::string name;
    int childrenCount;
    std::vector<AssimpNodeData> children;
};

class Animation
{
public:
    Animation() = default;

    Animation(const std::string& animationPath, Model* model)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate | aiProcess_FlipUVs);
        assert(scene && scene->mRootNode);
        auto animation = scene->mAnimations[0];
        m_Duration = animation->mDuration;
        m_TicksPerSecond = animation->mTicksPerSecond;
        aiMatrix4x4 globalTransformation = scene->mRootNode->mTransformation;
        globalTransformation = globalTransformation.Inverse();
        ReadHierarchyData(m_RootNode, scene->mRootNode);
        ReadMissingBones(animation, *model);
        CalculateFinalBonePositions(animation, m_RootNode, *model, globalTransformation);
    }

    ~Animation()
    {
    }

    Bone* FindBone(const std::string& name)
    {
        auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
                                 [&](const Bone& Bone)
                                 {
                                     return Bone.GetBoneName() == name;
                                 }
        );
        if (iter == m_Bones.end()) return nullptr;
        else return &(*iter);
    }

    inline float GetTicksPerSecond() { return m_TicksPerSecond; }
    inline float GetDuration() { return m_Duration; }
    inline const AssimpNodeData& GetRootNode() { return m_RootNode; }
    inline const std::map<std::string, BoneInfo>& GetBoneIDMap() { return m_BoneInfoMap; }

private:
    void ReadMissingBones(const aiAnimation* animation, Model& model)
    {
        int size = animation->mNumChannels;

        auto& boneInfoMap = model.GetBoneInfoMap();//getting m_BoneInfoMap from Model class
        int& boneCount = model.GetBoneCount(); //getting the m_BoneCounter from Model class

        //reading channels(bones engaged in an animation and their keyframes)
        for (int i = 0; i < size; i++)
        {
            auto channel = animation->mChannels[i];
            std::string boneName = channel->mNodeName.data;
            if (boneInfoMap.find(boneName) == boneInfoMap.end())
            {
                boneInfoMap[boneName].id = boneCount;
                boneCount++;
            }
            m_Bones.push_back(Bone(channel->mNodeName.data,
                                   boneInfoMap[channel->mNodeName.data].id, channel));
        }

        m_BoneInfoMap = boneInfoMap;
    }

    void ReadHierarchyData(AssimpNodeData& dest, const aiNode* src)
    {
        assert(src);

        dest.name = src->mName.data;
        dest.transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
        dest.childrenCount = src->mNumChildren;

        for (int i = 0; i < src->mNumChildren; i++)
        {
            AssimpNodeData newData;
            ReadHierarchyData(newData, src->mChildren[i]);
            dest.children.push_back(newData);
        }
    }


    float m_Duration;
    double m_TicksPerSecond;
    std::vector<Bone> m_Bones;
    AssimpNodeData m_RootNode;
    std::map<std::string, BoneInfo> m_BoneInfoMap;


public:
    std::map<string, glm::vec3> bonePositions;

    //Funzione che calcola la posizione del nodo nel mondo in base al nodo radice.
    //La ricorsione termina quando non ci sono pi� figli nella catena e la posizione nel mondo � stata individuata
    void CalculateInitialBonePositions(const AssimpNodeData& node, const glm::mat4& parentTransform, std::map<std::string, glm::vec3>& bonePositions, const glm::mat4& globalTransform)
    {
        glm::mat4 currentTransform = parentTransform * node.transformation;
        glm::mat4 globalCurrentTransform = globalTransform * currentTransform;

        if (bonePositions.find(node.name) != bonePositions.end())
        {
            bonePositions[node.name] = globalCurrentTransform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        }
        for (const auto& child : node.children)
        {
            CalculateInitialBonePositions(child, currentTransform, bonePositions, globalTransform);
        }
    }



    void CalculateFinalBonePositions(const aiAnimation* animation, AssimpNodeData root, Model& model, aiMatrix4x4 globalTrasformation) {
        AssimpNodeData rootNode = root;


        // Inizializzazione del vettore bonePositions
        for (auto it = model.GetBoneInfoMap().begin(); it != model.GetBoneInfoMap().end(); ++it) {
            bonePositions[it->first] = glm::vec3(0.0f);
        }

        // Calcolo della matrice di trasformazione globale dell'intero modello
        glm::mat4 globalTransform = glm::make_mat4(&globalTrasformation.a1);

        // Calcolo delle posizioni iniziali dei bones
        CalculateInitialBonePositions(rootNode, glm::mat4(1.0f), bonePositions, globalTransform);

        // Stampa delle coordinate dei bones
        for (auto it = model.GetBoneInfoMap().begin(); it != model.GetBoneInfoMap().end(); ++it) {
            std::string boneName = it->first;
            glm::vec3 singleBonePosition = bonePositions[boneName];
        }
    }

    void showBoneInfo() {
        printf("\nPosizione iniziale nel mondo dei bones: \n");
        for (auto it = bonePositions.begin(); it != bonePositions.end(); ++it)
            printf("    [%s]: (%f, %f, %f)\n", (it->first).c_str(), (bonePositions[it->first]).x, (bonePositions[it->first]).y, (bonePositions[it->first]).z);
        printf("\n");
    }

};