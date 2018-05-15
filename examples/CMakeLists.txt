
macro(add_serialization_example name files)
  add_executable(${name} ${files})
  target_link_libraries(${name} darma_serialization::darma_serialization)
endmacro()

add_serialization_example(moving_data moving_data/moving_data.cc)
add_serialization_example(copy_constructor_archive copy_constructor_archive/copy_constructor_archive.cc)
