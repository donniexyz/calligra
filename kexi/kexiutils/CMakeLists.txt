add_definitions(-DKDE_DEFAULT_DEBUG_AREA=44024)

include_directories( 
  ${CMAKE_SOURCE_DIR}/kexi 
  ${CMAKE_CURRENT_SOURCE_DIR}/completer
  ${KDE4_INCLUDES}
)

set(kexiutils_LIB_SRCS 
  identifier.cpp 
  multivalidator.cpp 
  utils.cpp 
  styleproxy.cpp 
  longlongvalidator.cpp 
  InternalPropertyMap.cpp
  SmallToolButton.cpp 
  KexiCommandLinkButton.cpp
  FlowLayout.cpp
  transliteration_table.cpp
  kmessagewidget.cpp 
  KexiContextMessage.cpp
  KexiTitleLabel.cpp
  KexiLinkWidget.cpp
  KexiLinkButton.cpp
  KexiCloseButton.cpp
  KexiAssistantPage.cpp
  KexiAssistantWidget.cpp
  KexiAnimatedLayout.cpp
  KexiCategorizedView.cpp
  KexiTester.cpp
  
  completer/KexiCompleter.cpp
  KexiPushButton.cpp

)

if (KEXI_MOBILE)

else ()
  if (KEXI_DEBUG_GUI)
    list(APPEND kexiutils_LIB_SRCS
      debuggui.cpp
    )
  endif ()
endif ()

kde4_add_library(kexiutils SHARED ${kexiutils_LIB_SRCS})

target_link_libraries(kexiutils  ${CALLIGRADB_LIBS} ${KDE4_KIO_LIBS} )
target_link_libraries(kexiutils  LINK_INTERFACE_LIBRARIES ${KDE4_KIO_LIBS})

set_target_properties(kexiutils PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)

install(TARGETS kexiutils  ${INSTALL_TARGETS_DEFAULT_ARGS})

if(FALSE) # TODO: install when we move to independent place
    install( FILES  tristate.h identifier.h validator.h utils.h kexiutils_export.h kexiutils_global.h
    styleproxy.h longlongvalidator.h InternalPropertyMap.h 
    SmallToolButton.h FlowLayout.h
    kmessagewidget.h KexiContextMessage.h KexiTitleLabel.h KexiAssistantPage.h 
    KexiAssistantWidget.h KexiAnimatedLayout.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kexiutils COMPONENT Devel)
endif()

add_subdirectory(tests)
