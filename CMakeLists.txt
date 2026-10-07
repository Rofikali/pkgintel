cmake_minimum_required(VERSION 3.28)
project(pkgintel VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(PKGINTEL_BUILD_TESTS "Build pkgintel tests" ON)
option(PKGINTEL_BUILD_CLI "Build pkgintel CLI" ON)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(PKGINTEL_SOURCES
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

add_library(pkgintel SHARED ${PKGINTEL_SOURCES})

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
    # Build a test-only static copy of production sources so allocator fault
    # injection reaches library-owned allocation sites. Executable-level
    # --wrap cannot reliably intercept calls made by a separately linked SO.
    add_library(pkgintel_test STATIC ${PKGINTEL_SOURCES})
    target_include_directories(pkgintel_test
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_compile_definitions(pkgintel_test PRIVATE PKGINTEL_BUILDING_LIBRARY=1 _GNU_SOURCE)
    target_compile_options(pkgintel_test PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wshadow
        -Wstrict-prototypes -Wmissing-prototypes -Wformat=2
        -Dmalloc=pkg_test_malloc
        -Dcalloc=pkg_test_calloc
        -Drealloc=pkg_test_realloc
        -include${CMAKE_CURRENT_SOURCE_DIR}/tests/unit/test_allocator.h)

    add_executable(pkgintel_benchmark tests/benchmark/benchmark_scan.c)
    target_link_libraries(pkgintel_benchmark PRIVATE pkgintel)
    target_compile_options(pkgintel_benchmark PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)

set_source_files_properties(src/core/context.c PROPERTIES COMPILE_DEFINITIONS PKGINTEL_BENCH_ALLOC_CLASS=PKG_BENCH_ALLOC_CONTEXT)

set_source_files_properties(src/core/scan.c PROPERTIES COMPILE_DEFINITIONS PKGINTEL_BENCH_ALLOC_CLASS=PKG_BENCH_ALLOC_SCAN)

set_source_files_properties(src/snapshot/snapshot.c PROPERTIES COMPILE_DEFINITIONS PKGINTEL_BENCH_ALLOC_CLASS=PKG_BENCH_ALLOC_SNAPSHOT)

set_source_files_properties(src/package/package.c PROPERTIES COMPILE_DEFINITIONS PKGINTEL_BENCH_ALLOC_CLASS=PKG_BENCH_ALLOC_PACKAGE)

set_source_files_properties(src/artifact/artifact.c PROPERTIES COMPILE_DEFINITIONS PKGINTEL_BENCH_ALLOC_CLASS=PKG_BENCH_ALLOC_ARTIFACT)

set_source_files_properties(src/diagnostic/diagnostic.c PROPERTIES COMPILE_DEFINITIONS PKGINTEL_BENCH_ALLOC_CLASS=PKG_BENCH_ALLOC_DIAGNOSTIC)

set_source_files_properties(src/target/target.c PROPERTIES COMPILE_DEFINITIONS PKGINTEL_BENCH_ALLOC_CLASS=PKG_BENCH_ALLOC_TARGET)

set_source_files_properties(src/support/string.c PROPERTIES COMPILE_DEFINITIONS PKGINTEL_BENCH_ALLOC_CLASS=PKG_BENCH_ALLOC_SUPPORT)

set_source_files_properties(src/support/status.c PROPERTIES COMPILE_DEFINITIONS PKGINTEL_BENCH_ALLOC_CLASS=PKG_BENCH_ALLOC_SUPPORT)

set_source_files_properties(src/backends/dpkg/dpkg.c PROPERTIES COMPILE_DEFINITIONS PKGINTEL_BENCH_ALLOC_CLASS=PKG_BENCH_ALLOC_DPKG)

    # Performance-only allocation instrumentation. Production sources are compiled
    # into a private static copy with allocator calls redirected to the benchmark
    # probe. This does not alter the production shared library or ABI.
    add_library(pkgintel_benchmark_instrumented_objects OBJECT ${PKGINTEL_SOURCES})
    target_include_directories(pkgintel_benchmark_instrumented_objects
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/benchmark)
    target_compile_definitions(pkgintel_benchmark_instrumented_objects PRIVATE
        PKGINTEL_BUILDING_LIBRARY=1 _GNU_SOURCE
        PKGINTEL_BENCHMARK_ALLOC_STATS=1)
    target_compile_options(pkgintel_benchmark_instrumented_objects PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wshadow
        -Wstrict-prototypes -Wmissing-prototypes -Wformat=2
        -include${CMAKE_CURRENT_SOURCE_DIR}/tests/benchmark/alloc_stats.h)

    add_library(pkgintel_benchmark_instrumented STATIC
        $<TARGET_OBJECTS:pkgintel_benchmark_instrumented_objects>
        tests/benchmark/alloc_stats.c)
    target_include_directories(pkgintel_benchmark_instrumented
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/tests/benchmark)
    target_compile_options(pkgintel_benchmark_instrumented PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wshadow)

    add_executable(pkgintel_benchmark_alloc tests/benchmark/benchmark_scan.c)
    target_link_libraries(pkgintel_benchmark_alloc PRIVATE pkgintel_benchmark_instrumented)
    target_include_directories(pkgintel_benchmark_alloc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests/benchmark)
    target_compile_definitions(pkgintel_benchmark_alloc PRIVATE PKGINTEL_BENCHMARK_ALLOC_STATS=1)
    target_compile_options(pkgintel_benchmark_alloc PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)

    add_executable(pkgintel_unit_tests tests/unit/test_main.c
        tests/unit/test_context.c
        tests/unit/test_scan.c
        tests/unit/test_resource.c
        tests/unit/fixtures.c)
    target_link_libraries(pkgintel_unit_tests PRIVATE pkgintel_test)
    target_include_directories(pkgintel_unit_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests/unit)
    add_test(NAME pkgintel.unit.core COMMAND pkgintel_unit_tests)
endif()
