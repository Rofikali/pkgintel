cmake_minimum_required(VERSION 3.28)
project(pkgintel VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(PKGINTEL_BUILD_TESTS "Build pkgintel tests" ON)
option(PKGINTEL_BUILD_CLI "Build pkgintel CLI" ON)

add_library(pkgintel SHARED src/pkgintel.c)
set_target_properties(pkgintel PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 0
    C_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
)

target_include_directories(pkgintel
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
           $<INSTALL_INTERFACE:include>
)

target_compile_options(pkgintel PRIVATE
    -Wall -Wextra -Wpedantic -Wconversion -Wshadow
    -Wstrict-prototypes -Wmissing-prototypes -Wformat=2
)

target_compile_definitions(pkgintel PRIVATE PKGINTEL_BUILDING_LIBRARY=1)

if(PKGINTEL_BUILD_CLI)
    add_executable(pkgintel_cli src/cli/main.c)
    target_link_libraries(pkgintel_cli PRIVATE pkgintel)
    set_target_properties(pkgintel_cli PROPERTIES OUTPUT_NAME pkgintel)
endif()

if(PKGINTEL_BUILD_TESTS)
    enable_testing()
    add_executable(pkgintel_unit_tests tests/unit/test_core.c)
    target_link_libraries(pkgintel_unit_tests PRIVATE pkgintel)
    add_test(NAME pkgintel.unit.core COMMAND pkgintel_unit_tests)
endif()
