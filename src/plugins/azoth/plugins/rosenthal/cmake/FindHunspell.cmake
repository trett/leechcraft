# - Try to find HUNSPELL
# Once done this will define
#
#  HUNSPELL_FOUND - system has HUNSPELL
#  HUNSPELL_INCLUDE_DIR - the HUNSPELL include directory
#  HUNSPELL_LIBRARIES - The libraries needed to use HUNSPELL
#  HUNSPELL_DEFINITIONS - Compiler switches required for using HUNSPELL


IF (HUNSPELL_INCLUDE_DIR AND HUNSPELL_LIBRARIES)
  # Already in cache, be silent
  SET(HUNSPELL_FIND_QUIETLY TRUE)
ENDIF (HUNSPELL_INCLUDE_DIR AND HUNSPELL_LIBRARIES)

INCLUDE(FindPackageHandleStandardArgs)

IF (WIN32)
	IF (MSVC)
		#MSVS 2010
		IF (MSVC_VERSION LESS 1600)
			MESSAGE(FATAL_ERROR "We currently support only MSVC 2010 or greater version")
		ENDIF (MSVC_VERSION LESS 1600)
	ENDIF (MSVC)
	IF (NOT DEFINED HUNSPELL_DIR)
		IF (HUNSPELL_FIND_REQUIRED)
			MESSAGE(FATAL_ERROR "Please set HUNSPELL_DIR variable")
		ELSE (HUNSPELL_FIND_REQUIRED)
			MESSAGE(STATUS "Please set HUNSPELL_DIR variable for HUNSPELL support")
		ENDIF (HUNSPELL_FIND_REQUIRED)
	ENDIF (NOT DEFINED HUNSPELL_DIR)
	SET (HUNSPELL_INCLUDE_WIN32 ${HUNSPELL_DIR}/src/)

	SET (PROBE_DIR
		${HUNSPELL_DIR}/src/win_api/Release_dll/libhunspell)

	FIND_LIBRARY (HUNSPELL_LIBRARIES NAMES libhunspell.lib PATHS ${PROBE_DIR})
ELSE (WIN32)
	FIND_LIBRARY (HUNSPELL_LIBRARIES NAMES hunspell-1.3 hunspell-1.2 HINTS ${HUNSPELL_DIR})
ENDIF (WIN32)

FIND_PATH (HUNSPELL_INCLUDE_DIR hunspell/hunspell.hxx HINTS ${HUNSPELL_DIR} ${HUNSPELL_INCLUDE_WIN32})


# handle the QUIETLY and REQUIRED arguments and set HUNSPELL_FOUND to TRUE if
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS (HUNSPELL DEFAULT_MSG HUNSPELL_LIBRARIES HUNSPELL_INCLUDE_DIR)

if (HUNSPELL_FOUND)
  if (NOT HUNSPELL_FIND_QUIETLY AND HUNSPELLCONFIG_EXECUTABLE)
    message(STATUS "Hunspell found: ${HUNSPELL_LIBRARIES}")
  endif(NOT HUNSPELL_FIND_QUIETLY AND HUNSPELLCONFIG_EXECUTABLE)
else(HUNSPELL_FOUND)
  if(HUNSPELL_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find Hunspell")
  endif(HUNSPELL_FIND_REQUIRED)
endif(HUNSPELL_FOUND)

MARK_AS_ADVANCED(HUNSPELL_INCLUDE_DIR HUNSPELL_LIBRARIES)
