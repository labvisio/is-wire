find_package(benchmark REQUIRED) 

add_executable(bm_client client.cpp)
set_property(TARGET bm_client PROPERTY CXX_STANDARD 11)
target_link_libraries(
  bm_client 
 PRIVATE 
  is-wire::is-wire-core 
  benchmark::benchmark
)

add_executable(bm_service service.cpp)
set_property(TARGET bm_service PROPERTY CXX_STANDARD 11)
target_link_libraries(
  bm_service 
 PRIVATE 
  is-wire::is-wire-rpc
)