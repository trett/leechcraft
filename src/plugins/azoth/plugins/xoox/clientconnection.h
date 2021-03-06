/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_CLIENTCONNECTION_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_CLIENTCONNECTION_H
#include <functional>
#include <memory>
#include <QObject>
#include <QMap>
#include <QHash>
#include <QSet>
#include <QXmppClient.h>
#include <QXmppMucIq.h>
#include <interfaces/azoth/imessage.h>
#include "glooxclentry.h"
#include "glooxaccount.h"
#include "riexmanager.h"

class QXmppMessage;
class QXmppMucManager;
class QXmppClient;
class QXmppDiscoveryManager;
class QXmppTransferManager;
class QXmppDiscoveryIq;
class QXmppBookmarkManager;
class QXmppArchiveManager;
class QXmppEntityTimeManager;
class QXmppMessageReceiptManager;
#ifdef ENABLE_MEDIACALLS
class QXmppCallManager;
class QXmppCall;
#endif

namespace LeechCraft
{
struct Entity;

namespace Azoth
{
class IProxyObject;

namespace Xoox
{
	class GlooxAccount;
	class GlooxMessage;
	class RoomCLEntry;
	class RoomHandler;
	class SelfContact;
	class CapsManager;
	class AnnotationsManager;
	class FetchQueue;
	class PubSubManager;
	class PrivacyListsManager;
	class AdHocCommandManager;
	class LastActivityManager;
	class JabberSearchManager;
	class XMPPBobManager;
	class XMPPCaptchaManager;
	class UserAvatarManager;
	class MsgArchivingManager;
	class SDManager;

	class InfoRequestPolicyManager;
	class ClientConnectionErrorMgr;
	class CryptHandler;
	class ServerInfoStorage;

	class ClientConnection : public QObject
	{
		Q_OBJECT

		GlooxAccount *Account_;
		AccountSettingsHolder *Settings_;

		QXmppClient *Client_;
		QXmppMucManager *MUCManager_;
		QXmppTransferManager *XferManager_;
		QXmppDiscoveryManager *DiscoveryManager_;
		QXmppBookmarkManager *BMManager_;
		QXmppEntityTimeManager *EntityTimeManager_;
		QXmppArchiveManager *ArchiveManager_;
		QXmppMessageReceiptManager *DeliveryReceiptsManager_;
		XMPPCaptchaManager *CaptchaManager_;
		XMPPBobManager *BobManager_;
#ifdef ENABLE_MEDIACALLS
		QXmppCallManager *CallManager_;
#endif
		PubSubManager *PubSubManager_;
		PrivacyListsManager *PrivacyListsManager_;
		AdHocCommandManager *AdHocCommandManager_;
		AnnotationsManager *AnnotationsManager_;
		LastActivityManager *LastActivityManager_;
		JabberSearchManager *JabberSearchManager_;
		UserAvatarManager *UserAvatarManager_;
		RIEXManager *RIEXManager_;
		MsgArchivingManager *MsgArchivingManager_;
		SDManager *SDManager_;

		CryptHandler *CryptHandler_;
		ClientConnectionErrorMgr *ErrorMgr_;

		InfoRequestPolicyManager *InfoReqPolicyMgr_;

		QString OurJID_;
		QString OurBareJID_;
		QString OurResource_;

		SelfContact *SelfContact_;

		IProxyObject *ProxyObject_;
		CapsManager *CapsManager_;

		ServerInfoStorage *ServerInfoStorage_;

		QHash<QString, GlooxCLEntry*> JID2CLEntry_;
		QHash<QString, GlooxCLEntry*> ODSEntries_;

		bool IsConnected_;
		bool FirstTimeConnect_;

		QHash<QString, RoomHandler*> RoomHandlers_;
		GlooxAccountState LastState_;
		QString Password_;

		struct JoinQueueItem
		{
			bool AsAutojoin_;
			QString RoomJID_;
			QString Nickname_;
		};
		QList<JoinQueueItem> JoinQueue_;

