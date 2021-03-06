cmake_minimum_required (VERSION 2.8)
project (leechcraft_blogique_metida)
include (InitLCPlugin OPTIONAL)

set (QT_USE_QTDECLARATIVE TRUE)
set (QT_USE_QTNETWORK TRUE)
set (QT_USE_QTXML TRUE)
set (QT_USE_QTXMLPATTERNS TRUE)
include (${QT_USE_FILE})

include_directories (${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${BLOGIQUE_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/blogique
	)

set (METIDA_SRCS
	addeditentrydialog.cpp
	core.cpp
	frienditemdelegate.cpp
	friendsmodel.cpp
	ljaccount.cpp
	ljaccountconfigurationdialog.cpp
	ljaccountconfigurationwidget.cpp
	ljbloggingplatform.cpp
	ljfriendentry.cpp
	ljprofile.cpp
	ljxmlrpc.cpp
	localstorage.cpp
	metida.cpp
	polldialog.cpp
	postoptionswidget.cpp
	profiletypes.cpp
	profilewidget.cpp
	recentcommentsmodel.cpp
	recentcommentssidewidget.cpp
	recentcommentsview.cpp
	selectgroupsdialog.cpp
	updatetypedialog.cpp
	utils.cpp
	xmlsettingsmanager.cpp
	)

set (METIDA_HEADERS
	addeditentrydialog.h
	core.h
	entryoptions.h
	frienditemdelegate.h
	friendsmodel.h
	ljaccount.h
	ljaccountconfigurationdialog.h
	ljaccountconfigurationwidget.h
	ljbloggingplatform.h
	ljfriendentry.h
	ljprofile.h
	ljxmlrpc.h
	localstorage.h
	metida.h
	postoptionswidget.h
	profiletypes.h
	profilewidget.h
	recentcommentsmodel.h
	recentcommentssidewidget.h
	recentcommentsview.h
	selectgroupsdialog.h
	updatetypedialog.h
	utils.h
	xmlsettingsmanager.h
	)

set (METIDA_FORMS
	addeditentrydialog.ui
	ljaccountconfigurationdialog.ui
	ljaccountconfigurationwidget.ui
	polldialog.ui 
	profilewidget.ui
	postoptionswidget.ui
	recentcommentssidewidget.ui
	selectgroupsdialog.ui
	updatetypedialog.ui
	)

set (METIDA_RESOURCES blogiquemetidaresources.qrc)

# CreateTrVars ("blogique_metida" "de;en;es;fr;it;ru_RU;uk_UA" METIDA_TRANSLATIONS METIDA_COMPILED_TRANSLATIONS)

if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (METIDA_MOC_SRCS ${METIDA_HEADERS})
endif ()
QT4_WRAP_UI (METIDA_UIS_H ${METIDA_FORMS})
QT4_ADD_RESOURCES (METIDA_RCCS ${METIDA_RESOURCES})

# add_custom_command (OUTPUT
# 	${METIDA_COMPILED_TRANSLATIONS}
# 	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${METIDA_TRANSLATIONS}
# 	DEPENDS ${METIDA_TRANSLATIONS}
# 	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
# 	)

set (METIDA_RESOURCES metidaresources.qrc)
QT4_ADD_RESOURCES (METIDA_RCCS ${METIDA_RESOURCES})

add_library (leechcraft_blogique_metida SHARED
	${METIDA_RCCS}
	${METIDA_COMPILED_TRANSLATIONS}
	${METIDA_SRCS}
	${METIDA_MOC_SRCS}
	${METIDA_UIS_H}
	${METIDA_RCCS}
	)

target_link_libraries (leechcraft_blogique_metida
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)

install (TARGETS leechcraft_blogique_metida
		DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${METIDA_COMPILED_TRANSLATIONS}
		DESTINATION ${LC_TRANSLATIONS_DEST})
install (FILES blogiquemetidasettings.xml
		DESTINATION ${LC_SETTINGS_DEST})
install(DIRECTORY share/qml/blogique DESTINATION ${LC_QML_DEST})
