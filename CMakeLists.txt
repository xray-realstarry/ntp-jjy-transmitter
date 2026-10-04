cmake_minimum_required(VERSION 3.16)
project(ntp_jjy_transmitter_host_tests CXX)

# Host-only build of the hardware-independent libraries and their tests.
# The firmware itself is built with PlatformIO (see platformio.ini).
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Werror)

add_library(jjy_encoder lib/jjy_encoder/src/jjy_encoder.cpp)
target_include_directories(jjy_encoder PUBLIC lib/jjy_encoder/src)

enable_testing()
add_executable(test_jjy_encoder tests/test_jjy_encoder.cpp)
target_link_libraries(test_jjy_encoder jjy_encoder)
add_test(NAME jjy_encoder COMMAND test_jjy_encoder)
