cmake_minimum_required(VERSION 3.15.0)

set(PROJECT "MatrixCpp")

project(${PROJECT} VERSION 0.1 DESCRIPTION "Simple matrix API C++/Qt wrapper")

option(BUILD_SHARED_LIBS "Build as a shared library" ON)
option(BUILD_TESTS "Build tests" ON)
option(BUILD_DOC "Build documentation" ON)

# Qt-specific options
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)

find_package(Qt5 COMPONENTS Core Network REQUIRED)
find_package(Olm REQUIRED)

add_library(${PROJECT}
    src/Client.cpp
    src/Structs.cpp

    src/Responses/ResponseFuture.cpp
    src/Responses/Responses.cpp
    
    include/${PROJECT}/Client.hpp
    include/${PROJECT}/Responses.hpp)

target_link_libraries(${PROJECT}
    Qt::Core
    Qt::Network
    Olm::Olm)

target_compile_definitions(${PROJECT} PRIVATE APP_NAME="${PROJECT}" APP_VERSION="${PROJECT_VERSION}")

# Include both <src>/include and <install>/include. These are public headers
target_include_directories(${PROJECT} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# Set version and other relevant properties
set_target_properties(${PROJECT} PROPERTIES
    VERSION ${PROJECT_VERSION})

#
# Documentation
#

if(BUILD_DOC)
    find_package(Doxygen REQUIRED)

    set(DOXYGEN_IN  ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    add_custom_target(doc ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)
endif()

#
# Installation
#

include(GNUInstallDirs)

install(TARGETS ${PROJECT}
    LIBRARY DESTINATION ${CMAKE_INSTALL_DIR}
)
install(DIRECTORY include/${PROJECT}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

if(BUILD_TESTS)
    find_package(Qt5 COMPONENTS Test REQUIRED)
    add_subdirectory(test)
endif()