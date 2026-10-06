cmake_minimum_required(VERSION 3.28)
project(pkgintel VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(PKGINTEL_BUILD_TESTS "Build pkgintel tests" ON)
option(PKGINTEL_BUILD_CLI "Build pkgintel CLI" ON)

add_library(pkgintel SHARED
    src/core/context.c
    src/core/scan.c
    src/snapshot/snapshot.c
    src/package/package.c
    src/artifact/artifact.c
    src/diagnostic/diagnostic.c
    src/target/target.c
    src/support/string.c
    src/support/status.c
    src/backends/dpkg/dpkg.c
)
set_target_properties(pkgintel PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 0
)

# v0.1 ABI symbols are intentionally default-visible while the public export
# set is being stabilized. Before v1.0, replace this with an explicit linker
# version script so only reviewed ABI symbols are exported.

target_include_directories(pkgintel
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
           $<INSTALL_INTERFACE:include>
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_options(pkgintel PRIVATE
    -Wall -Wextra -Wpedantic -Wconversion -Wshadow
    -Wstrict-prototypes -Wmissing-prototypes -Wformat=2
)

target_compile_definitions(pkgintel PRIVATE PKGINTEL_BUILDING_LIBRARY=1 _GNU_SOURCE)

if(PKGINTEL_BUILD_CLI)
    add_executable(pkgintel_cli src/cli/main.c)
    target_link_libraries(pkgintel_cli PRIVATE pkgintel)
    set_target_properties(pkgintel_cli PROPERTIES OUTPUT_NAME pkgintel)
endif()

if(PKGINTEL_BUILD_TESTS)
    enable_testing()
    add_executable(pkgintel_unit_tests tests/unit/test_main.c
        tests/unit/test_context.c
        tests/unit/test_scan.c
        tests/unit/fixtures.c)
    target_link_libraries(pkgintel_unit_tests PRIVATE pkgintel)
    target_include_directories(pkgintel_unit_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests/unit)
    add_test(NAME pkgintel.unit.core COMMAND pkgintel_unit_tests)
endif()
