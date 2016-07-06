set (CMAKE_INCLUDE_CURRENT_DIR 1)

list (APPEND tinto_tests_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/string-addins-test.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/dimension-test.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/path-utils-test.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/color32-test.c"
)

set (tinto_tests_SOURCES "${tinto_tests_SOURCES}" PARENT_SCOPE)
