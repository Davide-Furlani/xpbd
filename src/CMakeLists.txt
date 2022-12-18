# --- Source code ----
add_subdirectory(Headers/glm)

# glad library
add_library(glad STATIC
        Headers/glad/glad.c)
target_include_directories(glad PRIVATE ${CMAKE_SOURCE_DIR}/src)


add_executable(cloth_sim 
        main.cpp)

target_include_directories(cloth_sim PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/Headers/GLFW)

target_link_libraries(cloth_sim PRIVATE
        glfw
        glad)
