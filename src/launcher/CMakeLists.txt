target_include_directories (${tinto_panel_EXECUTABLE}
			    PUBLIC
			      "${CMAKE_CURRENT_LIST_DIR}")

target_sources (${tinto_panel_EXECUTABLE}
                PUBLIC
		  "${CMAKE_CURRENT_SOURCE_DIR}/launcher.c"
		  "${CMAKE_CURRENT_SOURCE_DIR}/desktop-entry.c"
		  "${CMAKE_CURRENT_SOURCE_DIR}/icon-theme-common.c"
		  "${CMAKE_CURRENT_SOURCE_DIR}/xsettings-client.c"
		  "${CMAKE_CURRENT_SOURCE_DIR}/xsettings-common.c")

# if (DEBUG)
#   add_subdirectory (t)
# endif ()
