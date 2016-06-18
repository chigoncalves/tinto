if (${CMAKE_CURRENT_BINARY_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  message (FATAL_ERROR "In source build are not allowed!")
endif ()

cmake_minimum_required (VERSION 3.0)
project (tinto VERSION 0.0.2.1 LANGUAGES C )

list (APPEND CMAKE_MODULE_PATH
  ${CMAKE_SOURCE_DIR}/cmake
  ${CMAKE_BINARY_DIR}/cmake
  ${CMAKE_SOURCE_DIR}/cmake/modules
)
include_directories (
  ${CMAKE_BINARY_DIR}/include
  ${CMAKE_SOURCE_DIR}/include
)
include (Utils)

option (ENABLE_BATTERY "Enable battery status plugin." ON)
option (ENABLE_TINT2CONF "Enable tint2conf build, a Gtk+2 theme configurator for `tinto`." OFF)
option (ENABLE_EXAMPLES "Install additional tin2rc examples" OFF )
option (ENABLE_RSVG "Enable `rsvg` support for launcher icons." ON )
option (ENABLE_SN "Enable `startup notification` support." ON)

set (tinto_IN_SOURCE_RES_DIR "${CMAKE_HOME_DIRECTORY}/res")
set (tinto_IN_SOURCE_BIN_DIR "${CMAKE_HOME_DIRECTORY}/bin")
set (tinto_IN_SOURCE_DOC_DIR "${CMAKE_HOME_DIRECTORY}/doc")

mark_as_advanced (
  tinto_IN_SOURCE_RES_DIR
  tinto_IN_SOURCE_BIN_DIR
  tinto_IN_SOURCE_DOC_DIR
)

find_package (X11)
if (NOT X11_Xdamage_FOUND OR NOT X11_Xrandr_FOUND OR NOT X11_Xrender_FOUND OR NOT X11_Xinerama_FOUND OR NOT X11_Xcomposite_FOUND)
  message (FATAL_ERROR "Could not find all X11 libraries!")
endif ()

include_directories (
  ${X11_Xrender_INCLUDE_PATH}
  ${X11_Xrandr_INCLUDE_PATH}
  ${X11_Xinerama_INCLUDE_PATH}
  ${X11_Xdamage_INCLUDE_PATH}
  ${X11_Xcomposite_INCLUDE_PATH}
  ${X11_X11_INCLUDE_PATH}
)

find_package (GTK2 2.8 REQUIRED COMPONENTS gtk)
if (NOT GTK2_FOUND)
  message (FATAL_ERROR "Gtk+2 is required to build tinto.")
endif ()

include_directories (${GTK2_INCLUDE_DIRS})

include( FindPkgConfig )
include( CheckLibraryExists )
pkg_check_modules( IMLIB2 REQUIRED imlib2>=1.4.2 )

string( REPLACE ";" " " FLAGS_REPLACED "${IMLIB2_LDFLAGS}" )
set( CMAKE_REQUIRED_FLAGS "${FLAGS_REPLACED}" )
check_library_exists( "${IMLIB2_LIBRARIES}" "imlib_context_set_display" "${IMLIB2_LIBRARY_DIRS}" IMLIB_BUILD_WITH_X )
if( NOT IMLIB_BUILD_WITH_X )
  message( FATAL_ERROR "Imlib is not built with X support" )
endif( NOT IMLIB_BUILD_WITH_X )

include_directories (
  src
  src/clock
  src/systray
  src/taskbar
  src/launcher
  src/tooltip
  src/util
  ${IMLIB2_INCLUDE_DIRS}
)

add_subdirectory (src)
add_subdirectory (src/clock)
add_subdirectory (src/launcher)
add_subdirectory (src/systray)
add_subdirectory (src/taskbar)
add_subdirectory (src/tooltip)
add_subdirectory (src/util)

if (ENABLE_BATTERY)
  add_subdirectory (src/battery)
  include_directories (src/battery)
endif ()

if (ENABLE_RSVG)
  pkg_check_modules (RSVG librsvg-2.0>=2.14.0)
  if (RSVG_FOUND)
    include_directories (${RSVG_INCLUDE_DIRS})
    link_libraries (${RSVG_LIBRARIES})
    link_directories (${RSVG_LIBRARY_DIRS})
    set (HAS_RSVG 1)
    set (HAVE_RSVG 1) # Update-me in `tint2conf'
  else ()
    message (FATAL_ERROR "SVG support enabled yet dependency not fulfilled: librsvg-2.0" )
  endif ()
endif ()

if (ENABLE_SN)
  pkg_check_modules( SN libstartup-notification-1.0>=0.12 )
  if (SN_FOUND)
    include_directories (${SN_INCLUDE_DIRS})
    link_libraries (${SN_LIBRARIES})
    link_directories (${SN_LIBRARY_DIRS})
    set (HAS_SN 1)
    set (SN_API_NOT_YET_FROZEN 1)
  else()
    message (FATAL_ERROR "Startup notification support enabled yet dependency not fulfilled: libstartup-notification-1.0")
  endif ()
endif ()

if (ENABLE_TINT2CONF)
  set (TINT2CONF 1)
  set (TINTO_CONFIG_BIN tint2conf)
  add_subdirectory (src/tint2conf)
endif ()

set (MANDIR share/man CACHE PATH "Directory for man pages")
set (DATADIR share CACHE PATH "Directory for shared data")
set (SYSCONFDIR etc CACHE PATH "Directory for configuration files")
set (DOCDIR share/doc/tinto CACHE PATH "Directory for documentation files")

link_directories (${IMLIB2_LIBRARY_DIRS})

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries (${PROJECT_NAME}
  ${X11_X11_LIB}
  ${X11_Xcomposite_LIB}
  ${X11_Xdamage_LIB}
  ${X11_Xinerama_LIB}
  ${X11_Xrandr_LIB}
  ${X11_Xrender_LIB}
  ${GTK2_LIBRARIES}
  ${IMLIB2_LIBRARIES}
  m
)

include_directories (src)

configure_file (${CMAKE_SOURCE_DIR}/include/conf.h.in ${CMAKE_BINARY_DIR}/include/conf.h)
configure_file (${CMAKE_SOURCE_DIR}/cmake/deinstall.cmake.in ${CMAKE_BINARY_DIR}/cmake/deinstall.cmake)

set_target_properties( ${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-Wall -fno-strict-aliasing -pthread ${ASAN_C_FLAGS}" )
set_target_properties( ${PROJECT_NAME} PROPERTIES LINK_FLAGS "-pthread -fno-strict-aliasing ${ASAN_L_FLAGS}" )

install (TARGETS ${PROJECT_NAME} DESTINATION bin)
install (FILES ${tinto_IN_SOURCE_RES_DIR}/config-sample/tinto.conf DESTINATION xdg/${PROJECT_NAME})
install (FILES ${tinto_IN_SOURCE_RES_DIR}/icon.png DESTINATION ${DATADIR}/${PROJECT_NAME})
install (FILES ChangeLog readme.rst DESTINATION ${DOCDIR})
install (FILES ${tinto_IN_SOURCE_DOC_DIR}/man/tinto.1 DESTINATION ${MANDIR}/man1)

add_custom_target (deinstall
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/cmake/deinstall.cmake
)

message (STATUS ${CMAKE_COMMAND})

if( ENABLE_EXAMPLES )
  file( GLOB SAMPLEFILES sample/*.tint2rc )
  install( FILES ${SAMPLEFILES} DESTINATION ${DOCDIR}/examples )
endif( ENABLE_EXAMPLES )

set (CPACK_GENERATOR "TXZ")
set (CPACK_STRIP_FILES 1)
set (CPACK_PACKAGE_CONTACT "crowseye <eatg75@gmail.com")
set (CPACK_PACKAGE_DESCRIPTION "A clone of tint2 panel")
set (CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/readme.rst")
include (CPack)
