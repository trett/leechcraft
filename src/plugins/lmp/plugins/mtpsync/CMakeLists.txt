cmake_minimum_required (VERSION 2.8)
project (leechcraft_lmp_mtpsync)
include (InitLCPlugin OPTIONAL)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package (MTP REQUIRED)

set (QT_USE_QTNETWORK TRUE)
set (QT_USE_QTXML TRUE)
include (${QT_USE_FILE})
include_directories (${LMP_INCLUDE_DIR}
	${MTP_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

set (MTPSYNC_SRCS
	mtpsync.cpp
	)

add_definitions (${MTP_DEFINITIONS})

add_library (leechcraft_lmp_mtpsync SHARED
	${MTPSYNC_COMPILED_TRANSLATIONS}
	${MTPSYNC_SRCS}
	)
target_link_libraries (leechcraft_lmp_mtpsync
	${QT_LIBRARIES}
	${MTP_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)

install (TARGETS leechcraft_lmp_mtpsync DESTINATION ${LC_PLUGINS_DEST})
#install (FILES lmpmtpsyncsettings.xml DESTINATION ${LC_SETTINGS_DEST})
