set(kritadefaultdockers_PART_SRCS 
    kis_layer_box.cpp
    defaultdockers.cpp
)

set(kritadefaultdockers_PART_HEADERS
    kis_layer_box.h
    defaultdockers.h
)

kde4_add_ui_files(kritadefaultdockers_PART_SRCS
    wdglayerbox.ui
)


kde4_add_plugin(kritadefaultdockers ${kritadefaultdockers_PART_SRCS} ${kritadefaultdockers_PART_HEADERS})

target_link_libraries(kritadefaultdockers kritaui)

install(TARGETS kritadefaultdockers  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES  kritadefaultdockers.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)

