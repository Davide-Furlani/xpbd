# --- Source code ----

file(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})



# node library
add_library(node STATIC
        node/node.cpp)
target_include_directories(node PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(node PRIVATE ${CMAKE_SOURCE_DIR}/third_party/glm)



# hashgrid library
add_library(hashgrid STATIC
        hashgrid/hashgrid.cpp)
target_include_directories(hashgrid PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(hashgrid PRIVATE ${CMAKE_SOURCE_DIR}/third_party/glm)




# constraints library
add_library(constr STATIC
        constraints/constraint.cpp
        )
target_include_directories(constr PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(constr PRIVATE ${CMAKE_SOURCE_DIR}/third_party/glm)
target_link_libraries(constr PRIVATE node)



# state library
add_library(state STATIC
        state/state.cpp
        )
target_include_directories(state PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(state PRIVATE
        ${CMAKE_SOURCE_DIR}/third_party/glad
        ${CMAKE_SOURCE_DIR}/third_party/glfw-3.3.8
        ${CMAKE_SOURCE_DIR}/third_party/glm
        )
target_link_libraries(state glfw)






# display library
add_library(display STATIC
        display/display.cpp
        display/camera.cpp
        )
target_include_directories(display PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(display PRIVATE 
        ${CMAKE_SOURCE_DIR}/third_party/glm
        ${CMAKE_SOURCE_DIR}/third_party/glfw-3.3.8
        ${CMAKE_SOURCE_DIR}/third_party/glad
        ${CMAKE_SOURCE_DIR}/third_party/stb)
target_link_libraries(display PRIVATE 
        cloth
        state
        glfw
        )



# cloth library
add_library(cloth STATIC
        cloth/cloth.cpp)
target_include_directories(cloth PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(cloth PRIVATE
        ${CMAKE_SOURCE_DIR}/third_party/glad
        ${CMAKE_SOURCE_DIR}/third_party/glm)
target_link_libraries(cloth PRIVATE 
        constr 
        state
        hashgrid
        display)





add_executable(cloth_sim 
        main.cpp)

target_include_directories(cloth_sim PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/third_party/glfw-3.3.8
        ${CMAKE_SOURCE_DIR}/third_party/glad
        ${CMAKE_SOURCE_DIR}/third_party/glm
        ${CMAKE_SOURCE_DIR}/third_party/stb)

target_link_libraries(cloth_sim PRIVATE
        glfw
        glad
        node
        hashgrid
        state
        constr
        cloth
        display
        )
