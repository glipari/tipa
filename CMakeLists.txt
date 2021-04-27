cmake_minimum_required(VERSION 3.15)

project (tipa VERSION 0.5)


set(CMAKE_CXX_STANDARD 17)          # C++17...
set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...
set(CMAKE_CXX_EXTENSIONS OFF)       #...without compiler extensions like gnu++11
set(CMAKE_CXX_FLAGS "-Wall -O3") 
set(LIBRARY_TYPE STATIC)
#set(CMAKE_INSTALL_PREFIX /usr/local/)

add_subdirectory (src)

enable_testing()
add_subdirectory (test)
add_subdirectory (examples)

include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set(CPACK_PACKAGE_VERSION_MAJOR "${tipa_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${tipa_VERSION_MINOR}")
include(CPack)
