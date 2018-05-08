include(CMakeFindDependencyMacro)

find_dependency(Protobuf)
find_dependency(Boost)
find_dependency(spdlog)
find_dependency(is-msgs)
find_dependency(OpenTracing)

include("${CMAKE_CURRENT_LIST_DIR}/is-wire-coreTargets.cmake")

