set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_SOURCE_DIR}/..    ${CMAKE_SOURCE_DIR}/krita/sdk/tests )

macro_add_unittest_definitions()

########### next target ###############

set(psd_header_test_SRCS psd_header_test.cpp ${CMAKE_SOURCE_DIR}/krita/libpsd/psd_utils.cpp ../psd_header.cpp )
kde4_add_unit_test(psd_header_test TESTNAME krita-psd-psd_header_test  ${psd_header_test_SRCS})
target_link_libraries(psd_header_test ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(psd_utils_test_SRCS psd_utils_test.cpp ${CMAKE_SOURCE_DIR}/krita/libpsd/psd_utils.cpp)
kde4_add_unit_test(psd_utils_test TESTNAME krita-psd-psd_utils_test  ${psd_utils_test_SRCS})
target_link_libraries(psd_utils_test ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(psd_colormode_block_test_SRCS psd_colormode_block_test.cpp ../psd_colormode_block.cpp ../psd_header.cpp ${CMAKE_SOURCE_DIR}/krita/libpsd/psd_utils.cpp)
kde4_add_unit_test(psd_colormode_block_test TESTNAME krita-psd-psd_colormode_block_test  ${psd_colormode_block_test_SRCS})
target_link_libraries(psd_colormode_block_test ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})

########### next target ###############

set(compression_test_SRCS compression_test.cpp ${CMAKE_SOURCE_DIR}/krita/libpsd/compression.cpp)
kde4_add_unit_test(compression_test TESTNAME krita-psd-compression_test  ${compression_test_SRCS})
target_link_libraries(compression_test ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})

########### next target ###############
set(kis_psd_test_SRCS kis_psd_test.cpp )
kde4_add_unit_test(kis_psd_test TESTNAME krita-plugins-formats-psd_test ${kis_psd_test_SRCS})
target_link_libraries(kis_psd_test  ${KDE4_KDEUI_LIBS} kritaui ${QT_QTTEST_LIBRARY})
