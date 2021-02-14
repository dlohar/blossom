cmake_minimum_required(VERSION 3.5)

# set the project name
project(linpack_aflgo)

# add the executable
add_executable(linpack_aflgo linpack_aflgo.c)

# link math library
target_link_libraries(linpack_aflgo m)
