cmake_minimum_required(VERSION 3.1.2)
project(CWTeamsCpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_GENERATOR_PLATFORM x64)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

set(CMAKE_GENERATOR_PLATFORM x64)

add_executable(CWTeamsCpp src/Main.cpp src/GenerateTeams.cpp src/Weights.cpp src/ExcelUtils.cpp)
target_link_libraries(CWTeamsCpp ${CONAN_LIBS})

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/xlnt)
target_link_libraries(CWTeamsCpp xlnt)
