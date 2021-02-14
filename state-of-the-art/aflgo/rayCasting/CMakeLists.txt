cmake_minimum_required(VERSION 3.5)

# set the project name
project(rayCasting_aflgo)

# add the executable
add_executable(rayCasting_aflgo rayCasting_aflgo.c)

# link math library
target_link_libraries(rayCasting_aflgo m)
