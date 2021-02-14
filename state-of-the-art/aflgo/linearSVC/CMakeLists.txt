cmake_minimum_required(VERSION 3.5)

# set the project name
project(linearSVC_aflgo)

# add the executable
add_executable(linearSVC_aflgo linearSVC_aflgo.c)

# link math library
target_link_libraries(linearSVC_aflgo m)
