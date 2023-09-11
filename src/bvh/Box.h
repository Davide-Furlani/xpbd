#pragma once

#include "glm.hpp"

struct Box
{
    // Front
    glm::vec3 up_right_front = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 up_left_front = glm::vec3(0.0f, 1.0f, 1.0f);
    glm::vec3 down_left_front = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 down_right_front = glm::vec3(1.0f, 0.0f, 1.0f);

    //Back
    glm::vec3 up_right_back = glm::vec3(1.0f, 1.0f, 0.0f);
    glm::vec3 up_left_back = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 down_left_back = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 down_right_back = glm::vec3(1.0f, 0.0f, 0.0f);

    //Center
    glm::vec3 box_center = glm::vec3(0.0f, 0.0f, 0.0f);
};

/*
                   up_left_back                          up_right_back
                        * * * * * * * * * * * * * * * * * * * *  
                      *                                     * *
                    *                                     *   *
  up_left_front   *                    up_right_front   *     *
                * * * * * * * * * * * * * * * * * * * *       *
                *     *                               *       *
                *     *                               *       *
                *     *                               *       *
                *     *                               *       *
                *     *           box_center          *       *
                *     *               *               *       *
                *     *                               *       *
                *     *                               *       *
                *     *                               *       *
                *     *                               *       *
                *     *                               *       *
                *     *  down_left_back               *       * down_right_back
                *     * * * * * * * * * * * * * * * * *     * 
                *   *                                 *   *
                * *                                   * *
                * * * * * * * * * * * * * * * * * * * * 
          down_left_front                        down_right_front
 */