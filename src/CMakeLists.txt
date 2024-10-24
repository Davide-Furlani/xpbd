# --- Source code ----

file(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})



add_executable(cloth_sim 
        main.cpp)

target_include_directories(cloth_sim PRIVATE
        ${CMAKE_SOURCE_DIR}/third_party
        )
find_package(glad CONFIG REQUIRED)
target_link_libraries(cloth_sim PRIVATE
        assimp
        glfw
        glad::glad
        glm::glm
        )
