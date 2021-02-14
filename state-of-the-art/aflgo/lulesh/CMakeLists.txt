cmake_minimum_required(VERSION 3.5)

# set the project name
project(lulesh_aflgo)

# add the executable
add_executable(lulesh_aflgo lulesh_aflgo.cc)

# link math library
target_link_libraries(lulesh_aflgo m)
