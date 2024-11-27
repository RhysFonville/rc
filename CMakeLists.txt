cmake_minimum_required(VERSION 3.28)
project(
	rc
	VERSION 1.0
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED)
set(EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE DEBUG)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})

add_executable(${PROJECT_NAME} main.cpp)
add_executable(testrc tests/test_compiler.cpp)

#target_include_directories(${PROJECT_NAME} PRIVATE compiler.h)
#target_include_directories(${PROJECT_NAME} PRIVATE preprocessor.h)
#target_include_directories(${PROJECT_NAME} PRIVATE util.h)

