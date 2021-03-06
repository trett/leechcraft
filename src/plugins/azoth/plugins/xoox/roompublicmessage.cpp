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

#include "roompublicmessage.h"
#include <QTextDocument>
#include <QtDebug>
#include <QXmppMessage.h>
#include <QXmppClient.h>
#include "roomclentry.h"
#include "roomparticipantentry.h"
#include "glooxaccount.h"
#include "clientconnection.h"
#include "roomhandler.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	RoomPublicMessage::RoomPublicMessage (const QString& msg, RoomCLEntry *entry)
	: QObject (entry)
	, ParentEntry_ (entry)
	, Message_ (msg)
	, Datetime_ (QDateTime::currentDateTime ())
	, Direction_ (DOut)
	, Type_ (MTMUCMessage)
	, SubType_ (MSTOther)
	{
	}

	RoomPublicMessage::RoomPublicMessage (const QString& msg,
			IMessage::Direction direction,
			RoomCLEntry *entry,
			IMessage::MessageType type,
			IMessage::MessageSubType subType,
			RoomParticipantEntry_ptr part)
	: QObject (entry)
	, ParentEntry_ (entry)
	, ParticipantEntry_ (part)
	, Message_ (msg)
	, Datetime_ (QDateTime::currentDateTime ())
	, Direction_ (direction)
	, Type_ (type)
	, SubType_ (subType)
	{
	}

	RoomPublicMessage::RoomPublicMessage (const QXmppMessage& msg,
			RoomCLEntry *entry,
			RoomParticipantEntry_ptr partEntry)
	: QObject (entry)
	, ParentEntry_ (entry)
	, ParticipantEntry_ (partEntry)
	, Message_ (msg.body ())
	, Datetime_ (msg.stamp ().isValid () ? msg.stamp ().toLocalTime () : QDateTime::currentDateTime ())
	, Direction_ (DIn)
	, Type_ (MTMUCMessage)
	, SubType_ (MSTOther)
	, XHTML_ (msg.xhtml ())
	{
		ClientConnection::Split (msg.from (), &FromJID_, &FromVariant_);
	}

	QObject* RoomPublicMessage::GetQObject ()
	{
		return this;
	}

	void RoomPublicMessage::Send ()
	{
		if (!ParentEntry_)
			return;

		QXmppClient *client =
				qobject_cast<GlooxAccount*> (ParentEntry_->GetParentAccount ())->
						GetClientConnection ()->GetClient ();

		QXmppMessage msg;
		msg.setBody (Message_);
		msg.setTo (ParentEntry_->GetRoomHandler ()->GetRoomJID ());
		msg.setType (QXmppMessage::GroupChat);
		msg.setXhtml (XHTML_);
		client->sendPacket (msg);
	}

	void RoomPublicMessage::Store ()
	{
		if (!ParentEntry_)
			return;

		ParentEntry_->HandleMessage (this);
	}

	IMessage::Direction RoomPublicMessage::GetDirection () const
	{
		return Direction_;
	}

	IMessage::MessageType RoomPublicMessage::GetMessageType () const
	{
		return Type_;
	}

	IMessage::MessageSubType RoomPublicMessage::GetMessageSubType () const
	{
		return SubType_;
	}

	QObject* RoomPublicMessage::OtherPart () const
	{
		switch (Direction_)
		{
		case DIn:
			return ParticipantEntry_.get ();
		case DOut:
			return ParentEntry_;
		default:
			qWarning () << Q_FUNC_INFO
					<< "unknown direction"
					<< Direction_;
			return ParentEntry_;
		}
	}

	QObject* RoomPublicMessage::ParentCLEntry () const
	{
		return ParentEntry_;
	}

	QString RoomPublicMessage::GetOtherVariant() const
	{
		return FromVariant_;
	}

	QString RoomPublicMessage::GetBody () const
	{
		return Qt::escape (Message_);
	}

	void RoomPublicMessage::SetBody (const QString& msg)
	{
		Message_ = msg;
	}

	QDateTime RoomPublicMessage::GetDateTime () const
	{
		return Datetime_;
	}

	void RoomPublicMessage::SetDateTime (const QDateTime& dt)
	{
		Datetime_ = dt;
	}

	QString RoomPublicMessage::GetRichBody () const
	{
		return XHTML_;
	}

	void RoomPublicMessage::SetRichBody (const QString& xhtml)
	{
		XHTML_ = xhtml;
	}
}
}
}
