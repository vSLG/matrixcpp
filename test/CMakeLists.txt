enable_testing(true)

set(ACCOUNT_INFO ${CMAKE_CURRENT_SOURCE_DIR}/account_info)
configure_file(ClientTest.cpp ${CMAKE_CURRENT_BINARY_DIR}/ClientTest.cpp @ONLY)
configure_file(LoginTest.cpp ${CMAKE_CURRENT_BINARY_DIR}/LoginTest.cpp @ONLY)

#
# Client test
#

add_executable(ClientTest ${CMAKE_CURRENT_BINARY_DIR}/ClientTest.cpp)
add_test(NAME ClientTest COMMAND ClientTest)
target_link_libraries(ClientTest ${PROJECT} Qt::Test Qt::Core Qt::Network)

# Include both <src>/include and <install>/include. These are public headers
target_include_directories(ClientTest PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)


#
# Login test
#

add_executable(LoginTest ${CMAKE_CURRENT_BINARY_DIR}/LoginTest.cpp)
add_test(NAME LoginTest COMMAND LoginTest)
target_link_libraries(LoginTest ${PROJECT} Qt::Test Qt::Core Qt::Network)

# Include both <src>/include and <install>/include. These are public headers
target_include_directories(LoginTest PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)