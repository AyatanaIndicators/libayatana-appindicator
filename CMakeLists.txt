cmake_minimum_required(VERSION 3.13)
project(libayatana-appindicator
    VERSION 0.5.90
    LANGUAGES C
)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "..." FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

option(ENABLE_TESTS "Enable all tests and checks" OFF)
option(ENABLE_COVERAGE "Enable coverage reports (includes enabling all tests and checks)" OFF)

# Check for prerequisites

set(DEPS glib-2.0>=2.58 ayatana-indicator3-0.4>=0.8.4 gtk+-3.0>=3.24 dbusmenu-gtk3-0.4)

if(ENABLE_COVERAGE)
    set(ENABLE_TESTS ON)
    set(CMAKE_BUILD_TYPE "Coverage")
else()
    set(CMAKE_BUILD_TYPE "Release")
endif()

if(ENABLE_TESTS)
    set(DEPS ${DEPS} dbus-1>=1.12 dbus-glib-1>=0.82)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(PROJECT_DEPS REQUIRED ${DEPS})

# Set global variables

include(GNUInstallDirs)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")

    set(COMPILE_FLAGS "${COMPILE_FLAGS} -Weverything")
    set(COMPILE_FLAGS "${COMPILE_FLAGS} -Wno-c++98-compat -Wno-padded")
    set(COMPILE_FLAGS "${COMPILE_FLAGS} -Wno-documentation")

else()

    set(COMPILE_FLAGS "${COMPILE_FLAGS} -Wall")

endif()

add_compile_definitions(APP_INDICATOR_COMPILATION)

# Make everything

add_subdirectory(src)
add_subdirectory(bindings)
add_subdirectory(docs)

if (ENABLE_TESTS)

    include(CTest)
    enable_testing()
    add_subdirectory(tests)
    add_subdirectory(example)

    if(ENABLE_COVERAGE)

        find_package(CoverageReport)
        ENABLE_COVERAGE_REPORT(TARGETS "ayatana-appindicator3" TESTS "test-libappindicator" "test-libappindicator-dbus-client" "test-libappindicator-dbus-server" "test-libappindicator-status-client" "test-libappindicator-status-server" "test-libappindicator-fallback-item" "test-libappindicator-fallback-watcher" FILTER /usr/include ${CMAKE_BINARY_DIR}/*)

    endif()

endif()

# Display config info

message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Unit tests: ${ENABLE_TESTS}")
