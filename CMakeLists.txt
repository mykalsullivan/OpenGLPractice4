cmake_minimum_required(VERSION 3.28)
project(OpenGLPractice4)

set(CMAKE_CXX_STANDARD 23)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)

include_directories(
        ${OPENGL_INCLUDE_DIRS}
        ${GLEW_INCLUDE_DIRS}
        ${GLM_INCLUDE_DIRS}
)

add_executable(OpenGLPractice4
        src/main.cpp
)

target_link_libraries(OpenGLPractice4
        glfw
        ${OPENGL_LIBRARIES}
        ${GLEW_LIBRARIES}
        libGLEW.so
)
