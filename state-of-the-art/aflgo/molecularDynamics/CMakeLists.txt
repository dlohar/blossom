cmake_minimum_required(VERSION 3.5)

# set the project name
project(molecularDynamics_aflgo)

# add the executable
add_executable(molecularDynamics_aflgo molecularDynamics_aflgo.cc)

# link math library
target_link_libraries(molecularDynamics_aflgo m)
