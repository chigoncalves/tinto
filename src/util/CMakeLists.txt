target_include_directories (${tinto_panel_EXECUTABLE}
			    PUBLIC
			      "${CMAKE_CURRENT_LIST_DIR}")

target_sources (${tinto_panel_EXECUTABLE}
                PUBLIC
		  "${CMAKE_CURRENT_SOURCE_DIR}/area.c"
		  "${CMAKE_CURRENT_SOURCE_DIR}/common.c"
		  "${CMAKE_CURRENT_SOURCE_DIR}/debug.c"
		  "${CMAKE_CURRENT_SOURCE_DIR}/misc.c"
		  "${CMAKE_CURRENT_SOURCE_DIR}/path-utils.c"
		  "${CMAKE_CURRENT_SOURCE_DIR}/strnatcmp.c"
		  "${CMAKE_CURRENT_SOURCE_DIR}/timer.c"
		  "${CMAKE_CURRENT_SOURCE_DIR}/string-addins.c"
		  "${CMAKE_CURRENT_SOURCE_DIR}/window.c")
