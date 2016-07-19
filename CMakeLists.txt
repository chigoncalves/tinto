if (${CMAKE_CURRENT_BINARY_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  message (FATAL_ERROR "In source build are not allowed!\n"
           "Run cmake this way: `cmake <source-dir> -B<build-dir>.'")
endif ()

cmake_minimum_required (VERSION 3.0)
project (tinto VERSION 0.0.2.2 LANGUAGES C )

list (APPEND CMAKE_MODULE_PATH
  "${CMAKE_SOURCE_DIR}/cmake"
  "${CMAKE_BINARY_DIR}/cmake"
  "${CMAKE_SOURCE_DIR}/cmake/modules"
)

include_directories (
  "${CMAKE_BINARY_DIR}/src"
  # ${CMAKE_SOURCE_DIR}/include
)

include (Utils)

# NOTE: Move me to Utils.cmake.
set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fPIE")
     #-Wall -Wextra -Werror -std=c99 -pedantic -fPIE")
# NOTE: Move me to Utils.cmake.
set (CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG}
     -fno-omit-frame-pointer -pie")
# NOTE: Move me to Utils.cmake.
set (CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=address -fsanitize=undefined")

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
  tinto_IN_SOURCE_DOC_DIR
)

find_package (X11)
if (NOT X11_Xdamage_FOUND OR NOT X11_Xrandr_FOUND
    OR NOT X11_Xrender_FOUND OR NOT X11_Xinerama_FOUND
    OR NOT X11_Xcomposite_FOUND)
  message (FATAL_ERROR "Could not find all X11 libraries!")
endif ()

include_directories (
  "${X11_Xrender_INCLUDE_PATH}"
  "${X11_Xrandr_INCLUDE_PATH}"
  "${X11_Xinerama_INCLUDE_PATH}"
  "${X11_Xdamage_INCLUDE_PATH}"
  "${X11_Xcomposite_INCLUDE_PATH}"
  "${X11_X11_INCLUDE_PATH}"
)

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
  src/util
)

if (DEBUG)
  set (tinto_tests_SOURCES)
endif ()

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
    message (FATAL_ERROR "SVG support enabled yet dependency not"
             " fulfilled: librsvg-2.0" )
  endif ()
endif ()

if (ENABLE_SN)
  find_package (SN)
  if (NOT SN_FOUND)
    message (FATAL_ERROR "Startup Notification is required.")
  endif ()
    include_directories ("${SN_INCLUDE_DIRS}")
    link_libraries ("${SN_LIBRARIES}")
    add_definitions ("${SN_DEFINITIONS}")
    set (HAS_SN 1)
endif ()

if (ENABLE_TINT2CONF)
  set (TINT2CONF 1)
  set (TINTO_CONFIG_BIN tint2conf)
  add_subdirectory (src/tint2conf)
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
  m
)

set (USER_HOME_DIR "$ENV{HOME}")
configure_file ("${CMAKE_SOURCE_DIR}/src/conf.h.in"
                "${CMAKE_BINARY_DIR}/src/conf.h")
configure_file ("${CMAKE_SOURCE_DIR}/cmake/deinstall.cmake.in"
                "${CMAKE_BINARY_DIR}/cmake/deinstall.cmake")

# NOTE: Move me to Utils.cmake.
set_target_properties (${PROJECT_NAME} PROPERTIES COMPILE_FLAGS
                       "-Wall -fno-strict-aliasing -pthread ${ASAN_C_FLAGS}")
# NOTE: Move me to Utils.cmake.
set_target_properties (${PROJECT_NAME} PROPERTIES LINK_FLAGS
                       "-pthread -fno-strict-aliasing ${ASAN_L_FLAGS}")

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
          "${CMAKE_BINARY_DIR}/cmake/deinstall.cmake"
)

# NOTE: Move me to Utils.cmake.
if (DEBUG)
  add_custom_target (lint COMMAND oclint-json-compilation-database
                     $ENV{OCLINT_EXTRA_OPTIONS})
endif ()

if (ENABLE_EXAMPLES)
  file (GLOB SAMPLEFILES
        "${tinto_IN_SOURCE_RES_DIR}/config-sample/*.conf")
  install (FILES ${SAMPLEFILES} DESTINATION "${DOCDIR}/examples")
endif ()

set (CPACK_GENERATOR "TXZ")
set (CPACK_STRIP_FILES 1)
set (CPACK_PACKAGE_CONTACT "chigoncalves <eatg75@gmail.com")
set (CPACK_PACKAGE_DESCRIPTION "A tiny panel forx X11. (tint2 fork.)")
set (CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/readme.rst")
include (CPack)
