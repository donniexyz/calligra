#add_subdirectory( tests )

include_directories(${KOODF_INCLUDES}
                    ${KOODF2_INCLUDES}

                    # FIXME: This should not be here. Make sure that the chart
                    #        classes parse the number formats while loading,
                    #        and not just store the unparsed format strings to
                    #        be parsed during saving.
 		    ${CMAKE_SOURCE_DIR}/filters/libmso  # for NumberFormatParser
)

########### libkoodf2 ###############

set(koodf2_LIB_SRCS
    # Storage objects, using old DOM based loading
    KoXmlStreamReader.cpp
    KoXmlUtils.cpp

    KoTable.cpp
    KoRow.cpp
    KoColumn.cpp
    KoCell.cpp
    KoCellValue.cpp
    KoDummyCellValue.cpp
    KoCellChild.cpp
    KoRawCellChild.cpp

    # Style objects, using old DOM based loading
    KoStyle.cpp
    KoTblStyle.cpp
    KoRowStyle.cpp
    KoColumnStyle.cpp
    KoCellStyle.cpp

    # Style objects, using newer stream based loading
    KoOdfStyleBase.cpp
    KoOdfStyleManager.cpp
    KoOdfStyle.cpp
    KoOdfListStyle.cpp
    KoOdfPageLayout.cpp
    KoOdfStyleProperties.cpp
    KoOdfTextProperties.cpp
    KoOdfParagraphProperties.cpp
    KoOdfListLevelProperties.cpp
    KoOdfGraphicProperties.cpp
    KoOdfPageLayoutProperties.cpp
    KoOdfHeaderFooterProperties.cpp

    chart/KoOdfChartWriter.cpp
)

kde4_add_library(koodf2 SHARED ${koodf2_LIB_SRCS})

target_link_libraries(koodf2 koodf mso ${KDE4_KIO_LIBS} ${QT_QTXML_LIBRARY})
target_link_libraries(koodf2 LINK_INTERFACE_LIBRARIES ${KDE4_KIO_LIBS})

set_target_properties(koodf2 PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)
install(TARGETS koodf2 ${INSTALL_TARGETS_DEFAULT_ARGS} )


install( FILES
    koodf2_export.h

    KoStyle.h
    KoTable.h
    KoTblStyle.h
    KoColumn.h
    KoColumnStyle.h
    KoRawCellChild.h
    KoRow.h
    KoRowStyle.h
    KoCell.h
    KoCellStyle.h

    KoOdfStyleBase.h
    KoOdfStyleManager.h
    KoOdfStyle.h
    KoOdfStyleProperties.h
    KoOdfTextProperties.h
    KoOdfParagraphProperties.h
    KoOdfGraphicProperties.h
    KoOdfPageLayoutProperties.h
    KoOdfHeaderFooterProperties.h

    chart/Charting.h
    chart/KoOdfChartWriter.h

DESTINATION ${INCLUDE_INSTALL_DIR}/calligra COMPONENT Devel)

