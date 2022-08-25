include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
find_package(Check REQUIRED)

include_directories(${CHECK_INCLUDE_DIRS})

enable_testing()

link_directories(${CHECK_LIBRARY_DIRS})

add_executable(gunc.test mjson.c)
target_link_libraries(gunc.test gunc ${CHECK_LIBRARIES} m subunit)
add_test(NAME gunc COMMAND gunc.test)

