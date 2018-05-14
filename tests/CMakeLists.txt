
find_package(GTest REQUIRED)

include(GoogleTest)

function(add_serialization_test test_name)
  if(DARMA_SERIALIZATION_TESTING_SINGLE_EXECUTABLE)
    set(serializationtestfiles "${serializationtestfiles};${test_name}.cc" PARENT_SCOPE)
  else()
    add_executable(${test_name} ${test_name}.cc)

    target_link_libraries(${test_name} GTest::GTest GTest::Main)
    target_link_libraries(${test_name} darma_serialization::darma_serialization)

    add_test(${test_name} ${CMAKE_CURRENT_BINARY_DIR}/${test_name} "")

    if (DARMA_SERIALIZATION_COVERAGE)
      setup_target_for_coverage(serialization_coverage ${test_name} coverage)
    endif()
  endif()
endfunction()

add_serialization_test(test_simple_arithmetic_types)
add_serialization_test(test_simple_std_string)
add_serialization_test(test_simple_std_pair)
add_serialization_test(test_simple_std_map)
add_serialization_test(test_simple_std_vector)
add_serialization_test(test_simple_std_tuple)
add_serialization_test(test_simple_std_set)
add_serialization_test(test_simple_array)


if(DARMA_SERIALIZATION_TESTING_SINGLE_EXECUTABLE)
  add_executable(run_all_serialization_tests ${serializationtestfiles})
  target_link_libraries(run_all_serialization_tests GTest::GTest GTest::Main)
  target_link_libraries(run_all_serialization_tests darma_serialization::darma_serialization)
  if (DARMA_SERIALIZATION_COVERAGE)
    setup_target_for_coverage(serialization_coverage run_all_serialization_tests coverage)
  endif()
endif()

