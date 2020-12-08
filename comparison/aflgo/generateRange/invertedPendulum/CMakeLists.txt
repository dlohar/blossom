cmake_minimum_required(VERSION 3.5)

# set the project name
project(invertedPendulum_aflgo)

# include computeRange as a header directory
include_directories(../../computeRange)

# add the executable
add_executable(invertedPendulum_aflgo invertedPendulum_aflgo.c ../../computeRange/compute_range.c)

# link math library
target_link_libraries(invertedPendulum_aflgo m)
