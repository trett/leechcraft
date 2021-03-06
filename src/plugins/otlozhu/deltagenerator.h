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

#pragma once

#include <QObject>
#include <QSettings>
#include <interfaces/isyncable.h>
#include "todoitem.h"

namespace LeechCraft
{
namespace Otlozhu
{
	class DeltaGenerator : public QObject
	{
		Q_OBJECT

		QSettings Settings_;
		bool IsEnabled_;
		bool IsMerging_;

		QStringList NewItems_;
		QHash<QString, QVariantMap> Diffs_;
		QStringList RemovedItems_;
	public:
		enum DeltaType
		{
			TodoCreated,
			TodoUpdated,
			TodoRemoved
		};

		DeltaGenerator (QObject* = 0);

		void BeginRecording ();

		Sync::Payloads_t GetAllDeltas ();
		Sync::Payloads_t GetNewDeltas ();
		void PurgeDeltas (quint32 num);
		void Apply (const Sync::Payloads_t&);
	private:
		void ApplyCreated (QDataStream&);
		void ApplyUpdated (QDataStream&);
		void ApplyRemoved (QDataStream&);
	private slots:
		void handleItemAdded (int);
		void handleItemRemoved (int);
		void handleItemDiffGenerated (const QString&, const QVariantMap&);
	};
}
}
