include (CMakePrintHelpers)

find_package (CUNIT QUIET)

find_package (GTK2 COMPONENTS gtk)

if (NOT CUNIT_FOUND)
  message (FATAL_ERROR "CUnit is required, but it's missing.")
endif ()

target_include_directories (${tinto_panel_test_EXECUTABLE}
			    PUBLIC
			      "${CUNIT_INCLUDE_DIRS}"
			      "${GTK2_INCLUDE_DIRS}")

cmake_print_variables (CUNIT_LIBRARIES GTK2_LIBRARIES)

target_link_libraries (${tinto_panel_test_EXECUTABLE}
		       PUBLIC
			 "${CUNIT_LIBRARIES}"
			 "${GTK2_LIBRARIES}")

add_custom_target (
  run-tests
  COMMAND "${CMAKE_CURRENT_BINARY_DIR}/${tinto_panel_test_EXECUTABLE}"
  DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${tinto_panel_test_EXECUTABLE}"
  USES_TERMINAL)
