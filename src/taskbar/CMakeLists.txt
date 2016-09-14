target_include_directories (${tinto_panel_EXECUTABLE}
			    PUBLIC
			      "${CMAKE_CURRENT_LIST_DIR}")

target_sources (${tinto_panel_EXECUTABLE}
                PUBLIC
		  "${CMAKE_CURRENT_SOURCE_DIR}/task.c"
		  "${CMAKE_CURRENT_SOURCE_DIR}/taskbar.c"
		  "${CMAKE_CURRENT_SOURCE_DIR}/taskbarname.c")
