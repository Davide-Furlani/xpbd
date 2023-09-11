cmake_minimum_required(VERSION 3.22)

# --- Project ---
project(
        XPBD
        VERSION 0.1
        LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# find custom scripts
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# testing
enable_testing()

# --- Include guards ---
if (PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(
            FATAL_ERROR
            "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
    )
endif ()

#set(CPM_SOURCE_CACHE
#        "${CMAKE_SOURCE_DIR}/third_party/cpm_sources"
#        CACHE PATH "Directory to download CPM dependencies"
#        FORCE)
# Include dependency manager
#include(CPM)
# Catch2 dependency
#CPMAddPackage(
#        NAME Catch2
#        GITHUB_REPOSITORY catchorg/Catch2
#        GIT_TAG v3.0.1
#        EXCLUDE_FROM_ALL TRUE)
#
# fmt dependency
#CPMAddPackage(
#        NAME fmt
#        GITHUB_REPOSITORY fmtlib/fmt
#        GIT_TAG 9.1.0
#        EXCLUDE_FROM_ALL TRUE)



# --- Add source files ---
add_subdirectory(third_party/glm)
add_subdirectory(third_party/glfw-3.3.8)
add_subdirectory(third_party/assimp-5.2.5)
add_subdirectory(src)

# --- Add test files ---
#add_subdirectory(tests)



# glad library
add_library(glad STATIC
        third_party/glad/glad.c)
target_include_directories(glad PRIVATE ${CMAKE_SOURCE_DIR}/src)


set(INSTALL_DIR ${PROJECT_SOURCE_DIR}/bin)
install(TARGETS cloth_sim
        RUNTIME DESTINATION ${INSTALL_DIR})

