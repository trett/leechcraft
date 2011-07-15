# Find gloox library

#
# QXMPP_INCLUDE_DIR
# QXMPP_LIBRARIES
# QXMPP_FOUND

# Copyright (c) 2010 Georg Rudoy <0xd34df00d@gmail.com>

FIND_PATH(QXMPP_INCLUDE_DIR qxmpp/QXmppClient.h PATH ENV)
FIND_LIBRARY(QXMPP_LIBRARIES NAMES qxmpp)

IF(QXMPP_LOCAL)
	FIND_PATH(QXMPP_INCLUDE_DIR QXmppClient.h "${QXMPP_LOCAL}/src")
	IF(QXMPP_INCLUDE_DIR)
		SET(QXMPP_LIBRARIES "${QXMPP_LOCAL}/lib/libqxmpp.a")
	ENDIF(QXMPP_INCLUDE_DIR)
ENDIF(QXMPP_LOCAL)

IF(QXMPP_LIBRARIES AND QXMPP_INCLUDE_DIR)
	IF(NOT QXMPP_LOCAL)
		SET(QXMPP_INCLUDE_DIR "${QXMPP_INCLUDE_DIR}/qxmpp")
	ENDIF(NOT QXMPP_LOCAL)
	SET(QXMPP_FOUND 1)
ENDIF(QXMPP_LIBRARIES AND QXMPP_INCLUDE_DIR)

IF(QXMPP_FOUND)
	MESSAGE(STATUS "Found QXmpp libraries at ${QXMPP_LIBRARIES}")
	MESSAGE(STATUS "Found QXmpp headers at ${QXMPP_INCLUDE_DIR}")
ELSE(QXMPP_FOUND)
	IF(QXMPP_FIND_REQUIRED)
		MESSAGE(FATAL_ERROR "Could NOT find required QXmpp library, aborting")
	ELSE()
		MESSAGE(STATUS "Could NOT find QXmpp")
	ENDIF()
ENDIF()
