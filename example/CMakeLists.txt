if (FLAVOUR_GTK3)
    set (ayatana_appindicator_gtkver "ayatana-appindicator3")
    set (gtk_valapkgver "gtk+-3.0")
elseif (FLAVOUR_GTK2)
    set (ayatana_appindicator_gtkver "ayatana-appindicator")
    set (gtk_valapkgver "gtk+-2.0")
endif()

# simple-client-vala.c

# Vala bindings are only available for FLAVOUR_GTK3

if (FLAVOUR_GTK3)
    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/simple-client-vala.c"
        DEPENDS "bindings-vala"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND
        ${VALA_COMPILER}
        --pkg ${ayatana_appindicator_gtkver}-0.1
        --pkg ${gtk_valapkgver}
        --vapidir=${CMAKE_BINARY_DIR}/bindings/vala
        -C simple-client-vala.vala
        --directory=${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM
        COMMAND
        sed
        -i "s|#include\\s*<\\s*libayatana-appindicator/app-indicator.h\\s*>||g"
        "${CMAKE_CURRENT_BINARY_DIR}/simple-client-vala.c"
    )

    # simple-client-vala

    set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/simple-client-vala.c" PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -include ${CMAKE_SOURCE_DIR}/src/app-indicator.h")
    add_executable("simple-client-vala" "${CMAKE_CURRENT_BINARY_DIR}/simple-client-vala.c")
    target_include_directories("simple-client-vala" PUBLIC ${PROJECT_DEPS_INCLUDE_DIRS})
    target_link_libraries("simple-client-vala" "${PROJECT_DEPS_LIBRARIES} -l${ayatana_appindicator_gtkver} -L${CMAKE_BINARY_DIR}/src")
    add_dependencies("simple-client-vala" "${ayatana_appindicator_gtkver}")

endif()

# simple-client

set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/simple-client.c" PROPERTIES COMPILE_FLAGS ${COMPILE_FLAGS})
add_executable("simple-client" "${CMAKE_CURRENT_SOURCE_DIR}/simple-client.c")
target_compile_definitions("simple-client" PUBLIC LOCAL_ICON="${CMAKE_CURRENT_SOURCE_DIR}/simple-client-test-icon.png")
target_include_directories("simple-client" PUBLIC ${PROJECT_DEPS_INCLUDE_DIRS})
target_include_directories("simple-client" PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_link_libraries("simple-client" "${PROJECT_DEPS_LIBRARIES} -l${ayatana_appindicator_gtkver} -L${CMAKE_BINARY_DIR}/src")
add_dependencies("simple-client" "${ayatana_appindicator_gtkver}")
