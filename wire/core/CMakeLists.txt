
include(GNUInstallDirs)

find_package(Boost REQUIRED system chrono)
find_package(spdlog REQUIRED)
find_package(rabbitmq-c REQUIRED)
find_package(SimpleAmqpClient REQUIRED)
find_package(Protobuf REQUIRED)
find_package(is-msgs REQUIRED)
find_package(OpenTracing REQUIRED)
find_package(Threads)

set(author "is")
set(module "wire")
set(submodule "core")

list(APPEND interfaces
  "message.hpp"
  "channel.hpp"
  "subscription.hpp"
  "opentracing.hpp"
  "logger.hpp"
)

list(APPEND sources 
  "message.cpp"
  "channel.cpp"
  "subscription.cpp"
  "opentracing.cpp"
  "logger.cpp"
  "internal/amqp.cpp"
  "internal/amqp.hpp"
  ${interfaces}
)

list(APPEND tests
  "message.t.cpp"
  "internal/amqp.t.cpp"
)

#######
####
#######

set(target ${author}-${module}-${submodule})
set(include_dir ${author}/${module}/${submodule})

add_library(${target} ${sources})

# compile options
set_property(TARGET ${target} PROPERTY CXX_STANDARD 11)

# link dependencies
target_link_libraries(
  ${target}
 PRIVATE
  ${SimpleAmqpClient_LIBRARIES}
  ${rabbitmq-c_LIBRARIES}
  Boost::chrono
  Boost::system
  Threads::Threads
 PUBLIC
  OpenTracing::opentracing-static
  protobuf::libprotobuf
  is-msgs::is-msgs
  spdlog::spdlog
)

# header dependencies
target_include_directories(
  ${target}
 PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../..> # for headers when building
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}> # for generated files in build mode
  $<INSTALL_INTERFACE:include/${include_dir}> # for clients in install mode
 PRIVATE
  ${SimpleAmqpClient_INCLUDE_DIRS}
  ${rabbitmq-c_INCLUDE_DIRS}
)

set(export_targets      ${target}Targets)
set(export_targets_file ${export_targets}.cmake)
set(export_namespace    ${author}-${module}::)
set(export_destination  ${CMAKE_INSTALL_LIBDIR}/cmake/${target})
set(export_config_file  ${target}Config.cmake)

# install artifacts
install(FILES ${interfaces} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${include_dir})
install(
  TARGETS   ${target}
  EXPORT    ${export_targets}
  LIBRARY   DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  ARCHIVE   DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  RUNTIME   DESTINATION "${CMAKE_INSTALL_BINDIR}"
  INCLUDES  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

# install export target
install(
  EXPORT      ${export_targets}
  FILE        ${export_targets_file}
  NAMESPACE   ${export_namespace}
  DESTINATION ${export_destination}
)

# install export config
install(FILES ${export_config_file} DESTINATION ${export_destination})

# create library alias (less error prone to typos)
set(target_alias ${export_namespace}${target})
add_library(${target_alias} ALIAS ${target})

#####
### Tests
#####

if(enable_tests)
  enable_testing()
  find_package(GTest REQUIRED)

  function(gtest_add target source deps)
    add_executable(${target} ${source})
    set_property(TARGET ${target} PROPERTY CXX_STANDARD 11)
    target_link_libraries(${target} GTest::GTest GTest::Main ${deps})
    add_test(NAME ${target} COMMAND $<TARGET_FILE:${target}>)
  endfunction()

  foreach(test ${tests})
    get_filename_component(test_target ${test} NAME_WE)
    gtest_add(${test_target}_test ${test} ${target})
  endforeach(test)
  
endif(enable_tests)
