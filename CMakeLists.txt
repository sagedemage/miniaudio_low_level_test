cmake_minimum_required(VERSION 3.22 VERSION 0.1.0)

project(out)

file(GLOB_RECURSE SOURCE_FILES "src/*.c" "src/*.cpp" "src/*.hpp")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(STANDARD_CXX_VERSION_FLAG "-std=c++11")
set(OPTIMIZE_FLAG "-O3")
set(WARNING_FLAGS "-Werror -Wpedantic -Wall -Wextra -Wno-unused-parameter")
set(SIMD_OPTIMIZE_FLAGS "-msse2 -mavx2")

file(COPY audio DESTINATION ${CMAKE_BINARY_DIR})
file(COPY lint_codebase.sh DESTINATION ${CMAKE_BINARY_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${STANDARD_CXX_VERSION_FLAG} ${OPTIMIZE_FLAG} ${SIMD_OPTIMIZE_FLAGS} ${WARNING_FLAGS}")

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

include_directories(SYSTEM include)

#target_include_directories(${PROJECT_NAME} PUBLIC include)

target_link_libraries(${PROJECT_NAME} -ldl -lpthread -lm)

target_precompile_headers(${PROJECT_NAME} PRIVATE ${HEADER_FILES})
