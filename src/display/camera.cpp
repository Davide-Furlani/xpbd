/**
* @file
* @brief Contains the implementation of Camera class.
* @author Davide Furlani
* @version 0.1
* @date January, 2023
* @copyright 2023 Davide Furlani
*/

#include "camera.h"
#include "glm.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/rotate_vector.hpp>

namespace render {
    
    Camera::Camera(vec3 pos, vec3 front_v, vec3 up_v) {

        Camera::pos = pos;
        Camera::front_v = front_v;
        Camera::up_v = up_v;
        Camera::yaw = 0.0;
        Camera::pitch = 0.0;
        Camera::roll = 0.0;
        Camera::max_pitch = 89.0;
        Camera::min_pitch = -89.0;
        Camera::speed = 7;
        Camera::sensitivity = 0.0035;
    }

    Camera::Camera(vec3 pos, vec3 front_v, vec3 up_v, float speed, float sensitivity) {

        Camera::pos = pos;
        Camera::front_v = front_v;
        Camera::up_v = up_v;
        Camera::yaw = 0.0;
        Camera::pitch = 0.0;
        Camera::roll = 0.0;
        Camera::max_pitch = 89.0;
        Camera::min_pitch = -89.0;
        Camera::speed = speed;
        Camera::sensitivity = sensitivity;
    }
    
    void Camera::forward(State& state) {
        pos -= (speed * static_cast<float>(state.delta_time)) * normalize(cross(cross(front_v, up_v), up_v));
    }

    void Camera::backwards(State& state){
        pos += (speed * static_cast<float>(state.delta_time)) * normalize(cross(cross(front_v, up_v), up_v));
    }

    void Camera::left(State& state){
        pos -= normalize(cross(front_v, up_v)) * (speed * static_cast<float>(state.delta_time));
    }

    void Camera::right(State& state){
        pos += normalize(cross(front_v, up_v)) * (speed * static_cast<float>(state.delta_time));
    }

    void Camera::up(State& state){
        pos += (speed * static_cast<float>(state.delta_time)) * up_v;
    }

    void Camera::down(State& state){
        pos -= (speed * static_cast<float>(state.delta_time)) * up_v;
    }

    void Camera::update_rotation(State& state){
        
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
}



















