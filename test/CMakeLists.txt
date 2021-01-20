enable_testing(true)

add_executable(ClientTest ClientTest.cpp)
add_test(NAME ClientTest COMMAND ClientTest)
target_link_libraries(ClientTest PRIVATE Qt::Test)