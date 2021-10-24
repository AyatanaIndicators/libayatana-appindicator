if (FLAVOUR_GTK3)
    set (ayatana_appindicator_gtkver "ayatana-appindicator3")
elseif (FLAVOUR_GTK2)
    set (ayatana_appindicator_gtkver "ayatana-appindicator")
endif()

# test-libappindicator

set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/test-libappindicator.c" PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -include ${CMAKE_SOURCE_DIR}/src/app-indicator.h")
add_executable("test-libappindicator" "${CMAKE_CURRENT_SOURCE_DIR}/test-libappindicator.c")
target_compile_definitions("test-libappindicator" PUBLIC SRCDIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories("test-libappindicator" PUBLIC ${PROJECT_DEPS_INCLUDE_DIRS})
target_include_directories("test-libappindicator" PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries("test-libappindicator" "${PROJECT_DEPS_LIBRARIES} -l${ayatana_appindicator_gtkver}")
target_link_directories("test-libappindicator" PUBLIC "${CMAKE_BINARY_DIR}/src")
add_dependencies("test-libappindicator" "${ayatana_appindicator_gtkver}")

# test-libappindicator-dbus-client

set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/test-libappindicator.c" PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")
add_executable("test-libappindicator-dbus-client" "${CMAKE_CURRENT_SOURCE_DIR}/test-libappindicator-dbus-client.c")
target_include_directories("test-libappindicator-dbus-client" PUBLIC ${PROJECT_DEPS_INCLUDE_DIRS})
target_include_directories("test-libappindicator-dbus-client" PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries("test-libappindicator-dbus-client" "${PROJECT_DEPS_LIBRARIES} -l${ayatana_appindicator_gtkver}")
target_link_directories("test-libappindicator-dbus-client" PUBLIC "${CMAKE_BINARY_DIR}/src")
add_dependencies("test-libappindicator-dbus-client" "${ayatana_appindicator_gtkver}")

# test-libappindicator-dbus-server

set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/test-libappindicator-dbus-server.c" PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")
add_executable("test-libappindicator-dbus-server" "${CMAKE_CURRENT_SOURCE_DIR}/test-libappindicator-dbus-server.c")
target_include_directories("test-libappindicator-dbus-server" PUBLIC ${PROJECT_DEPS_INCLUDE_DIRS})
target_include_directories("test-libappindicator-dbus-server" PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries("test-libappindicator-dbus-server" "${PROJECT_DEPS_LIBRARIES} -l${ayatana_appindicator_gtkver}")
target_link_directories("test-libappindicator-dbus-server" PUBLIC "${CMAKE_BINARY_DIR}/src")
add_dependencies("test-libappindicator-dbus-server" "${ayatana_appindicator_gtkver}")

# test-libappindicator-status-client

set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/test-libappindicator-status-client.c" PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")
add_executable("test-libappindicator-status-client" "${CMAKE_CURRENT_SOURCE_DIR}/test-libappindicator-status-client.c")
target_include_directories("test-libappindicator-status-client" PUBLIC ${PROJECT_DEPS_INCLUDE_DIRS})
target_link_libraries("test-libappindicator-status-client" "${PROJECT_DEPS_LIBRARIES} -l${ayatana_appindicator_gtkver}")
target_link_directories("test-libappindicator-status-client" PUBLIC "${CMAKE_BINARY_DIR}/src")
add_dependencies("test-libappindicator-status-client" "${ayatana_appindicator_gtkver}")

# test-libappindicator-status-server

set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/test-libappindicator-status-server.c" PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")
add_executable("test-libappindicator-status-server" "${CMAKE_CURRENT_SOURCE_DIR}/test-libappindicator-status-server.c")
target_include_directories("test-libappindicator-status-server" PUBLIC ${PROJECT_DEPS_INCLUDE_DIRS})
target_include_directories("test-libappindicator-status-server" PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries("test-libappindicator-status-server" "${PROJECT_DEPS_LIBRARIES} -l${ayatana_appindicator_gtkver}")
target_link_directories("test-libappindicator-status-server" PUBLIC "${CMAKE_BINARY_DIR}/src")
add_dependencies("test-libappindicator-status-server" "${ayatana_appindicator_gtkver}")

# test-libappindicator-fallback-item

set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/test-libappindicator-fallback-item.c" PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")
add_executable("test-libappindicator-fallback-item" "${CMAKE_CURRENT_SOURCE_DIR}/test-libappindicator-fallback-item.c")
target_include_directories("test-libappindicator-fallback-item" PUBLIC ${PROJECT_DEPS_INCLUDE_DIRS})
target_include_directories("test-libappindicator-fallback-item" PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries("test-libappindicator-fallback-item" "${PROJECT_DEPS_LIBRARIES} -l${ayatana_appindicator_gtkver}")
target_link_directories("test-libappindicator-fallback-item" PUBLIC "${CMAKE_BINARY_DIR}/src")
add_dependencies("test-libappindicator-fallback-item" "${ayatana_appindicator_gtkver}")

# test-libappindicator-fallback-watcher

set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/test-libappindicator-fallback-watcher.c" PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")
add_executable("test-libappindicator-fallback-watcher" "${CMAKE_CURRENT_SOURCE_DIR}/test-libappindicator-fallback-watcher.c")
target_include_directories("test-libappindicator-fallback-watcher" PUBLIC ${PROJECT_DEPS_INCLUDE_DIRS})
target_link_libraries("test-libappindicator-fallback-watcher" "${PROJECT_DEPS_LIBRARIES} -l${ayatana_appindicator_gtkver}")
target_link_directories("test-libappindicator-fallback-watcher" PUBLIC "${CMAKE_BINARY_DIR}/src")
add_dependencies("test-libappindicator-fallback-watcher" "${ayatana_appindicator_gtkver}")

