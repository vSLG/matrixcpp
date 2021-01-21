enable_testing(true)

add_executable(ClientTest ClientTest.cpp)
add_test(NAME ClientTest COMMAND ClientTest)
target_link_libraries(ClientTest ${PROJECT} Qt::Test Qt::Core Qt::Network)

# Include both <src>/include and <install>/include. These are public headers
target_include_directories(ClientTest PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)