		FetchQueue *VCardQueue_;
		FetchQueue *CapsQueue_;
		FetchQueue *VersionQueue_;

		QList<QXmppMessage> OfflineMsgQueue_;
		QList<QPair<QString, PEPEventBase*>> InitialEventQueue_;
		QHash<QString, QPointer<GlooxMessage>> UndeliveredMessages_;

		QHash<QString, QList<RIEXManager::Item>> AwaitingRIEXItems_;
	public:
		typedef std::function<void (const QXmppDiscoveryIq&)> DiscoCallback_t;
		typedef std::function<void (const QXmppVCardIq&)> VCardCallback_t;
		typedef std::function<void (QXmppIq)> PacketCallback_t;
	private:
		QHash<QString, DiscoCallback_t> AwaitingDiscoInfo_;
		QHash<QString, DiscoCallback_t> AwaitingDiscoItems_;

		typedef QHash<QString, PacketCallback_t> PacketID2Callback_t;
		QHash<QString, PacketID2Callback_t> AwaitingPacketCallbacks_;

		QHash<QString, QList<VCardCallback_t>> VCardFetchCallbacks_;
	public:
		ClientConnection (GlooxAccount*);
		virtual ~ClientConnection ();

		void SetState (const GlooxAccountState&);
		GlooxAccountState GetLastState () const;

		void SetPassword (const QString&);

		QString GetOurJID () const;
		void SetOurJID (const QString&);

		/** Joins the room and returns the contact list
		 * entry representing that room.
		 */
		RoomCLEntry* JoinRoom (const QString& room, const QString& user, bool asAutojoin = false);
		void Unregister (RoomHandler*);

		void CreateEntry (const QString&);

		QXmppMucManager* GetMUCManager () const;
		QXmppDiscoveryManager* GetDiscoveryManager () const;
		QXmppVersionManager* GetVersionManager () const;
		QXmppTransferManager* GetTransferManager () const;
		CapsManager* GetCapsManager () const;
		AnnotationsManager* GetAnnotationsManager () const;
		PubSubManager* GetPubSubManager () const;
		PrivacyListsManager* GetPrivacyListsManager () const;
		XMPPBobManager* GetBobManager () const;
#ifdef ENABLE_MEDIACALLS
		QXmppCallManager* GetCallManager () const;
#endif
		AdHocCommandManager* GetAdHocCommandManager () const;
		JabberSearchManager* GetJabberSearchManager () const;
		UserAvatarManager* GetUserAvatarManager () const;
		RIEXManager* GetRIEXManager () const;
		SDManager* GetSDManager () const;

		InfoRequestPolicyManager* GetInfoReqPolicyManager () const;

		CryptHandler* GetCryptHandler () const;
		ServerInfoStorage* GetServerInfoStorage () const;

		void SetSignaledLog (bool);

		void RequestInfo (const QString&) const;

		void RequestInfo (const QString&, DiscoCallback_t, bool reportErrors = false, const QString& = "");
		void RequestItems (const QString&, DiscoCallback_t, bool reportErrors = false, const QString& = "");

		void Update (const QXmppRosterIq::Item&);
		void Update (const QXmppMucItem&, const QString& room);

		void AckAuth (QObject*, bool);
		void AddEntry (const QString&, const QString&, const QStringList&);
		void Subscribe (const QString&,
				const QString& = QString (), const QString& = QString (),
				const QStringList& = QStringList ());
		void Unsubscribe (const QString&, const QString&);
		void GrantSubscription (const QString&, const QString&);
		void RevokeSubscription (const QString&, const QString&);
		void Remove (GlooxCLEntry*);

		void WhitelistError (const QString&);

