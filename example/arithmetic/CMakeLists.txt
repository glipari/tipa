# Add include directories
include_directories (.)
include_directories (../../src)

set (EXECUTABLE_NAME arithmetic)
set (EXECUTABLE_SOURCES arithmetic.cpp)

# Create the executable.
add_executable (${EXECUTABLE_NAME} ${EXECUTABLE_SOURCES})

target_compile_features (${EXECUTABLE_NAME} PRIVATE cxx_range_for)

# Indicate that the executable needs tipalibrary.
target_link_libraries (${EXECUTABLE_NAME} ${PROJECT_NAME})
