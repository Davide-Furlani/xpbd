//
// Created by davide on 28/10/24.
//

#pragma once

#include <glm/glm.hpp>

struct CollisionSphere {
    glm::vec3 center;
    float radius;

    CollisionSphere(glm::vec3 center, float radius){
        this->center = center;
        this->radius = radius;
    }

    void translate(glm::vec3 t){
        this->center = this->center + t;
    }

    void scale(float s){
        this->radius = this->radius * s;
    }
};