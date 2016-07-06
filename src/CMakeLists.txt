set (CMAKE_INCLUDE_CURRENT_DIR 1)

list (APPEND SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/config.c
  ${CMAKE_CURRENT_SOURCE_DIR}/panel.c
  ${CMAKE_CURRENT_SOURCE_DIR}/server.c
  ${CMAKE_CURRENT_SOURCE_DIR}/main.c
  ${CMAKE_CURRENT_SOURCE_DIR}/tinto.c
)

add_subdirectory (clock)
add_subdirectory (launcher)
add_subdirectory (systray)
add_subdirectory (taskbar)
add_subdirectory (tooltip)
add_subdirectory (util)

set (SOURCES "${SOURCES}" PARENT_SCOPE)

if (DEBUG)
  add_subdirectory (t)
  set (tinto_tests_SOURCES "${tinto_tests_SOURCES}" PARENT_SCOPE)
endif ()
