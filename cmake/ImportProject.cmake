cmake_minimum_required(VERSION 3.5)

include(ExternalProject)
include(ProcessorCount)
ProcessorCount(NumberOfCores)

function(ternary var condition on_true on_false)
  if(${condition})
    set(${var} ${${var}} ${on_true} PARENT_SCOPE)
  else()
    set(${var} ${${var}} ${on_false} PARENT_SCOPE)
  endif()
endfunction()

function(import_project target)
  cmake_parse_arguments(""
    ""
    "GIT;GIT_TAG;BUILD_SYSTEM;URL;URL_HASH"
    "EXTRA_ARGS;DEPENDS"
    ${ARGN}
  )

  if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE release)
  endif()

  if (NOT _BUILD_SYSTEM)
    set(_BUILD_SYSTEM cmake)
  endif()
  string(TOLOWER "${_BUILD_SYSTEM}" _BUILD_SYSTEM)
  
  string(COMPARE EQUAL ${_BUILD_SYSTEM} cmake is_cmake)
  string(COMPARE EQUAL ${_BUILD_SYSTEM} b2 is_b2)
  string(COMPARE EQUAL ${_BUILD_SYSTEM} autotools is_autotools)

  if(is_cmake)
    ExternalProject_Add(${target} 
      PREFIX          ${target}      DEPENDS  ${_DEPENDS} 
      GIT_REPOSITORY  ${_GIT}        GIT_TAG  ${_GIT_TAG}
      URL             ${_URL}        URL_HASH ${_URL_HASH}
      CMAKE_ARGS 
        -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
        -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}
        ${_EXTRA_ARGS}
    )  
  elseif(is_b2)
    string(TOLOWER "${CMAKE_BUILD_TYPE}" b2_variant)
    ternary(b2_link BUILD_SHARED_LIBS "shared" "static")
    ExternalProject_Add(${target}
      PREFIX          ${target}      DEPENDS  ${_DEPENDS} 
      GIT_REPOSITORY  ${_GIT}        GIT_TAG  ${_GIT_TAG}
      URL             ${_URL}        URL_HASH ${_URL_HASH}
      BUILD_IN_SOURCE 1
      CONFIGURE_COMMAND ./bootstrap.sh --prefix=${CMAKE_PREFIX_PATH}
      BUILD_COMMAND ./b2 install -j ${NumberOfCores} variant=${b2_variant} link=${b2_link} ${_EXTRA_ARGS}
      INSTALL_COMMAND ""
    )
  elseif(is_autotools)
    ExternalProject_Add(${target}
      PREFIX          ${target}      DEPENDS  ${_DEPENDS} 
      GIT_REPOSITORY  ${_GIT}        GIT_TAG  ${_GIT_TAG}
      URL             ${_URL}        URL_HASH ${_URL_HASH}
      BUILD_IN_SOURCE 1
      CONFIGURE_COMMAND ./autogen.sh && ./configure --prefix=${CMAKE_PREFIX_PATH} ${_EXTRA_ARGS}
      BUILD_COMMAND make -j ${NumberOfCores}
      INSTALL_COMMAND make install
    )
  else()
    message(FATAL_ERROR "Build system '${_BUILD_SYSTEM}' not supported")
  endif()
endfunction()
