cmake_minimum_required(VERSION 2.8.12)
project(CWTeamsCpp)

add_definitions("-std=c++17")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(CWTeamsCpp src/Main.cpp src/GenerateTeams.cpp)
target_link_libraries(CWTeamsCpp ${CONAN_LIBS})

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/xlnt)
target_link_libraries(CWTeamsCpp xlnt)
