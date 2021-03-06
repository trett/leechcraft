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

#include "customwebview.h"
#include <cmath>
#include <limits>
#include <qwebframe.h>
#include <QMenu>
#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QFile>
#include <QWebElement>
#include <QTextCodec>
#include <QWindowsStyle>
#include <QFileDialog>
#include <QtDebug>
#include <util/util.h>
#include <util/defaulthookproxy.h>
#include <util/xpc/stddatafiltermenucreator.h>
#include <interfaces/core/icoreproxy.h>
#include "interfaces/poshuku/poshukutypes.h"
#include "core.h"
#include "customwebpage.h"
#include "browserwidget.h"
#include "searchtext.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Poshuku
{
	CustomWebView::CustomWebView (QWidget *parent)
	: QWebView (parent)
	, ScrollTimer_ (new QTimer (this))
	, ScrollDelta_ (0)
	, AccumulatedScrollShift_ (0)
	{
		Zooms_ << 0.3
			<< 0.5
			<< 0.67
			<< 0.8
			<< 0.9
			<< 1
			<< 1.1
			<< 1.2
			<< 1.33
			<< 1.5
			<< 1.7
			<< 2
			<< 2.4
			<< 3;

		Core::Instance ().GetPluginManager ()->RegisterHookable (this);

#if QT_VERSION >= 0x040600
		QPalette p;
		if (p.color (QPalette::Window) != Qt::white)
			setPalette (QWindowsStyle ().standardPalette ());
#endif

		connect (ScrollTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (handleAutoscroll ()));

		CustomWebPage *page = new CustomWebPage (this);
		setPage (page);

		connect (this,
				SIGNAL (urlChanged (const QUrl&)),
				this,
				SLOT (remakeURL (const QUrl&)));
		connect (page,
				SIGNAL (loadingURL (const QUrl&)),
				this,
				SLOT (remakeURL (const QUrl&)));

		connect (this,
				SIGNAL (loadFinished (bool)),
				this,
				SLOT (handleLoadFinished (bool)));

		connect (page,
				SIGNAL (couldHandle (const LeechCraft::Entity&, bool*)),
				this,
				SIGNAL (couldHandle (const LeechCraft::Entity&, bool*)));
		connect (page,
				SIGNAL (gotEntity (const LeechCraft::Entity&)),
				this,
				SIGNAL (gotEntity (const LeechCraft::Entity&)));
		connect (page,
				SIGNAL (delegateEntity (const LeechCraft::Entity&, int*, QObject**)),
				this,
				SIGNAL (delegateEntity (const LeechCraft::Entity&, int*, QObject**)));
		connect (page,
				SIGNAL (printRequested (QWebFrame*)),
				this,
				SIGNAL (printRequested (QWebFrame*)));
		connect (page,
				SIGNAL (windowCloseRequested ()),
				this,
				SIGNAL (closeRequested ()));
		connect (page,
				SIGNAL (storeFormData (const PageFormsData_t&)),
				this,
				SIGNAL (storeFormData (const PageFormsData_t&)));

		QList<QByteArray> renderSettings;
		renderSettings << "PrimitivesAntialiasing"
			<< "TextAntialiasing"
			<< "SmoothPixmapTransform"
			<< "HighQualityAntialiasing";
		XmlSettingsManager::Instance ()->RegisterObject (renderSettings,
				this, "renderSettingsChanged");
		renderSettingsChanged ();
	}

	CustomWebView::~CustomWebView ()
	{
	}

	void CustomWebView::SetBrowserWidget (BrowserWidget *widget)
	{
		Browser_ = widget;
	}

	BrowserWidget* CustomWebView::GetBrowserWidget () const
	{
		return Browser_;
	}

	void CustomWebView::Load (const QString& string, QString title)
	{
		Load (Core::Instance ().MakeURL (string), title);
	}

	void CustomWebView::Load (const QUrl& url, QString title)
	{
		if (url.isEmpty () || !url.isValid ())
			return;

		if (url.scheme () == "javascript")
		{
			QVariant result = page ()->mainFrame ()->
				evaluateJavaScript (url.toString ().mid (11));
			if (result.canConvert (QVariant::String))
				setHtml (result.toString ());
			return;
		}
		if (url.scheme () == "about")
		{
			if (url.path () == "plugins")
				NavigatePlugins ();
			else if (url.path () == "home")
				NavigateHome ();
			return;
		}
		if (title.isEmpty ())
			title = tr ("Loading...");
		remakeURL (url);
		emit titleChanged (title);
		load (url);
	}

	void CustomWebView::Load (const QNetworkRequest& req,
			QNetworkAccessManager::Operation op, const QByteArray& ba)
	{
		emit titleChanged (tr ("Loading..."));
		QWebView::load (req, op, ba);
	}

	QString CustomWebView::URLToProperString (const QUrl& url)
	{
		QString string = url.toString ();
		QWebElement equivs = page ()->mainFrame ()->
				findFirstElement ("meta[http-equiv=\"Content-Type\"]");
		if (!equivs.isNull ())
		{
			QString content = equivs.attribute ("content", "text/html; charset=UTF-8");
			const QString charset = "charset=";
			int pos = content.indexOf (charset);
			if (pos >= 0)
				PreviousEncoding_ = content.mid (pos + charset.length ()).toLower ();
		}

		if (PreviousEncoding_ != "utf-8" &&
				PreviousEncoding_ != "utf8" &&
				!PreviousEncoding_.isEmpty ())
			string = url.toEncoded ();

		return string;
	}

	void CustomWebView::mousePressEvent (QMouseEvent *e)
	{
		qobject_cast<CustomWebPage*> (page ())->SetButtons (e->buttons ());
		qobject_cast<CustomWebPage*> (page ())->SetModifiers (e->modifiers ());

		const bool mBack = e->button () == Qt::XButton1;
		const bool mForward = e->button () == Qt::XButton2;
		if (mBack || mForward)
		{
			pageAction (mBack ? QWebPage::Back : QWebPage::Forward)->trigger ();
			e->accept ();
			return;
		}

		QWebView::mousePressEvent (e);
	}

	void CustomWebView::wheelEvent (QWheelEvent *e)
	{
		if (e->modifiers () & Qt::ControlModifier)
		{
			int degrees = e->delta () / 8;
			qreal delta = static_cast<qreal> (degrees) / 150;
			setZoomFactor (zoomFactor () + delta);
			e->accept ();
		}
		else
			QWebView::wheelEvent (e);
	}

	namespace
	{
		const QRegExp UrlInText ("://|www\\.|\\w\\.\\w");
	}

	void CustomWebView::contextMenuEvent (QContextMenuEvent *e)
	{
		QPointer<QMenu> menu (new QMenu ());
		const auto& r = page ()->mainFrame ()->hitTestContent (e->pos ());

		IHookProxy_ptr proxy (new Util::DefaultHookProxy ());

		emit hookWebViewContextMenu (proxy, this, e, r,
				menu, WVSStart);

		if (!r.linkUrl ().isEmpty ())
		{
			QUrl url = r.linkUrl ();
			QString text = r.linkText ();

			if (XmlSettingsManager::Instance ()->
					property ("TryToDetectRSSLinks").toBool ())
			{
				bool hasAtom = text.contains ("Atom");
				bool hasRSS = text.contains ("RSS");

				if (hasAtom || hasRSS)
				{
					LeechCraft::Entity e;
					if (hasAtom)
					{
						e.Additional_ ["UserVisibleName"] = "Atom";
						e.Mime_ = "application/atom+xml";
					}
					else
					{
						e.Additional_ ["UserVisibleName"] = "RSS";
						e.Mime_ = "application/rss+xml";
					}

					e.Entity_ = url;
					e.Parameters_ = LeechCraft::FromUserInitiated |
						LeechCraft::OnlyHandle;

					bool ch = false;
					emit couldHandle (e, &ch);
					if (ch)
					{
						QList<QVariant> datalist;
						datalist << url
							<< e.Mime_;
						menu->addAction (tr ("Subscribe"),
								this,
								SLOT (subscribeToLink ()))->setData (datalist);
						menu->addSeparator ();
					}
				}
			}

			menu->addAction (tr ("Open &here"),
					this, SLOT (openLinkHere ()))->setData (url);
			menu->addAction (tr ("Open in new &tab"),
					this, SLOT (openLinkInNewTab ()))->setData (url);
			menu->addSeparator ();
			menu->addAction (tr ("&Save link..."),
					this, SLOT (saveLink ()));

			QList<QVariant> datalist;
			datalist << url
				<< text;
			menu->addAction (tr ("&Bookmark link..."),
					this, SLOT (bookmarkLink ()))->setData (datalist);

			menu->addSeparator ();
			if (!page ()->selectedText ().isEmpty ())
				menu->addAction (pageAction (QWebPage::Copy));
			menu->addAction (tr ("&Copy link"),
					this, SLOT (copyLink ()));
			if (page ()->settings ()->testAttribute (QWebSettings::DeveloperExtrasEnabled))
				menu->addAction (pageAction (QWebPage::InspectElement));
		}
		else if (page ()->selectedText ().contains (UrlInText))
		{
			menu->addAction (tr ("Open as link"),
					this, SLOT (openLinkInNewTab ()))->
							setData (page ()->selectedText ());
		}

		emit hookWebViewContextMenu (proxy, this, e, r,
				menu, WVSAfterLink);

		if (!r.imageUrl ().isEmpty ())
		{
			if (!menu->isEmpty ())
				menu->addSeparator ();
			menu->addAction (tr ("Open image here"),
					this, SLOT (openImageHere ()))->setData (r.imageUrl ());
			menu->addAction (tr ("Open image in new tab"),
					this, SLOT (openImageInNewTab ()));
			menu->addSeparator ();
			menu->addAction (tr ("Save image..."),
					this, SLOT (saveImage ()));

			QAction *spx = menu->addAction (tr ("Save pixmap..."),
					this, SLOT (savePixmap ()));
			spx->setToolTip (tr ("Saves the rendered pixmap without redownloading."));
			spx->setProperty ("Poshuku/OrigPX", r.pixmap ());
			spx->setProperty ("Poshuku/OrigURL", r.imageUrl ());

			menu->addAction (tr ("Copy image"),
					this, SLOT (copyImage ()));
			menu->addAction (tr ("Copy image location"),
					this, SLOT (copyImageLocation ()))->setData (r.imageUrl ());
		}

		emit hookWebViewContextMenu (proxy, this, e, r,
				menu, WVSAfterImage);

		bool hasSelected = !page ()->selectedText ().isEmpty ();
		if (hasSelected)
		{
			if (!menu->isEmpty ())
				menu->addSeparator ();
			menu->addAction (pageAction (QWebPage::Copy));
		}

		if (r.isContentEditable ())
			menu->addAction (pageAction (QWebPage::Paste));

		if (hasSelected)
		{
			Browser_->Find_->setData (page ()->selectedText ());
			menu->addAction (Browser_->Find_);
			menu->addAction (tr ("Search..."),
					this, SLOT (searchSelectedText ()));
			new Util::StdDataFilterMenuCreator (page ()->selectedText (),
					Core::Instance ().GetProxy ()->GetEntityManager (), menu);
		}

		emit hookWebViewContextMenu (proxy, this, e, r,
				menu, WVSAfterSelectedText);

		if (menu->isEmpty ())
			menu = page ()->createStandardContextMenu ();

		if (!menu->isEmpty ())
			menu->addSeparator ();

		menu->addAction (Browser_->Add2Favorites_);
		menu->addSeparator ();
		menu->addAction (Browser_->Print_);
		menu->addAction (Browser_->PrintPreview_);
		menu->addSeparator ();
		menu->addAction (Browser_->ViewSources_);
		menu->addAction (Browser_->SavePage_);
		menu->addAction (Browser_->ScreenSave_);
		menu->addSeparator ();
		menu->addAction (pageAction (QWebPage::ReloadAndBypassCache));
		menu->addAction (Browser_->ReloadPeriodically_);
		menu->addAction (Browser_->NotifyWhenFinished_);
		menu->addSeparator ();
		menu->addAction (Browser_->ChangeEncoding_->menuAction ());

		emit hookWebViewContextMenu (proxy, this, e, r,
				menu, WVSAfterFinish);

		if (!menu->isEmpty ())
			menu->exec (mapToGlobal (e->pos ()));
		else
			QWebView::contextMenuEvent (e);

		if (menu)
			delete menu;
	}

	void CustomWebView::keyReleaseEvent (QKeyEvent *event)
	{
		bool handled = false;
		if (event->matches (QKeySequence::Copy))
		{
			pageAction (QWebPage::Copy)->trigger ();
			/* TODO
			const QString& text = selectedText ();
			if (!text.isEmpty ())
			{
				QApplication::clipboard ()->setText (text,
						QClipboard::Clipboard);
				handled = true;
			}
			*/
		}
		else if (event->key () == Qt::Key_F6)
			Browser_->focusLineEdit ();
		else if (event->modifiers () == Qt::SHIFT &&
				(event->key () == Qt::Key_PageUp || event->key () == Qt::Key_PageDown))
		{
			ScrollDelta_ += event->key () == Qt::Key_PageUp ? -0.1 : 0.1;
			if (!ScrollTimer_->isActive ())
				ScrollTimer_->start (30);
		}
		else if (event->modifiers () == Qt::SHIFT &&
				event->key () == Qt::Key_Plus)
		{
			ScrollDelta_ = 0;
			ScrollTimer_->stop ();
		}

		if (!handled)
			QWebView::keyReleaseEvent (event);
	}

	int CustomWebView::LevelForZoom (qreal zoom)
	{
		int i = Zooms_.indexOf (zoom);

		if (i >= 0)
			return i;

		for (i = 0; i < Zooms_.size (); ++i)
			if (zoom <= Zooms_ [i])
				break;

		if (i == Zooms_.size ())
			return i - 1;

		if (i == 0)
			return i;

		if (zoom - Zooms_ [i - 1] > Zooms_ [i] - zoom)
			return i;
		else
			return i - 1;
	}

	void CustomWebView::NavigatePlugins ()
	{
		QFile pef (":/resources/html/pluginsenum.html");
		pef.open (QIODevice::ReadOnly);
		QString contents = QString (pef.readAll ())
			.replace ("INSTALLEDPLUGINS", tr ("Installed plugins"))
			.replace ("NOPLUGINS", tr ("No plugins installed"))
			.replace ("FILENAME", tr ("File name"))
			.replace ("MIME", tr ("MIME type"))
			.replace ("DESCR", tr ("Description"))
			.replace ("SUFFIXES", tr ("Suffixes"))
			.replace ("ENABLED", tr ("Enabled"))
			.replace ("NO", tr ("No"))
			.replace ("YES", tr ("Yes"));
		setHtml (contents);
	}

	void CustomWebView::NavigateHome ()
	{
		QFile file (":/resources/html/home.html");
		file.open (QIODevice::ReadOnly);
		QString data = file.readAll ();
		data.replace ("{pagetitle}",
				tr ("Welcome to LeechCraft!"));
		data.replace ("{title}",
				tr ("Welcome to LeechCraft!"));
		data.replace ("{body}",
				tr ("Welcome to LeechCraft, the integrated internet-client.<br />"
					"More info is available on the <a href='http://leechcraft.org'>"
					"project's site</a>."));

		QBuffer iconBuffer;
		iconBuffer.open (QIODevice::ReadWrite);
		QPixmap pixmap ("lcicons:/resources/images/poshuku.svg");
		pixmap.save (&iconBuffer, "PNG");

		data.replace ("{img}",
				QByteArray ("data:image/png;base64,") + iconBuffer.buffer ().toBase64 ());

		setHtml (data);
	}

	void CustomWebView::zoomIn ()
	{
		int i = LevelForZoom (zoomFactor ());

		if (i < Zooms_.size () - 1)
			setZoomFactor (Zooms_ [i + 1]);

		emit invalidateSettings ();
	}

	void CustomWebView::zoomOut ()
	{
		int i = LevelForZoom (zoomFactor ());

		if (i > 0)
			setZoomFactor (Zooms_ [i - 1]);

		emit invalidateSettings ();
	}

	void CustomWebView::zoomReset ()
	{
		setZoomFactor (1);

		emit invalidateSettings ();
	}

	void CustomWebView::remakeURL (const QUrl& url)
	{
		emit urlChanged (URLToProperString (url));
	}

	void CustomWebView::handleLoadFinished (bool ok)
	{
		if (ok)
			remakeURL (url ());
	}

	void CustomWebView::openLinkHere ()
	{
		Load (qobject_cast<QAction*> (sender ())->data ().toUrl ());
	}

	void CustomWebView::openLinkInNewTab ()
	{
		CustomWebView *view = Core::Instance ().MakeWebView (false);
		view->Load (qobject_cast<QAction*> (sender ())->data ().toString ());
	}

	void CustomWebView::saveLink ()
	{
		pageAction (QWebPage::DownloadLinkToDisk)->trigger ();
	}

	void CustomWebView::subscribeToLink ()
	{
		const auto& list = qobject_cast<QAction*> (sender ())->data ().toList ();
		Entity e = Util::MakeEntity (list.at (0),
				QString (),
				FromUserInitiated | OnlyHandle,
				list.at (1).toString ());
		emit gotEntity (e);
	}

	void CustomWebView::bookmarkLink ()
	{
		const auto& list = qobject_cast<QAction*> (sender ())->data ().toList ();
		emit addToFavorites (list.at (1).toString (),
				list.at (0).toUrl ().toString ());
	}

	void CustomWebView::copyLink ()
	{
		pageAction (QWebPage::CopyLinkToClipboard)->trigger ();
	}

	void CustomWebView::openImageHere ()
	{
		Load (qobject_cast<QAction*> (sender ())->data ().toUrl ());
	}

	void CustomWebView::openImageInNewTab ()
	{
		pageAction (QWebPage::OpenImageInNewWindow)->trigger ();
	}

	void CustomWebView::saveImage ()
	{
		pageAction (QWebPage::DownloadImageToDisk)->trigger ();
	}

	void CustomWebView::savePixmap ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
					<< "sender is not an action"
					<< sender ();
			return;
		}

		const QPixmap& px = action->property ("Poshuku/OrigPX").value<QPixmap> ();
		if (px.isNull ())
			return;

		const QUrl& url = action->property ("Poshuku/OrigURL").value<QUrl> ();
		const QString origName = url.scheme () == "data" ?
				QString () :
				QFileInfo (url.path ()).fileName ();

		QString filter;
		QString fname = QFileDialog::getSaveFileName (0,
				tr ("Save pixmap"),
				QDir::homePath () + '/' + origName,
				tr ("PNG image (*.png);;JPG image (*.jpg);;All files (*.*)"),
				&filter);

		if (fname.isEmpty ())
			return;

		if (QFileInfo (fname).suffix ().isEmpty ())
		{
			if (filter.contains ("png"))
				fname += ".png";
			else if (filter.contains ("jpg"))
				fname += ".jpg";
		}

		QFile file (fname);
		if (!file.open (QIODevice::WriteOnly))
		{
			emit gotEntity (Util::MakeNotification ("Poshuku",
						tr ("Unable to save the image. Unable to open file for writing: %1.")
							.arg (file.errorString ()),
						PCritical_));
			return;
		}

		const QString& suf = QFileInfo (fname).suffix ();
		const bool isLossless = suf.toLower () == "png";
		px.save (&file,
				suf.toUtf8 ().constData (),
				isLossless ? 0 : 100);
	}

	void CustomWebView::copyImage ()
	{
		pageAction (QWebPage::CopyImageToClipboard)->trigger ();
	}

	void CustomWebView::copyImageLocation ()
	{
		QString url = qobject_cast<QAction*> (sender ())->data ().toUrl ().toString ();
		QClipboard *cb = QApplication::clipboard ();
		cb->setText (url, QClipboard::Clipboard);
		cb->setText (url, QClipboard::Selection);
	}

	void CustomWebView::searchSelectedText ()
	{
		QString text = page ()->selectedText ();
		if (text.isEmpty ())
			return;

		SearchText *st = new SearchText (text, Browser_);
		connect (st,
				SIGNAL (gotEntity (const LeechCraft::Entity&)),
				this,
				SIGNAL (gotEntity (const LeechCraft::Entity&)));
		st->setAttribute (Qt::WA_DeleteOnClose);
		st->show ();
	}

	void CustomWebView::renderSettingsChanged ()
	{
#if QT_VERSION >= 0x040800
		QPainter::RenderHints hints;
		if (XmlSettingsManager::Instance ()->
				property ("PrimitivesAntialiasing").toBool ())
			hints |= QPainter::Antialiasing;
		if (XmlSettingsManager::Instance ()->
				property ("TextAntialiasing").toBool ())
			hints |= QPainter::TextAntialiasing;
		if (XmlSettingsManager::Instance ()->
				property ("SmoothPixmapTransform").toBool ())
			hints |= QPainter::SmoothPixmapTransform;
		if (XmlSettingsManager::Instance ()->
				property ("HighQualityAntialiasing").toBool ())
			hints |= QPainter::HighQualityAntialiasing;

		setRenderHints (hints);
#endif
	}

	void CustomWebView::handleAutoscroll ()
	{
		if (std::fabs (ScrollDelta_) < std::numeric_limits<decltype (ScrollDelta_)>::epsilon ())
			return;

		AccumulatedScrollShift_ += ScrollDelta_;

		if (std::abs (AccumulatedScrollShift_) >= 1)
		{
			QWebFrame *mf = page ()->mainFrame ();
			QPoint pos = mf->scrollPosition ();
			pos += QPoint (0, AccumulatedScrollShift_);
			mf->setScrollPosition (pos);

			AccumulatedScrollShift_ -= static_cast<int> (AccumulatedScrollShift_);
		}
	}
}
}
