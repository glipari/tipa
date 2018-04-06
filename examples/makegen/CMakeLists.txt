# Add include directories
include_directories (.)
include_directories (../../src)

set (EXECUTABLE_NAME makegen)
set (EXECUTABLE_SOURCES makegen.cpp)

# Create the executable.
add_executable (${EXECUTABLE_NAME} ${EXECUTABLE_SOURCES})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/script.txt
    ${CMAKE_CURRENT_BINARY_DIR}/script.txt COPYONLY)

target_compile_features (${EXECUTABLE_NAME} PRIVATE cxx_range_for)

# Indicate that the executable needs tipalibrary.
target_link_libraries (${EXECUTABLE_NAME} ${PROJECT_NAME})
