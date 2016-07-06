set (CMAKE_INCLUDE_CURRENT_DIR 1)

list (APPEND tinto_tests_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/desktop-entry-test.c")

set (tinto_tests_SOURCES "${tinto_tests_SOURCES}" PARENT_SCOPE)
