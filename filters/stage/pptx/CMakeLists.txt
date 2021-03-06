add_definitions(-DKDE_DEFAULT_DEBUG_AREA=30528)

include_directories(
    ${KOODF2_INCLUDES}
    ${CMAKE_SOURCE_DIR}/filters/libmsooxml
    ${CMAKE_SOURCE_DIR}/filters/libmso
    ${CMAKE_SOURCE_DIR}/filters/sheets/xlsx
    ${KOMAIN_INCLUDES}
)

########### next target ###############

set(pptx2odp_PART_SRCS
    PptxImport.cpp
    PptxXmlDocumentReader.cpp
    PptxXmlSlideReader.cpp
    PptxXmlCommentsReader.cpp
    PptxXmlCommentAuthorsReader.cpp
    ${CMAKE_SOURCE_DIR}/filters/sheets/xlsx/XlsxChartOdfWriter.cpp
    ${CMAKE_SOURCE_DIR}/filters/libodf2/chart/KoOdfChartWriter.cpp
    ${CMAKE_SOURCE_DIR}/filters/sheets/xlsx/XlsxXmlChartReader.cpp
)

kde4_add_plugin(calligra_filter_pptx2odp ${pptx2odp_PART_SRCS})

target_link_libraries(
    calligra_filter_pptx2odp
    komsooxml
    mso
    koodf2
    koodf
    komain
)

install(TARGETS calligra_filter_pptx2odp DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES
  calligra_filter_pptx2odp.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}/calligra
)

if(SHOULD_BUILD_FILEMANAGER_THUMBNAIL)
    install( FILES stage_pptx_thumbnail.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
endif()

if(SHOULD_BUILD_OKULARGENERATOR_PPTX)
    install( FILES libokularGenerator_pptx_calligra.desktop okularPptx_calligra.desktop DESTINATION ${SERVICES_INSTALL_DIR}/calligra )
    install( PROGRAMS okularApplication_pptx_calligra.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
endif()
