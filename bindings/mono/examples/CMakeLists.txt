# ayatana-indicator-example

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/ayatana-indicator-example.in" "${CMAKE_CURRENT_BINARY_DIR}/ayatana-indicator-example" @ONLY)

# AyatanaIndicatorExample.exe

find_program(MONO mono)

if (FLAVOUR_GTK3)
    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/AyatanaIndicatorExample.exe"
            DEPENDS "bindings-mono"
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMAND
            ${MCS}
            -out:"${CMAKE_CURRENT_BINARY_DIR}/AyatanaIndicatorExample.exe"
            -target:exe
            ${GTK_SHARP_3_0_LDFLAGS} "-r:${CMAKE_BINARY_DIR}/bindings/mono/ayatana-appindicator3-sharp.dll" "${CMAKE_CURRENT_SOURCE_DIR}/AyatanaIndicatorExample.cs"
        )
elseif (FLAVOUR_GTK2)
    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/AyatanaIndicatorExample.exe"
            DEPENDS "bindings-mono"
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMAND
            ${MCS}
            -out:"${CMAKE_CURRENT_BINARY_DIR}/AyatanaIndicatorExample.exe"
            -target:exe
            ${GTK_SHARP_2_0_LDFLAGS} "-r:${CMAKE_BINARY_DIR}/bindings/mono/ayatana-appindicator-sharp.dll" "${CMAKE_CURRENT_SOURCE_DIR}/AyatanaIndicatorExample.cs"
        )
endif()

add_custom_target("bindings-mono-examples" ALL DEPENDS "AyatanaIndicatorExample.exe")
