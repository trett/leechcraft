/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include "postoptionswidget.h"
#include "entryoptions.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	PostOptionsWidget::PostOptionsWidget (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);

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

		QStringList tags;
		for (auto tag : Ui_.Tags_->text ().split (","))
			tags << tag.trimmed ();
		map ["tags"] = tags;
		map ["time"] = !Ui_.TimestampBox_->isChecked () ?
			QDateTime::currentDateTime () :
			QDateTime (QDate (Ui_.Year_->value (),
						Ui_.Month_->currentIndex (),
						Ui_.Date_->value ()),
					Ui_.Time_->time ());
		map ["access"] = Ui_.Access_->itemData (Ui_.Access_->currentIndex (),
				Qt::UserRole);
		//TODO mood
// 		map ["mood"]
		map ["place"] = Ui_.Place_->text ();
		map ["music"] = Ui_.Music_->text ();
		map ["comment"] = Ui_.Comments_->itemData (Ui_.Comments_->currentIndex (),
				Qt::UserRole);
		map ["hidecomment"] = Ui_.ScreenComments_->
				itemData (Ui_.ScreenComments_->currentIndex (), Qt::UserRole);
		map ["adults"] =Ui_.Adult_->itemData (Ui_.Adult_->currentIndex (),
				Qt::UserRole);

		return map;
	}

	QVariantMap PostOptionsWidget::GetCustomData () const
	{
		return QVariantMap ();
	}

	void PostOptionsWidget::FillItems ()
	{
		Ui_.Access_->addItem (tr ("Public"), Access::Public);
		Ui_.Access_->addItem (tr ("Friends only"), Access::FriendsOnly);
		Ui_.Access_->addItem (tr ("Private"), Access::Private);
		Ui_.Access_->addItem (tr ("Custom"), Access::Custom);

		Ui_.Comments_->addItem (tr ("Enable"), CommentsManagement::EnableComments);
		Ui_.Comments_->addItem (tr ("Disable"), CommentsManagement::DisableComments);
		Ui_.Comments_->addItem (tr ("Don't notify"), CommentsManagement::WithoutNotification);

		Ui_.ScreenComments_->addItem (tr ("Default"), CommentsManagement::Default);
		Ui_.ScreenComments_->addItem (tr ("Anonymouse only"),
				CommentsManagement::ScreenAnonymouseComments);
		Ui_.ScreenComments_->addItem (tr ("Not from friends"),
				CommentsManagement::ShowFriendsComments);
		Ui_.ScreenComments_->addItem (tr ("Don't hide'"),
				CommentsManagement::ShowComments);
		Ui_.ScreenComments_->addItem (tr ("All"),
				CommentsManagement::ScreenComments);

		Ui_.Adult_->addItem (tr ("Without adult content"),
				AdultContent::WithoutAdultContent);
		Ui_.Adult_->addItem (tr ("For adults (>14)"), AdultContent::AdultsFrom14);
		Ui_.Adult_->addItem (tr ("For adults (>18)"), AdultContent::AdultsFrom18);
	}


	void PostOptionsWidget::on_CurrentTime__released ()
	{
		QDateTime current = QDateTime::currentDateTime ();
		Ui_.Year_->setValue (current.date ().year ());
		Ui_.Month_->setCurrentIndex (current.date ().month () - 1);
		Ui_.Date_->setValue (current.date ().day ());
		Ui_.Time_->setTime (current.time ());
	}

}
}
}
