set(author "is")
set(module "wire")

add_subdirectory(core)
add_subdirectory(rpc)

list(APPEND interfaces
  "core.hpp"
  "rpc.hpp"
)

######
####
######

# install public headers
install(FILES ${interfaces} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${author}/${module})

# install export config
set(export_config_file ${author}-${module}Config.cmake)
set(export_destination ${CMAKE_INSTALL_LIBDIR}/cmake/${author}-${module})
install(FILES ${export_config_file} DESTINATION ${export_destination})

#######
#### Tests
#######

if(build_tests)
  find_package(zipkin-cpp-opentracing)

  add_executable(service service.t.cpp)
  target_link_libraries(service PUBLIC is-wire::is-wire-rpc zipkin-cpp-opentracing::zipkin-cpp-opentracing)
  set_property(TARGET service PROPERTY CXX_STANDARD 11)

  add_executable(client client.t.cpp)
  target_link_libraries(client PUBLIC is-wire::is-wire-core zipkin-cpp-opentracing::zipkin-cpp-opentracing)
  set_property(TARGET client PROPERTY CXX_STANDARD 11)
endif(build_tests)