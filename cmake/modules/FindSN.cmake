#[[.rst

FindStartupNotification
-----------------------

Finds Startup Notifaction library.

Upon return it sets the following variables:

SN_FOUND
  With a ``true`` value if it manages to find the library.

SN_DEFINITIONS

SN_ROOT_DIR

SN_VERSION_STRING

SN_VERSION_MAJOR

SN_VERSION_MINOR

SN_VERSION_PATCH

SN_INCLUDE_DIRS

SN_LIBRARIES
#]]

include (SelectLibraryConfigurations)
include (FindPackageHandleStandardArgs)

function (_sn_find_include_dir)
  find_path (SN_INCLUDE_DIR
             libsn/sn.h
	     HINT ${SN_ROOT_DIR}
	     ENV SN_ROOT_DIR
	     PATH_SUFFIXES startup-notification
	     startup-notification-1
             startup-notification-1.0)

  if (SN_INCLUDE_DIR)
    set (SN_INCLUDE_DIR ${SN_INCLUDE_DIR} PARENT_SCOPE)
  endif ()
endfunction ()

function (_sn_find_definitions)
  if (SN_INCLUDE_DIR)
    file (STRINGS "${SN_INCLUDE_DIR}/libsn/sn-common.h" _RESULTS REGEX
          "#ifndef[\t ]+[a-bA-Z0-9_-]+")

    set (SN_DEFINITIONS)
    foreach (definition ${_RESULTS})
      string (REPLACE "#ifndef" "" definition "${definition}")
      string (STRIP "${definition}" definition)
      string (REGEX MATCH "^_+|_+$" matched "${definition}")

      if (NOT matched)
	list (APPEND SN_DEFINITIONS "-D${definition}=1")
      endif ()
    endforeach ()
    string (REPLACE ";" " " SN_DEFINITIONS "${SN_DEFINITIONS}")
    set (SN_DEFINITIONS "${SN_DEFINITIONS}" PARENT_SCOPE)
  endif ()
endfunction ()

function (_sn_find_library)
  set (suffixes 1.0 1)
  set (sn_names startup-notification)
  foreach (suffix ${suffixes})
    list (APPEND sn_names startup-notification-${suffix})
  endforeach ()

  find_library (SN_LIBRARY_RELEASE
    ${sn_names}

    HINT ${SN_ROOT_DIR}
    ENV SN_ROOT_DIR
    PATHS ${CMAKE_LIBRARY_ARCHITECTURE})

  if (SN_LIBRARY_RELEASE)
    select_library_configurations (SN)
    set (SN_LIBRARY ${SN_LIBRARY} PARENT_SCOPE)
    set (SN_LIBRARY_RELEASE ${SN_LIBRARY_RELEASE} PARENT_SCOPE)
  endif ()
endfunction ()


##### Entry Point #####
set (SN_FOUND)
set (SN_INCLUDE_DIRS)
set (SN_LIBRARIES)
set (SN_DEFINITIONS)
# StartupNotification does not export versions via macro, there no
# way to find it, so we are setting it manually.
set (SN_VERSION_MAJOR 1)
set (SN_VERSION_MINOR 0)
set (SN_VERSION_PATCH 0)
set (SN_VERSION_STRING "${SN_VERSION_MAJOR}.${SN_VERSION_MINOR}.\
${SN_VERSION_PATCH}")

_sn_find_include_dir ()
if (SN_INCLUDE_DIR)
  _sn_find_definitions ()
endif ()
_sn_find_library ()

find_package_handle_standard_args (SN
                                   FOUND_VAR SN_FOUND
				   REQUIRED_VARS
				   SN_INCLUDE_DIR
				   SN_LIBRARY
				   SN_DEFINITIONS
				   VERSION_VAR
				   SN_VERSION_STRING)

if (SN_FOUND)
  set (SN_INCLUDE_DIRS ${SN_INCLUDE_DIR})
  set (SN_LIBRARIES ${SN_LIBRARY})
endif ()

if (NOT TARGET SN::Library)
  add_library (SN::LIBRARY UNKNOWN IMPORTED)
  set_property (TARGET SN::LIBRARY APPEND PROPERTY
                IMPORTED_CONFIGURATIONS RELEASE)

  set_target_properties (SN::LIBRARY
                         PROPERTIES
			 INTERFACE_LOCATION "${SN_LIBRARY}"
			 INTERFACE_INCLUDE_DIRECTORIES
			 "${SN_INCLUDE_DIR}")

  set_target_properties (SN::LIBRARY PROPERTIES
                         IMPORTED_LOCATION_RELEASE
			 ${SN_LIBRARY_RELEASE})
endif ()

mark_as_advanced (SN_DEFINITIONS
                  SN_VERSION_STRING
		  SN_VERSION_MAJOR
		  SN_VERSION_MINOR
		  SN_VERSION_PATCH
		  SN_FOUND
		  SN_INCLUDE_DIRS
		  SN_INCLUDE_DIR
		  SN_LIBRARIES
		  SN_LIBRARY_RELEASE
		  SN_LIBRARY)
