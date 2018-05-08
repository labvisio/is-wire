cmake_minimum_required(VERSION 3.5)
include(GNUInstallDirs)

include(${CMAKE_CURRENT_LIST_DIR}/../cmake/ImportProject.cmake)

set(protobuf_git_tag "fc922d16ed7ae3c19defe82228a9760f3d8d1073" CACHE STRING "Git branch name, commit id or tag")
set(rabbitmq-c_git_tag "e1eaaac34f87c4aed2ba435b9a6e599ff63b178d" CACHE STRING "Git branch name, commit id or tag")
set(simpleamqpclient_git_tag "6323892d3e8701489fb945f45aa877a7e2a0ce31" CACHE STRING "Git branch name, commit id or tag")
set(spdlog_git_tag "71e93a4f2d52fb9f9dafdeea7386bf0c780981e3" CACHE STRING "Git branch name, commit id or tag")
set(googletest_git_tag "278aba369c41e90e9e77a6f51443beb3692919cf" CACHE STRING "Git branch name, commit id or tag")
set(googlebenchmark_git_tag "8986839e4ac67facb52abc7fff3d5082e854aab5" CACHE STRING "Git branch name, commit id or tag")
set(is-msgs_git_tag "2327d69291375209091b97a59c5f65f95b07b960" CACHE STRING "Git branch name, commit id or tag")
set(zlib_git_tag "cacf7f1d4e3d44d871b605da3b647f07d718623f" CACHE STRING "Git branch name, commit id or tag")
set(prometheus_git_tag "6f7d712eba93f807cca58d144f525ac53147a234" CACHE STRING "Git branch name, commit id or tag")
set(opentracing-cpp_git_tag "d2e12fee4ff0b76b1e3ef7b0455d41217383e4aa" CACHE STRING "Git branch name, commit id or tag")
#set(curl_git_tag "b692d6650a2282335edabd9a7f1eeff1cbd02fa1" CACHE STRING "Git branch name, commit id or tag")
#set(zipkin-cpp-opentracing_git_tag "4077589309f72c0885c706125f5b5cc5ee49a53a" CACHE STRING "Git branch name, commit id or tag")

ternary(protobuf_config BUILD_SHARED_LIBS "--enable-shared;--disable-static" "--enable-static;--disable-shared")
import_project(protobuf 
  BUILD_SYSTEM autotools
  GIT https://github.com/google/protobuf
  GIT_TAG ${protobuf_git_tag}
  EXTRA_ARGS
    --without-zlib
    ${protobuf_config}
)

import_project(spdlog 
  GIT https://github.com/gabime/spdlog
  GIT_TAG ${spdlog_git_tag}
)

ternary(rabbitmq-c_config BUILD_SHARED_LIBS "-DBUILD_STATIC_LIBS=OFF" "-DBUILD_STATIC_LIBS=ON")
import_project(rabbitmq-c 
  GIT https://github.com/alanxz/rabbitmq-c 
  GIT_TAG ${rabbitmq-c_git_tag}
  EXTRA_ARGS
    -DENABLE_SSL_SUPPORT=OFF
    -DBUILD_EXAMPLES=OFF
    -DBUILD_TESTS=OFF
    ${rabbimq-c_config}
)

import_project(boost 
  BUILD_SYSTEM b2
  URL https://dl.bintray.com/boostorg/release/1.66.0/source/boost_1_66_0.tar.gz
  URL_HASH SHA256=bd0df411efd9a585e5a2212275f8762079fed8842264954675a4fddc46cfcf60
  EXTRA_ARGS
    --with-timer
    --with-chrono
    --with-system
    --with-program_options
    --with-filesystem
    --with-thread
    --with-date_time
)

import_project(SimpleAmqpClient 
  DEPENDS rabbitmq-c boost
  GIT https://github.com/alanxz/SimpleAmqpClient
  GIT_TAG ${simpleamqpclient_git_tag}
  EXTRA_ARGS
    -DENABLE_TESTING=OFF
)

import_project(googletest 
  GIT https://github.com/google/googletest
  GIT_TAG ${googletest_git_tag}
)

import_project(is-msgs 
  DEPENDS protobuf 
  GIT https://github.com/labviros/is-msgs
  GIT_TAG ${is-msgs_git_tag}
)

import_project(zlib
  GIT https://github.com/madler/zlib
  GIT_TAG ${zlib_git_tag}
)

import_project(prometheus-cpp
  DEPENDS protobuf zlib
  GIT https://github.com/jupp0r/prometheus-cpp
  GIT_TAG ${prometheus_git_tag}
)

import_project(opentracing-cpp
  GIT https://github.com/opentracing/opentracing-cpp 
  GIT_TAG ${opentracing-cpp_git_tag}
)

#import_project(zipkin-cpp-opentracing
#  DEPENDS opentracing-cpp #curl
#  GIT https://github.com/rnburn/zipkin-cpp-opentracing 
#  GIT_TAG ${zipkin-cpp-opentracing_git_tag}
#)