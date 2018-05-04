
function(import_find target)
  cmake_parse_arguments(""
    ""
    ""
    "LIBRARY_HINTS;PATH_HINTS"
    ${ARGN}
  )
  set(library ${target}::${target})
  mark_as_advanced(library)

  find_library(${target}_LIBRARIES NAMES ${_LIBRARY_HINTS})
  find_path(${target}_INCLUDE_DIRS NAMES ${_PATH_HINTS})
  mark_as_advanced(${target}_LIBRARIES)
  mark_as_advanced(${target}_INCLUDE_DIRS)

  if (${target}_LIBRARIES AND ${target}_INCLUDE_DIRS)
    set(${target}_FOUND true)
    message(STATUS "Found ${target} \n  lib=${${target}_LIBRARIES}\n  path=${${target}_INCLUDE_DIRS}")
    if(NOT TARGET ${library})
      add_library(${library} UNKNOWN IMPORTED)
      set_target_properties(${library} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${${target}_INCLUDE_DIRS})
      set_target_properties(${library} PROPERTIES IMPORTED_LOCATION ${${target}_LIBRARIES})
    endif()
  else()
    message(STATUS "${target} not found!")
  endif()

endfunction()
