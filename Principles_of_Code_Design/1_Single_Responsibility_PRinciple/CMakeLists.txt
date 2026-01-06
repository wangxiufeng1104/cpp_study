# Single Responsibility Principle Demonstration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create executable for SRP demo
add_executable(srp_demo
    srp_demo.cpp
    bad_design.h
    good_design.h
)

# Link common library if needed
# target_link_libraries(srp_demo PRIVATE common)

# Set output directory
set_target_properties(srp_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
