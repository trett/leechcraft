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

#include "syncitemdelegate.h"
#include <QComboBox>
#include <QMessageBox>
#include <QtDebug>
#include <QDir>
#include "accountsmanager.h"
#include "directorywidget.h"
#include "interfaces/netstoremanager/istorageaccount.h"
#include "interfaces/netstoremanager/istorageplugin.h"

namespace LeechCraft
{
namespace NetStoreManager
{
	SyncItemDelegate::SyncItemDelegate (AccountsManager *am, QObject *parent)
	: QItemDelegate (parent)
	, AM_ (am)
	{
	}

	QWidget* SyncItemDelegate::createEditor (QWidget *parent,
			const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		switch (index.column ())
		{
		case Account:
		{
			QComboBox *box = new QComboBox (parent);
			FillAccounts (box);
			return box;
		}
		case Directory:
		{
			DirectoryWidget *dw = new DirectoryWidget (parent);
			connect (dw,
					SIGNAL (finished (QWidget*)),
					this,
					SLOT (handleCloseDirectoryEditor (QWidget*)));
			return dw;
		}
		default:
			return QItemDelegate::createEditor (parent, option, index);
		}
	}

	void SyncItemDelegate::setEditorData (QWidget *editor,
			const QModelIndex& index) const
	{
		switch (index.column ())
		{
		case Account:
		{
			auto box = static_cast<QComboBox*> (editor);
			QString accText = index.data (Qt::EditRole).toString ();
			box->setCurrentIndex (box->findText (accText, Qt::MatchExactly));
			break;
		}
		case Directory:
		{
			auto dw = static_cast<DirectoryWidget*> (editor);
			dw->SetPath (index.data (Qt::EditRole).toString ());
			break;
		}
		default:
			QItemDelegate::setEditorData (editor, index);
		}
	}

	void SyncItemDelegate::setModelData (QWidget *editor,
			QAbstractItemModel *model, const QModelIndex& index) const
	{
		switch (index.column ())
		{
		case Account:
		{
			auto box = static_cast<QComboBox*> (editor);

			model->setData (index, box->currentText (), Qt::EditRole);
			model->setData (index,
					box->itemData (box->currentIndex (),
							SyncItemDelegateRoles::AccountId),
					SyncItemDelegateRoles::AccountId);
			break;
		}
		case Directory:
		{
			auto dw = static_cast<DirectoryWidget*> (editor);
			model->setData (index, dw->GetPath (), Qt::EditRole);
			break;
		}
		default:
			QItemDelegate::setModelData (editor, model, index);
		}
	}

	void SyncItemDelegate::updateEditorGeometry (QWidget *editor,
			const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		editor->setGeometry (option.rect);
	}

	void SyncItemDelegate::FillAccounts (QComboBox *box) const
	{
		const auto& accounts = AM_->GetAccounts ();
		for (auto acc : accounts)
		{
			auto isp = qobject_cast<IStoragePlugin*> (acc->GetParentPlugin ());
			if (!isp)
				continue;

			box->addItem (isp->GetStorageIcon (),
					isp->GetStorageName() + ": " + acc->GetAccountName ());
			box->setItemData (box->count () - 1,
					acc->GetUniqueID (),
					SyncItemDelegateRoles::AccountId);
		}
	}

	void SyncItemDelegate::handleCloseDirectoryEditor (QWidget *w)
	{
		emit commitData (w);
		emit closeEditor (w);
	}

}
}
