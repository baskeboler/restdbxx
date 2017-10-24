cmake_minimum_required(VERSION 3.8)
project(restdbxx)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Debug)
find_library(ROCKSDB rocksdb)

set(BOOST_USE_STATIC_LIBS ON)
set(BOOST_USE_MULTITHREADED ON)
set(BOOST_USE_STATIC_RUNTIME OFF)

find_package(Boost 1.60 COMPONENTS date_time filesystem system atomic thread serialization)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

find_library(FOLLY folly)
find_library(WANGLE wangle)
find_library(PTHREAD pthread)
find_library(GLOG glog)
find_library(Z z)
find_library(GFLAGS gflags)
find_library(SSL ssl)
find_library(CRYPTO crypto)
find_library(PROXYGENLIB proxygenlib)
find_library(PROXYGENHTTPSERVER proxygenhttpserver)
find_library(DCONV double-conversion)
find_library(GTEST gtest)
find_library(GTEST_MAIN gtest_main)
find_library(DL dl)
find_library(DEMANGLE demangle)

set(LIBS
        ${DL}
        ${Boost_LIBRARIES}
        ${ROCKSDB}
        ${Z}
        ${PTHREAD}
        ${CRYPTO}
        ${SSL}
        ${DCONV}
        ${GLOG}
        ${GFLAGS}
        ${FOLLY}
        ${WANGLE}
        ${PROXYGENLIB}
        ${PROXYGENHTTPSERVER}
        )

set(SOURCE_FILES main.cpp DbManager.cpp DbManager.h RestDbRequestHandler.cpp RestDbRequestHandler.h RestDbRequestHandlerFactory.cpp RestDbRequestHandlerFactory.h LoggingFilter.cpp LoggingFilter.h)
add_executable(restdbxx ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} ${LIBS})