add_library(
BlackBoxTesting
SHARED
BlackBoxTester.cpp
)

set_target_properties(BlackBoxTesting PROPERTIES PREFIX "")
set_target_properties(BlackBoxTesting PROPERTIES LIBRARY_OUTPUT_NAME "black_box_pass")
