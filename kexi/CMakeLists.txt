project(kexi)

include(CheckFunctionExists)

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

option(KEXI_MOBILE "Compile a mobile version of Kexi" OFF)

check_function_exists("uname" HAVE_UNAME)

option(KEXI_DEBUG_GUI "Debugging GUI for Kexi (requires CALLIGRADB_DEBUG_GUI to be set too)" OFF)
option(KEXI_SHOW_UNFINISHED, "Show unfinished features in Kexi. Thus is useful for testing but may confuse end-user." OFF)
option(KEXI_SHOW_UNIMPLEMENTED "Forces to show menu entries and dialogs just to give impression about development plans for Kexi. Only recommended for test/development versions." OFF)

# Experimental:
option(KEXI_SCRIPTS_SUPPORT "Experimental: Enable scripting in Kexi" ON)
option(KEXI_PROJECT_TEMPLATES "Experimental: Enable support for project templates in Kexi" OFF)
option(KEXI_AUTORISE_TABBED_TOOLBAR "Experimental: Autorise the main tabbed toolbar in Kexi" OFF)

add_definitions(
  -DKEXI_NO_CURSOR_PROPERTY
  -DKEXI_NO_CTXT_HELP
  -DKEXI_NO_SUBFORM
  -DKEXI_NO_QUICK_PRINTING # print/print preview for tables/queries in the navigator
  -DKEXI_NO_AUTOFIELD_WIDGET
)

#no default: add_definitions(-DKDE_DEFAULT_DEBUG_AREA=44010)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-kexi.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kexi.h )

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${KOMAIN_INCLUDES}
  ${KOPROPERTY_INCLUDES}
  ${CALLIGRADB_INCLUDES}
  ${CMAKE_SOURCE_DIR}/libs
)

add_subdirectory( kexiutils )

macro_optional_find_package(MySQL)
macro_log_feature(MYSQL_FOUND "libmysqlclient" "MySQL Client Library" "http://www.mysql.com" FALSE "" "Required by Kexi MySQL driver")

set (PQXX_MIN_VERSION "3.0.0")
set (PQXX_MAX_VERSION "5.0.0")

macro_optional_find_package(CalligraPostgreSQL)
macro_log_feature(POSTGRESQL_FOUND "libpq" "C application programmer's interface to PostgreSQL" "http://www.postgresql.org" FALSE "" "Required by Kexi PostgreSQL driver")

pkg_check_modules (PQXX libpqxx)

if (POSTGRESQL_FOUND AND PQXX_FOUND)
   if (PQXX_VERSION VERSION_GREATER PQXX_MIN_VERSION AND PQXX_VERSION VERSION_LESS PQXX_MAX_VERSION)
      macro_log_feature(PQXX_FOUND "libpqxx" "Official C++ client API for PostgreSQL" "http://pqxx.org/development/libpqxx/" FALSE "" "Required by Kexi PostgreSQL driver")
   else ()
      macro_log_feature(FALSE "libpqxx" "Official C++ client API for PostgreSQL, version >= ${PQXX_MIN_VERSION} and older than ${PQXX_MAX_VERSION}" "http://pqxx.org/development/libpqxx/" FALSE "" "Required by Kexi PostgreSQL driver")
   endif ()
endif ()

macro_optional_find_package(FreeTDS)
macro_log_feature(FREETDS_FOUND "FreeTDS" "Open source implementation of the TDS (Tabular Data Stream) protocol" "http://www.freetds.org" FALSE "" "Required by Kexi Sybase driver")

macro_optional_find_package(XBase)
macro_log_feature(XBASE_FOUND "XBase" "XBase compatible C++ class library" "http://linux.techass.com/projects/xdb" FALSE "" "Required by Kexi XBase driver")

add_subdirectory( kexidb )
add_subdirectory( core )
add_subdirectory( widget )
add_subdirectory( data )
add_subdirectory( pics )
add_subdirectory( plugins )
add_subdirectory( tests )

if(KEXI_MOBILE)

else()
  add_subdirectory( main )
  add_subdirectory( formeditor )
  add_subdirectory( migration )
endif()

########### next target ###############

if(KEXI_MOBILE)
  add_subdirectory( mobile )
else()
  set(kexi_SRCS main.cpp )
  kde4_add_app_icon(kexi_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/pics/app/hi*-app-calligrakexi.png")
  kde4_add_executable(kexi ${kexi_SRCS})
  target_link_libraries(kexi
    ${KDE4_KFILE_LIBS}
    ${CALLIGRADB_LIBS}
    keximain
    kexicore
  )
  install(TARGETS kexi ${INSTALL_TARGETS_DEFAULT_ARGS})
endif()

########### install files ###############

install(PROGRAMS kexi.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES kexi.appdata.xml DESTINATION ${SHARE_INSTALL_PREFIX}/appdata/)

#kde4_create_doxygen_docs( EXCLUDE 3rdparty kexidb/parser/sqlparser.h kexidb/drivers/odbc  )
