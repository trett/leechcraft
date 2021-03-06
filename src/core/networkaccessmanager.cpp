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

#include "networkaccessmanager.h"
#include <stdexcept>
#include <algorithm>
#include <QNetworkRequest>
#include <QDir>
#include <QFile>
#include <QAuthenticator>
#include <QNetworkReply>
#include <QNetworkProxy>
#include <QSettings>
#include <util/util.h>
#include <util/customcookiejar.h>
#include <util/defaulthookproxy.h>
#include "core.h"
#include "networkdiskcache.h"
#include "authenticationdialog.h"
#include "sslerrorsdialog.h"
#include "xmlsettingsmanager.h"
#include "mainwindow.h"
#include "storagebackend.h"

Q_DECLARE_METATYPE (QNetworkReply*);

using namespace LeechCraft;
using namespace LeechCraft::Util;

NetworkAccessManager::NetworkAccessManager (QObject *parent)
: QNetworkAccessManager (parent)
, CookieSaveTimer_ (new QTimer ())
{
	connect (this,
			SIGNAL (authenticationRequired (QNetworkReply*,
					QAuthenticator*)),
			this,
			SLOT (handleAuthentication (QNetworkReply*,
					QAuthenticator*)));
	connect (this,
			SIGNAL (proxyAuthenticationRequired (const QNetworkProxy&,
					QAuthenticator*)),
			this,
			SLOT (handleAuthentication (const QNetworkProxy&,
					QAuthenticator*)));
	connect (this,
			SIGNAL (sslErrors (QNetworkReply*,
					const QList<QSslError>&)),
			this,
			SLOT (handleSslErrors (QNetworkReply*,
					const QList<QSslError>&)));

	XmlSettingsManager::Instance ()->RegisterObject ("FilterTrackingCookies",
			this,
			"handleFilterTrackingCookies");

	CustomCookieJar *jar = new CustomCookieJar (this);
	setCookieJar (jar);
	handleFilterTrackingCookies ();

	try
	{
		CreateIfNotExists ("core/cache");
		NetworkDiskCache *cache = new NetworkDiskCache (this);
		setCache (cache);
	}
	catch (const std::runtime_error& e)
	{
		qWarning () << Q_FUNC_INFO
			<< e.what ()
			<< "so continuing without cache";
	}

	QFile file (QDir::homePath () +
			"/.leechcraft/core/cookies.txt");
	if (file.open (QIODevice::ReadOnly))
		jar->Load (file.readAll ());
	else
		qWarning () << Q_FUNC_INFO
			<< "could not open file"
			<< file.fileName ()
			<< file.errorString ();

	connect (CookieSaveTimer_.get (),
			SIGNAL (timeout ()),
			this,
			SLOT (saveCookies ()));
	CookieSaveTimer_->start (10000);

	QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName ());
	settings.beginGroup ("NAMLocales");
	int size = settings.beginReadArray ("Locales");
	for (int i = 0; i < size; ++i)
	{
		settings.setArrayIndex (i);
		Locales_ << QLocale (settings.value ("LocaleName").toString ());
	}
	settings.endArray ();
	settings.endGroup ();
}

NetworkAccessManager::~NetworkAccessManager ()
{
	CustomCookieJar *jar = static_cast<CustomCookieJar*> (cookieJar ());
	if (!jar)
	{
		qWarning () << Q_FUNC_INFO
			<< "jar is NULL";
		return;
	}
	else
	{
		jar->CollectGarbage ();
		saveCookies ();
	}
}

QList<QLocale> NetworkAccessManager::GetAcceptLangs () const
{
	return Locales_;
}

void NetworkAccessManager::SetAcceptLangs (const QList<QLocale>& locales)
{
	Locales_ = locales;

	QStringList localesStrs;
	std::transform (locales.begin (), locales.end (), std::back_inserter (localesStrs), Util::GetInternetLocaleName);
	LocaleStr_ = localesStrs.join (", ");

	emit acceptableLanguagesChanged ();

	QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName ());
	settings.beginGroup ("NAMLocales");
	settings.beginWriteArray ("Locales");
	settings.remove ("");
	for (int i = 0; i < Locales_.size (); ++i)
	{
		settings.setArrayIndex (i);
		settings.setValue ("LocaleName", Locales_.at (i).name ());
	}
	settings.endArray ();
	settings.endGroup ();
}

QNetworkReply* NetworkAccessManager::createRequest (QNetworkAccessManager::Operation op,
		const QNetworkRequest& req, QIODevice *out)
{
	QNetworkRequest r = req;

	DefaultHookProxy_ptr proxy (new DefaultHookProxy);
	proxy->SetValue ("request", QVariant::fromValue<QNetworkRequest> (r));
	emit hookNAMCreateRequest (proxy, this, &op, &out);

	if (proxy->IsCancelled ())
		return proxy->GetReturnValue ().value<QNetworkReply*> ();

	proxy->FillValue ("request", r);

	if (r.url ().scheme ().startsWith ("http") && !LocaleStr_.isEmpty ())
		r.setRawHeader ("Accept-Language", LocaleStr_.toUtf8 ());

	if (XmlSettingsManager::Instance ()->property ("SetDNT").toBool ())
	{
		const bool dnt = XmlSettingsManager::Instance ()->property ("DNTValue").toBool ();
		r.setRawHeader ("DNT", dnt ? "1" : "0");
	}

	QNetworkReply *result = QNetworkAccessManager::createRequest (op, r, out);
	emit requestCreated (op, r, result);
	return result;
}

