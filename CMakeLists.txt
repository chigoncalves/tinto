if (${CMAKE_CURRENT_BINARY_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  message (FATAL_ERROR [[In source build are not allowed!
Run cmake this way: `cmake <source-dir> -B<build-dir>.']])
endif ()

cmake_minimum_required (VERSION 3.0)
project (tinto-panel VERSION 0.0.2.5 LANGUAGES C)
cmake_policy (SET CMP0058 NEW)

list (APPEND CMAKE_MODULE_PATH
	     "${CMAKE_SOURCE_DIR}/cmake"
	     "${CMAKE_BINARY_DIR}/cmake"
	     "${CMAKE_SOURCE_DIR}/vendor/cmake/cmake/modules")

include (Utils)

set (tinto_panel_EXECUTABLE tinto-panel)
set (USER_HOME_DIR "$ENV{HOME}")

set (tinto_panel_IN_SOURCE_RES_DIR "${CMAKE_HOME_DIRECTORY}/res")
set (tinto_panel_IN_SOURCE_BIN_DIR "${CMAKE_HOME_DIRECTORY}/bin")
set (tinto_panel_IN_SOURCE_DOC_DIR "${CMAKE_HOME_DIRECTORY}/doc")

set (MANDIR share/man CACHE PATH "Directory for man pages")
set (DATADIR share CACHE PATH "Directory for shared data")
set (SYSCONFDIR etc CACHE PATH "Directory for configuration files")
set (DOCDIR share/doc/tinto CACHE PATH "Directory for documentation"
     " files")

option (ENABLE_BATTERY "Enable battery status plugin." ON)
option (ENABLE_TINT2CONF "Enable tint2conf build, a Gtk+2 theme \
configurator for `tinto`." OFF)
option (ENABLE_EXAMPLES "Install additional tin2rc examples" OFF)
option (ENABLE_RSVG "Enable `rsvg` support for launcher icons." ON)
option (ENABLE_SN "Enable `startup notification` support." ON)

add_executable (${tinto_panel_EXECUTABLE}
                "${CMAKE_CURRENT_LIST_DIR}/src/main.c")

find_package (X11 COMPONENTS
		    Xdamage
		    Xrandr
		    Xrender
		    Xcomposite
		    Xinerama
	          RQUIRED)
if (NOT X11_FOUND)
  message (FATAL_ERROR "tinto-panel: X11 library not found!")
endif ()

find_package (GTK2 2.8 REQUIRED COMPONENTS gtk)
if (NOT GTK2_FOUND)
  message (FATAL_ERROR "tinto-panel: Gtk+2 library not found!")
endif ()

include (FindPkgConfig)

find_package (Imlib2 REQUIRED)
if (NOT Imlib2_FOUND)
  message (FATAL_ERROR "tinto-panel: Imlib2 library not found!")
endif ()

if (ENABLE_RSVG)
  pkg_check_modules (RSVG librsvg-2.0>=2.14.0)
  if (RSVG_FOUND)
    include_directories ("${RSVG_INCLUDE_DIRS}")
    target_link_libraries (${tinto_panel_EXECUTABLE}
                       PUBLIC
"${RSVG_LIBRARIES}")

    set (HAS_RSVG 1)
    set (HAVE_RSVG 1) # NOTE: Update-me in `tint2conf'
  else ()
    message (FATAL_ERROR "tinto-panel: RSVG library not found!")
  endif ()
endif ()

if (ENABLE_SN)
  find_package (SN)
  if (NOT SN_FOUND)
    message (FATAL_ERROR "tinto-panel: Sartup Notification library not found!")
  endif ()
    target_include_directories (${tinto_panel_EXECUTABLE}
                                PUBLIC
				  "${SN_INCLUDE_DIRS}")
    target_link_libraries (${tinto_panel_EXECUTABLE}
			   PUBLIC
			     ${SN_LIBRARIES})
    add_definitions (${SN_DEFINITIONS})
    set (HAS_SN 1)
endif ()

if (ENABLE_TINT2CONF)
  set (TINT2CONF 1)
  set (TINTO_CONFIG_BIN tint2conf)
  add_subdirectory (bin/tint2conf)
endif ()

target_include_directories (${tinto_panel_EXECUTABLE}
                            PUBLIC
			      "${GTK2_INCLUDE_DIRS}"
			      "${X11_X11_INCLUDE_PATH}"
			      ${Imlib2_INCLUDE_DIRECTORIES}
)

target_link_libraries (${tinto_panel_EXECUTABLE}
		       PUBLIC
			 "${X11_X11_LIB}"
			 "${X11_Xcomposite_LIB}"
			 "${X11_Xdamage_LIB}"
			 "${X11_Xinerama_LIB}"
			 "${X11_Xrandr_LIB}"
			 "${X11_Xrender_LIB}"
			 "${GTK2_LIBRARIES}"
			 "${Imlib2_LIBRARIES}"
			 m
)

# if (DEBUG)
#   set (tinto_panel_test_EXECUTABLE "tinto-panel-tests"
#                                    CACHE
# 				   INTERNAL
# 				   "Name for the executable for tests.")

#   add_executable (${tinto_panel_test_EXECUTABLE}
#                   "${CMAKE_CURRENT_LIST_DIR}/t/main.c")
#   add_subdirectory (t)
# endif ()

add_subdirectory (src)

configure_file ("${CMAKE_SOURCE_DIR}/src/conf.h.in"
                "${CMAKE_BINARY_DIR}/src/conf.h")
configure_file ("${CMAKE_SOURCE_DIR}/cmake/deinstall.cmake.in"
                "${CMAKE_BINARY_DIR}/cmake/deinstall.cmake")

install (TARGETS ${tinto_panel_EXECUTABLE} DESTINATION bin)
# Fix me: incorrect dest path.
install (FILES "${tinto_panel_IN_SOURCE_RES_DIR}/config-sample/tinto.conf"
         DESTINATION xdg/${PROJECT_NAME})
install (FILES "${tinto_panel_IN_SOURCE_RES_DIR}/icon.png" DESTINATION
         "${DATADIR}/${PROJECT_NAME}")
install (FILES ChangeLog readme.rst DESTINATION "${DOCDIR}")
install (FILES "${tinto_IN_SOURCE_DOC_DIR}/man/tinto.1"
         DESTINATION "${MANDIR}/man1")

add_custom_target (deinstall COMMAND ${CMAKE_COMMAND} -P
				     "${CMAKE_BINARY_DIR}/cmake/deinstall.cmake")

if (ENABLE_EXAMPLES)
  file (GLOB SAMPLEFILES
        "${tinto_panel_IN_SOURCE_RES_DIR}/config-sample/*.conf")
  install (FILES ${SAMPLEFILES} DESTINATION "${DOCDIR}/examples")
endif ()

mark_as_advanced (tinto_panel_IN_SOURCE_RES_DIR
		  tinto_panel_IN_SOURCE_BIN_DIR
		  tinto_panel_IN_SOURCE_DOC_DIR)
