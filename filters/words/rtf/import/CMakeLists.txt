add_subdirectory(3rdparty)

include_directories(
    3rdparty/rtf-qt/src/
)

set(rtf2odt_PART_SRCS rtfimport.cpp )
kde4_add_plugin(calligra_filter_rtf2odt ${rtf2odt_PART_SRCS})
target_link_libraries(calligra_filter_rtf2odt RtfReader komain)

install(TARGETS calligra_filter_rtf2odt  DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES  calligra_filter_rtf2odt.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)

if(SHOULD_BUILD_FILEMANAGER_THUMBNAIL)
    install( FILES  words_rtf_thumbnail.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
endif()
