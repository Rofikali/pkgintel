cmake_minimum_required(VERSION 3.28)
project(pkgintel VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(PKGINTEL_BUILD_TESTS "Build pkgintel tests" ON)
option(PKGINTEL_BUILD_CLI "Build pkgintel CLI" ON)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

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
    C_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
)

# Public ABI is opt-in through PKGINTEL_API. This prevents private
# implementation symbols from becoming accidental ABI.
target_include_directories(pkgintel
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_options(pkgintel PRIVATE
    -Wall -Wextra -Wpedantic -Wconversion -Wshadow
    -Wstrict-prototypes -Wmissing-prototypes -Wformat=2
)

target_compile_definitions(pkgintel PRIVATE PKGINTEL_BUILDING_LIBRARY=1 _GNU_SOURCE)

install(TARGETS pkgintel
    EXPORT pkgintelTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(DIRECTORY include/pkgintel/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/pkgintel
)
install(EXPORT pkgintelTargets
    FILE pkgintelTargets.cmake
    NAMESPACE pkgintel::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/pkgintel
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/pkgintelConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/pkgintelConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/pkgintel
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/pkgintelConfigVersion.cmake
    COMPATIBILITY SameMajorVersion
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/pkgintelConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/pkgintelConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/pkgintel
)

if(PKGINTEL_BUILD_CLI)
    add_executable(pkgintel_cli src/cli/main.c)
    target_link_libraries(pkgintel_cli PRIVATE pkgintel)
    set_target_properties(pkgintel_cli PROPERTIES OUTPUT_NAME pkgintel)
    install(TARGETS pkgintel_cli
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

if(PKGINTEL_BUILD_TESTS)
    enable_testing()
    add_executable(pkgintel_unit_tests tests/unit/test_main.c
        tests/unit/test_context.c
        tests/unit/test_scan.c
        tests/unit/test_resource.c
        tests/unit/fixtures.c)
    target_link_libraries(pkgintel_unit_tests PRIVATE pkgintel)
    target_include_directories(pkgintel_unit_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests/unit)
    add_test(NAME pkgintel.unit.core COMMAND pkgintel_unit_tests)
endif()
