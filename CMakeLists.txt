cmake_minimum_required(VERSION 3.10)
project(lab4)

set(CMAKE_CXX_STANDARD 14)
set (Boost_USE_STATIC_LIBS OFF)
set (Boost_USE_MULTITHREAD ON)

add_executable(lab4 src/main.cpp src/conf_reader.cpp src/conf_reader.h)

find_package(Threads REQUIRED)
find_package(LibArchive REQUIRED)
find_package(Boost REQUIRED COMPONENTS locale)
include_directories(Boost::locale)

target_link_libraries(lab4 ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(lab4 ${LibArchive_LIBRARIES})
target_link_libraries(lab4 Boost::locale)
