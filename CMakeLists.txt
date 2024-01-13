cmake_minimum_required(VERSION 3.5)

project(carousel VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(SOURCES include/carousel/carousel.h lib/carousel.cpp)
add_library(carousel ${SOURCES})

target_link_libraries(carousel Qt${QT_VERSION_MAJOR}::Widgets)
target_include_directories(carousel PUBLIC "${CMAKE_SOURCE_DIR}/include")

if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    enable_testing()
    add_subdirectory(tests)
    add_subdirectory(example)
endif()
