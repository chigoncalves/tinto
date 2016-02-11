set (CMAKE_INCLUDE_CURRENT_DIR 1)

add_executable (
  test-string-addins
  ../string-addins.c
  test-string-addins.c
)

add_executable (
  test-dimension
  ../misc.c
  ../string-addins.c
  test-dimension.c
)

add_executable (
  test-path-utils
  ../path-utils.c
  ../string-addins.c
  test-path-utils.c
)

add_executable (
  test-color32
  ../misc.c
  ../string-addins.c
  ../debug.c
  ../path-utils.c
  test-color32.c
)

target_link_libraries (
  test-string-addins
  cunit
)

target_link_libraries (
  test-dimension
  cunit
)

target_link_libraries (
  test-path-utils
  cunit
)

target_link_libraries (
  test-color32
  cunit
)

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

message (STATUS ${CMAKE_BINARY_DIR})
