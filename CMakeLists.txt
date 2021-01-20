cmake_minimum_required(VERSION 3.15.0)

set(PROJECT "MatrixCpp")

project(${PROJECT} VERSION 0.1 DESCRIPTION "Simple matrix API C++/Qt wrapper")

option(BUILD_SHARED_LIBS "Build as a shared library" ON)
option(BUILD_TESTS "Build tests" ON)

# Qt-specific options
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)

# XXX
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DAPP_NAME=\\\"${PROJECT}\\\"")

find_package(Qt5 COMPONENTS Core Network REQUIRED)
find_package(Olm REQUIRED)

add_library(${PROJECT} src/Client.cpp)

target_link_libraries(${PROJECT}
    Qt::Core
    Qt::Network
    Olm::Olm)

# Include both <src>/include and <install>/include. These are public headers
target_include_directories(${PROJECT} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# Set version and other relevant properties
set_target_properties(${PROJECT} PROPERTIES
    VERSION ${PROJECT_VERSION}
    PUBLIC_HEADER include/${PROJECT}/Client.hpp)

#
# Installation
#

include(GNUInstallDirs)

install(TARGETS ${PROJECT}
    LIBRARY       DESTINATION ${CMAKE_INSTALL_DIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT}
)

if (BUILD_TESTS)
    find_package(Qt5 COMPONENTS Test REQUIRED)
    add_subdirectory(test)
endif()