#[[.rst

FindStartupNotification
-----------------------

Finds Startup Notifaction library.

Upon return it sets the following variables:

SN_FOUND
  With a ``true`` value if it manages to find the library.

SN_VERSION
  With the version.

SN_INCLUDE_DIRS

SN_LIBRARIES
#]]

include (SelectLibraryConfigurations)
include (FindPackageHandleStandardArgs)
include (CMakePrintHelpers)

function (__sn_find_include_dir)
  find_path (SN_INCLUDE_DIR
    libsn/sn.h
    PATH_SUFFIXES startup-notification-1.0 startup-notification-1 startup-notification)

  if (SN_INCLUDE_DIR)
    set (SN_INCLUDE_DIR ${SN_INCLUDE_DIR} PARENT_SCOPE)
  else ()
    message (SEND_ERROR "Failed to find header..")
  endif ()
endfunction ()

function (__sn_find_library)
  set (suffixes 1.0 1)
  set (sn_names startup-notification)
  foreach (suffix ${suffixes})
    list (APPEND sn_names startup-notification-${suffix})
  endforeach ()

  find_library (SN_LIBRARY_RELEASE
    ${sn_names}
    PATHS ${CMAKE_LIBRARY_ARCHITECTURE})

  if (SN_LIBRARY_RELEASE)
    select_library_configurations (SN)
    set (SN_LIBRARY ${SN_LIBRARY} PARENT_SCOPE)
    set (SN_LIBRARY_RELEASE ${SN_LIBRARY_RELEASE} PARENT_SCOPE)
    set (SN_LIBRARY_DEBUG ${SN_LIBRARY_DEBUG} PARENT_SCOPE)
    set (SN_LIBRARY_DIR ${SN_LIBRARY_DEBUG} PARENT_SCOPE)
  else ()
    message (SEND_ERROR "Failed to find lib.")
  endif ()
endfunction ()


##### Entry Point #####


set (SN_INCLUDE_DIRS)
set (SN_LIBRARIES)
set (SN_VERSION 1.0)
set (SN_FOUND)

if (SN_FIND_COMPONENTS)
  message (SEND_ERROR "Cannot find components..")
endif ()

if (SN_FIND_VERSION)
  if (SN_FIND_VERSION_EXACT)
    if (NOT SN_FIND_VERSION VERSION_EQUAL SN_VERSION)
      message (SEND_ERROR "Failed to find requested version.")
    endif ()
  else ()
    if (SN_FIND_VERSION VERSION_GREATER SN_VERSION)
      message (SEND_ERROR "Failed inferior.")
    endif ()
  endif ()
endif ()

__sn_find_include_dir ()
__sn_find_library ()
set (SN_INCLUDE_DIRS ${SN_INCLUDE_DIR})
set (SN_LIBRARIES ${SN_LIBRARY})

select_library_configurations (SN
  FOUND_VAR SN_FOUND
  REQUIRED_VARS SN_INCLUDE_DIRS SN_LIBRARIESA
  VERSION_VAR SN_VERSION
)
