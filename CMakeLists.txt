cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17) # C++17...
set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...
set(CMAKE_CXX_EXTENSIONS OFF) #...without compiler extensions like gnu++11
set(CMAKE_CXX_FLAGS "-Wall -O3") 

project (tipa)

set(LIBRARY_TYPE STATIC)

add_subdirectory (src)
# add_subdirectory (examples)

enable_testing (true)
add_subdirectory (test)
add_subdirectory (examples)

