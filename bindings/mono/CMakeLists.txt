# ayatana-appindicator3-sharp-0.1.pc

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/ayatana-appindicator3-sharp-0.1.pc.in" "${CMAKE_CURRENT_BINARY_DIR}/ayatana-appindicator3-sharp-0.1.pc" @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/ayatana-appindicator3-sharp-0.1.pc" DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig")

# ayatana-appindicator3-sharp.dll.config

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/ayatana-appindicator3-sharp.dll.config" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/ayatana-appindicator3-sharp.dll.config" DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/cli/ayatana-appindicator3-sharp-0.1")

# app-indicator.sources.xml

file(RELATIVE_PATH REL_BIN ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR})
file(RELATIVE_PATH REL_SRC ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR})
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/app-indicator.sources.xml.in" "${CMAKE_CURRENT_BINARY_DIR}/app-indicator.sources.xml" @ONLY)

# libayatana-appindicator3-api.raw

find_program(GAPI3_PARSER gapi3-parser)

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator3-api.raw"
    DEPENDS "src" "${CMAKE_CURRENT_BINARY_DIR}/app-indicator.sources.xml"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND
    ${GAPI3_PARSER}
    "${CMAKE_CURRENT_BINARY_DIR}/app-indicator.sources.xml"
)

# libayatana-appindicator3-api.middle

find_program(GAPI3_FIXUP gapi3-fixup)

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator3-api.middle"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator3-api.raw"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND
    ${CMAKE_COMMAND}
    -E copy
    "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator3-api.raw"
    "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator3-api.middle"
    COMMAND
    ${GAPI3_FIXUP}
    --api=libayatana-appindicator3-api.middle
    --metadata="${CMAKE_CURRENT_SOURCE_DIR}/libayatana-appindicator3-api.metadata"
)

# libayatana-appindicator3-api.xml

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator3-api.xml"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator3-api.middle"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    VERBATIM
    COMMAND
    sed
    -e "s|PROP_ID_S|id|"
    -e "s|PROP_STATUS_S|Status|"
    -e "s|PROP_STATUS_S|status|"
    -e "s|PROP_CATEGORY_S|Category|"
    -e "s|PROP_CATEGORY_S|category|"
    -e "s|PROP_ICON_NAME_S|icon-name|"
    -e "s|PROP_ICON_DESC_S|IconDesc|"
    -e "s|PROP_ICON_DESC_S|icon-desc|"
    -e "s|PROP_ATTENTION_ICON_NAME_S|AttentionIconName|"
    -e "s|PROP_ATTENTION_ICON_NAME_S|attention-icon-name|"
    -e "s|PROP_ATTENTION_ICON_DESC_S|AttentionIconDesc|"
    -e "s|PROP_ATTENTION_ICON_DESC_S|attention-icon-desc|"
    -e "s|PROP_ICON_THEME_PATH_S|icon-theme-path|"
    -e "s|PROP_MENU_S|menu|"
    -e "s|PROP_CONNECTED_S|connected|"
    -e "s|PROP_LABEL_S|label|"
    -e "s|PROP_LABEL_GUIDE_S|label-guide|"
    -e "s|PROP_ORDERING_INDEX_S|ordering-index|"
    -e "s|PROP_DBUS_MENU_SERVER_S|DbusMenuServer|"
    -e "s|PROP_DBUS_MENU_SERVER_S|dbus-menu-server|"
    -e "s|PROP_TITLE_S|Title|"
    -e "s|PROP_TITLE_S|title|"
    "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator3-api.middle" > "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator3-api.xml"
)

# AppIndicatorCategory.cs
# NewAttentionIconHandler.cs
# NewStatusHandler.cs
# AppIndicatorStatus.cs
# NewIconHandler.cs
# ObjectManager.cs
# ApplicationIndicator.cs
# NewIconThemePathHandler.cs
# ScrollEventHandler.cs
# ConnectionChangedHandler.cs
# NewLabelHandler.cs

pkg_check_modules(GTK_SHARP_3_0 REQUIRED gtk-sharp-3.0>=2.99)
string(REPLACE ":" ";-I:" GTK_SHARP_3_0_INCLUDE_DIRS ${GTK_SHARP_3_0_INCLUDE_DIRS})
find_program(GAPI3_CODEGEN gapi3-codegen)

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/GtkSharp/ObjectManager.cs"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicator/AppIndicatorCategory.cs"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicator/NewAttentionIconHandler.cs"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicator/NewStatusHandler.cs"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicator/AppIndicatorStatus.cs"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicator/NewIconHandler.cs"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicator/ApplicationIndicator.cs"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicator/NewIconThemePathHandler.cs"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicator/ScrollEventHandler.cs"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicator/ConnectionChangedHandler.cs"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicator/NewLabelHandler.cs"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator3-api.xml"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND
    ${GAPI3_CODEGEN}
    --generate libayatana-appindicator3-api.xml
    --outdir=${CMAKE_CURRENT_BINARY_DIR}
    --assembly-name=ayatana-appindicator3-sharp
    ${GTK_SHARP_3_0_INCLUDE_DIRS}
)

