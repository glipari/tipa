cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 11) # C++11...
set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...
set(CMAKE_CXX_EXTENSIONS OFF) #...without compiler extensions like gnu++11

project (tipa)

set(LIBRARY_TYPE STATIC)

add_subdirectory (src)
# add_subdirectory (examples)

enable_testing (true)
add_subdirectory (test)
add_subdirectory (examples)

