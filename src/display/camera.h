/**
* @file
* @brief Contains the definition of Camera class.
* @author Davide Furlani
* @version 0.1
* @date January, 2023
* @copyright 2023 Davide Furlani
*/
#pragma once
#include "glm.hpp"
#include "state/state.h"

namespace render {
    using namespace glm;

    class Camera {
    public:
        vec3 pos;
        vec3 front_v;
        vec3 up_v;
        float yaw;
        float pitch;
        float roll;
        float max_pitch;
        float min_pitch;
        float speed;
        float sensitivity;
        
        Camera(vec3 pos, vec3 front, vec3 up);
        Camera(vec3 pos, vec3 front, vec3 up, float speed, float sensitivity);

        void forward(State& state);

        void backwards(State& state);

        void left(State& state);

        void right(State& state);

        void up(State& state);

        void down(State& state);

        void update_rotation(State& state);

        [[nodiscard]] mat4 GetViewMatrix() const;
        
    };
}