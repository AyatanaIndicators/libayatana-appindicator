if (FLAVOUR_GTK3)
    set (ayatana_appindicator_gtkver "ayatana-appindicator3")
elseif (FLAVOUR_GTK2)
    set (ayatana_appindicator_gtkver "ayatana-appindicator")
endif()

# libayatana-appindicator-docs.sgml

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/libayatana-appindicator-docs.sgml.in" "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator-docs.sgml" @ONLY)

# version.xml

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/version.xml.in" "${CMAKE_CURRENT_BINARY_DIR}/version.xml" @ONLY)

# libayatana-appindicator-decl.txt
# libayatana-appindicator-decl-list.txt
# libayatana-appindicator-overrides.txt
# libayatana-appindicator-sections.txt

find_program(GTKDOC_SCAN gtkdoc-scan)

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator-decl.txt"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator-decl-list.txt"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator-overrides.txt"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator-sections.txt"
    DEPENDS "src"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND
    ${GTKDOC_SCAN}
    --ignore-headers="app-indicator-enum-types.h;application-service-appstore.h;application-service-client.h;application-service-lru-file.h;application-service-marshal.h;application-service-server.h;application-service-watcher.h;dbus-properties-client.h;dbus-properties-server.h;dbus-shared.h;generate-id.h;notification-item-client.h;notification-item-server.h;notification-watcher-client.h;notification-watcher-server.h"
    --module=libayatana-appindicator
    --source-dir="${CMAKE_SOURCE_DIR}/src"
    --source-dir="${CMAKE_BINARY_DIR}/src"
)

# libayatana-appindicator.types

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator.types"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator-decl.txt"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator-decl-list.txt"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator-overrides.txt"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator-sections.txt"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND
    ${CMAKE_COMMAND}
    -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/libayatana-appindicator.types"
    "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator.types"
)

# scangobj.sh

list(JOIN PROJECT_DEPS_INCLUDE_DIRS " -I" GTKDOC_SCANGOBJ_INCLUDE_DIRS)
list(JOIN PROJECT_DEPS_LIBRARIES " -l" GTKDOC_SCANGOBJ_LIBRARIES)

set (SCANGOBJ_AYATANA_APPINDICATOR "${ayatana_appindicator_gtkver}")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/scangobj.sh.in" "${CMAKE_CURRENT_BINARY_DIR}/scangobj.sh")

# libayatana-appindicator.actions
# libayatana-appindicator.args
# libayatana-appindicator.hierarchy
# libayatana-appindicator.interfaces
# libayatana-appindicator.prerequisites
# libayatana-appindicator.signals

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator.actions"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator.args"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator.hierarchy"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator.interfaces"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator.prerequisites"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator.signals"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator.types"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND sh scangobj.sh
)

# gtkdocentities.ent

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/xml/gtkdocentities.ent" "<!ENTITY package \"libayatana-appindicator\">")
file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/xml/gtkdocentities.ent" "\n<!ENTITY package_bugreport \"https://github.com/AyatanaIndicators/libayatana-appindicator/issues\">")
file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/xml/gtkdocentities.ent" "\n<!ENTITY package_name \"libayatana-appindicator\">")
file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/xml/gtkdocentities.ent" "\n<!ENTITY package_string \"libayatana-appindicator ${PROJECT_VERSION}\">")
file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/xml/gtkdocentities.ent" "\n<!ENTITY package_tarname \"libayatana-appindicator\">")
file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/xml/gtkdocentities.ent" "\n<!ENTITY package_url \"https://github.com/AyatanaIndicators/libayatana-appindicator\">")
file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/xml/gtkdocentities.ent" "\n<!ENTITY package_version \"${PROJECT_VERSION}\">")

# libayatana-appindicator-undeclared.txt
# libayatana-appindicator-undocumented.txt
# libayatana-appindicator-unused.txt
# xml/annotation-glossary.xml
# xml/api-index-0.5.xml
# xml/api-index-deprecated.xml
# xml/api-index-full.xml
# xml/app-indicator.xml
# xml/libayatana-appindicator-doc.bottom
# xml/libayatana-appindicator-doc.top
# xml/object_index.sgml
# xml/tree_index.sgml

