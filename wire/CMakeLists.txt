cmake_minimum_required(VERSION 3.5)

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
