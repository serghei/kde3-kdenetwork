/*
 * newssourcedlgimpl.cpp
 *
 * Copyright (c) 2001 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#include "newssourcedlgimpl.h"
#include "xmlnewsaccess.h"
#include "configaccess.h"
#include "newsiconmgr.h"

#include <kcombobox.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kurlrequester.h>

#include <qcheckbox.h>
#include <qlabel.h>
#include <qprogressbar.h>
#include <qtimer.h>
#include <qvbox.h>

SuggestProgressDlg::SuggestProgressDlg(const KURL &url, QWidget *parent, const char *name)
	: KDialogBase(parent, name, true, i18n("Downloading Data"), Cancel, Cancel),
	m_gotSourceFile(false),
	m_gotIcon(false)
{
	QVBox *mainWidget = makeVBoxMainWidget();

	new QLabel(i18n("<qt>Please wait while KNewsTicker is downloading some "
	                "data necessary to suggest reasonable values.<br/>"
	                "<br/>"
	                "This will not take longer than one minute.</qt>" ),
	                mainWidget);

	m_progressBar = new QProgressBar(60, mainWidget);
	m_progressBar->setPercentageVisible(false);

	m_timeoutTimer = new QTimer(this);
	connect(m_timeoutTimer, SIGNAL(timeout()), this, SLOT(slotTimeoutTick()));
	m_timeoutTimer->start(1000);

	m_xmlSrc = new XMLNewsSource;
	connect(m_xmlSrc, SIGNAL(loadComplete(XMLNewsSource *, bool)),
	        this, SLOT(slotLoadComplete(XMLNewsSource *, bool)));
	m_xmlSrc->loadFrom(url);

	connect(NewsIconMgr::self(), SIGNAL(gotIcon(const KURL &, const QPixmap &)),
	        this, SLOT(slotGotIcon(const KURL &, const QPixmap &)));
	KURL u = url;
	if (url.isLocalFile())
		u = QString::null;
	else
		u.setEncodedPathAndQuery(QString::fromLatin1("/favicon.ico"));
	NewsIconMgr::self()->getIcon(u);
}

SuggestProgressDlg::~SuggestProgressDlg()
{
	delete m_xmlSrc;
}

void SuggestProgressDlg::slotTimeoutTick()
{
	if (m_progressBar->progress() == m_progressBar->totalSteps()) {
		m_timeoutTimer->stop();
		KMessageBox::error(this, i18n("Could not retrieve the specified source file."));
		reject();
		return;
	}
	m_progressBar->setProgress(m_progressBar->progress() + 1);
}

void SuggestProgressDlg::slotLoadComplete(XMLNewsSource *, bool succeeded)
{
	m_gotSourceFile = true;
	m_succeeded = succeeded;

	if (m_gotIcon)
		done(succeeded ? QDialog::Accepted : QDialog::Rejected);
}

void SuggestProgressDlg::slotGotIcon(const KURL &url, const QPixmap &pixmap)
{
	m_gotIcon = true;
	m_icon = pixmap;
	m_iconURL = url;

	if (m_gotIcon)
		done(m_succeeded ? QDialog::Accepted : QDialog::Rejected);
}

NewsSourceDlgImpl::NewsSourceDlgImpl(QWidget *parent,  const char *name, bool modal, WFlags fl)
	: NewsSourceDlg(parent, name, modal, fl),
	m_modified(false)
{
	connect(NewsIconMgr::self(), SIGNAL(gotIcon(const KURL &, const QPixmap &)),
	        this, SLOT(slotGotIcon(const KURL &, const QPixmap &)));

	for (unsigned int i = 0; i < DEFAULT_SUBJECTS; i++)
		comboCategory->insertItem(
				NewsSourceBase::subjectText(static_cast<NewsSourceBase::Subject>(i)));

}

void NewsSourceDlgImpl::slotCancelClicked()
{
	close();
}

void NewsSourceDlgImpl::slotOkClicked()
{
	KURL url (polishedURL(KURL(urlSourceFile->url())));

	if (!validateURL(url))
		return;

	if (leName->text().isEmpty()) {
		KMessageBox::error(this, i18n("You have to specify a name for this news"
					" source to be able to use it."), i18n("No Name Specified"));
		return;
	}

	// This finds out which subject is selected in the 'Subject' combo box.
	NewsSourceBase::Subject subject = NewsSourceBase::Computers;
	for (unsigned int i = 0; i < DEFAULT_SUBJECTS; i++) {
		NewsSourceBase::Subject thisSubj = static_cast<NewsSourceBase::Subject>(i);
		if (comboCategory->currentText() == NewsSourceBase::subjectText(thisSubj)) {
			subject = thisSubj;
			break;
		}
	}

	KURL iconURL ( leIcon->text() );
	if (iconURL.protocol().isEmpty())
		if (iconURL.host().startsWith(QString::fromLatin1("ftp.")))
			iconURL.setProtocol(QString::fromLatin1("ftp"));
		else if (iconURL.host().startsWith(QString::fromLatin1("www.")))
			iconURL.setProtocol(QString::fromLatin1("http"));
		else
			iconURL.setProtocol(QString::fromLatin1("file"));

	NewsSourceBase::Data nsd(leName->text(), url.url(), iconURL.url(), subject,
			sbMaxArticles->value(), true, cbProgram->isChecked());

	emit newsSource(nsd);

	close();
}

void NewsSourceDlgImpl::slotSourceFileChanged()
{
	bSuggest->setEnabled(!urlSourceFile->url().isEmpty());
}

void NewsSourceDlgImpl::slotSuggestClicked()
{
	KURL url ( polishedURL(KURL( urlSourceFile->url() )) );

	if (!validateURL(url))
		return;

	SuggestProgressDlg dlg(url, this);
	if (dlg.exec() == QDialog::Accepted) {
		pixmapIcon->setPixmap(dlg.icon());
		if (NewsIconMgr::self()->isStdIcon(dlg.icon()))
			leIcon->clear();
		else
			leIcon->setText(dlg.iconURL().url());
		cbProgram->setChecked(false);
		leName->setText(dlg.xmlSrc()->newsSourceName());
		sbMaxArticles->setValue(dlg.xmlSrc()->articles().count());
	}
}

void NewsSourceDlgImpl::slotModified()
{
	m_modified = true;
}

void NewsSourceDlgImpl::setup(const NewsSourceBase::Data &nsd, bool modify)
{
	leName->setText(nsd.name);
	urlSourceFile->setURL(nsd.sourceFile);
	cbProgram->setChecked(nsd.isProgram);
	comboCategory->setCurrentItem(nsd.subject);
	sbMaxArticles->setValue(nsd.maxArticles);
	KURL iconURL ( nsd.icon );
	if (iconURL.protocol() == QString::fromLatin1("file"))
		iconURL.setProtocol(QString::null);
	leIcon->setText(iconURL.url());
	NewsIconMgr::self()->getIcon(iconURL);
	if (modify == true) {
		setCaption(i18n("Edit News Source"));
	}
}

KURL NewsSourceDlgImpl::polishedURL(const KURL &url) const
{
	KURL newURL = url;

	if (url.protocol().isEmpty())
		if (url.url().startsWith(QString::fromLatin1("ftp")))
			newURL = QString::fromLatin1("ftp://") + url.url();
		else
			newURL = QString::fromLatin1("http://") + url.url();

	return newURL;
}

bool NewsSourceDlgImpl::validateURL(const KURL &url)
{
	if (url.isEmpty()) {
		KMessageBox::error(this, i18n("You have to specify the source file for this"
					" news source to be able to use it."), i18n("No Source File"
					" Specified"));
		return false;
	}

	if (!url.isValid() || !url.hasPath() || url.encodedPathAndQuery() == QString::fromLatin1("/")) {
		KMessageBox::error(this, i18n("KNewsTicker needs a valid RDF or RSS file to"
					" suggest sensible values. The specified source file is invalid."),
					i18n("Invalid Source File"));
		return false;
	}

	return true;
}

void NewsSourceDlgImpl::slotGotIcon(const KURL &, const QPixmap &pixmap)
{
	pixmapIcon->setPixmap(pixmap);
}

#include "newssourcedlgimpl.moc"
