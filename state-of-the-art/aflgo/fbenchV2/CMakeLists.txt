cmake_minimum_required(VERSION 3.5)

# set the project name
project(fbench_aflgo)

# add the executable
add_executable(fbench_aflgo fbench_aflgo.c)

# link math library
target_link_libraries(fbench_aflgo m)