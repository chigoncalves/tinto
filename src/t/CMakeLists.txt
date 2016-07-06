set (CMAKE_INCLUDE_CURRENT_DIR 1)

find_package (CUNIT QUIET)

find_package (GTK2 COMPONENTS gtk)

if (NOT CUNIT_FOUND)
  message (FATAL_ERROR "CUnit is required, but it's missing.")
endif ()

list (APPEND tinto_tests_SOURCES main.c)
set (tinto_tests_SOURCES "${tinto_tests_SOURCES}" PARENT_SCOPE)

include_directories (
  "${CUNIT_INCLUDE_DIRS}"
  "${GTK2_INCLUDE_DIRS}"
  )
link_libraries (
  "${CUNIT_LIBRARIES}"
  "${GTK2_LIBRARIES}"
)

# include (CMakePrintHelpers)
# cmake_print_variables (tinto_tests_SOURCES)

add_executable (tinto-tests "${tinto_tests_SOURCES}")

add_custom_target (
  run-tests
  COMMAND ${CMAKE_BINARY_DIR}/src/t/tinto-tests
)
