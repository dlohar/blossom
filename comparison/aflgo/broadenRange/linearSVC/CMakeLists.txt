cmake_minimum_required(VERSION 3.5)

# set the project name
project(linearSVC_aflgo)

# include computeRange as a header directory
include_directories(../../computeRange)

# add the executable
add_executable(linearSVC_aflgo linearSVC_aflgo.c ../../computeRange/compute_range.c)

# link math library
target_link_libraries(linearSVC_aflgo m)
