/**
* @file
* @brief Contains the definition of Camera class.
* @author Davide Furlani
* @version 0.1
* @date January, 2023
* @copyright 2023 Davide Furlani
*/
#pragma once
#include <glm/glm.hpp>
#include "state.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

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
        
        Camera(vec3 pos, vec3 front, vec3 up) {

            Camera::pos = pos;
            Camera::front_v = front;
            Camera::up_v = up;
            Camera::yaw = 0.0;
            Camera::pitch = 0.0;
            Camera::roll = 0.0;
            Camera::max_pitch = 89.0;
            Camera::min_pitch = -89.0;
            Camera::speed = 7;
            Camera::sensitivity = 0.0035;
        }
        Camera(vec3 pos, vec3 front, vec3 up, float speed, float sensitivity){

            Camera::pos = pos;
            Camera::front_v = front;
            Camera::up_v = up;
            Camera::yaw = 0.0;
            Camera::pitch = 0.0;
            Camera::roll = 0.0;
            Camera::max_pitch = 89.0;
            Camera::min_pitch = -89.0;
            Camera::speed = speed;
            Camera::sensitivity = sensitivity;
        }

        void forward(State& state){
            pos -= (speed * static_cast<float>(state.delta_time)) * normalize(cross(cross(front_v, up_v), up_v));
        }

        void backwards(State& state){
            pos += (speed * static_cast<float>(state.delta_time)) * normalize(cross(cross(front_v, up_v), up_v));
        }

        void left(State& state){
            pos -= normalize(cross(front_v, up_v)) * (speed * static_cast<float>(state.delta_time));
        }

        void right(State& state){
            pos += normalize(cross(front_v, up_v)) * (speed * static_cast<float>(state.delta_time));
        }

        void up(State& state){
            pos += (speed * static_cast<float>(state.delta_time)) * up_v;
        }

        void down(State& state){
            pos -= (speed * static_cast<float>(state.delta_time)) * up_v;
        }

        void update_rotation(State& state){

            if(state.current_time_from_start < 0.5)
                return;

            yaw = sensitivity * state.delta_mouseX_pos;
            pitch = sensitivity * state.delta_mouseY_pos;

            if(pitch > max_pitch)
                pitch = max_pitch;
            if(pitch < min_pitch)
                pitch = min_pitch;

            front_v = normalize(rotate(front_v, pitch, cross(front_v, up_v)));
            front_v = normalize(rotateZ(front_v, -yaw));
        }
        
    };
}