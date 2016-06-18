set (CMAKE_INCLUDE_CURRENT_DIR 1)

include (FindCUNIT QUIET)

if (NOT CUNIT_FOUND)
  message (FATAL_ERROR "CUnit is required, but it's missing.")
endif ()

include_directories (CUNIT_INCLUDE_DIRS)

add_executable (
  test-string-addins
  ../string-addins.c
  test-string-addins.c
)
set (targets test-string-addins)

add_executable (
  test-dimension
  ../misc.c
  ../string-addins.c
  test-dimension.c
)
list (APPEND targets test-dimension)

add_executable (
  test-path-utils
  ../path-utils.c
  ../string-addins.c
  test-path-utils.c
)
list (APPEND targets test-path-utils)

add_executable (
  test-color32
  ../misc.c
  ../string-addins.c
  ../debug.c
  ../path-utils.c
  test-color32.c
)
list (APPEND targets test-color32)

foreach (target ${targets})
  target_link_libraries (${target} ${CUNIT_LIBRARIES})
endforeach ()


add_custom_target (
  tests
  COMMAND ${CMAKE_BINARY_DIR}/src/util/t/test-string-addins
  COMMAND ${CMAKE_BINARY_DIR}/src/util/t/test-dimension
  COMMAND ${CMAKE_BINARY_DIR}/src/util/t/test-path-utils
  COMMAND ${CMAKE_BINARY_DIR}/src/util/t/test-color32
  DEPENDS test-string-addins
  DEPENDS test-dimension
  DEPENDS test-path-utils
  DEPENDS test-color32
)
