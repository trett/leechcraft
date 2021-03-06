cmake_minimum_required (VERSION 2.8)
project (leechcraft_advancednotifications)
include (InitLCPlugin OPTIONAL)

set (QT_USE_QTDECLARATIVE TRUE)
include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	advancednotifications.cpp
	core.cpp
	generalhandler.cpp
	concretehandlerbase.cpp
	systemtrayhandler.cpp
	notificationruleswidget.cpp
	xmlsettingsmanager.cpp
	notificationrule.cpp
	fieldmatch.cpp
	typedmatchers.cpp
	matchconfigdialog.cpp
	visualhandler.cpp
	audiohandler.cpp
	cmdrunhandler.cpp
	enablesoundactionmanager.cpp
	wmurgenthandler.cpp
	rulesmanager.cpp
	quarkproxy.cpp
	actionsmodel.cpp
	qml/visualnotificationsview.cpp
	qml/eventproxyobject.cpp
	qml/actionsproxyobject.cpp
	)
set (HEADERS
	advancednotifications.h
	core.h
	generalhandler.h
	concretehandlerbase.h
	systemtrayhandler.h
	notificationruleswidget.h
	xmlsettingsmanager.h
	notificationrule.h
	fieldmatch.h
	typedmatchers.h
	matchconfigdialog.h
	visualhandler.h
	audiohandler.h
	cmdrunhandler.h
	enablesoundactionmanager.h
	wmurgenthandler.h
	rulesmanager.h
	quarkproxy.h
	actionsmodel.h
	qml/visualnotificationsview.h
	qml/eventproxyobject.h
	qml/actionsproxyobject.h
	)
set (FORMS
	notificationruleswidget.ui
	matchconfigdialog.ui
	intmatcherconfigwidget.ui
	stringlikematcherconfigwidget.ui
	)

add_definitions(-DHAVE_QML=1)

set (RESOURCES advancednotificationsresources.qrc)

CreateTrs ("advancednotifications" "en;es;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
endif ()
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

add_library (leechcraft_advancednotifications SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${RCCS}
	${UIS_H}
	)
target_link_libraries (leechcraft_advancednotifications
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_advancednotifications DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
install (FILES advancednotificationssettings.xml DESTINATION ${LC_SETTINGS_DEST})
install (DIRECTORY share/qml/advancednotifications DESTINATION ${LC_QML_DEST})
install (DIRECTORY share/sounds DESTINATION ${LC_SHARE_DEST})
