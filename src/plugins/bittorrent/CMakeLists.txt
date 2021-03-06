cmake_minimum_required (VERSION 2.8)
project (leechcraft_bittorrent)
include (InitLCPlugin OPTIONAL)

find_package (Boost REQUIRED COMPONENTS date_time filesystem system thread)

set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package (RBTorrent)
if (NOT RBTorrent_FOUND)
	message (ERROR "Rasterbar libtorrent not found, not building BitTorrent")
endif ()

set (QT_USE_QTXML TRUE)
include (${QT_USE_FILE})
include_directories (${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${Boost_INCLUDE_DIR}
	${RBTorrent_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

if (WIN32)
	include_directories (${RBTorrent_INCLUDE_DIR}/../zlib)
endif ()

set (SRCS
	torrentplugin.cpp
	tabwidget.cpp
	core.cpp
	addtorrent.cpp
	newtorrentwizard.cpp
	intropage.cpp
	firststep.cpp
	secondstep.cpp
	thirdstep.cpp
	addmultipletorrents.cpp
	pieceswidget.cpp
	xmlsettingsmanager.cpp
	piecesmodel.cpp
	torrentfilesmodel.cpp
	peersmodel.cpp
	filesviewdelegate.cpp
	movetorrentfiles.cpp
	trackerschanger.cpp
	peerstablinker.cpp
	addpeerdialog.cpp
	ipvalidators.cpp
	addwebseeddialog.cpp
	wizardgenerator.cpp
	startupfirstpage.cpp
	fastspeedcontrolwidget.cpp
	banpeersdialog.cpp
	ipfilterdialog.cpp
	livestreammanager.cpp
	livestreamdevice.cpp
	speedselectoraction.cpp
	torrentmaker.cpp
	singletrackerchanger.cpp
	torrenttab.cpp
	torrenttabwidget.cpp
	tabviewproxymodel.cpp
	notifymanager.cpp
	)
set (HEADERS
	torrentplugin.h
	tabwidget.h
	core.h
	addtorrent.h
	newtorrentwizard.h
	intropage.h
	firststep.h
	secondstep.h
	thirdstep.h
	addmultipletorrents.h
	pieceswidget.h
	xmlsettingsmanager.h
	piecesmodel.h
	torrentfilesmodel.h
	peersmodel.h
	filesviewdelegate.h
	movetorrentfiles.h
	trackerschanger.h
	peerstablinker.h
	addpeerdialog.h
	ipvalidators.h
	addwebseeddialog.h
	wizardgenerator.h
	startupfirstpage.h
	fastspeedcontrolwidget.h
	banpeersdialog.h
	ipfilterdialog.h
	livestreammanager.h
	livestreamdevice.h
	speedselectoraction.h
	torrentmaker.h
	singletrackerchanger.h
	torrenttab.h
	torrenttabwidget.h
	tabviewproxymodel.h
	notifymanager.h
	newtorrentparams.h
	torrentinfo.h
	fileinfo.h
	peerinfo.h
	)
set (FORMS
    tabwidget.ui
	addtorrent.ui
	addmultipletorrents.ui
	newtorrentfirststep.ui
	newtorrentsecondstep.ui
	newtorrentthirdstep.ui
	movetorrentfiles.ui
	trackerschanger.ui
	addpeerdialog.ui
	addwebseeddialog.ui
	startupfirstpage.ui
	fastspeedcontrolwidget.ui
	banpeersdialog.ui
	ipfilterdialog.ui
	singletrackerchanger.ui
	torrenttab.ui
	torrenttabwidget.ui
	)
set (RESOURCES bittorrentresources.qrc)
CreateTrs ("bittorrent" "ar;be;de;en;eo;es;fr;it;oc;pl;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("bittorrent" "en;ru_RU" "${SRCS}" "${FORMS}" "torrentsettings.xml")

if (APPLE)
	add_definitions (-DWITH_SHIPPED_GEOIP_H)
endif ()

option (ENABLE_BITTORRENT_GEOIP "Enable support for GeoIP in BitTorrent (requires building libtorrent with GeoIP support)" OFF)

if (ENABLE_BITTORRENT_GEOIP)
	add_definitions (-DENABLE_GEOIP)
elseif (ENABLE_BITTORRENT_GEOIP)
	if (WIN32)
		add_definitions (-DTORRENT_DISABLE_GEO_IP)
	endif ()
endif ()

if (UNIX)
	find_program (LPKGC_CMD pkg-config)
	execute_process (COMMAND "${LPKGC_CMD}" libtorrent-rasterbar --cflags
		OUTPUT_VARIABLE RB_LT_CFLAGS
		OUTPUT_STRIP_TRAILING_WHITESPACE
		)
	execute_process (COMMAND "${LPKGC_CMD}" libtorrent-rasterbar --modversion
		OUTPUT_VARIABLE RB_LT_VERSION
		OUTPUT_STRIP_TRAILING_WHITESPACE
		)
	message (STATUS "BitTorrent: fetched flags for version ${RB_LT_VERSION}: ${RB_LT_CFLAGS}")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${RB_LT_CFLAGS}")

	if (RB_LT_VERSION VERSION_GREATER 0.15.9999)
		add_definitions (-DBOOST_ASIO_DYN_LINK)
	endif ()
endif ()

if (WIN32)
	add_definitions (-DWIN32)
	add_definitions (-D_WIN32_WINNT=0x0502)
	add_definitions (-DWIN32_LEAN_AND_MEAN)
	add_definitions (-DBOOST_ASIO_DYN_LINK)
	add_definitions (-DBOOST_ALL_NO_LIB)
endif ()
#add_definitions(-DBOOST_DYN_LINK)
add_definitions(-DTORRENT_LINKING_SHARED)
#add_definitions(-D_DLL)
if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
endif ()
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

add_library (leechcraft_bittorrent SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCCS}
	)

if (APPLE)
	set (CRYPTOLIB "crypto")
endif ()
target_link_libraries (leechcraft_bittorrent
	${Boost_SYSTEM_LIBRARY}
	${Boost_THREAD_LIBRARY}
	${Boost_DATE_TIME_LIBRARY}
	${Boost_FILESYSTEM_LIBRARY}
	${QT_LIBRARIES}
	${RBTorrent_LIBRARY}
	${LEECHCRAFT_LIBRARIES}
	${CRYPTOLIB}
)
install (TARGETS leechcraft_bittorrent DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
install (FILES torrentsettings.xml DESTINATION ${LC_SETTINGS_DEST})
if (UNIX AND NOT APPLE)
	install (FILES freedesktop/leechcraft-bittorrent.desktop DESTINATION share/applications)
endif ()
