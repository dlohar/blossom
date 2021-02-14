cmake_minimum_required(VERSION 3.5)

# set the project name
project(reactorSimulation_aflgo)

# add the executable
add_executable(reactorSimulation_aflgo reactorSimulation_aflgo.cc)

# link math library
target_link_libraries(reactorSimulation_aflgo m)