void LeechCraft::NetworkAccessManager::DoCommonAuth (const QString& msg, QAuthenticator *authen)
{
	QString realm = authen->realm ();

	QString suggestedUser = authen->user ();
	QString suggestedPassword = authen->password ();

	StorageBackend *backend = Core::Instance ().GetStorageBackend ();

	if (suggestedUser.isEmpty ())
		backend->GetAuth (realm, suggestedUser, suggestedPassword);

	std::auto_ptr<AuthenticationDialog> dia (
			new AuthenticationDialog (msg,
				suggestedUser,
				suggestedPassword,
				qApp->activeWindow ())
			);
	if (dia->exec () == QDialog::Rejected)
		return;

	QString login = dia->GetLogin ();
	QString password = dia->GetPassword ();
	authen->setUser (login);
	authen->setPassword (password);

	if (dia->ShouldSave ())
		backend->SetAuth (realm, login, password);
}

void LeechCraft::NetworkAccessManager::handleAuthentication (QNetworkReply *reply,
		QAuthenticator *authen)
{
	QString msg = tr ("%1<br /><em>%2</em><br />requires authentication.")
		.arg (authen->realm ())
		.arg (QApplication::fontMetrics ()
				.elidedText (reply->url ().toString (),
						Qt::ElideMiddle,
						300));

	DoCommonAuth (msg, authen);
}

void LeechCraft::NetworkAccessManager::handleAuthentication (const QNetworkProxy& proxy,
		QAuthenticator *authen)
{
	QString msg = tr ("%1<br /><em>%2</em><br />requires authentication.")
		.arg (authen->realm ())
		.arg (proxy.hostName ());

	DoCommonAuth (msg, authen);
}

void LeechCraft::NetworkAccessManager::handleSslErrors (QNetworkReply *reply,
		const QList<QSslError>& errors)
{
	QSettings settings (QCoreApplication::organizationName (),
			QCoreApplication::applicationName ());
	settings.beginGroup ("SSL exceptions");
	const auto& keys = settings.allKeys ();
	const auto& url = reply->url ();
	const auto& urlString = url.toString ();
	const auto& host = url.host ();

	if (keys.contains (urlString))
	{
		if (settings.value (urlString).toBool ())
			reply->ignoreSslErrors ();
	}
	else if (keys.contains (host))
	{
		if (settings.value (host).toBool ())
			reply->ignoreSslErrors ();
	}
	else
	{
		QPointer<QNetworkReply> repGuarded (reply);
		QString msg = tr ("<code>%1</code><br />has SSL errors."
				" What do you want to do?")
			.arg (QApplication::fontMetrics ().elidedText (urlString, Qt::ElideMiddle, 300));

		std::auto_ptr<SslErrorsDialog> errDialog (new SslErrorsDialog ());
		errDialog->Update (msg, errors);

		bool ignore = (errDialog->exec () == QDialog::Accepted);
		SslErrorsDialog::RememberChoice choice = errDialog->GetRememberChoice ();

		if (choice != SslErrorsDialog::RCNot)
		{
			if (choice == SslErrorsDialog::RCFile)
				settings.setValue (urlString, ignore);
			else
				settings.setValue (host, ignore);
		}

		if (ignore)
		{
			if (repGuarded)
				repGuarded->ignoreSslErrors ();
			else
				qWarning () << Q_FUNC_INFO
						<< "reply destructed while in errors dialog";
		}
	}
	settings.endGroup ();
}

void LeechCraft::NetworkAccessManager::saveCookies () const
{
	QDir dir = QDir::home ();
	dir.cd (".leechcraft");
	if (!dir.exists ("core") &&
			!dir.mkdir ("core"))
	{
		emit error (tr ("Could not create Core directory."));
		return;
	}

	QFile file (QDir::homePath () +
			"/.leechcraft/core/cookies.txt");
	if (!file.open (QIODevice::WriteOnly | QIODevice::Truncate))
	{
		emit error (tr ("Could not save cookies, error opening cookie file."));
		qWarning () << Q_FUNC_INFO
			<< file.errorString ();
	}
	else
	{
		CustomCookieJar *jar = static_cast<CustomCookieJar*> (cookieJar ());
		if (!jar)
		{
			qWarning () << Q_FUNC_INFO
				<< "jar is NULL";
			return;
		}
		file.write (jar->Save ());
	}
}

void LeechCraft::NetworkAccessManager::handleFilterTrackingCookies ()
{
	qobject_cast<CustomCookieJar*> (cookieJar ())->
		SetFilterTrackingCookies (XmlSettingsManager::Instance ()->
				property ("FilterTrackingCookies").toBool ());
}

