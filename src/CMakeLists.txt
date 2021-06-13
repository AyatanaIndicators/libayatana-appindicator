set(HEADERS
    app-indicator.h
)

set(SOURCES
    app-indicator.c
    app-indicator-enum-types.c
    application-service-marshal.c
    generate-id.c
    gen-notification-item.xml.c
    gen-notification-watcher.xml.c
)

set(GIR_SOURCES
    app-indicator.c
    app-indicator.h
)

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/app-indicator.h" DESTINATION "${CMAKE_INSTALL_FULL_INCLUDEDIR}/libayatana-appindicator3-0.1/libayatana-appindicator")

# ayatana-appindicator3-0.1.pc

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/ayatana-appindicator3-0.1.pc.in" "${CMAKE_CURRENT_BINARY_DIR}/ayatana-appindicator3-0.1.pc" @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/ayatana-appindicator3-0.1.pc" DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig")

# app-indicator-enum-types.h

find_program(GLIB_MKENUMS glib-mkenums)

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/app-indicator-enum-types.h"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND
    ${GLIB_MKENUMS}
    --template app-indicator-enum-types.h.in
    ${HEADERS}
    --output="${CMAKE_CURRENT_BINARY_DIR}/app-indicator-enum-types.h"
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/app-indicator-enum-types.h" DESTINATION "${CMAKE_INSTALL_FULL_INCLUDEDIR}/libayatana-appindicator3-0.1/libayatana-appindicator")

# app-indicator-enum-types.c

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/app-indicator-enum-types.c"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/app-indicator-enum-types.h"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND
    ${GLIB_MKENUMS}
    --template app-indicator-enum-types.c.in
    ${HEADERS}
    --output="${CMAKE_CURRENT_BINARY_DIR}/app-indicator-enum-types.c"
)

find_program(GLIB_GENMARSHAL glib-genmarshal)

# application-service-marshal.h

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/application-service-marshal.h"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/app-indicator-enum-types.c"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND
    ${GLIB_GENMARSHAL}
    --prefix=_application_service_marshal
    --header application-service-marshal.list
    --quiet
    --output="${CMAKE_CURRENT_BINARY_DIR}/application-service-marshal.h"
)

# application-service-marshal.c

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/application-service-marshal.c"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/application-service-marshal.h"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND
    ${GLIB_GENMARSHAL}
    --prefix=_application_service_marshal
    --body application-service-marshal.list
    --include-header=application-service-marshal.h
    --quiet
    --output="${CMAKE_CURRENT_BINARY_DIR}/application-service-marshal.c"
)

# gen-notification-item.xml.h

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/gen-notification-item.xml.h" "extern const char * _notification_item;")

# gen-notification-item.xml.c

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/notification-item.xml" GEN_NOTIFICATION_ITEM_XML_C)
string(REPLACE "\"" "\\\"" GEN_NOTIFICATION_ITEM_XML_C ${GEN_NOTIFICATION_ITEM_XML_C})
string(REPLACE "\n" "\\n\"\n\"" GEN_NOTIFICATION_ITEM_XML_C ${GEN_NOTIFICATION_ITEM_XML_C})
string(REGEX REPLACE "\n\"$" "\n" GEN_NOTIFICATION_ITEM_XML_C ${GEN_NOTIFICATION_ITEM_XML_C})
string(PREPEND GEN_NOTIFICATION_ITEM_XML_C "const char * _notification_item = \n\"")
string(APPEND GEN_NOTIFICATION_ITEM_XML_C "\;")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/gen-notification-item.xml.c" ${GEN_NOTIFICATION_ITEM_XML_C})

# gen-notification-watcher.xml.h

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/gen-notification-watcher.xml.h" "extern const char * _notification_watcher;")

# gen-notification-watcher.xml.c

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/notification-watcher.xml" GEN_NOTIFICATION_WATCHER_XML_C)
string(REPLACE "\"" "\\\"" GEN_NOTIFICATION_WATCHER_XML_C ${GEN_NOTIFICATION_WATCHER_XML_C})
string(REPLACE "\n" "\\n\"\n\"" GEN_NOTIFICATION_WATCHER_XML_C ${GEN_NOTIFICATION_WATCHER_XML_C})
string(REGEX REPLACE "\n\"$" "\n" GEN_NOTIFICATION_WATCHER_XML_C ${GEN_NOTIFICATION_WATCHER_XML_C})
string(PREPEND GEN_NOTIFICATION_WATCHER_XML_C "const char * _notification_watcher = \n\"")
string(APPEND GEN_NOTIFICATION_WATCHER_XML_C "\;")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/gen-notification-watcher.xml.c" ${GEN_NOTIFICATION_WATCHER_XML_C})

# libayatana-appindicator3.so

set_source_files_properties(${SOURCES} PROPERTIES COMPILE_FLAGS ${COMPILE_FLAGS})
add_library("ayatana-appindicator3" SHARED ${SOURCES})
set_target_properties("ayatana-appindicator3" PROPERTIES VERSION 1.0.0 SOVERSION 1)
target_compile_definitions("ayatana-appindicator3" PUBLIC G_LOG_DOMAIN="libayatana-appindicator")
target_include_directories("ayatana-appindicator3" PUBLIC ${PROJECT_DEPS_INCLUDE_DIRS})
target_include_directories("ayatana-appindicator3" PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories("ayatana-appindicator3" PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries("ayatana-appindicator3" ${PROJECT_DEPS_LIBRARIES})
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator3.so" DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator3.so.1" DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator3.so.1.0.0" DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}")

# AyatanaAppIndicator3-0.1.gir

add_custom_command(
    TARGET "ayatana-appindicator3" PRE_BUILD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND
    ${CMAKE_COMMAND}
    -E copy
    ${GIR_SOURCES}
    ${CMAKE_CURRENT_BINARY_DIR}
)

find_package(GObjectIntrospection REQUIRED QUIET)

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicator3-0.1.gir"
    DEPENDS "ayatana-appindicator3" "${CMAKE_CURRENT_BINARY_DIR}/app-indicator-version.h"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND
    ${INTROSPECTION_SCANNER}
    ${GIR_SOURCES}
    --symbol-prefix=app
    --identifier-prefix=App
    --namespace=AyatanaAppIndicator3
    --nsversion=0.1
    --quiet
    --warn-all
    --include=GObject-2.0
    --include=Gtk-3.0
    --library-path=${CMAKE_CURRENT_BINARY_DIR}
    --library="ayatana-appindicator3"
    --output "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicator3-0.1.gir"
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicator3-0.1.gir" DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/gir-1.0")

# AyatanaAppIndicator3-0.1.typelib

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicator3-0.1.typelib"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicator3-0.1.gir"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND
    ${INTROSPECTION_COMPILER}
    --includedir=${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicator3-0.1.gir
    -o "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicator3-0.1.typelib"
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicator3-0.1.typelib" DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/girepository-1.0")

add_custom_target(src ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/AyatanaAppIndicator3-0.1.typelib")

# app-indicator-version.h

configure_file(app-indicator-version.h.in app-indicator-version.h @ONLY)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/app-indicator-version.h" DESTINATION "${CMAKE_INSTALL_FULL_INCLUDEDIR}/libayatana-appindicator3-0.1/libayatana-appindicator")
