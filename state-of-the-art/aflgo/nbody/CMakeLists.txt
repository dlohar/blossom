cmake_minimum_required(VERSION 3.5)

# set the project name
project(nbody_aflgo)

# add the executable
add_executable(nbody_aflgo nbody_aflgo.c)

# link math library
target_link_libraries(nbody_aflgo m)
