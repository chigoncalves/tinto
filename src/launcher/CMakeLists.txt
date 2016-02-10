set (CMAKE_INCLUDE_CURRENT_DIR 1)

list (APPEND SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/launcher.c
  ${CMAKE_CURRENT_SOURCE_DIR}/apps-common.c
  ${CMAKE_CURRENT_SOURCE_DIR}/icon-theme-common.c
  ${CMAKE_CURRENT_SOURCE_DIR}/xsettings-client.c
  ${CMAKE_CURRENT_SOURCE_DIR}/xsettings-common.c
)

set (SOURCES "${SOURCES}" PARENT_SCOPE)
