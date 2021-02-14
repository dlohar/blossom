cmake_minimum_required(VERSION 3.5)

# set the project name
project(invertedPendulum_aflgo)

# add the executable
add_executable(invertedPendulum_aflgo invertedPendulum_aflgo.c)

# link math library
target_link_libraries(invertedPendulum_aflgo m)
