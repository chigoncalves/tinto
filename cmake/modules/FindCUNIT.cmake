#[[.rst

FindCUnit
---------

This module finds the package CUnit.

Usage to use it just issue the following command:

.. code-block:: cmake

   find_package (CUNIT)

Upon ``find_package ()`` return the following variables will be set.

CUNIT_FOUND
  Will have a *true* value if this module manages to find CUnit.

CUNIT_INCLUDE_DIRS
  Will have a list of include directories.

CUNIT_LIBRARIES
  Will have a list of libraries needed by CUnit.

CUNIT_VERSION
  Wil have the version of the library found.

#]]

include (CMakePrintHelpers)
include (SelectLibraryConfigurations)
include (FindPackageHandleStandardArgs)

set (CUNIT_FOUND)
set (CUNIT_INCLUDE_DIRS)
set (CUNIT_LIBRARIES)

if (CUNIT_FIND_COMPONENTS)
  message (SEND_ERROR "FindCUNIT: This module does not have any components. \
Fix the invocation of `find_package ()'.")
endif ()

find_path (CUNIT_INCLUDE_DIR
  CUnit.h

  PATH_SUFFIXES CUnit CUnit-2.0 cunit cunit-2.0)

if (NOT CUNIT_INCLUDE_DIR)
  message (FATAL_ERROR "FindCUNIT: Failed to find CUnit.")
else ()
  if (NOT CUNIT_FIND_QUIETLY)
    message (STATUS "FindCUNIT: Found CUnit header in `${CUNIT_INCLUDE_DIR}'.")
  endif ()
endif ()

find_library (CUNIT_LIBRARY_RELEASE
  cunit
  PATHS /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}
)

if (NOT CUNIT_LIBRARY_RELEASE)
  message (FATAL_ERROR "Failed to find CUnit.")
else ()
  if (NOT CUNIT_FIND_QUIETLY)
    get_filename_component (library_path ${CUNIT_LIBRARY_RELEASE} DIRECTORY)
    message (STATUS "FindCUNIT: Found CUnit library in `${library_path}'.")
    unset (library_path)
  endif ()
endif ()

select_library_configurations (CUNIT)

set (header_file "${CUNIT_INCLUDE_DIR}/CUnit.h")
file (STRINGS ${header_file} version_macro REGEX "#define[ \t]CU_VERSION")
string (STRIP ${version_macro} version_macro)
string (REGEX REPLACE "\"" "" version_macro ${version_macro})
string (REGEX REPLACE "-" "." version_macro ${version_macro})

string (REGEX MATCH "[0-9]+\\.[0-9]+[.-][0-9]" CUNIT_VERSION ${version_macro})
unset (version_macro)

if (CUNIT_FIND_VERSION AND CUNIT_FIND_VERSION_EXACT)
  if (NOT CUNIT_VERSION VERSION_EQUAL CUNIT_FIND_VERSION)
    message (FATAL_ERROR "FindCUNIT failed! Requested version `v${CUNIT_FIND_VERSION}', but found `v${CUNIT_VERSION}'.")
  endif ()
endif ()

if (CUNIT_FOUND AND NOT TARGET CUNIT::LIBRARY)
  add_library (CUNIT::LIBRARY UNKNOWN IMPORTED)
  set_target_properties (CUNIT::LIBRARY
    PROPERTIES
      INTERFACE_LOCATION "${CUNIT_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${CUNIT_INCLUDE_DIR}")
endif ()

list (APPEND CUNIT_INCLUDE_DIRS ${CUNIT_INCLUDE_DIR})
find_package_handle_standard_args (cunit
  FOUND_VAR CUNIT_FOUND
  REQUIRED_VARS CUNIT_LIBRARIES CUNIT_INCLUDE_DIRS
  VERSION_VAR CUNIT_VERSION
)

mark_as_advanced (
  CUNIT_LIBRARIES
  CUNIT_LIBRARY
  CUNIT_LIBRARY_RELEASE
  CUNIT_LIBRARY_DEBUG
  CUNIT_INCLUDE_DIR
  CUNIT_INCLUDE_DIRS
  CUNIT_VERSION
)
