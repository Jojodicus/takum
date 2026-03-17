cmake_minimum_required(VERSION 3.16)
project(takum LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

add_library(takum INTERFACE)
target_include_directories(takum INTERFACE src/)

enable_testing()
add_subdirectory(tests)
