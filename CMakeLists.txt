cmake_minimum_required(VERSION 3.13)
project(rcp_divan C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fsanitize-address-use-after-scope -fno-omit-frame-pointer -fno-common -g -O0 -Werror -Wpedantic -Wall")

add_executable(cprem_server server.c helpers.c)
add_executable(cprem client.c helpers.c)