# test-simple-app

set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/test-simple-app.c" PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")
add_executable("test-simple-app" "${CMAKE_CURRENT_SOURCE_DIR}/test-simple-app.c")
target_include_directories("test-simple-app" PUBLIC ${PROJECT_DEPS_INCLUDE_DIRS})
target_include_directories("test-simple-app" PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries("test-simple-app" "${PROJECT_DEPS_LIBRARIES} -l${ayatana_appindicator_gtkver}")
target_link_directories("test-simple-app" PUBLIC "${CMAKE_BINARY_DIR}/src")
add_dependencies("test-simple-app" "${ayatana_appindicator_gtkver}")

# test-libappindicator-fallback

find_program(DBUS_TEST_RUNNER dbus-test-runner)

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/test-libappindicator-fallback"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/test-libappindicator-fallback-watcher"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/test-libappindicator-fallback-item"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    VERBATIM
    COMMAND
    echo "#!/bin/bash" > "${CMAKE_CURRENT_BINARY_DIR}/test-libappindicator-fallback"
    COMMAND
    echo "export DISPLAY=" >> "${CMAKE_CURRENT_BINARY_DIR}/test-libappindicator-fallback"
    COMMAND
    echo ". ${CMAKE_CURRENT_SOURCE_DIR}/run-xvfb.sh" >> "${CMAKE_CURRENT_BINARY_DIR}/test-libappindicator-fallback"
    COMMAND
    echo "${DBUS_TEST_RUNNER} --keep-env -m 300 --dbus-config /usr/share/dbus-test-runner/session.conf --task ${CMAKE_CURRENT_BINARY_DIR}/test-libappindicator-fallback-watcher --task-name Watcher --ignore-return --task ${CMAKE_CURRENT_BINARY_DIR}/test-libappindicator-fallback-item --task-name Item" >> "${CMAKE_CURRENT_BINARY_DIR}/test-libappindicator-fallback"
    COMMAND
    chmod +x "${CMAKE_CURRENT_BINARY_DIR}/test-libappindicator-fallback"
)

add_test("test-libappindicator-fallback" "test-libappindicator-fallback")

