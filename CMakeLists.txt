# cmake_minimum_required(VERSION 3.15)

# project(GraphicsStuff)
# # Add executable target
# add_executable(${PROJECT_NAME} 
#     src/main.cpp 
#     src/glad.c
#     src/shader.h
# )

# target_include_directories(
#     ${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
#     ${PROJECT_NAME} PUBLIC 
#     ${CMAKE_CURRENT_SOURCE_DIR}/include
# )

# # Link libraries
# target_link_libraries(${PROJECT_NAME} 
#     dl 
#     glfw 
#     GL
# )
cmake_minimum_required(VERSION 3.15)

project(GraphicsStuff)

# Add executable target
add_executable(${PROJECT_NAME} 
    src/main.cpp 
    src/glad.c
    src/shader.h
)

target_include_directories(
    ${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    glfw 
    dl
    GL
)

# Set the MinGW runtime to be statically linked
if(MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
endif(MINGW)