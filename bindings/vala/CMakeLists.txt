# ayatana-appindicator3-0.1.deps

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/ayatana-appindicator3-0.1.deps" DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/vala/vapi")

# AyatanaAppIndicator3-0.1.vala

find_package(Vala REQUIRED QUIET)

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicator3-0.1.vala"
    DEPENDS "src" "${CMAKE_BINARY_DIR}/src/AyatanaAppIndicator3-0.1.typelib"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND
    ${VAPI_GEN}
    --library=ayatana-appindicator3-0.1
    --pkg gtk+-3.0
    --girdir="${CMAKE_SOURCE_DIR}/src"
    --metadatadir "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_BINARY_DIR}/src/AyatanaAppIndicator3-0.1.gir"
    "${CMAKE_CURRENT_SOURCE_DIR}/ayatana-appindicator-0.1-custom.vala"
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/ayatana-appindicator3-0.1.vapi" DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/vala/vapi")

add_custom_target("bindings-vala" ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicator3-0.1.vala")

if (ENABLE_TESTS)
    add_subdirectory(examples)
endif()
