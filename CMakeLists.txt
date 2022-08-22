
cmake_minimum_required(VERSION 3.10)

project(gunc VERSION 0.1)

find_package(Libwebsockets CONFIG REQUIRED)
find_library(WS websockets REQUIRED)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(gunc
  src/ht.c
  src/mjson.c
  src/log.c
  src/dup.c
  src/gun.c
  src/com.c
  src/url.c)

add_executable(gun src/cli.c)

include_directories("include")

target_link_libraries(gunc ${WS})
target_link_libraries(gun gunc ${WS})