find_program(GTKDOC_MKDB gtkdoc-mkdb)
list(JOIN PROJECT_DEPS_INCLUDE_DIRS ";-I" GTKDOC_SCANGOBJ_INCLUDE_DIRS)
list(JOIN PROJECT_DEPS_LIBRARIES ";-l" GTKDOC_SCANGOBJ_LIBRARIES)

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator-undeclared.txt"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator-undocumented.txt"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator-unused.txt"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/xml/annotation-glossary.xml"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/xml/api-index-0.5.xml"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/xml/api-index-deprecated.xml"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/xml/api-index-full.xml"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/xml/app-indicator.xml"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/xml/libayatana-appindicator-doc.bottom"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/xml/libayatana-appindicator-doc.top"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/xml/object_index.sgml"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/xml/tree_index.sgml"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator.actions"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator.args"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator.hierarchy"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator.interfaces"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator.prerequisites"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator.signals"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND
    ${GTKDOC_MKDB}
    --module=libayatana-appindicator
    --output-format=xml
    --expand-content-files=""
    --main-sgml-file=libayatana-appindicator-docs.sgml
    --source-dir="${CMAKE_SOURCE_DIR}/src"
    --sgml-mode
)

# html
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/html")

# html/annotation-glossary.html
# html/api-index-0-5.html
# html/api-index-deprecated.html
# html/api-index-full.html
# html/AppIndicator.html
# html/ch01.html
# html/home.png
# html/index.html
# html/left.png
# html/left-insensitive.png
# html/libayatana-appindicator.devhelp2
# html/object-tree.html
# html/right.png
# html/right-insensitive.png
# html/style.css
# html/up.png
# html/up-insensitive.png

find_program(GTKDOC_MKHTML gtkdoc-mkhtml)

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/html/annotation-glossary.html"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/html/api-index-0-5.html"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/html/api-index-deprecated.html"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/html/api-index-full.html"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/html/AppIndicator.html"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/html/ch01.html"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/html/home.png"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/html/index.html"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/html/left.png"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/html/left-insensitive.png"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/html/libayatana-appindicator.devhelp2"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/html/object-tree.html"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/html/right.png"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/html/right-insensitive.png"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/html/style.css"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/html/up.png"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/html/up-insensitive.png"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator-undeclared.txt"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator-undocumented.txt"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/libayatana-appindicator-unused.txt"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/xml/annotation-glossary.xml"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/xml/api-index-0.5.xml"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/xml/api-index-deprecated.xml"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/xml/api-index-full.xml"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/xml/app-indicator.xml" "xml/libayatana-appindicator-doc.bottom"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/xml/libayatana-appindicator-doc.top"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/xml/object_index.sgml"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/xml/tree_index.sgml"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/html"
    COMMAND
    ${GTKDOC_MKHTML}
    --path=${CMAKE_CURRENT_BINARY_DIR}
    libayatana-appindicator
    libayatana-appindicator-docs.sgml
)

# html/annotation-glossary.html
# html/api-index-0-5.html
# html/api-index-deprecated.html
# html/api-index-full.html
# html/AppIndicator.html
# html/ch01.html
# html/index.html
# html/object-tree.html
# html/style.css

find_program(GTKDOC_FIXXREF gtkdoc-fixxref)

add_custom_command(
    OUTPUT "fixxref"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/html/annotation-glossary.html"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/html/api-index-0-5.html"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/html/api-index-deprecated.html"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/html/api-index-full.html"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/html/AppIndicator.html"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/html/ch01.html"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/html/home.png"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/html/index.html"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/html/left.png"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/html/left-insensitive.png"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/html/libayatana-appindicator.devhelp2"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/html/object-tree.html"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/html/right.png"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/html/right-insensitive.png"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/html/style.css"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/html/up.png"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/html/up-insensitive.png"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND
    ${GTKDOC_FIXXREF}
    --module=libayatana-appindicator
    --module-dir=html
    --html-dir="${CMAKE_INSTALL_FULL_DATADIR}/gtk-doc/html"
)

set_source_files_properties("fixxref" PROPERTIES SYMBOLIC True)
install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/html/" DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/gtk-doc/html/lib${ayatana_appindicator_gtkver}")

add_custom_target("docs-references" ALL DEPENDS "fixxref")
