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

#include "historymanager.h"

namespace LeechCraft
{
namespace HotSensors
{
	HistoryManager::HistoryManager (QObject *parent)
	: QObject (parent)
	{
	}

	void HistoryManager::handleReadings (const Readings_t& readings)
	{
		for (auto i = History_.begin (); i != History_.end (); )
		{
			const auto pos = std::find_if (readings.begin (), readings.end (),
					[i] (const Reading& r) { return r.Name_ == i.key (); });
			if (pos == readings.end ())
				i = History_.erase (i);
			else
				++i;
		}

		for (const auto& r : readings)
		{
			auto& vec = History_ [r.Name_];
			vec << r.Value_;
			if (vec.size () >= 100)
				vec.pop_front ();
		}

		emit historyChanged (History_);
	}
}
}
