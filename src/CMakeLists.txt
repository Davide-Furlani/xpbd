# --- Source code ----


add_executable(cloth_sim 
        main.cpp)

target_include_directories(cloth_sim PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/third_party/GLFW
        ${CMAKE_SOURCE_DIR}/third_party/glad
        ${CMAKE_SOURCE_DIR}/third_party/glm)

target_link_libraries(cloth_sim PRIVATE
        glfw
        glad)
