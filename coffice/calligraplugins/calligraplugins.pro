include($${TOP_SOURCE_DIR}/common.pri)

TEMPLATE = lib
TARGET = calligraplugins

#QT += core gui xml sql network widgets
QT += core gui xml svg

DEFINES += CALLIGRAPLUGINS_LIBRARY

CALLIGRAPLUGINS_TEXTSHAPE_DIR = $${TOP_SOURCE_DIR}/../plugins/textshape
CALLIGRAPLUGINS_PICTURESHAPE_DIR = $${TOP_SOURCE_DIR}/../plugins/pictureshape
CALLIGRAPLUGINS_VARIABLES_DIR = $${TOP_SOURCE_DIR}/../plugins/variables

INCLUDEPATH = \
     $${CALLIGRAPLUGINS_TEXTSHAPE_DIR} \
     $${CALLIGRAPLUGINS_PICTURESHAPE_DIR} \
     $${CALLIGRAPLUGINS_VARIABLES_DIR} \
     $$INCLUDEPATH

unix {
    !macx: LIBEXT = so
    macx: LIBEXT = dylib
    CONFIG( static ): LIBEXT = a
    LIBS += $${TOP_BUILD_DIR}/calligralibs/libcalligralibs.$${LIBEXT}
}
win32: LIBS += -L$${TOP_BUILD_DIR} -lcalligralibs

SOURCES += $$files($$CALLIGRAPLUGINS_TEXTSHAPE_DIR/TextPlugin.cpp)
SOURCES += $$files($$CALLIGRAPLUGINS_TEXTSHAPE_DIR/TextShapeFactory.cpp)
SOURCES += $$files($$CALLIGRAPLUGINS_TEXTSHAPE_DIR/TextShape.cpp)
SOURCES += $$files($$CALLIGRAPLUGINS_TEXTSHAPE_DIR/SimpleRootAreaProvider.cpp)
SOURCES += $$files($$CALLIGRAPLUGINS_TEXTSHAPE_DIR/ShrinkToFitShapeContainer.cpp)

#SOURCES += $$files($$CALLIGRAPLUGINS_PICTURESHAPE_DIR/Plugin.cpp)
#SOURCES += $$files($$CALLIGRAPLUGINS_PICTURESHAPE_DIR/PictureShapeFactory.cpp)
#SOURCES += $$files($$CALLIGRAPLUGINS_PICTURESHAPE_DIR/PictureShape.cpp)
#SOURCES += $$files($$CALLIGRAPLUGINS_PICTURESHAPE_DIR/ClippingRect.cpp)
#SOURCES += $$files($$CALLIGRAPLUGINS_PICTURESHAPE_DIR/filters/GreyscaleFilterEffect.cpp)
#SOURCES += $$files($$CALLIGRAPLUGINS_PICTURESHAPE_DIR/filters/MonoFilterEffect.cpp)
#SOURCES += $$files($$CALLIGRAPLUGINS_PICTURESHAPE_DIR/filters/WatermarkFilterEffect.cpp)

#SOURCES += $$files($$CALLIGRAPLUGINS_VARIABLES_DIR/*.cpp)

HEADERS += $$files($$CALLIGRAPLUGINS_TEXTSHAPE_DIR/TextPlugin.h)
HEADERS += $$files($$CALLIGRAPLUGINS_TEXTSHAPE_DIR/TextShapeFactory.h)
HEADERS += $$files($$CALLIGRAPLUGINS_TEXTSHAPE_DIR/TextShape.h)
HEADERS += $$files($$CALLIGRAPLUGINS_TEXTSHAPE_DIR/SimpleRootAreaProvider.h)
HEADERS += $$files($$CALLIGRAPLUGINS_TEXTSHAPE_DIR/ShrinkToFitShapeContainer.h)

#HEADERS += $$files($$CALLIGRAPLUGINS_PICTURESHAPE_DIR/Plugin.h)
#HEADERS += $$files($$CALLIGRAPLUGINS_PICTURESHAPE_DIR/PictureShapeFactory.h)
#HEADERS += $$files($$CALLIGRAPLUGINS_PICTURESHAPE_DIR/PictureShape.h)
#HEADERS += $$files($$CALLIGRAPLUGINS_PICTURESHAPE_DIR/ClippingRect.h)
#HEADERS += $$files($$CALLIGRAPLUGINS_PICTURESHAPE_DIR/filters/GreyscaleFilterEffect.h)
#HEADERS += $$files($$CALLIGRAPLUGINS_PICTURESHAPE_DIR/filters/MonoFilterEffect.h)
#HEADERS += $$files($$CALLIGRAPLUGINS_PICTURESHAPE_DIR/filters/WatermarkFilterEffect.h)

#HEADERS += $$files($$CALLIGRAPLUGINS_VARIABLES_DIR/*.h)

#FORMS += $$files($$CALLIGRAPLUGINS_VARIABLES_DIR/*.ui)

mocWrapper(HEADERS)
