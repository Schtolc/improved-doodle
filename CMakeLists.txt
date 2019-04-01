cmake_minimum_required(VERSION 3.13)
project(rcp_divan C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fsanitize-address-use-after-scope -fno-omit-frame-pointer -fno-common -g -O0 -Werror -Wpedantic -Wall")

add_executable(rcp_server server.c helpers.c write_request.c)
# add_executable(rcp_server_tests tests.c helpers.c write_request.c contrib/unity/unity.c)