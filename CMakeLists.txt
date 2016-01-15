if (${CMAKE_CURRENT_BINARY_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  message (FATAL_ERROR "In source build are not allowed!")
endif ()

cmake_minimum_required (VERSION 3.0)
project (tinto VERSION 0.0.1.3 LANGUAGES C )

list (APPEND CMAKE_MODULE_PATH ";${CMAKE_SOURCE_DIR}/cmake")
include_directories ("${CMAKE_BINARY_DIR}/include" "${CMAKE_SOURCE_DIR}/include")
include (Utils)

option (ENABLE_BATTERY "Enable battery status plugin." ON)
option (ENABLE_TINT2CONF "Enable tint2conf build, a Gtk+2 theme configurator for `tinto`." OFF)
option( ENABLE_EXAMPLES "Install additional tin2rc examples" OFF )
option (ENABLE_RSVG "Enable `rsvg` support for launcher icons." ON )
option( ENABLE_SN "Enable `startup notification` support." ON )

if (TINTO_DEVEL_MODE)
  option (ENABLE_SAN "Build with for Googles: AddressSanitizec AddressSanitizer, LeakSanitizer, \
ThreadSanitizer or MemorySanitizer, use this options with theses values: asan, msan, tsan or lsan." on)


  add_executable (
    test-string-addins
    src/util/string-addins.c
    src/util/t/test-string-addins.c
  )

  target_link_libraries (
    test-string-addins
    cunit
  )

  add_custom_target (
    tests
    COMMAND ${CMAKE_BINARY_DIR}/test-string-addins
    DEPENDS test-string-addins
  )
endif ()

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
pkg_check_modules( RSVG librsvg-2.0>=2.14.0 )
pkg_check_modules( SN libstartup-notification-1.0>=0.12 )

string( REPLACE ";" " " FLAGS_REPLACED "${IMLIB2_LDFLAGS}" )
set( CMAKE_REQUIRED_FLAGS "${FLAGS_REPLACED}" )
check_library_exists( "${IMLIB2_LIBRARIES}" "imlib_context_set_display" "${IMLIB2_LIBRARY_DIRS}" IMLIB_BUILD_WITH_X )
if( NOT IMLIB_BUILD_WITH_X )
  message( FATAL_ERROR "Imlib is not built with X support" )
endif( NOT IMLIB_BUILD_WITH_X )

include_directories (
  src
  src/battery
  src/clock
  src/systray
  src/taskbar
  src/launcher
  src/tooltip
  src/util
  ${IMLIB2_INCLUDE_DIRS}
  ${RSVG_INCLUDE_DIRS}
  ${SN_INCLUDE_DIRS}
)

set( SOURCES src/config.c
             src/panel.c
             src/server.c
             src/tint.c
             src/clock/clock.c
             src/systray/systraybar.c
             src/launcher/launcher.c
             src/launcher/apps-common.c
             src/launcher/icon-theme-common.c
             src/launcher/xsettings-client.c
             src/launcher/xsettings-common.c
             src/taskbar/task.c
             src/taskbar/taskbar.c
             src/taskbar/taskbarname.c
             src/tooltip/tooltip.c
             src/util/area.c
             src/util/common.c
	     src/util/debug.c
             src/util/strnatcmp.c
             src/util/timer.c
	     src/util/string-addins.c
             src/util/window.c )

if (ENABLE_BATTERY)
  list (APPEND SOURCES src/battery/battery.c)
endif ()

if (ENABLE_RSVG)
  if (RSVG_FOUND)
    set (HAS_RSVG 1)
  else ()
    message (FATAL_ERROR "SVG support enabled yet dependency not fulfilled: librsvg-2.0" )
  endif ()
endif ()

if (ENABLE_SN)
  if (SN_FOUND)
    set (HAS_SN 1)
    set (SN_API_NOT_YET_FROZEN 1)
  else()
    message (FATAL_ERROR "Startup notification support enabled yet dependency not fulfilled: libstartup-notification-1.0")
  endif ()
endif ()

if (ENABLE_TINT2CONF)
  add_subdirectory (src/tint2conf)
endif ()

if( ENABLE_ASAN )
  SET(ASAN_C_FLAGS " -O0 -g3 -gdwarf-2 -fsanitize=address -fno-common -fno-omit-frame-pointer -rdynamic ")
  SET(ASAN_L_FLAGS " -O0 -g3 -gdwarf-2 -fsanitize=address -fno-common -fno-omit-frame-pointer -rdynamic ")
else()
  SET(ASAN_C_FLAGS "")
  SET(ASAN_L_FLAGS "")
endif()

set( MANDIR share/man CACHE PATH "Directory for man pages" )
set( DATADIR share CACHE PATH "Directory for shared data" )
set( SYSCONFDIR /etc CACHE PATH "Directory for configuration files" )
set( DOCDIR share/doc/tint2 CACHE PATH "Directory for documentation files" )

link_directories(
  ${IMLIB2_LIBRARY_DIRS}
  ${RSVG_LIBRARY_DIRS}
  ${SN_LIBRARY_DIRS}
)

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
  ${RSVG_LIBRARIES}
  ${SN_LIBRARIES}
)

configure_file ("${CMAKE_SOURCE_DIR}/include/conf.h.in" "${CMAKE_BINARY_DIR}/include/conf.h")
target_link_libraries( ${PROJECT_NAME} m )

set_target_properties( ${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-Wall -fno-strict-aliasing -pthread ${ASAN_C_FLAGS}" )
set_target_properties( ${PROJECT_NAME} PROPERTIES LINK_FLAGS "-pthread -fno-strict-aliasing ${ASAN_L_FLAGS}" )

install( TARGETS ${PROJECT_NAME} DESTINATION bin )
install( FILES sample/tint2rc DESTINATION ${SYSCONFDIR}/xdg/${PROJECT_NAME} )
install( FILES default_icon.png DESTINATION ${DATADIR}/${PROJECT_NAME} )
install( FILES AUTHORS ChangeLog README DESTINATION ${DOCDIR} )
install( FILES doc/tint2.1 DESTINATION ${MANDIR}/man1 )

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
