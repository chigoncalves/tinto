if (${CMAKE_CURRENT_BINARY_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  message (FATAL_ERROR "In source build are not allowed!\n"
           "Run cmake this way: `cmake <source-dir> -B<build-dir>.'")
endif ()

cmake_minimum_required (VERSION 3.0)
project (tinto VERSION 0.0.2.3 LANGUAGES C )
cmake_policy (SET CMP0058 NEW)

list (APPEND CMAKE_MODULE_PATH
  "${CMAKE_SOURCE_DIR}/cmake"
  "${CMAKE_BINARY_DIR}/cmake"
  "${CMAKE_SOURCE_DIR}/cmake/modules")

include_directories ("${CMAKE_BINARY_DIR}/src")

include (Utils)

option (ENABLE_BATTERY "Enable battery status plugin." ON)
option (ENABLE_TINT2CONF "Enable tint2conf build, a Gtk+2 theme \
configurator for `tinto`." OFF)
option (ENABLE_EXAMPLES "Install additional tin2rc examples" OFF)
option (ENABLE_RSVG "Enable `rsvg` support for launcher icons." ON)
option (ENABLE_SN "Enable `startup notification` support." ON)

set (tinto_IN_SOURCE_RES_DIR "${CMAKE_HOME_DIRECTORY}/res")
set (tinto_IN_SOURCE_BIN_DIR "${CMAKE_HOME_DIRECTORY}/bin")
set (tinto_IN_SOURCE_DOC_DIR "${CMAKE_HOME_DIRECTORY}/doc")

mark_as_advanced (
  tinto_IN_SOURCE_RES_DIR
  tinto_IN_SOURCE_BIN_DIR
  tinto_IN_SOURCE_DOC_DIR)

find_package (X11 COMPONENTS Xdamage Xrandr Xrender Xcomposite
                             Xinerama
	          RQUIRED)
if (NOT X11_FOUND)
  message (FATAL_ERROR "Could not find X11 libraries!")
endif ()

include_directories ("${X11_X11_INCLUDE_PATH}")

find_package (GTK2 2.8 REQUIRED COMPONENTS gtk)
if (NOT GTK2_FOUND)
  message (FATAL_ERROR "Gtk+2 is required to build tinto.")
endif ()

include_directories ("${GTK2_INCLUDE_DIRS}")

include (FindPkgConfig)
include (CheckLibraryExists)
pkg_check_modules (IMLIB2 REQUIRED imlib2>=1.4.2)

string (REPLACE ";" " " FLAGS_REPLACED "${IMLIB2_LDFLAGS}")
set (CMAKE_REQUIRED_FLAGS "${FLAGS_REPLACED}")
check_library_exists ("${IMLIB2_LIBRARIES}"
                      "imlib_context_set_display"
                      "${IMLIB2_LIBRARY_DIRS}" IMLIB_BUILD_WITH_X)
if (NOT IMLIB_BUILD_WITH_X)
  message (FATAL_ERROR "Imlib is not built with X support")
endif ()

include_directories (
  src
  src/clock
  src/systray
  src/taskbar
  src/launcher
  src/launcher/t
  src/util/t
  src/tooltip
  src/util)

add_subdirectory (src)

if (ENABLE_BATTERY)
  add_subdirectory (src/battery)
  include_directories (src/battery)
endif ()

if (ENABLE_RSVG)
  pkg_check_modules (RSVG librsvg-2.0>=2.14.0)
  if (RSVG_FOUND)
    include_directories ("${RSVG_INCLUDE_DIRS}")
    link_libraries ("${RSVG_LIBRARIES}")
    link_directories ("${RSVG_LIBRARY_DIRS}")
    set (HAS_RSVG 1)
    set (HAVE_RSVG 1) # NOTE: Update-me in `tint2conf'
  else ()
    message (FATAL_ERROR "Could not find RSVG.")
  endif ()
endif ()

if (ENABLE_SN)
  find_package (SN)
  if (NOT SN_FOUND)
    message (FATAL_ERROR "Could not find Startup Notification.")
  endif ()
    include_directories ("${SN_INCLUDE_DIRS}")
    link_libraries ("${SN_LIBRARIES}")
    add_definitions ("${SN_DEFINITIONS}")
    set (HAS_SN 1)
endif ()

if (ENABLE_TINT2CONF)
  set (TINT2CONF 1)
  set (TINTO_CONFIG_BIN tint2conf)
  add_subdirectory (bin/tint2conf)
endif ()

set (MANDIR share/man CACHE PATH "Directory for man pages")
set (DATADIR share CACHE PATH "Directory for shared data")
set (SYSCONFDIR etc CACHE PATH "Directory for configuration files")
set (DOCDIR share/doc/tinto CACHE PATH "Directory for documentation"
     " files")

link_directories ("${IMLIB2_LIBRARY_DIRS}")

add_executable (${PROJECT_NAME} ${SOURCES})

target_link_libraries (${PROJECT_NAME}
  "${X11_X11_LIB}"
  "${X11_Xcomposite_LIB}"
  "${X11_Xdamage_LIB}"
  "${X11_Xinerama_LIB}"
  "${X11_Xrandr_LIB}"
  "${X11_Xrender_LIB}"
  "${GTK2_LIBRARIES}"
  "${IMLIB2_LIBRARIES}"
  m)

set (USER_HOME_DIR "$ENV{HOME}")
configure_file ("${CMAKE_SOURCE_DIR}/src/conf.h.in"
                "${CMAKE_BINARY_DIR}/src/conf.h")
configure_file ("${CMAKE_SOURCE_DIR}/cmake/deinstall.cmake.in"
                "${CMAKE_BINARY_DIR}/cmake/deinstall.cmake")

install (TARGETS ${PROJECT_NAME} DESTINATION bin)
# Fix me: incorrect dest path.
install (FILES "${tinto_IN_SOURCE_RES_DIR}/config-sample/tinto.conf"
         DESTINATION xdg/${PROJECT_NAME})
install (FILES "${tinto_IN_SOURCE_RES_DIR}/icon.png" DESTINATION
         "${DATADIR}/${PROJECT_NAME}")
install (FILES ChangeLog readme.rst DESTINATION "${DOCDIR}")
install (FILES "${tinto_IN_SOURCE_DOC_DIR}/man/tinto.1" DESTINATION
         "${MANDIR}/man1")

add_custom_target (deinstall
  COMMAND ${CMAKE_COMMAND} -P
          "${CMAKE_BINARY_DIR}/cmake/deinstall.cmake")

if (ENABLE_EXAMPLES)
  file (GLOB SAMPLEFILES
        "${tinto_IN_SOURCE_RES_DIR}/config-sample/*.conf")
  install (FILES ${SAMPLEFILES} DESTINATION "${DOCDIR}/examples")
endif ()
