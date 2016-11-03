cmake_minimum_required(VERSION 3.3)

project(utest)

aux_source_directory(. DIR_SRC_TEST)

add_subdirectory(source)

add_executable(utest ${DIR_SRC_TEST})

target_link_libraries(utest uavs2d)
target_link_libraries(utest m)
target_link_libraries(utest pthread)
