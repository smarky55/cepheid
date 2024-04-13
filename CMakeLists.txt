cmake_minimum_required(VERSION 3.25)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(cepheid CXX)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(compiler)