		void SendPacketWCallback (const QXmppIq&, PacketCallback_t);
		void SendMessage (GlooxMessage*);
		QXmppClient* GetClient () const;
		QObject* GetCLEntry (const QString& fullJid) const;
		QObject* GetCLEntry (const QString& bareJid, const QString& variant) const;
		GlooxCLEntry* AddODSCLEntry (OfflineDataSource_ptr);
		QList<QObject*> GetCLEntries () const;
		void FetchVCard (const QString&, bool reportErrors = false);
		void FetchVCard (const QString&, VCardCallback_t, bool reportErrors = false);
		void FetchVersion (const QString&, bool reportErrors = false);

		QXmppBookmarkSet GetBookmarks () const;
		void SetBookmarks (const QXmppBookmarkSet&);
		QXmppBookmarkManager* GetBMManager () const;

		GlooxMessage* CreateMessage (IMessage::MessageType,
				const QString&, const QString&, const QString&);

		static void Split (const QString& full,
				QString *bare, QString *resource);
	private:
		void SetupLogger ();
		void HandleOtherPresence (const QXmppPresence&);
		void HandleRIEX (QString, QList<RIEXManager::Item>, QString = QString ());
		void InvokeCallbacks (const QXmppIq&);
	public slots:
		void handlePendingForm (QXmppDataForm*, const QString&);
	private slots:
		void handleConnected ();
		void handleDisconnected ();
		void handleIqReceived (const QXmppIq&);
		void handleRosterReceived ();
		void handleRosterChanged (const QString&);
		void handleRosterItemRemoved (const QString&);
		void handleVCardReceived (const QXmppVCardIq&);
		void handleVersionReceived (const QXmppVersionIq&);
		void handlePresenceChanged (const QXmppPresence&);
		void handleMessageReceived (QXmppMessage);
		void handlePEPEvent (const QString&, PEPEventBase*);
		void handlePEPAvatarUpdated (const QString&, const QImage&);
		void handleMessageDelivered (const QString&, const QString&);
		void handleCaptchaReceived (const QString&, const QXmppDataForm&);
		void handleRoomInvitation (const QString&, const QString&, const QString&);
		void handleGotRIEXItems (QString, QList<RIEXManager::Item>, bool);

		void handleBookmarksReceived (const QXmppBookmarkSet&);
		void handleAutojoinQueue ();

		void handleDiscoInfo (const QXmppDiscoveryIq&);
		void handleDiscoItems (const QXmppDiscoveryIq&);

		void handleLog (QXmppLogger::MessageType, const QString&);

		void setKAParams (const QPair<int, int>&);
		void setFileLogging (bool);
		void handlePhotoHash ();
		void handlePriorityChanged (int);
		void updateFTSettings ();
		void handleDetectedBSProxy (const QString&);

		void handleVersionSettingsChanged ();
	private:
		void ScheduleFetchVCard (const QString&, bool);
		GlooxCLEntry* CreateCLEntry (const QString&);
		GlooxCLEntry* CreateCLEntry (const QXmppRosterIq::Item&);
		GlooxCLEntry* ConvertFromODS (const QString&, const QXmppRosterIq::Item&);
	signals:
		void gotRosterItems (const QList<QObject*>&);
		void rosterItemRemoved (QObject*);
		void rosterItemsRemoved (const QList<QObject*>&);
		void rosterItemUpdated (QObject*);
		void rosterItemSubscribed (QObject*, const QString&);
		void rosterItemUnsubscribed (QObject*, const QString&);
		void rosterItemUnsubscribed (const QString&, const QString&);
		void rosterItemCancelledSubscription (QObject*, const QString&);
		void rosterItemGrantedSubscription (QObject*, const QString&);
		void gotSubscriptionRequest (QObject*, const QString&);
		void gotMUCInvitation (const QVariantMap&, const QString&, const QString&);

		void gotConsoleLog (const QByteArray&, int, const QString&);

		void gotRequestedPosts (const QList<LeechCraft::Azoth::Post>&, const QString&);
		void gotNewPost (const LeechCraft::Azoth::Post&);

		void serverAuthFailed ();
		void needPassword ();
		void statusChanged (const EntryStatus&);
	};
}
}
}

#endif
