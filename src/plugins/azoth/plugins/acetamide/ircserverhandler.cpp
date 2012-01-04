/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "ircserverhandler.h"
#include <boost/bind.hpp>
#include <QTextCodec>
#include <QMessageBox>
#include <QInputDialog>
#include <util/util.h>
#include <util/notificationactionhandler.h>
#include "channelhandler.h"
#include "channelclentry.h"
#include "servercommandmessage.h"
#include "clientconnection.h"
#include "ircaccount.h"
#include "ircmessage.h"
#include "ircparser.h"
#include "ircserverclentry.h"
#include "xmlsettingsmanager.h"
#include "channelpublicmessage.h"
#include "ircerrorhandler.h"
#include "ircserversocket.h"
#include "usercommandmanager.h"
#include "serverresponcemanager.h"
#include "rplisupportparser.h"
#include "channelsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	IrcServerHandler::IrcServerHandler (const ServerOptions& server,
			IrcAccount *account)
	: Account_ (account)
	, ErrorHandler_ (new IrcErrorHandler (this))
	, IrcParser_ (0)
	, ServerCLEntry_ (new IrcServerCLEntry (this, account))
	, ServerConnectionState_ (NotConnected)
	, IsConsoleEnabled_ (false)
	, IsInviteDialogActive_ (false)
	, ServerID_ (server.ServerName_ + ":" +
			QString::number (server.ServerPort_))
	, NickName_ (server.ServerNickName_)
	, ServerOptions_ (server)
	{
		IrcParser_ = new IrcParser (this);
		CmdManager_ = new UserCommandManager (this);
		ServerResponceManager_ = new ServerResponceManager (this);
		RplISupportParser_ = new RplISupportParser (this);
		ChannelsManager_ = new ChannelsManager (this);

		connect (this,
				SIGNAL (connected (const QString&)),
				Account_->GetClientConnection ().get (),
				SLOT (serverConnected (const QString&)));

		connect (this,
				SIGNAL (disconnected (const QString&)),
				Account_->GetClientConnection ().get (),
				SLOT (serverDisconnected (const QString&)));

		connect (this,
				SIGNAL (nicknameConflict (const QString&)),
				ServerCLEntry_,
				SIGNAL (nicknameConflict (const QString&)));
	}

	IrcServerCLEntry* IrcServerHandler::GetCLEntry () const
	{
		return ServerCLEntry_;
	}

	IrcAccount* IrcServerHandler::GetAccount () const
	{
		return Account_;
	}

	IrcParser* IrcServerHandler::GetParser () const
	{
		return IrcParser_;
	}

	ChannelsManager* IrcServerHandler::GetChannelManager () const
	{
		return ChannelsManager_;
	}

	QString IrcServerHandler::GetNickName () const
	{
		return NickName_;
	}

	QString IrcServerHandler::GetServerID () const
	{
		return ServerID_;
	}

	ServerOptions IrcServerHandler::GetServerOptions () const
	{
		return ServerOptions_;
	}

	QObjectList IrcServerHandler::GetCLEntries () const
	{
		QObjectList result;

		result << ChannelsManager_->GetCLEntries ();

		Q_FOREACH (ServerParticipantEntry_ptr spe, Nick2Entry_.values ())
			result << spe.get ();

		return result;
	}

	QStringList IrcServerHandler::GetPrivateChats () const
	{
		QStringList result;
		Q_FOREACH (ServerParticipantEntry_ptr spe, Nick2Entry_.values ())
			result << spe->GetEntryName ();
		return result;
	}

	ChannelHandler* IrcServerHandler::GetChannelHandler (const QString& channel)
	{
		return ChannelsManager_->GetChannelHandler (channel);
	}

	QList<ChannelHandler*> IrcServerHandler::GetChannelHandlers () const
	{
		return ChannelsManager_->GetChannels ();
	}

	IrcMessage* IrcServerHandler::CreateMessage (IMessage::MessageType type,
			const QString& variant, const QString& body)
	{
		IrcMessage *msg = new IrcMessage (type,
				IMessage::DIn,
				variant,
				QString (),
				Account_->GetClientConnection ().get ());
		msg->SetBody (body);
		msg->SetDateTime (QDateTime::currentDateTime ());

		return msg;
	}

	bool IrcServerHandler::IsChannelExists (const QString& channel)
	{
		return ChannelsManager_->IsChannelExists (channel);
	}

	void IrcServerHandler::SetNickName (const QString& nick)
	{
		NickName_ = nick;
	}

	void IrcServerHandler::Add2ChannelsQueue (const ChannelOptions& ch)
	{
		if (!ch.ChannelName_.isEmpty ())
			ChannelsManager_->AddChannel2Queue (ch);
	}

	void IrcServerHandler::JoinChannel (const ChannelOptions& channel)
	{
		if (ServerConnectionState_ == Connected)
		{
			if (!ChannelsManager_->IsChannelExists (channel.ChannelName_.toLower ()))
				IrcParser_->JoinCommand (QStringList () << channel.ChannelName_
						<< channel.ChannelPassword_);
		}
		else
			Add2ChannelsQueue (channel);
	}

	bool IrcServerHandler::JoinedChannel (const ChannelOptions& channel)
	{
		if (ServerConnectionState_ == Connected &&
				!ChannelsManager_->IsChannelExists (channel.ChannelName_.toLower ()))
			return ChannelsManager_->AddChannel (channel);
		else
			Add2ChannelsQueue (channel);

		return true;
	}

	void IrcServerHandler::JoinParticipant (const QString& nick,
			const QString& msg, const QString& user, const QString& host)
	{
		ChannelsManager_->AddParticipant (msg.toLower (), nick, user, host);
	}

	void IrcServerHandler::CloseChannel (const QString& channel)
	{
		ChannelsManager_->CloseChannel (channel.toLower ());
	}

	void IrcServerHandler::LeaveParticipant (const QString& nick,
			const QString& channel, const QString& msg)
	{
		ChannelsManager_->LeaveParticipant (channel, nick, msg);
	}

	void IrcServerHandler::QuitServer ()
	{
		//TODO quit message
		IrcParser_->QuitCommand (QStringList ());
		DisconnectFromServer ();
	}

	void IrcServerHandler::QuitParticipant (const QString& nick, const QString& msg)
	{
		ChannelsManager_->QuitParticipant (nick, msg);
	}

	void IrcServerHandler::SendMessage (const QStringList& cmd)
	{
		if (cmd.isEmpty ())
			return;

		const QString target = cmd.first ();
		const QStringList msg = cmd.mid (1);

		if (ChannelsManager_->IsChannelExists (target.toLower ()))
			ChannelsManager_->SendPublicMessage (target.toLower (), msg.join (" "));
		else
			IrcParser_->PrivMsgCommand (cmd);
	}

	void IrcServerHandler::IncomingMessage (const QString& nick,
			const QString& target, const QString& msg)
	{
		if (ChannelsManager_->IsChannelExists (target))
			ChannelsManager_->ReceivePublicMessage (target, nick, msg);
		else
		{
			ServerParticipantEntry_ptr entry = GetParticipantEntry (nick);
			if (!entry)
				return;

			IrcMessage *message = new IrcMessage (IMessage::MTChatMessage,
					IMessage::DIn,
					ServerID_,
					nick,
					Account_->GetClientConnection ().get ());
			message->SetBody (msg);
			message->SetDateTime (QDateTime::currentDateTime ());
			entry->SetStatus (EntryStatus (SOnline, QString ()));
			entry->HandleMessage (message);
		}
	}

	void IrcServerHandler::IncomingNoticeMessage (const QString& nick, const QString& msg)
	{
		ShowAnswer ("NOTICE", msg);
		QList<NickServIdentify> list = Core::Instance ()
				.GetNickServIdentifyWithMainParams (ServerOptions_.ServerName_,
						GetNickName (),
						nick);

		if (list.isEmpty ())
			return;

		Q_FOREACH (const NickServIdentify& nsi, list)
		{
			QRegExp authRegExp (nsi.AuthString_,
					Qt::CaseInsensitive,
					QRegExp::Wildcard);
			if (authRegExp.indexIn (msg) == -1)
				continue;

			SendMessage2Server (nsi.AuthMessage_.split (' '));
			return;
		}
	}

	void IrcServerHandler::ChangeNickname (const QString& nick,
			const QString& msg)
	{
		ChannelsManager_->ChangeNickname (nick, msg);

		if (!Nick2Entry_.contains (nick))
			return;

		Account_->handleEntryRemoved (Nick2Entry_ [nick].get ());
		ServerParticipantEntry_ptr entry = Nick2Entry_.take (nick);
		entry->SetEntryName (msg);
		Account_->handleGotRosterItems (QList<QObject*> () << entry.get ());
		Nick2Entry_ [msg] = entry;

		if (nick == NickName_)
			NickName_ = msg;
	}

	bool IrcServerHandler::IsCmdHasLongAnswer (const QString& cmd)
	{
		return IrcParser_->IsCmdHasLongAnswer (cmd);
	}

	void IrcServerHandler::GetBanList (const QString& channel)
	{
		IrcParser_->ChanModeCommand (QStringList () << channel << "b");
	}

	void IrcServerHandler::GetExceptList (const QString& channel)
	{
		IrcParser_->ChanModeCommand (QStringList () << channel << "e");
	}

	void IrcServerHandler::GetInviteList (const QString& channel)
	{
		IrcParser_->ChanModeCommand (QStringList () << channel << "I");
	}

	void IrcServerHandler::AddBanListItem (const QString& channel, QString mask)
	{
		IrcParser_->ChanModeCommand (QStringList () << channel << "+b" << mask);
	}

	void IrcServerHandler::RemoveBanListItem (const QString& channel, QString mask)
	{
		IrcParser_->ChanModeCommand (QStringList () << channel << "-b" << mask);
	}

	void IrcServerHandler::AddExceptListItem (const QString& channel, QString mask)
	{
		IrcParser_->ChanModeCommand (QStringList () << channel << "+e" << mask);
	}

	void IrcServerHandler::RemoveExceptListItem (const QString& channel, QString mask)
	{
		IrcParser_->ChanModeCommand (QStringList () << channel << "-e" << mask);
	}

	void IrcServerHandler::AddInviteListItem (const QString& channel, QString mask)
	{
		IrcParser_->ChanModeCommand (QStringList () << channel << "+I" << mask);
	}

	void IrcServerHandler::RemoveInviteListItem (const QString& channel, QString mask)
	{
		IrcParser_->ChanModeCommand (QStringList () << channel << "-I" << mask);
	}

	void IrcServerHandler::SetNewChannelModes(const QString& channel, const ChannelModes& modes)
	{
		if (!ChannelsManager_->IsChannelExists (channel.toLower ()))
			return;

		IrcParser_->ChanModeCommand (QStringList () << channel
				<< (modes.BlockOutsideMessageMode_ ? "+n" : "-n"));

		if (modes.ChannelKey_.first)
			IrcParser_->ChanModeCommand (QStringList () << channel
					<< "+k" << modes.ChannelKey_.second);
		else
			IrcParser_->ChanModeCommand (QStringList () << channel << "-k");

		IrcParser_->ChanModeCommand (QStringList () << channel
				<< (modes.InviteMode_ ? "+i" : "-i"));

		IrcParser_->ChanModeCommand (QStringList () << channel
				<< (modes.ModerateMode_ ? "+m" : "-m"));

		IrcParser_->ChanModeCommand (QStringList () << channel
				<< (modes.OnlyOpChangeTopicMode_ ? "+t" : "-t"));

		IrcParser_->ChanModeCommand (QStringList () << channel
				<< (modes.PrivateMode_ ? "+p" : "-p"));

		IrcParser_->ChanModeCommand (QStringList () << channel
				<< (modes.ReOpMode_ ? "+r" : "-r"));

		IrcParser_->ChanModeCommand (QStringList () << channel
				<< (modes.SecretMode_ ? "+s" : "-s"));

		if (modes.UserLimit_.first)
			IrcParser_->ChanModeCommand (QStringList () << channel
					<< "+l" << QString::number(modes.UserLimit_.second));
		else
			IrcParser_->ChanModeCommand (QStringList () << channel << "-l");
	}

	void IrcServerHandler::SetNewChannelMode (const QString& channel,
			const QString& mode, const QString& target)
	{
		if (!ChannelsManager_->IsChannelExists (channel))
			return;

		IrcParser_->ChanModeCommand (QStringList () << channel << mode << target);
	}

	void IrcServerHandler::PongMessage (const QString& msg)
	{
		IrcParser_->PongCommand (QStringList () << msg);
	}

	void IrcServerHandler::GotTopic (const QString& channel,
			const QString& message)
	{
		if (ChannelsManager_->IsChannelExists (channel))
			ChannelsManager_->SetMUCSubject (channel, message);
		else
			ShowAnswer ("TOPIC", message);
	}

	void IrcServerHandler::GotKickCommand (const QString& nick,
			const QString& channel, const QString& target,
			const QString& msg)
	{
		const QString& chnnl = channel.toLower ();
		if (ChannelsManager_->IsChannelExists (chnnl))
			ChannelsManager_->KickParticipant (chnnl, target, msg, nick);
	}

	void IrcServerHandler::KickParticipant (const QString& channel,
			const QString& nick, const QString& reason)
	{
		if (ChannelsManager_->IsChannelExists (channel.toLower ()))
			IrcParser_->KickCommand (QStringList () << channel << nick << reason);
	}

	void IrcServerHandler::GotInvitation (const QString& nick,
			const QString& msg)
	{
		if (IsInviteDialogActive_)
			InviteChannelsDialog_->AddInvitation (msg, nick);
		else
		{
			IsInviteDialogActive_ = true;
			InviteChannelsDialog_.reset (new InviteChannelsDialog (msg, nick));
			InviteChannelsDialog_->setModal (true);

			connect (InviteChannelsDialog_.get (),
					SIGNAL (accepted ()),
					this,
					SLOT (joinAfterInvite ()));
		}
		InviteChannelsDialog_->show ();
	}

	void IrcServerHandler::ShowAnswer (const QString& cmd, const QString& answer, bool isEndOf)
	{
		QString msg = "[" + cmd.toUpper () + "] " + answer;
		if (!ChannelsManager_->IsCmdQueueEmpty ())
			ChannelsManager_->ReceiveCmdAnswerMessage (cmd, msg, isEndOf);
		else
			ServerCLEntry_->HandleMessage (CreateMessage (IMessage::MTEventMessage,
					ServerID_,
					msg));
	}

	void IrcServerHandler::CTCPReply (const QString& nick,
			const QString& cmd, const QString& mess)
	{
		ChannelsManager_->CTCPReply (mess);
		IrcParser_->CTCPReply (QStringList () << nick << cmd);
	}

	void IrcServerHandler::CTCPRequestResult (const QString& msg)
	{
		ChannelsManager_->CTCPRequestResult (msg);
	}

	void IrcServerHandler::CTCPRequst (const QStringList& cmd)
	{
		IrcParser_->CTCPRequest (cmd);
	}

	void IrcServerHandler::GotNames (const QString& channel,
			const QStringList& participants)
	{
		ChannelsManager_->GotNames (channel.toLower (), participants);
	}

	void IrcServerHandler::GotEndOfNames (const QString& channel)
	{
		ChannelsManager_->GotEndOfNamesCmd (channel.toLower ());
	}

	void IrcServerHandler::ShowUserHost (const QString& nick,
			const QString& host)
	{
		ShowAnswer ("userhost", nick + tr (" is a ") + host);
	}

	void IrcServerHandler::ShowIsUserOnServer (const QString& nick)
	{
		ShowAnswer ("ison", nick + tr (" is on server"));
	}

	void IrcServerHandler::ShowWhoIsReply (const QString& msg, bool isEndOf)
	{
		ShowAnswer ("whois", msg, isEndOf);
	}

	void IrcServerHandler::ShowWhoWasReply (const QString& msg, bool isEndOf)
	{
		ShowAnswer ("whowas", msg, isEndOf);
	}

	void IrcServerHandler::ShowWhoReply (const QString& msg, bool isEndOf)
	{
        ShowAnswer ("who", msg, isEndOf);
	}

	void IrcServerHandler::ShowLinksReply (const QString& msg, bool isEndOf)
	{
        ShowAnswer ("links", msg, isEndOf);
	}

	void IrcServerHandler::ShowInfoReply (const QString& msg, bool isEndOf)
	{
        ShowAnswer ("info", msg, isEndOf);
	}

	void IrcServerHandler::ShowMotdReply (const QString& msg, bool isEndOf)
	{
        ShowAnswer ("motd", msg, isEndOf);
	}

	void IrcServerHandler::ShowUsersReply (const QString& msg, bool isEndOf)
	{
        ShowAnswer ("users", msg, isEndOf);
	}

	void IrcServerHandler::ShowTraceReply (const QString& msg, bool isEndOf)
	{
        ShowAnswer ("trace", msg, isEndOf);
	}

	void IrcServerHandler::ShowStatsReply (const QString& msg, bool isEndOf)
	{
        ShowAnswer ("stats", msg, isEndOf);
	}

	void IrcServerHandler::ShowBanList (const QString& channel,
			const QString& mask, const QString& nick, const QDateTime& time)
	{
		const QString& chnnl = channel.toLower ();
		if (!ChannelsManager_->IsChannelExists (chnnl))
			return;

		ChannelsManager_->SetBanListItem (chnnl, mask, nick, time);
	}

	void IrcServerHandler::ShowBanListEnd (const QString& msg)
	{
		ShowAnswer ("mode", msg);
	}

	void IrcServerHandler::ShowExceptList (const QString& channel,
			const QString& mask, const QString& nick, const QDateTime& time)
	{
		const QString& chnnl = channel.toLower ();
		if (!ChannelsManager_->IsChannelExists (chnnl))
			return;

		ChannelsManager_->SetExceptListItem (chnnl, mask, nick, time);
	}

	void IrcServerHandler::ShowExceptListEnd (const QString& msg)
	{
		ShowAnswer ("MODE", msg);
	}

	void IrcServerHandler::ShowInviteList (const QString& channel,
			const QString& mask, const QString& nick, const QDateTime& time)
	{
		const QString& chnnl = channel.toLower ();
		if (!ChannelsManager_->IsChannelExists (chnnl))
			return;

		ChannelsManager_->SetInviteListItem (chnnl, mask, nick, time);
	}

	void IrcServerHandler::ShowInviteListEnd (const QString& msg)
	{
		ShowAnswer ("mode", msg);
	}

	void IrcServerHandler::SendPublicMessage (const QString& msg,
			const QString& channel)
	{
		Q_FOREACH (const QString& str, msg.split ('\n'))
			IrcParser_->PrivMsgCommand (QStringList ()
					<< channel
					<< str);
	}

	void IrcServerHandler::SendPrivateMessage (IrcMessage* msg)
	{
		Q_FOREACH (const QString& str, msg->GetBody ().split ('\n'))
			IrcParser_->PrivMsgCommand (QStringList ()
					<< msg->GetOtherVariant ()
					<< str);

		ServerParticipantEntry_ptr entry = GetParticipantEntry (msg->GetOtherVariant ());
		entry->HandleMessage (msg);
	}

	void IrcServerHandler::SendMessage2Server (const QStringList& list)
	{
		QString msg = list.join (" ");
		const QString& cmd = CmdManager_->VerifyMessage (msg, QString ());
		if (!cmd.isEmpty ())
		{
			if (msg.startsWith ('/'))
				IrcParser_->RawCommand (msg.mid (1).split (' '));
			else
				IrcParser_->RawCommand (list);
		}
		ShowAnswer (cmd, msg);
	}

	QString IrcServerHandler::ParseMessageForCommand (const QString& msg,
			const QString& channel)
	{
		QString cmd = CmdManager_->VerifyMessage (msg, channel);
		if (cmd.isEmpty ())
			IrcParser_->RawCommand (msg.mid (1).split (' '));

		return cmd;
	}

	void IrcServerHandler::LeaveChannel (const QString& channel,
			const QString& msg)
	{
		IrcParser_->PartCommand (QStringList () << channel << msg);
	}

	void IrcServerHandler::ClosePrivateChat (const QString& nick)
	{
		if (Nick2Entry_.contains (nick))
		{
			Account_->handleEntryRemoved (Nick2Entry_ [nick].get ());
			RemoveParticipantEntry (nick);
			if (!Nick2Entry_.count () &&
					!ChannelsManager_->Count () &&
					XmlSettingsManager::Instance ()
							.property ("AutoDisconnectFromServer").toBool ())
				DisconnectFromServer ();
		}
	}

	void IrcServerHandler::ConnectToServer ()
	{
		if (ServerConnectionState_ != NotConnected)
			return;

		Socket_ = new IrcServerSocket (this);
		Socket_->ConnectToHost (ServerOptions_.ServerName_,
				ServerOptions_.ServerPort_);
		ServerConnectionState_ = InProgress;
	}

	void IrcServerHandler::DisconnectFromServer ()
	{
		ChannelsManager_->CloseAllChannels ();
		if (ServerConnectionState_ != NotConnected)
			Socket_->DisconnectFromHost ();
	}

	void IrcServerHandler::SendCommand (const QString& cmd)
	{
		SendToConsole (IMessage::DOut, cmd.trimmed ());
		Socket_->Send (cmd);
	}

	void IrcServerHandler::SendToConsole (IMessage::Direction dir,
			const QString& message)
	{
		if (!IsConsoleEnabled_)
			return;

		if (dir == IMessage::DIn)
			emit sendMessageToConsole (dir, message);
		else
			emit sendMessageToConsole (dir, message);
	}

	void IrcServerHandler::NickCmdError ()
	{
		int index = Account_->GetNickNames ().indexOf (NickName_);

		if (index != Account_->GetNickNames ().count () - 1)
			NickName_ = Account_->GetNickNames ().at (++index);
		else
			NickName_ = Account_->GetNickNames ().at (0);

		if (NickName_.isEmpty ())
		{
			NickCmdError ();
			return;
		}

		if (NickName_ == OldNickName_)
		{
			emit nicknameConflict (NickName_);
			return;
		}

		IrcParser_->NickCommand (QStringList () << NickName_);
	}

	ServerParticipantEntry_ptr IrcServerHandler::GetParticipantEntry (const QString& nick)
	{
		if (Nick2Entry_.contains (nick))
			return Nick2Entry_ [nick];

		ServerParticipantEntry_ptr entry (CreateParticipantEntry (nick));
		Nick2Entry_ [nick] = entry;
		return entry;
	}

	void IrcServerHandler::RemoveParticipantEntry (const QString& nick)
	{
		Nick2Entry_.remove (nick);
	}

	void IrcServerHandler::SetConsoleEnabled (bool enabled)
	{
		IsConsoleEnabled_ = enabled;
	}

	void IrcServerHandler::ReadReply (const QByteArray& msg)
	{
		SendToConsole (IMessage::DIn, msg.trimmed ());
		if (!IrcParser_->ParseMessage (msg))
			return;

		const IrcMessageOptions& opts = IrcParser_->GetIrcMessageOptions ();
		if (ErrorHandler_->IsError (opts.Command_.toInt ()))
		{
			ErrorHandler_->HandleError (opts);
			if (opts.Command_ == "433")
			{
				if (OldNickName_.isEmpty ())
					OldNickName_ = NickName_;
				else if (!Account_->GetNickNames ().contains (OldNickName_))
					OldNickName_ = Account_->GetNickNames ().first ();
				NickCmdError ();
			}
		}
		else
			ServerResponceManager_->DoAction (opts);
	}

	ServerParticipantEntry_ptr IrcServerHandler::CreateParticipantEntry (const QString& nick)
	{
		ServerParticipantEntry_ptr entry (new ServerParticipantEntry (nick, this, Account_));
		Account_->handleGotRosterItems (QList<QObject*> () << entry.get ());
		return entry;
	}

	void IrcServerHandler::JoinFromQueue ()
	{
		Q_FOREACH (const ChannelOptions& co, ChannelsManager_->GetChannelsQueue ())
			IrcParser_->JoinCommand (QStringList () << co.ChannelName_
					<< co.ChannelPassword_);

		ChannelsManager_->CleanQueue ();
	}

	void IrcServerHandler::SayCommand (const QStringList& params)
	{
		if (params.isEmpty ())
			return;

		const QString& channel = params.first ();
		SendPublicMessage (QStringList (params.mid (1)).join (" "),
				channel.toLower ());
	}

	void IrcServerHandler::ParseChanMode (const QString& channel,
			const QString& mode, const QString& value)
	{
		if (mode.isEmpty ())
			return;

		const QString& chnnl = channel.toLower();
		if (!ChannelsManager_->IsChannelExists (chnnl))
			return;

		ChannelsManager_->ParseChanMode (chnnl, mode, value);
	}

	void IrcServerHandler::ParseUserMode (const QString& nick,
			const QString& mode)
	{
		Q_UNUSED (nick);
		Q_UNUSED (mode);
		//TODO but I don't know how it use
	}

	void IrcServerHandler::ParserISupport (const QString& msg)
	{
		if (RplISupportParser_->ParseISupportReply (msg))
			ISupport_ = RplISupportParser_->GetISupportMap ();
	}

	QMap<QString, QString> IrcServerHandler::GetISupport () const
	{
		return ISupport_;
	}

	void IrcServerHandler::RequestWhoIs (const QString& id, const QString& nick)
	{
		IrcParser_->WhoisCommand (QStringList () << nick);
	}

	void IrcServerHandler::RequestWhoWas (const QString& id, const QString& nick)
	{
		IrcParser_->WhowasCommand (QStringList () << nick);
	}

	void IrcServerHandler::RequestWho (const QString& id, const QString& nick)
	{
		IrcParser_->WhoCommand (QStringList () << nick);
	}

	void IrcServerHandler::connectionEstablished ()
	{
		ServerConnectionState_ = Connected;
		emit connected (ServerID_);
		ServerCLEntry_->SetStatus (EntryStatus (SOnline, QString ()));
		IrcParser_->AuthCommand ();
	}

	void IrcServerHandler::connectionClosed ()
	{
		ServerConnectionState_ = NotConnected;
		ServerCLEntry_->SetStatus (EntryStatus (SOffline, QString ()));
		Socket_->Close ();
		emit disconnected (ServerID_);
	}

	void IrcServerHandler::joinAfterInvite ()
	{
		Q_FOREACH (const QString& channel,
				InviteChannelsDialog_->GetChannels ())
		{
			ChannelOptions co;
			co.ChannelName_ = channel;
			co.ChannelPassword_ = QString ();
			co.ServerName_ = ServerOptions_.ServerName_;
			JoinChannel (co);
		}
	}

};
};
};
