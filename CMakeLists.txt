cmake_minimum_required(VERSION 3.10)
project(lab4)

set(CMAKE_CXX_STANDARD 11)
set (Boost_USE_STATIC_LIBS OFF)
set (Boost_USE_MULTITHREAD ON)
file (GLOB DEPENDECIES dependencies/*.*)

add_executable(lab4
    src/main.cpp src/conf_reader.cpp src/conf_reader.h
    dependencies/function_wrapper.cpp dependencies/function_wrapper.h
    dependencies/thread_pool.cpp dependencies/thread_pool.h
    dependencies/thread_safe_queue.cpp dependencies/thread_safe_queue.h
    dependencies/work_stealing_queue.cpp dependencies/work_stealing_queue.h
    dependencies/join_threads.h
#    ${DEPENDECIES}
        src/read.cpp src/read.h src/main_config.h src/map_manipulation.cpp src/map_manipulation.h)

find_package(Threads REQUIRED)
find_package(LibArchive REQUIRED)
find_package(Boost REQUIRED system)
find_package(Boost COMPONENTS system filesystem locale REQUIRED)
#include_directories(Boost::locale)

target_link_libraries(lab4 ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(lab4 ${LibArchive_LIBRARIES})
#target_link_libraries(lab4 Boost::locale)
target_link_libraries(lab4
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_LOCALE_LIBRARY}
        )