# ayatana-appindicator3-sharp.dll

pkg_check_modules(MONO REQUIRED mono>=5.18)
find_program(MCS mcs)

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/ayatana-appindicator3-sharp.dll"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/GtkSharp/ObjectManager.cs"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicator/AppIndicatorCategory.cs"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicator/NewAttentionIconHandler.cs"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicator/NewStatusHandler.cs"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicator/AppIndicatorStatus.cs"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicator/NewIconHandler.cs"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicator/ApplicationIndicator.cs"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicator/NewIconThemePathHandler.cs"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicator/ScrollEventHandler.cs"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicator/ConnectionChangedHandler.cs"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicator/NewLabelHandler.cs"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND
    ${MCS}
    -keyfile:"${CMAKE_CURRENT_SOURCE_DIR}/ayatana-appindicator3-sharp.snk"
    -nowarn:0169,0612,0618
    -unsafe
    -out:"${CMAKE_CURRENT_BINARY_DIR}/ayatana-appindicator3-sharp.dll"
    -target:library
    ${GTK_SHARP_3_0_LDFLAGS} "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicator/*.cs" "${CMAKE_CURRENT_BINARY_DIR}/GtkSharp/ObjectManager.cs" "${CMAKE_CURRENT_SOURCE_DIR}/AssemblyInfo.cs" "${CMAKE_CURRENT_SOURCE_DIR}/ApplicationIndicatorCustom.cs"
)

find_program(GACUTIL gacutil)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/ayatana-appindicator3-sharp.dll" DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/cli/ayatana-appindicator3-sharp-0.1")
install(CODE "execute_process(WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} COMMAND ${GACUTIL} -i ${CMAKE_CURRENT_BINARY_DIR}/ayatana-appindicator3-sharp.dll -package ayatana-appindicator3-sharp -root ${CMAKE_INSTALL_FULL_LIBDIR})")

# policy.0.0.ayatana-appindicator3-sharp.config

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/policy.0.0.ayatana-appindicator3-sharp.config.in" "${CMAKE_CURRENT_BINARY_DIR}/policy.0.0.ayatana-appindicator3-sharp.config" @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/policy.0.0.ayatana-appindicator3-sharp.config" DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/cli/ayatana-appindicator3-sharp-0.1")

# policy.0.0.ayatana-appindicator3-sharp.dll

find_program(AL al)

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/policy.0.0.ayatana-appindicator3-sharp.dll"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/ayatana-appindicator3-sharp.dll"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND
    ${AL}
    -link:"${CMAKE_CURRENT_BINARY_DIR}/policy.0.0.ayatana-appindicator3-sharp.config"
    -out:"${CMAKE_CURRENT_BINARY_DIR}/policy.0.0.ayatana-appindicator3-sharp.dll"
    -keyfile:"${CMAKE_CURRENT_SOURCE_DIR}/ayatana-appindicator3-sharp.snk"
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/policy.0.0.ayatana-appindicator3-sharp.dll" DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/cli/ayatana-appindicator3-sharp-0.1")
install(CODE "execute_process(WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} COMMAND ${GACUTIL} -i ${CMAKE_CURRENT_BINARY_DIR}/policy.0.0.ayatana-appindicator3-sharp.dll -package ayatana-appindicator3-sharp -root ${CMAKE_INSTALL_FULL_LIBDIR})")

# policy.0.1.ayatana-appindicator3-sharp.config

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/policy.0.1.ayatana-appindicator3-sharp.config.in" "${CMAKE_CURRENT_BINARY_DIR}/policy.0.1.ayatana-appindicator3-sharp.config" @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/policy.0.1.ayatana-appindicator3-sharp.config" DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/cli/ayatana-appindicator3-sharp-0.1")

# policy.0.1.ayatana-appindicator3-sharp.dll

find_program(AL al)

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/policy.0.1.ayatana-appindicator3-sharp.dll"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/policy.0.0.ayatana-appindicator3-sharp.dll"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND
    ${AL}
    -link:"${CMAKE_CURRENT_BINARY_DIR}/policy.0.1.ayatana-appindicator3-sharp.config"
    -out:"${CMAKE_CURRENT_BINARY_DIR}/policy.0.1.ayatana-appindicator3-sharp.dll"
    -keyfile:"${CMAKE_CURRENT_SOURCE_DIR}/ayatana-appindicator3-sharp.snk"
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/policy.0.1.ayatana-appindicator3-sharp.dll" DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/cli/ayatana-appindicator3-sharp-0.1")

add_custom_target("bindings-mono" ALL DEPENDS "policy.0.1.ayatana-appindicator3-sharp.dll")

if (ENABLE_TESTS)
    add_subdirectory(examples)
endif()
