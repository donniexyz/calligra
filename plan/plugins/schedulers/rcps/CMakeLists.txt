
set(LIBRCPS_INCLUDE_DIR 3rdparty/LibRCPS/src)
include_directories( ${LIBRCPS_INCLUDE_DIR} ${KOODF_INCLUDES} ${CMAKE_SOURCE_DIR}/plan/libs/kernel ${KDE4_INCLUDES} )

add_subdirectory( 3rdparty )
add_subdirectory( tests )

set ( RCPSScheduler_SRCS
    KPlatoRCPSPlugin.cpp
    KPlatoRCPSScheduler.cpp
)

kde4_add_plugin( kplatorcpsscheduler ${RCPSScheduler_SRCS} )

target_link_libraries(
    kplatorcpsscheduler
    kplatokernel
    koodf
    rcps_plan
#    ${LIBRCPS_LIBRARIES}
)

install( TARGETS kplatorcpsscheduler DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

install( FILES planrcpsscheduler.desktop DESTINATION ${SERVICES_INSTALL_DIR}/calligra )
