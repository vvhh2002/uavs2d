cmake_minimum_required(VERSION 3.3)

project(utest)

aux_source_directory(. DIR_SRC_TEST)

add_subdirectory(source)
find_library(OPENGL_FRAMEWORK OpenGL)
find_library(GLUT_FRAMEWORK GLUT)
add_executable(utest ${DIR_SRC_TEST})

target_link_libraries(utest PRIVATE uavs2d m pthread ${GLUT_FRAMEWORK} ${OPENGL_FRAMEWORK})

set_property(TARGET utest PROPERTY C_STANDARD 99)