aux_source_directory(gen-cpp example_SRCS)
aux_source_directory(. example_SRCS)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_library(example ${example_SRCS})

target_link_libraries(
  example
  pthread rt
  thrift
  protobuf
  boost_thread
  boost_system
  boost_filesystem
  boost_date_time
  boost_program_options
  boost_regex
)

add_executable(server server_main.cpp)
target_link_libraries(server example)

add_executable(client client_main.cpp)
target_link_libraries(client example)
