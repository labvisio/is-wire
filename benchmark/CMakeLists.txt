cmake_minimum_required(VERSION 3.5)

find_package(is-wire REQUIRED)
find_package(benchmark REQUIRED) 

add_executable(client client.cpp)
set_property(TARGET client PROPERTY CXX_STANDARD 11)
target_link_libraries(
  client 
 PRIVATE 
  is-wire::is-wire-core 
  benchmark::benchmark
)

add_executable(service service.cpp)
set_property(TARGET service PROPERTY CXX_STANDARD 11)
target_link_libraries(
  service 
 PRIVATE 
  is-wire::is-wire-rpc
)