set (CMAKE_INCLUDE_CURRENT_DIR 1)

list (APPEND SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/launcher.c
  ${CMAKE_CURRENT_SOURCE_DIR}/desktop-entry.c
  ${CMAKE_CURRENT_SOURCE_DIR}/icon-theme-common.c
  ${CMAKE_CURRENT_SOURCE_DIR}/xsettings-client.c
  ${CMAKE_CURRENT_SOURCE_DIR}/xsettings-common.c
)


if (DEBUG)
  add_subdirectory (t)
  list (APPEND tinto_tests_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/desktop-entry.c")
  set (tinto_tests_SOURCES "${tinto_tests_SOURCES}" PARENT_SCOPE)
endif ()

set (SOURCES "${SOURCES}" PARENT_SCOPE)
