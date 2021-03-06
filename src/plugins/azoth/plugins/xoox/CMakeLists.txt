cmake_minimum_required (VERSION 2.8)
project (leechcraft_azoth_xoox)
include (InitLCPlugin OPTIONAL)

set (QT_USE_QTNETWORK TRUE)
set (QT_USE_QTXML TRUE)
include (${QT_USE_FILE})

set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package (QXmpp REQUIRED)
find_package (Speex REQUIRED)

include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${QXMPP_INCLUDE_DIR}
	${QCA2_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (XOOX_SRCS
	xoox.cpp
	glooxprotocol.cpp
	glooxaccountconfigurationdialog.cpp
	glooxaccountconfigurationwidget.cpp
	glooxaccount.cpp
	clientconnection.cpp
	glooxclentry.cpp
	glooxmessage.cpp
	joingroupchatwidget.cpp
	roomhandler.cpp
	roomclentry.cpp
	roompublicmessage.cpp
	roomparticipantentry.cpp
	entrybase.cpp
	util.cpp
	core.cpp
	vcarddialog.cpp
	transfermanager.cpp
	transferjob.cpp
	bookmarkeditwidget.cpp
	formbuilder.cpp
	roomconfigwidget.cpp
	capsmanager.cpp
	capsdatabase.cpp
	sdsession.cpp
	sdmodel.cpp
	affiliationselectordialog.cpp
	annotationsmanager.cpp
	fetchqueue.cpp
	legacyformbuilder.cpp
	inbandaccountregfirstpage.cpp
	inbandaccountregsecondpage.cpp
	inbandaccountregthirdpage.cpp
	legacyentitytimeext.cpp
	pubsubmanager.cpp
	pepeventbase.cpp
	useractivity.cpp
	usermood.cpp
	usertune.cpp
	privacylistsmanager.cpp
	imagemediawidget.cpp
	privacylistsconfigdialog.cpp
	privacylistsitemdialog.cpp
	adhoccommandmanager.cpp
	executecommanddialog.cpp
	userlocation.cpp
	selfcontact.cpp
	adhoccommandserver.cpp
	adhoccommand.cpp
	lastactivitymanager.cpp
	riexmanager.cpp
	jabbersearchsession.cpp
	jabbersearchmanager.cpp
	useravatarmanager.cpp
	useravatardata.cpp
	useravatarmetadata.cpp
	sdmanager.cpp
	msgarchivingmanager.cpp
	avatarsstorage.cpp
	xep0232handler.cpp
	pepmicroblog.cpp
	vcardlisteditdialog.cpp
	regformhandlerwidget.cpp
	gwoptionsdialog.cpp
	gwitemsremovaldialog.cpp
	clientconnectionerrormgr.cpp
	accstatusrestorer.cpp
	accountsettingsholder.cpp
	crypthandler.cpp
	serverinfostorage.cpp
	inforequestpolicymanager.cpp
	xmppannotationsiq.cpp
	xmppannotationsmanager.cpp
	xmppbobiq.cpp
	xmppbobmanager.cpp
	xmppcaptchaiq.cpp
	xmppcaptchamanager.cpp
	xmlsettingsmanager.cpp
	)
set (XOOX_FORMS
	glooxaccountconfigurationdialog.ui
	glooxaccountconfigurationwidget.ui
	joingroupchatwidget.ui
	vcarddialog.ui
	bookmarkeditwidget.ui
	roomconfigwidget.ui
	affiliationselectordialog.ui
	inbandaccountregfirstpage.ui
	privacylistsconfigdialog.ui
	privacylistsitemdialog.ui
	executecommanddialog.ui
	commandslistpage.ui
	commandresultpage.ui
	vcardlisteditdialog.ui
	gwoptionsdialog.ui
	gwitemsremovaldialog.ui
	)
set (XOOX_RESOURCES azothxooxresources.qrc)

if (ENABLE_CRYPT)
	add_definitions (-DENABLE_CRYPT)

	set (XOOX_SRCS ${XOOX_SRCS} pgpmanager.cpp)
endif ()

if (ENABLE_MEDIACALLS)
	add_definitions (-DENABLE_MEDIACALLS)

	set (XOOX_SRCS ${XOOX_SRCS} mediacall.cpp)
endif ()

CreateTrs ("azoth_xoox" "de;en;es;fr;it;ru_RU;uk_UA" XOOX_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("azoth_xoox" "en;ru_RU" "${XOOX_SRCS}" "${XOOX_FORMS}" "azothxooxsettings.xml")
QT4_WRAP_UI (XOOX_UIS_H ${XOOX_FORMS})
QT4_ADD_RESOURCES (XOOX_RCCS ${XOOX_RESOURCES})

add_library (leechcraft_azoth_xoox SHARED
	${XOOX_COMPILED_TRANSLATIONS}
	${XOOX_SRCS}
	${XOOX_MOC_SRCS}
	${XOOX_UIS_H}
	${XOOX_RCCS}
	)
target_link_libraries (leechcraft_azoth_xoox
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${QXMPP_LIBRARIES}
	${SPEEX_LIBRARIES}
	${QCA2_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_xoox DESTINATION ${LC_PLUGINS_DEST})
install (FILES azothxooxsettings.xml DESTINATION ${LC_SETTINGS_DEST})
if (UNIX AND NOT APPLE)
	install (FILES freedesktop/leechcraft-azoth-xoox.desktop DESTINATION share/applications)
endif ()
