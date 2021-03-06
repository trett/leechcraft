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

#include "newtorrentwizard.h"
#include "intropage.h"
#include "firststep.h"
#include "secondstep.h"
#include "thirdstep.h"

namespace LeechCraft
{
namespace Plugins
{
namespace BitTorrent
{
	NewTorrentWizard::NewTorrentWizard (QWidget *parent)
	: QWizard (parent)
	{
		setWindowTitle (tr ("New torrent wizard"));
		setWizardStyle (QWizard::ModernStyle);

		setPage (PageIntro, new IntroPage);
		setPage (PageFirstStep, new FirstStep);
		setPage (PageSecondStep, new ThirdStep);
	}

	void NewTorrentWizard::accept ()
	{
		QWizard::accept ();
	}

	NewTorrentParams NewTorrentWizard::GetParams () const
	{
		NewTorrentParams result;

		result.Output_ = field ("Output").toString ();
		result.AnnounceURL_ = field ("AnnounceURL").toString ();
		result.Date_ = field ("Date").toDate ();
		result.Comment_ = field ("Comment").toString ();
		result.Path_ = field ("RootPath").toString ();
		result.URLSeeds_ = field ("URLSeeds").toString ().split (QRegExp("\\s+"));
		result.DHTEnabled_ = field ("DHTEnabled").toBool ();
		result.DHTNodes_ = field ("DHTNodes").toString ().split (QRegExp("\\s+"));
		result.PieceSize_ = 32 * 1024;
		int index = field ("PieceSize").toInt ();
		while (index--)
			result.PieceSize_ *= 2;

		if (result.Path_.endsWith ('/'))
			result.Path_.remove (result.Path_.size () - 1, 1);

		return result;
	}
}
}
}
