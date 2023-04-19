cmake_minimum_required(VERSION 3.15)

project (tipa VERSION 0.5)

set(CMAKE_CXX_STANDARD 17)            # C++17...
set(CMAKE_CXX_STANDARD_REQUIRED True) #...is required...
set(CMAKE_CXX_EXTENSIONS OFF)         #...without compiler extensions like gnu++11
set(CMAKE_CXX_FLAGS "-Wall -O3") 

set(CMAKE_INSTALL_PREFIX /usr/local/)

add_subdirectory (src)
add_subdirectory (examples)

enable_testing()
add_subdirectory (test)

# include(InstallRequiredSystemLibraries)
# set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
# set(CPACK_PACKAGE_VERSION_MAJOR "${tipa_VERSION_MAJOR}")
# set(CPACK_PACKAGE_VERSION_MINOR "${tipa_VERSION_MINOR}")
# include(CPack)


## For the find_package ..
include(CMakePackageConfigHelpers)
# generate the config file that includes the exports
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/tipaConfig.cmake"
  INSTALL_DESTINATION "lib/cmake/example"
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
  )
# generate the version file for the config file
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/tipaConfigVersion.cmake"
  VERSION "${Tutorial_VERSION_MAJOR}.${Tutorial_VERSION_MINOR}"
  COMPATIBILITY AnyNewerVersion
)

install(EXPORT tipaTargets
  FILE tipaTargets.cmake
  DESTINATION lib/cmake/tipa)

# install the configuration file
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/tipaConfig.cmake
  DESTINATION lib/cmake/tipa
  )
