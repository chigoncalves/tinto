set (CMAKE_INCLUDE_CURRENT_DIR 1)

list (APPEND SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/area.c
  ${CMAKE_CURRENT_SOURCE_DIR}/common.c
  ${CMAKE_CURRENT_SOURCE_DIR}/debug.c
  ${CMAKE_CURRENT_SOURCE_DIR}/misc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/path-utils.c
  ${CMAKE_CURRENT_SOURCE_DIR}/strnatcmp.c
  ${CMAKE_CURRENT_SOURCE_DIR}/timer.c
  ${CMAKE_CURRENT_SOURCE_DIR}/string-addins.c
  ${CMAKE_CURRENT_SOURCE_DIR}/window.c
)

set (SOURCES "${SOURCES}" PARENT_SCOPE)

if (DEBUG)
  add_subdirectory (t)
  list (APPEND tinto_tests_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/debug.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/path-utils.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/string-addins.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/misc.c")
  set (tinto_tests_SOURCES "${tinto_tests_SOURCES}" PARENT_SCOPE)
endif ()
