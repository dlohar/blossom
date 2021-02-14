add_library(
GuidedBlackBoxTesting
SHARED
GuidedBlackBoxTester.cpp
)

set_target_properties(GuidedBlackBoxTesting PROPERTIES PREFIX "")
set_target_properties(GuidedBlackBoxTesting PROPERTIES LIBRARY_OUTPUT_NAME "guided_black_box_pass")
