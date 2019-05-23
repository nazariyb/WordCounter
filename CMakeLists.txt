cmake_minimum_required(VERSION 3.10)
project(lab4)

set(CMAKE_CXX_STANDARD 11)
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREAD ON)
set(CMAKE_AUTOMOC ON)

file(GLOB DEPENDECIES dependencies/*.*)

set(OWN_QUEUE own_queue)
set(QT_POOL qt_pool)
add_executable(${OWN_QUEUE}
        src/main.cpp src/conf_reader.cpp src/conf_reader.h
        dependencies/thread_safe_queue.cpp dependencies/thread_safe_queue.h
        dependencies/file_reader.cpp dependencies/file_reader.h
        src/main_config.h
        src/map_manipulation.cpp src/map_manipulation.h
        src/Task.cpp src/Task.h)

find_package(Threads REQUIRED)
find_package(LibArchive REQUIRED)
find_package(Boost COMPONENTS system filesystem locale REQUIRED)

target_link_libraries(${OWN_QUEUE} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(${OWN_QUEUE} ${LibArchive_LIBRARIES})
target_link_libraries(${OWN_QUEUE}
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_LOCALE_LIBRARY}
        )

add_executable(${QT_POOL}
        src/pool_from_qt.cpp src/conf_reader.cpp src/conf_reader.h
        dependencies/thread_safe_queue.cpp dependencies/thread_safe_queue.h
        dependencies/file_reader.cpp dependencies/file_reader.h
        src/main_config.h
        src/map_manipulation.cpp src/map_manipulation.h src/pool_from_qt.cpp
        )

find_package(Qt5Core)
find_package(Qt5Concurrent)
find_package(Qt5Widgets)
include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Concurrent_INCLUDE_DIRS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})

get_target_property(QtCore_location Qt5::Core LOCATION)

#find_package(Qt5 COMPONENTS Core Concurrent REQUIRED)
target_link_libraries(${QT_POOL} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(${QT_POOL} ${LibArchive_LIBRARIES})
target_link_libraries(${QT_POOL}
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_LOCALE_LIBRARY}
        Qt5::Core
        Qt5::Concurrent
        Qt5::Widgets
        )


