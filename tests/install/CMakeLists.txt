cmake_minimum_required(VERSION 3.28)
project(pkgintel_consumer LANGUAGES C)

find_package(pkgintel CONFIG REQUIRED)

add_executable(pkgintel_consumer consumer.c)
target_link_libraries(pkgintel_consumer PRIVATE pkgintel::pkgintel)
