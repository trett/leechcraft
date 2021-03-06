/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011-2012  Minh Ngo
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
#include <memory>
#include <QFrame>
#include "vlcwrapper.h"

class QTime;
class QPushButton;
class QTimer;
class QSlider;

namespace LeechCraft
{
namespace Laure
{
	class VLCWrapper;
	
	/** @brief Provides a video frame.
	 * 
	 * @author Minh Ngo <nlminhtl@gmail.com>
	 */
	class Player : public QFrame
	{
		Q_OBJECT
		
		QTimer *Poller_;
		std::shared_ptr<VLCWrapper> VLCWrapper_;
	public:
		/** @brief Constructs a new PlaybackModeMenu class
		 * with the given parent.
		 */
		Player (QWidget* = 0);
		
		/** @brief Sets a libvlc wrapper.
		 * 
		 * @param[in] core VLCWrapper.
		 * 
		 * @sa VLCWrapper
		 */
		void SetVLCWrapper (std::shared_ptr<VLCWrapper> core);
		
		/** @brief Returns current media time in the QTime format.
		 */
		QTime GetTime () const;
		
		/** @brief Returns current media length in the QTime format.
		 */
		QTime GetLength () const;
		
		/** @brief Returns current media position as the track slider position.
		 * 
		 * The minimum media position is 0. The maximum position is 10000.
		 * 
		 * @sa setPosition()
		 */
		int GetPosition () const;
	public slots:
		
		/** @brief Sets media position as the track slider position.
		 * 
		 * The minimum media position is 0. The maximum position is 10000.
		 * 
		 * @param[in] pos Media postion.
		 * 
		 * @sa GetPosition()
		 */
		void setPosition (int pos);
	signals:
		/** @brief Is emitted to update the GUI interface.
		 */
		void timeout ();
	};
}
}