# test-libappindicator-dbus

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/test-libappindicator-dbus"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/test-libappindicator-dbus-client"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/test-libappindicator-dbus-server"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    VERBATIM
    COMMAND
    echo "#!/bin/bash" > "${CMAKE_CURRENT_BINARY_DIR}/test-libappindicator-dbus"
    COMMAND
    echo "export DISPLAY=" >> "${CMAKE_CURRENT_BINARY_DIR}/test-libappindicator-dbus"
    COMMAND
    echo ". ${CMAKE_CURRENT_SOURCE_DIR}/run-xvfb.sh" >> "${CMAKE_CURRENT_BINARY_DIR}/test-libappindicator-dbus"
    COMMAND
    echo "${DBUS_TEST_RUNNER} --keep-env -m 300 --dbus-config /usr/share/dbus-test-runner/session.conf --task ${CMAKE_CURRENT_BINARY_DIR}/test-libappindicator-dbus-client --task-name Client --ignore-return --task ${CMAKE_CURRENT_BINARY_DIR}/test-libappindicator-dbus-server --task-name Server" >> "${CMAKE_CURRENT_BINARY_DIR}/test-libappindicator-dbus"
    COMMAND
    chmod +x "${CMAKE_CURRENT_BINARY_DIR}/test-libappindicator-dbus"
)

add_test("test-libappindicator-dbus" "test-libappindicator-dbus")

# test-libappindicator-status

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/test-libappindicator-status"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/test-libappindicator-status-client"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/test-libappindicator-status-server"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    VERBATIM
    COMMAND
    echo "#!/bin/bash" > "${CMAKE_CURRENT_BINARY_DIR}/test-libappindicator-status"
    COMMAND
    echo "export DISPLAY=" >> "${CMAKE_CURRENT_BINARY_DIR}/test-libappindicator-status"
    COMMAND
    echo ". ${CMAKE_CURRENT_SOURCE_DIR}/run-xvfb.sh" >> "${CMAKE_CURRENT_BINARY_DIR}/test-libappindicator-status"
    COMMAND
    echo "${DBUS_TEST_RUNNER} --keep-env -m 300 --dbus-config /usr/share/dbus-test-runner/session.conf --task ${CMAKE_CURRENT_BINARY_DIR}/test-libappindicator-status-client --task-name Client --ignore-return --task ${CMAKE_CURRENT_BINARY_DIR}/test-libappindicator-status-server --task-name Server" >> "${CMAKE_CURRENT_BINARY_DIR}/test-libappindicator-status"
    COMMAND
    chmod +x "${CMAKE_CURRENT_BINARY_DIR}/test-libappindicator-status"
)

add_test("test-libappindicator-status" "test-libappindicator-status")

# libappindicator-tests-gtester

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/libappindicator-tests-gtester"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/test-libappindicator"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    VERBATIM
    COMMAND
    echo "#!/bin/sh" > "${CMAKE_CURRENT_BINARY_DIR}/libappindicator-tests-gtester"
    COMMAND
    echo "gtester -k --verbose -o=${CMAKE_CURRENT_BINARY_DIR}/libappindicator-check-results.xml ${CMAKE_CURRENT_BINARY_DIR}/test-libappindicator" >> "${CMAKE_CURRENT_BINARY_DIR}/libappindicator-tests-gtester"
    COMMAND
    chmod +x "${CMAKE_CURRENT_BINARY_DIR}/libappindicator-tests-gtester"
)

# libappindicator-tests

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/libappindicator-tests"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/libappindicator-tests-gtester"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    VERBATIM
    COMMAND
    echo "#!/bin/bash" > "${CMAKE_CURRENT_BINARY_DIR}/libappindicator-tests"
    COMMAND
    echo "export DISPLAY=" >> "${CMAKE_CURRENT_BINARY_DIR}/libappindicator-tests"
    COMMAND
    echo ". ${CMAKE_CURRENT_SOURCE_DIR}/run-xvfb.sh" >> "${CMAKE_CURRENT_BINARY_DIR}/libappindicator-tests"
    COMMAND
    echo "${DBUS_TEST_RUNNER} --keep-env -m 300 --dbus-config /usr/share/dbus-test-runner/session.conf --task ${CMAKE_CURRENT_BINARY_DIR}/libappindicator-tests-gtester" >> "${CMAKE_CURRENT_BINARY_DIR}/libappindicator-tests"
    COMMAND
    chmod +x "${CMAKE_CURRENT_BINARY_DIR}/libappindicator-tests"
)

add_test("libappindicator-tests" "libappindicator-tests")

add_custom_target("tests" ALL DEPENDS "test-libappindicator-fallback" "test-libappindicator-dbus" "test-libappindicator-status" "libappindicator-tests")
