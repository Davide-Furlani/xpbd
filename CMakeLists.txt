cmake_minimum_required(VERSION 3.22)

# --- Project ---
project(
        Cloth_Simulation
        VERSION 1.0
        LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)



# testing
enable_testing()

# --- Include guards ---
if (PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(
            FATAL_ERROR
            "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
    )
endif ()

# --- Add source files ---
add_subdirectory(src)
add_subdirectory(third_party/glm)


# glad library
add_library(glad STATIC
        third_party/glad/glad.c)
target_include_directories(glad PRIVATE ${CMAKE_SOURCE_DIR}/src)


set(INSTALL_DIR ${PROJECT_SOURCE_DIR}/bin)
install(TARGETS cloth_sim
        RUNTIME DESTINATION ${INSTALL_DIR})

