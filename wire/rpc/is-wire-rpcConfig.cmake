include(CMakeFindDependencyMacro)

find_dependency(is-wire-core)
find_dependency(is-msgs)

include("${CMAKE_CURRENT_LIST_DIR}/is-wire-rpcTargets.cmake")


