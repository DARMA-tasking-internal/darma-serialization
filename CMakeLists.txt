
cmake_minimum_required(VERSION 3.4)

project (DarmaSerialization
  VERSION 0.5
  LANGUAGES CXX
)

set(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
)

################################################################################
# Options
################################################################################

option(DARMA_SERIALIZATION_ENABLE_TESTING "Enable testing for DARMA Serialization" OFF)
option(DARMA_SERIALIZATION_TESTING_SINGLE_EXECUTABLE "build tests as a single executable" OFF)
mark_as_advanced(DARMA_SERIALIZATION_TESTING_SINGLE_EXECUTABLE)
option(DARMA_SERIALIZATION_ENABLE_COVERALLS "Enable coveralls coverage suport" Off)
option(DARMA_SERIALIZATION_COVERALLS_UPLOAD "Upload coveralls results" Off)
mark_as_advanced(DARMA_SERIALIZATION_ENABLE_COVERALLS)
mark_as_advanced(DARMA_SERIALIZATION_COVERALLS_UPLOAD)

option(DARMA_SERIALIZATION_BUILD_EXAMPLES "Build examples for DARMA serialization" Off)


################################################################################
# darma_serialization interface library
################################################################################

add_library(darma_serialization INTERFACE)
add_library(darma_serialization::darma_serialization ALIAS darma_serialization)

target_include_directories(darma_serialization INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source/include>
  $<INSTALL_INTERFACE:include>
)
target_compile_features(darma_serialization INTERFACE cxx_std_14)

install(TARGETS darma_serialization EXPORT darmaSerializationTargets
  INCLUDES DESTINATION include
)

install(EXPORT darmaSerializationTargets
  FILE darmaSerializationTargets.cmake
  NAMESPACE darma_serialization::
  DESTINATION cmake
)

export(TARGETS darma_serialization
  NAMESPACE darma_serialization::
  FILE darmaSerializationTargets.cmake
)

################################################################################
# Dependencies
################################################################################

find_package(DarmaUtility REQUIRED 0.5)

target_link_libraries(darma_serialization INTERFACE darma_utility::darma_utility)

install(DIRECTORY source/include/darma/serialization DESTINATION include/darma FILES_MATCHING PATTERN "*.h")


################################################################################
# Create the hooks for FindPackage to use
################################################################################

include(CMakePackageConfigHelpers)
configure_package_config_file(cmake/DarmaSerializationConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/DarmaSerializationConfig.cmake
  INSTALL_DESTINATION cmake
)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/DarmaSerializationConfigVersion.cmake
  COMPATIBILITY SameMajorVersion
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/DarmaSerializationConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/DarmaSerializationConfigVersion.cmake
    DESTINATION cmake
)


################################################################################
# Testing
################################################################################

if(DARMA_SERIALIZATION_ENABLE_TESTING)
  enable_testing()
  add_subdirectory(tests)
endif()

if (DARMA_ENABLE_COVERALLS)
  if (!DARMA_SERIALIZATION_ENABLE_TESTING)
    message(FATAL_ERROR "Can't run coveralls coverage without tests enabled")
  endif()
  include(Coveralls)
  coveralls_turn_on_coverage()
  file(GLOB_RECURSE all_source_files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" source/include/darma/*.h)
  coveralls_setup(
    "${all_source_files}"
    ${DARMA_COVERALLS_UPLOAD}
    "${PROJECT_SOURCE_DIR}/cmake-modules/coveralls/cmake"
  )
endif()


################################################################################
# Examples
################################################################################

if(DARMA_SERIALIZATION_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
