if (NOT WIN32 AND NOT APPLE)
    add_subdirectory(tests)
endif ()

set(kritadefaulttools_PART_SRCS
    default_tools.cc
    kis_tool_colorpicker.cc
    kis_tool_brush.cc
    kis_tool_line.cc
    kis_tool_line_helper.cpp
    kis_tool_fill.cc
    kis_tool_rectangle.cc
    kis_tool_ellipse.cc
    kis_tool_gradient.cc
    kis_tool_measure.cc
    kis_tool_path.cc
    kis_tool_move.cc
    kis_tool_movetooloptionswidget.cpp
    strokes/move_stroke_strategy.cpp
    strokes/move_selection_stroke_strategy.cpp
    kis_tool_multihand.cpp
    kis_tool_pencil.cc
    )

kde4_add_ui_files(kritadefaulttools_PART_SRCS wdgcolorpicker.ui wdgmovetool.ui)

kde4_add_plugin(kritadefaulttools ${kritadefaulttools_PART_SRCS})

target_link_libraries(kritadefaulttools kritaui basicflakes)
target_link_libraries(kritadefaulttools ${Boost_SYSTEM_LIBRARY})

install(TARGETS kritadefaulttools  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES
    krita_tool_color_fill.png
    krita_tool_color_picker.png
    krita_tool_move.png
    krita_tool_gradient.png
    krita_tool_line.png
    krita_tool_rectangle.png
    krita_tool_ellipse.png
    krita_tool_freehandvector.png
    krita_tool_multihand.png
    krita_tool_measure.png
    krita_draw_path.png
    
    dark_krita_draw_path.png
    dark_krita_tool_color_fill.png
    dark_krita_tool_color_picker.png
    dark_krita_tool_ellipse.png
    dark_krita_tool_freehand.png
    dark_krita_tool_freehandvector.png
    dark_krita_tool_gradient.png
    dark_krita_tool_line.png
    dark_krita_tool_measure.png
    dark_krita_tool_move.png
    dark_krita_tool_multihand.png
    dark_krita_tool_rectangle.png
    
    light_krita_draw_path.png
    light_krita_tool_color_fill.png
    light_krita_tool_color_picker.png
    light_krita_tool_ellipse.png
    light_krita_tool_freehand.png
    light_krita_tool_freehandvector.png
    light_krita_tool_gradient.png
    light_krita_tool_line.png
    light_krita_tool_measure.png
    light_krita_tool_move.png
    light_krita_tool_multihand.png
    light_krita_tool_rectangle.png
# cursors
    tool_rectangle_cursor.png
    tool_ellipse_cursor.png
    tool_line_cursor.png
    tool_freehand_cursor.png
    tool_fill_cursor.png
    tool_gradient_cursor.png
    DESTINATION ${DATA_INSTALL_DIR}/krita/pics)

install( FILES  kritadefaulttools.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)

install( FILES
    KisToolPath.action
    KisToolPencil.action
DESTINATION  ${DATA_INSTALL_DIR}/krita/actions)
