
cmake_minimum_required(VERSION 3.10)

project(gunc VERSION 0.1)

add_library(gunc src/gun.c)

target_include_directories(gunc PUBLIC "${CMAKE_ROOT}/include")
