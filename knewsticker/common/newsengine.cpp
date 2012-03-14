/*
 * newsengine.cpp
 *
 * Copyright (c) 2000, 2001 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#include "newsengine.h"
#include "configiface.h"
#include "configaccess.h"
#include "xmlnewsaccess.h"
#include "newsiconmgr.h"

#include <kapplication.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <krun.h>
#include <kstandarddirs.h>

#include <qbuffer.h>

Article::Article(NewsSourceBase *parent, const QString &headline,
		const KURL &address)
	: XMLNewsArticle(headline, address),
	m_parent(parent),
	m_read(false)
{
}

bool Article::operator==(const Article &other) const
{
	return headline() == other.headline() && address() == other.address();
}

void Article::open()
{
	(void) new KRun(address());
	m_read = true;
}

NewsSourceBase::NewsSourceBase(const Data &nsd, ConfigIface *config)
	: XMLNewsSource(),
	m_data(nsd),
	m_icon(QPixmap()),
	m_cfg(dynamic_cast<ConfigAccess *>(config)),
	m_newsIconMgr(NewsIconMgr::self())
{
	connect(this, SIGNAL(loadComplete(XMLNewsSource *, bool)),
			SLOT(slotProcessArticles(XMLNewsSource *, bool)));
}

void NewsSourceBase::getIcon()
{
	connect(m_newsIconMgr, SIGNAL(gotIcon(const KURL &, const QPixmap &)),
			this, SLOT(slotGotIcon(const KURL &, const QPixmap &)));
	m_newsIconMgr->getIcon( KURL( m_data.icon ) );
}

QString NewsSourceBase::newsSourceName() const
{
	if (m_cfg->customNames() || m_name.isEmpty())
		return m_data.name;
	else
		return m_name;
}

QString NewsSourceBase::subjectText(const Subject subject)
{
	switch (subject) {
		case Arts: return i18n("Arts");
		case Business: return i18n("Business");
		case Computers: return i18n("Computers");
		case Games: return i18n("Games");
		case Health: return i18n("Health");
		case Home: return i18n("Home");
		case Recreation: return i18n("Recreation");
		case Reference: return i18n("Reference");
		case Science: return i18n("Science");
		case Shopping: return i18n("Shopping");
		case Society: return i18n("Society");
		case Sports: return i18n("Sports");
		case Misc: return i18n("Miscellaneous");
		case Magazines: return i18n("Magazines");
		default: return i18n("Unknown");
	}
}

void NewsSourceBase::slotProcessArticles(XMLNewsSource *, bool gotEm)
{
	if (!gotEm) {
		emit invalidInput(this);
		return;
	}

	Article::List oldArticles = m_articles;

	// Truncate the list of articles if necessary.
	m_articles.clear();
	XMLNewsArticle::List::ConstIterator it = XMLNewsSource::articles().begin();
	XMLNewsArticle::List::ConstIterator end = XMLNewsSource::articles().end();
	for (; it != end; ++it)
		m_articles.append(new Article(this, (*it).headline(), (*it).address()));

	// Fill the list with old articles until maxArticles is reached.
	if (m_articles.count() < m_data.maxArticles) {
		Article::List::ConstIterator oldArtIt = oldArticles.begin();
		Article::List::ConstIterator oldArtEnd = oldArticles.end();
		bool isNewArticle;
		for (; oldArtIt != oldArtEnd; ++oldArtIt) {
			isNewArticle = true;
			Article::List::ConstIterator newArtIt = m_articles.begin();
			Article::List::ConstIterator newArtEnd = m_articles.end();
			for (; newArtIt != newArtEnd; ++newArtIt) {
				Article newArt = **newArtIt;
				Article oldArt = **oldArtIt;
				if (newArt == oldArt)
					isNewArticle = false;
			}

			if (isNewArticle)
				m_articles.append(*oldArtIt);
			if (m_articles.count() == m_data.maxArticles)
				break;
		}
	} else
		while (m_articles.count() > m_data.maxArticles)
			m_articles.remove(m_articles.fromLast());

	// Copy the read flag of known articles
	Article::List::ConstIterator oldArtIt = oldArticles.begin();
	Article::List::ConstIterator oldArtEnd = oldArticles.end();
	for (; oldArtIt != oldArtEnd; ++oldArtIt) {
		Article::List::Iterator newArtIt = m_articles.begin();
		Article::List::Iterator newArtEnd = m_articles.end();
		for (; newArtIt != newArtEnd; ++newArtIt)
			if (**oldArtIt == **newArtIt)
				(*newArtIt)->setRead((*oldArtIt)->read());
	}

	emit newNewsAvailable(this, oldArticles != m_articles);
}

Article::Ptr NewsSourceBase::article(const QString &headline)
{
	Article::List::ConstIterator it = m_articles.begin();
	Article::List::ConstIterator end = m_articles.end();
	for (; it != end; ++it)
		if ((*it)->headline() == headline)
			return *it;

	return 0L;
}

void NewsSourceBase::slotGotIcon(const KURL &url, const QPixmap &pixmap)
{
	if (url.url() == m_data.icon) {
		m_icon = pixmap;

		disconnect(m_newsIconMgr, SIGNAL(gotIcon(const KURL &, const QPixmap &)),
			this, SLOT(slotGotIcon(const KURL &, const QPixmap &)));
	}
}

SourceFileNewsSource::SourceFileNewsSource(const NewsSourceBase::Data &nsd,
		ConfigIface *config)
	: NewsSourceBase(nsd, config)
{
}

void SourceFileNewsSource::retrieveNews()
{
	loadFrom(KURL( m_data.sourceFile ));
}

ProgramNewsSource::ProgramNewsSource(const NewsSourceBase::Data &nsd,
		ConfigIface *config) : NewsSourceBase(nsd, config),
	m_program(new KProcess()),
	m_programOutput(0)
{
	connect(m_program, SIGNAL(processExited(KProcess *)),
			SLOT(slotProgramExited(KProcess *)));
	connect(m_program, SIGNAL(receivedStdout(KProcess *, char *, int)),
			SLOT(slotGotProgramOutput(KProcess *, char *, int)));

	m_data.sourceFile = KURL(m_data.sourceFile).encodedPathAndQuery();
}

ProgramNewsSource::~ProgramNewsSource()
{
	delete m_program;
	delete m_programOutput;
}

void ProgramNewsSource::retrieveNews()
{
	m_programOutput = new QBuffer;
	m_programOutput->open(IO_WriteOnly);

	*m_program << m_data.sourceFile;
	m_program->start(KProcess::NotifyOnExit, KProcess::Stdout);
}

void ProgramNewsSource::slotGotProgramOutput(KProcess *, char *data, int length)
{
	m_programOutput->writeBlock(data, length);
}

void ProgramNewsSource::slotProgramExited(KProcess *proc)
{
	bool okSoFar = true;

	QString errorMsg;

	if (!proc->normalExit()) {
		errorMsg = i18n("<p>The program '%1' was terminated abnormally.<br>This can"
				" happen if it receives the SIGKILL signal.</p>");
		okSoFar = false;
	} else {
		ErrorCode error = static_cast<ErrorCode>(proc->exitStatus());
		if (error != NOERR) {
			errorMsg = errorMessage(error).arg(m_data.sourceFile);
			okSoFar = false;
		}
	}

	if (!okSoFar) {
		QString output = QString(m_programOutput->buffer());
		if (!output.isEmpty()) {
			output = QString::fromLatin1("\"") + output + QString::fromLatin1("\"");
			errorMsg += i18n("<p>Program output:<br>%1<br>").arg(output);
		}
		KMessageBox::detailedError(0, i18n("An error occurred while updating the"
					" news source '%1'.").arg(newsSourceName()), errorMsg,
				i18n("KNewsTicker Error"));
	}

	processData(m_programOutput->buffer(), okSoFar);

	delete m_programOutput;
	m_programOutput = 0;
}

QString ProgramNewsSource::errorMessage(const ErrorCode errorCode)
{
	switch (errorCode) {
		case EPERM: return i18n("The program '%1' could not be started at all.");
		case ENOENT: return i18n("The program '%1' tried to read or write a file or"
				" directory which could not be found.");
		case EIO: return i18n("An error occurred while the program '%1' tried to"
				" read or write data.");
		case E2BIG: return i18n("The program '%1' was passed too many arguments."
				" Please adjust the command line in the configuration dialog.");
		case ENOEXEC: return i18n("An external system program upon which the"
				" program '%1' relied could not be executed.");
		case EACCESS: return i18n("The program '%1' tried to read or write a file or"
				" directory but lacks the permission to do so.");
		case ENODEV: return i18n("The program '%1' tried to access a device which"
				" was not available.");
		case ENOSPC: return i18n("There is no more space left on the device used by"
				" the program '%1'.");
		case EROFS: return i18n("The program '%1' tried to create a temporary file"
				" on a read only file system.");
		case ENOSYS: return i18n("The program '%1' tried to call a function which"
				" is not implemented or attempted to access an external resource which"
				" does not exist.");
		case ENODATA: return i18n("The program '%1' was unable to retrieve input data and"
				" was therefore unable to return any XML data.");
		case ENONET: return i18n("The program '%1' tried to access a host which is not"
				" connected to a network.");
		case EPROTO: return i18n("The program '%1' tried to access a protocol which is not"
				" implemented.");
		case EDESTADDRREQ: return i18n("The program '%1' requires you to configure a"
				" destination address to retrieve data from. Please refer to the"
				" documentation of the program for information on how to do that.");
		case ESOCKTNOSUPPORT: return i18n("The program '%1' tried to use a socket"
				" type which is not supported by this system.");
		case ENETUNREACH: return i18n("The program '%1' tried to access an unreachable"
				" network.");
		case ENETRESET: return i18n("The network the program '%1' was trying to access"
				" dropped the connection with a reset.");
		case ECONNRESET: return i18n("The connection of the program '%1' was reset by"
				" peer.");
		case ETIMEDOUT: return i18n("The connection the program '%1' was trying to"
				" establish timed out.");
		case ECONNREFUSED: return i18n("The connection the program '%1' was trying to"
				" establish was refused.");
		case EHOSTDOWN: return i18n("The host the program '%1' was trying to reach is"
				" down.");
		case EHOSTUNREACH: return i18n("The host the program '%1' was trying to reach is"
				" unreachable, no route to host.");
		case ENOEXECBIT: return i18n("KNewsTicker could not execute the program '%1'"
				" because its executable bit was not set. You can mark that program as"
				" executable by executing the following steps:<ul>"
				"<li>Open a Konqueror window and browse to the program</li>"
				"<li>Click on the file with the right mouse button, and select 'Properties'</li>"
				"<li>Open the 'Permissions' tab and make sure that the box in the column"
				" 'Exec' and the row 'User' is checked to ensure that the current user"
				" is allowed to execute that file.</li></ul>");
		case EBADREQ: return i18n("The program '%1' sent a bad request which was not"
				" understood by the server.");
		case ENOAUTH: return i18n("The program '%1' failed to issue an authorization for"
				" an area which needs some form of authorization before it can be"
				" accessed.");
		case EMUSTPAY: return i18n("The program '%1' aborted because it could not access"
				" the data without paying for it.");
		case EFORBIDDEN: return i18n("The program '%1' tried to access a forbidden"
				" source.");
		case ENOTFOUND: return i18n("The program '%1' tried to access data which"
				" could not be found.");
		case ETIMEOUT: return i18n("The HTTP request of the program '%1' timed out.");
		case ESERVERE: return i18n("A server error has been encountered. It is likely"
				" that you cannot do anything about it.");
		case EHTTPNOSUP: return i18n("The HTTP protocol version used by the program"
				" '%1' was not understood by the HTTP server or source.");
		default: return i18n("KNewsTicker was unable to detect the exact reasons for"
				" the error.");
	}
}

#include "newsengine.moc"
