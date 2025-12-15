cmake_minimum_required(VERSION 3.10)
project(backend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# include and source dirs
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
file(GLOB BACKEND_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(backend STATIC ${BACKEND_SRC})
target_include_directories(backend PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# place archive in bin folder under backend (relative to source dir)
set_target_properties(backend PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
)
