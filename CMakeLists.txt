cmake_minimum_required(VERSION 3.13)
project(libayatana-appindicator C)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "..." FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# Options

option(ENABLE_TESTS "Enable all tests and checks" OFF)
option(ENABLE_COVERAGE "Enable coverage reports (includes enabling all tests and checks)" OFF)
option(ENABLE_WERROR "Treat all build warnings as errors" OFF)
option(ENABLE_GTKDOC "Enable building GTK documentation" OFF)
option(ENABLE_BINDINGS_VALA "Enable Vala bindings (GTK+-3.0 and beyond only)" ON)
option(ENABLE_BINDINGS_MONO "Enable Mono bindings" ON)

if(ENABLE_COVERAGE)
    set(ENABLE_TESTS ON)
    set(CMAKE_BUILD_TYPE "Coverage")
else()
    set(CMAKE_BUILD_TYPE "Release")
endif()

if(ENABLE_WERROR)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
endif()

# Check for prerequisites

option(FLAVOUR_GTK2 "Build against GTK+-2.0" OFF)
option(FLAVOUR_GTK3 "Build against GTK+-3.0" ON)

if (FLAVOUR_GTK2)
    set (FLAVOUR_GTK3 OFF)
endif()
if (FLAVOUR_GTK3)
    set (FLAVOUR_GTK2 OFF)
endif()

set(DEPS glib-2.0>=2.58)

if (FLAVOUR_GTK3)
    set(DEPS
        ${DEPS}
        ayatana-indicator3-0.4>=0.8.4
        gtk+-3.0>=3.24
        dbusmenu-gtk3-0.4
    )
elseif (FLAVOUR_GTK2)
    set(DEPS
        ${DEPS}
        ayatana-indicator-0.4>=0.8.4
        gtk+-2.0>=2.18
        dbusmenu-gtk-0.4
    )
endif()

if(ENABLE_TESTS)
    set(DEPS ${DEPS} dbus-1>=1.12 dbus-glib-1>=0.82)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(PROJECT_DEPS REQUIRED ${DEPS})

if (ENABLE_GTKDOC)
    find_program (GTKDOC "gtkdoc-scan")
    if (NOT GTKDOC)
        message(WARNING "Building the gtk-doc API documentation is enabled, but the gtkdoc-scan executable has not been found.  Disabling API documentation building, although requested.")
        set (ENABLE_GTKDOC OFF)
    endif()
endif()

# Set global variables

include(GNUInstallDirs)
set(PROJECT_VERSION "0.5.90")
set(PROJECT_NAME "libayatana-appindicator")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")

    set(COMPILE_FLAGS "${COMPILE_FLAGS} -Weverything")
    set(COMPILE_FLAGS "${COMPILE_FLAGS} -Wno-c++98-compat -Wno-padded")
    set(COMPILE_FLAGS "${COMPILE_FLAGS} -Wno-documentation")

else()

    set(COMPILE_FLAGS "${COMPILE_FLAGS} -Wall")

endif()

# Make everything

add_subdirectory(src)
add_subdirectory(bindings)
if (ENABLE_GTKDOC)
    add_subdirectory(docs)
endif()

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
message(STATUS "GTK+-3.0 build: ${FLAVOUR_GTK3}")
message(STATUS "GTK+-2.0 build: ${FLAVOUR_GTK2}")
message(STATUS "Vala bindings: ${ENABLE_BINDINGS_VALA}")
message(STATUS "Mono bindings: ${ENABLE_BINDINGS_MONO}")
message(STATUS "Unit tests: ${ENABLE_TESTS}")
message(STATUS "Build with -Werror: ${ENABLE_WERROR}")
message(STATUS "API Documentation: ${ENABLE_GTKDOC}")
