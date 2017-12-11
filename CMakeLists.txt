cmake_minimum_required(VERSION 3.1)
project(restdbxx)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS -Wall)
set(BOOST_USE_STATIC_LIBS ON)
set(BOOST_USE_MULTITHREADED ON)
set(BOOST_USE_STATIC_RUNTIME OFF)
#set(BOOST_)
#set(BUILD_SHARED_LIBS OFF)
#set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
#set(CMAKE_CXX_COMPILER clang++)
find_library(ROCKSDB rocksdb)
find_package(Boost 1.60 COMPONENTS date_time filesystem system atomic thread serialization)
INCLUDE_DIRECTORIES(/usr/local/include ${Boost_INCLUDE_DIRS})
link_directories(/usr/local/lib)

find_library(FOLLY folly)
find_library(SNAPPY snappy)
find_library(WANGLE wangle)
find_package(Threads)
find_library(GFLAGS gflags)
find_library(GLOG glog)
find_library(Z z)
find_library(SSL ssl)
find_library(CRYPTO crypto)
find_library(PROXYGENLIB proxygenlib)
find_library(PROXYGENHTTPSERVER proxygenhttpserver)
find_library(GTEST gtest)
find_library(GTEST_MAIN gtest_main)
find_library(DL dl)
find_library(jemalloc jemalloc)
find_library(EVENT event)
find_library(D double-conversion)

set(LIBS
        ${PROXYGENHTTPSERVER}
        ${PROXYGENLIB}
        ${WANGLE}
        ${FOLLY}
        ${ROCKSDB}
        ${GLOG}
        ${GFLAGS}
        ${SSL}
        ${Z}
        ${EVENT}
        ${CRYPTO}
        ${Boost_LIBRARIES}
        ${PTHREAD}
        ${CMAKE_THREAD_LIBS_INIT}
        ${D}
        ${SNAPPY}
        ${jemalloc}
        )

set(SOURCE_FILES
        main.cpp DbManager.cpp DbManager.h
        RestDbRequestHandler.cpp RestDbRequestHandler.h
        RestDbRequestHandlerFactory.cpp RestDbRequestHandlerFactory.h
        LoggingFilter.cpp LoggingFilter.h
        User.cpp User.h UserManager.cpp
        UserManager.h UserRequestHandler.cpp
        UserRequestHandler.h UserRequestHandlerFactory.cpp
        UserRequestHandlerFactory.h Validations.cpp

        RestDbConfiguration.cpp includes.h BaseRequestHandler.cpp BaseRequestHandler.h CorsFilter.cpp CorsFilter.h EndpointDescriptor.cpp EndpointDescriptor.h EndpointController.cpp EndpointController.h EndpointControllerFactory.cpp EndpointControllerFactory.h FiltersFactory.cpp FiltersFactory.h AuthenticationRequestHandler.cpp AuthenticationRequestHandler.h AuthenticationFilter.cpp AuthenticationFilter.h FileServerRequestHandler.cpp FileServerRequestHandler.h GiphySearchRequestHandler.cpp GiphySearchRequestHandler.h JsonClient.cpp JsonClient.h XkcdRequestHandler.cpp XkcdRequestHandler.h SimpleRoutingRequestHandlerFactory.cpp SimpleRoutingRequestHandlerFactory.h)
add_executable(restdbxx ${SOURCE_FILES})

target_link_libraries(restdbxx ${LIBS})
