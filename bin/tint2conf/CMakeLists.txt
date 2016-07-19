set (CMAKE_INCLUDE_CURRENT_DIR 1)

set (TINTO_CONFIG_SOURCES
  "${CMAKE_SOURCE_DIR}/src/util/common.c"
  "${CMAKE_SOURCE_DIR}/src/util/string-addins.c"
  "${CMAKE_SOURCE_DIR}/src/util/path-utils.c"
  "${CMAKE_SOURCE_DIR}/src/config.c"
  "${CMAKE_SOURCE_DIR}/src/server.c"
  "${CMAKE_SOURCE_DIR}/src/launcher/desktop-entry.c"
  "${CMAKE_SOURCE_DIR}/src/launcher/icon-theme-common.c"
  "${CMAKE_CURRENT_LIST_DIR}/main.c"
  "${CMAKE_CURRENT_LIST_DIR}/properties.c"
  "${CMAKE_CURRENT_LIST_DIR}/properties_rw.c"
  "${CMAKE_CURRENT_LIST_DIR}/theme_view.c")

add_executable (${TINTO_CONFIG_BIN} ${TINTO_CONFIG_SOURCES})

target_link_libraries (${TINTO_CONFIG_BIN}
  ${GTK2_LIBRARIES}
  ${IMLIB2_LIBRARIES}
  ${RSVG_LIBRARIES}
  ${X11_X11_LIB}
  ${X11_Xdamage_LIB}
  ${X11_Xcomposite_LIB}
  ${X11_Xinerama_LIB}
  ${X11_Xrandr_LIB}
  ${X11_Xrender_LIB}
)

if ( NOT DATADIR )
  set( DATADIR share )
endif( NOT DATADIR )

set_target_properties (${TINTO_CONFIG_BIN} PROPERTIES COMPILE_FLAGS "-Wall" )
set_target_properties (${TINTO_CONFIG_BIN} PROPERTIES LINK_FLAGS "-pthread" )

install (TARGETS ${TINTO_CONFIG_BIN} DESTINATION bin)
install (PROGRAMS ${tinto_IN_SOURCE_BIN_DIR}/tintwizard.py DESTINATION bin)
install (FILES ${tinto_IN_SOURCE_RES_DIR}/taskbar.svg DESTINATION ${DATADIR}/icons/hicolor/scalable/apps )
install (FILES ${tinto_IN_SOURCE_RES_DIR}/tint2conf.desktop DESTINATION ${DATADIR}/applications)
# install( CODE "execute_process(COMMAND gtk-update-icon-cache -f -t ${DATADIR}/icons/hicolor WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX})" )
