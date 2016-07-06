set (CMAKE_INCLUDE_CURRENT_DIR 1)

set (TINTO_CONFIG_SOURCES
  ../util/common.c
  ../util/string-addins.c
  ../util/path-utils.c
  ../config.c
  ../server.c
  ../launcher/desktop-entry.c
  ../launcher/icon-theme-common.c
  main.c
  properties.c
  properties_rw.c
  theme_view.c
)

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
