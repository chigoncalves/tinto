target_include_directories (${tinto_panel_EXECUTABLE} PUBLIC
                            "${CMAKE_CURRENT_LIST_DIR}"
			    "${CMAKE_CURRENT_BINARY_DIR}")



target_sources (${tinto_panel_EXECUTABLE}
                PUBLIC
		  "${CMAKE_CURRENT_SOURCE_DIR}/config.c"
		  "${CMAKE_CURRENT_SOURCE_DIR}/panel.c"
		  "${CMAKE_CURRENT_SOURCE_DIR}/server.c"
		  "${CMAKE_CURRENT_SOURCE_DIR}/tinto.c")

add_subdirectory (clock)
add_subdirectory (launcher)
add_subdirectory (systray)
add_subdirectory (taskbar)
add_subdirectory (tooltip)
add_subdirectory (util)

if (ENABLE_BATTERY)
  add_subdirectory (battery)
endif ()

# if (DEBUG)
#   add_subdirectory (t)
# target_include_directories (${tinto_panel_test_EXECUTABLE}
#                             PUBLIC
# 			      "${CMAKE_CURRENT_BINARY_DIR}")

# endif ()
