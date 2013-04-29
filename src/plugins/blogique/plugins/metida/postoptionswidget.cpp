/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include "postoptionswidget.h"
#include <QtDebug>
#include <util/util.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/media/icurrentsongkeeper.h>
#include "entryoptions.h"
#include "ljaccount.h"
#include "ljprofile.h"
#include "selectgroupsdialog.h"
#include "core.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	PostOptionsWidget::PostOptionsWidget (QWidget *parent)
	: QWidget (parent)
	, Account_ (0)
	, AllowMask_ (0)
	{
		Ui_.setupUi (this);
		XmlSettingsManager::Instance ().RegisterObject ("AutoUpdateCurrentMusic",
				this, "handleAutoApdateCurrentMusic");
		handleAutoApdateCurrentMusic ();
		FillItems ();
	}

	QString PostOptionsWidget::GetName () const
	{
		return tr ("Post options");
	}

	SideWidgetType PostOptionsWidget::GetWidgetType () const
	{
		return SideWidgetType::PostOptionsSideWidget;
	}

	QVariantMap PostOptionsWidget::GetPostOptions () const
	{
		QVariantMap map;

		map ["access"] = Ui_.Access_->itemData (Ui_.Access_->currentIndex (),
				Qt::UserRole);
		map ["allowMask"] = AllowMask_;

		const bool customMood = (Ui_.Mood_->itemData (Ui_.Mood_->currentIndex ()) == QVariant::Invalid);
		if (!customMood)
			map ["moodId"] = Ui_.Mood_->itemData (Ui_.Mood_->currentIndex ()).toInt ();
		else
		{
			map ["mood"] = Ui_.Mood_->currentText ();
			map ["moodId"] = -1;
		}

		map ["place"] = Ui_.Place_->text ();
		map ["music"] = Ui_.Music_->text ();
		map ["comment"] = Ui_.Comments_->itemData (Ui_.Comments_->currentIndex (),
				Qt::UserRole);
		map ["notify"] = Ui_.NotifyAboutComments_->isChecked ();
		map ["hidecomment"] = Ui_.ScreenComments_->
				itemData (Ui_.ScreenComments_->currentIndex (), Qt::UserRole);
		map ["adults"] = Ui_.Adult_->itemData (Ui_.Adult_->currentIndex (),
				Qt::UserRole);
		map ["showInFriendsPage"] = Ui_.ShowInFriendsPage_->isChecked ();
		if (Ui_.UserPic_->currentIndex ())
			map ["avatar"] = Ui_.UserPic_->currentText ();

		return map;
	}

	void PostOptionsWidget::SetPostOptions (const QVariantMap& map)
	{
		if (map.contains ("access"))
		{
			const QVariant& access = map ["access"];
			for (int i = 0; i < Ui_.Access_->count (); ++i)
				if (Ui_.Access_->itemData (i, Qt::UserRole) == access)
				{
					Ui_.Access_->setCurrentIndex (i);
					break;
				}
		}
		else
			Ui_.Access_->setCurrentIndex (0);

		if (Ui_.Access_->itemData (Ui_.Access_->currentIndex ()) != Access::Private)
			Ui_.ShowInFriendsPage_->setChecked (map.contains ("showInFriendsPage") ?
				map ["showInFriendsPage"].toBool () : true);
		else
			Ui_.ShowInFriendsPage_->setChecked (false);

		//TODO AllowMask_

		const QString& mood = map ["mood"].toString ();
		if (!mood.isEmpty ())
		{
			Ui_.Mood_->addItem (mood);
			Ui_.Mood_->setCurrentIndex (Ui_.Mood_->count () - 1);
		}
		else
		{
			int moodId = map ["moodId"].toInt ();
			for (int i = 0; i < Ui_.Mood_->count (); ++i)
				if (Ui_.Mood_->itemData (i).toInt () == moodId)
				{
					Ui_.Mood_->setCurrentIndex (i);
					break;
				}
		}

		Ui_.Place_->setText (map ["place"].toString ());
		Ui_.Music_->setText (map ["music"].toString ());

		if (map.contains ("comment"))
		{
			const QVariant& comment = map ["comment"];
			for (int i = 0; i < Ui_.Comments_->count (); ++i)
				if (Ui_.Comments_->itemData (i, Qt::UserRole) == comment)
				{
					Ui_.Comments_->setCurrentIndex (i);
					break;
				}
		}
		else
			Ui_.Comments_->setCurrentIndex (0);

		if (map.contains ("hidecomments"))
		{
			const QVariant& hideComments = map ["hidecomment"];
			for (int i = 0; i < Ui_.ScreenComments_->count (); ++i)
				if (Ui_.ScreenComments_->itemData (i, Qt::UserRole) == hideComments)
				{
					Ui_.ScreenComments_->setCurrentIndex (i);
					break;
				}
		}
		else
			Ui_.ScreenComments_->setCurrentIndex (0);

		if (map.contains ("adults"))
		{
			const QVariant& adults = map ["adults"];
			for (int i = 0; i < Ui_.Adult_->count (); ++i)
				if (Ui_.Adult_->itemData (i, Qt::UserRole) == adults)
				{
					Ui_.Adult_->setCurrentIndex (i);
					break;
				}
		}
		else
			Ui_.Adult_->setCurrentIndex (0);

		Ui_.UserPic_->setCurrentIndex (!map ["avatar"].toString ().isEmpty () ?
			Ui_.UserPic_->findText (map ["avatar"].toString ()) :
			0);
		Ui_.NotifyAboutComments_->setChecked (map.contains ("notify") ?
			map ["notify"].toBool () :
			true);
	}

	QVariantMap PostOptionsWidget::GetCustomData () const
	{
		return QVariantMap ();
	}

	void PostOptionsWidget::SetCustomData (const QVariantMap&)
	{
	}

	void PostOptionsWidget::SetAccount (QObject *accObj)
	{
		Account_ = qobject_cast<LJAccount*> (accObj);
		if (!Account_)
		{
			qWarning () << Q_FUNC_INFO
					<< "account"
					<< accObj
					<< "doesn't belong to LiveJournal";
			return;
		}

		LJProfile *profile = qobject_cast<LJProfile*> (Account_->GetProfile ());
		if (!profile)
			return;

		Ui_.Mood_->clear ();
		Ui_.Mood_->addItem (QString ());
		for (const auto& mood : profile->GetProfileData ().Moods_)
			Ui_.Mood_->addItem (mood.Name_, mood.Id_);

		Ui_.UserPic_->addItem (tr ("(default)"));
		const QString& path = Util::CreateIfNotExists ("blogique/metida/avatars")
				.absoluteFilePath (Account_->GetAccountID ().toBase64 ().replace ('/', '_'));
		QPixmap pxm (path);
		Ui_.UserPicLabel_->setPixmap (pxm.scaled (64, 64));

		Ui_.UserPic_->addItems (profile->GetProfileData ().AvatarsID_);
	}

	QStringList PostOptionsWidget::GetTags () const
	{
		QStringList tags;
		for (auto tag : Ui_.Tags_->text ().split (","))
			tags << tag.trimmed ();
		return tags;
	}

	void PostOptionsWidget::SetTags (const QStringList& tags)
	{
		Ui_.Tags_->setText (tags.join (", "));
	}

	QDateTime PostOptionsWidget::GetPostDate () const
	{
		return !Ui_.TimestampBox_->isChecked () ?
				QDateTime::currentDateTime () :
				QDateTime (QDate (Ui_.Year_->value (),
							Ui_.Month_->currentIndex () + 1,
							Ui_.Date_->value ()),
						Ui_.Time_->time ());
	}

	void PostOptionsWidget::SetPostDate (const QDateTime& date)
	{
		Ui_.TimestampBox_->setChecked (true);
		Ui_.Year_->setValue (date.date ().year ());
		Ui_.Month_->setCurrentIndex (date.date ().month () - 1);
		Ui_.Date_->setValue (date.date ().day ());
		Ui_.Time_->setTime (date.time ());
	}

	void PostOptionsWidget::FillItems ()
	{
		Ui_.Access_->addItem (tr ("Public"), Access::Public);
		Ui_.Access_->addItem (tr ("Friends only"), Access::FriendsOnly);
		Ui_.Access_->addItem (tr ("Private"), Access::Private);
		Ui_.Access_->addItem (tr ("Custom"), Access::Custom);

		Ui_.Comments_->addItem (tr ("Enable"), CommentsManagement::EnableComments);
		Ui_.Comments_->addItem (tr ("Disable"), CommentsManagement::DisableComments);

		Ui_.ScreenComments_->addItem (tr ("Default"), CommentsManagement::Default);
		Ui_.ScreenComments_->addItem (tr ("Anonymouse only"),
				CommentsManagement::ScreenAnonymouseComments);
		Ui_.ScreenComments_->addItem (tr ("Not from friends"),
				CommentsManagement::ShowFriendsComments);
		Ui_.ScreenComments_->addItem (tr ("Not from friends with links"),
				CommentsManagement::ScreenNotFromFriendsWithLinks);
		Ui_.ScreenComments_->addItem (tr ("Don't hide'"),
				CommentsManagement::ShowComments);
		Ui_.ScreenComments_->addItem (tr ("All"),
				CommentsManagement::ScreenComments);

		Ui_.Adult_->addItem (tr ("Without adult content"),
				AdultContent::WithoutAdultContent);
		Ui_.Adult_->addItem (tr ("For adults (>14)"), AdultContent::AdultsFrom14);
		Ui_.Adult_->addItem (tr ("For adults (>18)"), AdultContent::AdultsFrom18);
	}

	namespace
	{
		Media::ICurrentSongKeeper* GetFirstICurrentSongKeeperInstance ()
		{
			auto plugins = Core::Instance ().GetCoreProxy ()->GetPluginsManager ()->
					GetAllCastableTo<Media::ICurrentSongKeeper*> ();
			return !plugins.count () ?
				0 :
				plugins.at (0);
		}
	}

	void PostOptionsWidget::handleAutoApdateCurrentMusic ()
	{
		if (XmlSettingsManager::Instance ().Property ("AutoUpdateCurrentMusic", false).toBool ())
			connect (GetFirstICurrentSongKeeperInstance ()->GetQObject (),
					SIGNAL (currentSongChanged (AudioInfo)),
					this,
					SLOT (handleCurrentSongChanged (AudioInfo)),
					Qt::UniqueConnection);
	}

	void PostOptionsWidget::on_CurrentTime__released ()
	{
		QDateTime current = QDateTime::currentDateTime ();
		Ui_.Year_->setValue (current.date ().year ());
		Ui_.Month_->setCurrentIndex (current.date ().month () - 1);
		Ui_.Date_->setValue (current.date ().day ());
		Ui_.Time_->setTime (current.time ());
	}

	void PostOptionsWidget::on_Access__activated (int index)
	{
		if (static_cast<Access> (Ui_.Access_->itemData (index).toInt ()) == Access::Custom)
		{
			SelectGroupsDialog dlg (qobject_cast<LJProfile*> (Account_->GetProfile ()),
					AllowMask_);

			if (dlg.exec () == QDialog::Rejected ||
					dlg.GetSelectedGroupsIds ().isEmpty ())
				Ui_.Access_->setCurrentIndex (0);
			else
				for (uint num : dlg.GetSelectedGroupsIds ())
					AllowMask_ |= 1 << num;
		}
	}

	void PostOptionsWidget::on_UserPic__currentIndexChanged (int index)
	{
		QString path = index ?
			Util::CreateIfNotExists ("blogique/metida/avatars")
				.absoluteFilePath ((Account_->GetAccountID () +
					Ui_.UserPic_->itemText (index).toUtf8 ())
						.toBase64 ().replace ('/', '_')) :
			Util::CreateIfNotExists ("blogique/metida/avatars")
						.absoluteFilePath ((Account_->GetAccountID ())
								.toBase64 ().replace ('/', '_'));
		QPixmap pxm (path);
		Ui_.UserPicLabel_->setPixmap (pxm.scaled (pxm.width (), pxm.height ()));
	}

	void PostOptionsWidget::on_AutoDetect__released ()
	{
		auto plugin = GetFirstICurrentSongKeeperInstance ();
		if (!plugin)
			return;

		const auto& song = plugin->GetCurrentSong ();
		Ui_.Music_->setText (QString ("\"%1\" by %2").arg (song.Title_)
				.arg (song.Artist_));
	}

	void PostOptionsWidget::handleCurrentSongChanged (const Media::AudioInfo& ai)
	{
		if (XmlSettingsManager::Instance ().Property ("AutoUpdateCurrentMusic", false).toBool ())
			Ui_.Music_->setText (QString ("\"%1\" by %2").arg (ai.Title_)
					.arg (ai.Artist_));
	}

}
}
}
