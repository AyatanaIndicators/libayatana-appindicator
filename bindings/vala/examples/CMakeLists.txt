# ayatana-indicator-example.c

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/ayatana-indicator-example.c"
    DEPENDS "bindings-vala"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND
    ${VALA_COMPILER}
    --pkg ayatana-appindicator3-0.1
    --pkg gtk+-3.0
    --vapidir=${CMAKE_BINARY_DIR}/bindings/vala
    -C ayatana-indicator-example.vala
    --metadatadir "${CMAKE_SOURCE_DIR}/src"
    --directory=${CMAKE_CURRENT_BINARY_DIR}
    VERBATIM
    COMMAND
    sed
    -i "s|#include\\s*<\\s*libayatana-appindicator/app-indicator.h\\s*>||g"
    "${CMAKE_CURRENT_BINARY_DIR}/ayatana-indicator-example.c"
)

# ayatana-indicator-example

set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/ayatana-indicator-example.c" PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -include ${CMAKE_SOURCE_DIR}/src/app-indicator.h")
add_executable("ayatana-indicator-example" "${CMAKE_CURRENT_BINARY_DIR}/ayatana-indicator-example.c")
target_include_directories("ayatana-indicator-example" PUBLIC ${PROJECT_DEPS_INCLUDE_DIRS})
target_link_libraries("ayatana-indicator-example" "${PROJECT_DEPS_LIBRARIES} -layatana-appindicator3 -L${CMAKE_BINARY_DIR}/src")
