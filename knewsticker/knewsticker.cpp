/*
 * knewsticker.cpp
 *
 * Copyright (c) 2000, 2001 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#include "knewsticker.h"
#include "newsengine.h"
#include "newsscroller.h"
#include "configaccess.h"
#include "newsiconmgr.h"
#include "knewstickerconfig.h"

#include <kaboutapplication.h>
#include <kapplication.h>
#include <karrowbutton.h>
#include <kbugreport.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <knotifyclient.h>
#include <kprocess.h>
#include <kprotocolmanager.h>
#include <kstandarddirs.h>
#include <kstartupinfo.h>

#include <qcursor.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qtooltip.h>

#include <dcopclient.h>

KNewsTicker::KNewsTicker(const QString &cfgFile, Type t, int actions, QWidget *parent, const char *name)
	: ConfigIface(), DCOPObject("KNewsTicker"),
	KPanelApplet(cfgFile, t, actions, parent, name),
	m_instance(new KInstance("knewsticker")),
	m_dcopClient(new DCOPClient()),
	m_cfg(new ConfigAccess(config())),
	m_newsTimer(new QTimer(this)),
	m_updateTimer(new QTimer(this)),
	m_newsIconMgr(NewsIconMgr::self()),
	m_aboutData(new KAboutData("knewsticker", I18N_NOOP("KNewsTicker"), "v0.2",
			I18N_NOOP("A news ticker applet."), KAboutData::License_BSD,
			I18N_NOOP("(c) 2000, 2001 The KNewsTicker developers")))
{
	QHBoxLayout *layout = new QHBoxLayout(this);

	m_contextMenu = new KNewsTickerMenu(this);
	connect(m_contextMenu, SIGNAL(aboutToHide()),
			SLOT(slotContextMenuAboutToHide()));
	setCustomMenu(m_contextMenu);

	m_arrowButton = new KArrowButton(this);
	QToolTip::add(m_arrowButton, i18n("Show menu"));
	connect(m_arrowButton, SIGNAL(clicked()), SLOT(slotArrowButtonPressed()));
	m_arrowButton->setFocusPolicy(NoFocus);
	setupArrowButton();
	layout->addWidget(m_arrowButton);

	m_scroller = new NewsScroller(this, m_cfg);
	layout->addWidget(m_scroller);

	m_dcopClient->registerAs("knewsticker", false);

	QToolTip::add(m_scroller, QString::null);
	connect(m_scroller, SIGNAL(contextMenu()), SLOT(slotOpenContextMenu()));

	connect(m_newsTimer, SIGNAL(timeout()), SLOT(slotUpdateNews()));

	connect(m_updateTimer, SIGNAL(timeout()), SLOT(slotNotifyOfFailures()));

	m_aboutData->addAuthor("Frerich Raabe", I18N_NOOP("Original author"),
			"raabe@kde.org");
	m_aboutData->addAuthor("Malte Starostik", I18N_NOOP("Hypertext headlines"
				" and much more"), "malte@kde.org");
	m_aboutData->addAuthor("Wilco Greven", I18N_NOOP("Mouse wheel support"),
			"greven@kde.org");
	m_aboutData->addAuthor("Adriaan de Groot", I18N_NOOP("Rotated scrolling text"
			" modes"), "adridg@sci.kun.nl");

	reparseConfig();

	KStartupInfo::appStarted();
}

KNewsTicker::~KNewsTicker()
{
	delete m_cfg;
	delete m_dcopClient;
}

int KNewsTicker::heightForWidth(int) const
{
	return m_scroller->sizeHint().height() + m_arrowButton->height();
}

int KNewsTicker::widthForHeight(int) const
{
	return m_scroller->sizeHint().width() + m_arrowButton->width();
}

void KNewsTicker::preferences()
{
	KNewsTickerConfig dlg(m_cfg, this);
	if (dlg.exec() == QDialog::Accepted) {
		reparseConfig();
	}
}

void KNewsTicker::about()
{
	KAboutApplication aboutDlg(m_aboutData);
	aboutDlg.exec();
}

void KNewsTicker::help()
{
	kapp->invokeHelp(QString::null, QString::fromLatin1("knewsticker"));
}

void KNewsTicker::reportBug()
{
	KBugReport bugReport(this, true, m_aboutData);
	bugReport.exec();
}

void KNewsTicker::reparseConfig()
{
	m_cfg->reparseConfiguration();
	m_newsSources.clear();

	QStringList newsSources = m_cfg->newsSources();
	QStringList::ConstIterator it = newsSources.begin();
	QStringList::ConstIterator end = newsSources.end();
	for (; it != end; ++it) {
		NewsSourceBase::Ptr ns = m_cfg->newsSource((*it));
		if (!ns->data().enabled)
			continue;

		connect(ns, SIGNAL(newNewsAvailable(const NewsSourceBase::Ptr &, bool)),
				SLOT(slotNewsSourceUpdated(const NewsSourceBase::Ptr &, bool)));
		connect(ns, SIGNAL(invalidInput(const NewsSourceBase::Ptr &)),
				SLOT(slotNewsSourceFailed(const NewsSourceBase::Ptr &)));
		m_newsSources.append(ns);
	}

	setOfflineMode(m_cfg->offlineMode());
	if (!m_cfg->offlineMode())
		slotUpdateNews();
}

void KNewsTicker::slotUpdateNews()
{
	kdDebug(5005) << "slotUpdateNews()" << endl;
	m_newNews = false;

	m_updateTimer->start(KProtocolManager::responseTimeout(), true);

	m_failedNewsUpdates.clear();
	m_pendingNewsUpdates.clear();

	m_scroller->clear();

	NewsSourceBase::List::Iterator it = m_newsSources.begin();
	NewsSourceBase::List::Iterator end = m_newsSources.end();
	for (; it != end; ++it) {
		m_pendingNewsUpdates += (*it)->data().name;
		(*it)->retrieveNews();
		(*it)->getIcon();
	}
	kdDebug(5005) << "m_pendingNewsUpdates = " << m_pendingNewsUpdates.join(",")
	              << endl;
}

void KNewsTicker::slotNewsSourceUpdated(const NewsSourceBase::Ptr &ns,
		bool newNews)
{
	kdDebug(5005) << "slotNewsSourceUpdate()" << endl;
	if (newNews)
		m_newNews = true;

	if (!ns->articles().isEmpty())
		if (m_cfg->scrollMostRecentOnly())
			m_scroller->addHeadline(ns->articles().first());
		else {
			Article::List articles = ns->articles();
			Article::List::ConstIterator artIt = articles.begin();
			Article::List::ConstIterator artEnd = articles.end();
			for (; artIt != artEnd; ++artIt)
				m_scroller->addHeadline(*artIt);
		}

	m_scroller->reset(true);

	m_pendingNewsUpdates.remove(ns->data().name);
	kdDebug(5005) << "Updated news source: '" << ns->data().name << "'" << "\n"
	              << "m_pendingNewsUpdates = " << m_pendingNewsUpdates.join(",") << "\n"
	              << "m_failedNewsUpdates = " << m_failedNewsUpdates.join(",")
	              << endl;

	if (!m_pendingNewsUpdates.isEmpty())
		return;

	m_updateTimer->stop();

	if (!m_failedNewsUpdates.isEmpty())
		slotNotifyOfFailures();

	if (m_newNews) {
		KNotifyClient::Instance instance(m_instance);
		KNotifyClient::event(winId(), QString::fromLatin1("NewNews"));
	}
}

void KNewsTicker::slotNewsSourceFailed(const NewsSourceBase::Ptr &ns)
{
	m_failedNewsUpdates += ns->newsSourceName();
	slotNewsSourceUpdated(ns);
}

void KNewsTicker::mousePressEvent(QMouseEvent *e)
{
	if (e->button() == QMouseEvent::RightButton)
		slotOpenContextMenu();
}

void KNewsTicker::slotOpenContextMenu()
{
	m_contextMenu->setFullMenu(true);
	m_contextMenu->exec(QCursor::pos());
}

void KNewsTicker::slotArrowButtonPressed()
{
	QPoint pos(m_arrowButton->mapToGlobal(QPoint(0, 0)));
	QSize size(m_arrowButton->size());

	if (position() == KPanelApplet::pTop) {
		pos.setY(pos.y() + size.height() + 2);
	} else if (position() == KPanelApplet::pBottom) {
		const int y = pos.y() - m_contextMenu->sizeHint().height() - 2;
		pos.setY(QMAX(0, y));
	} else if (position() == KPanelApplet::pLeft ) {
		pos.setX(pos.x() + size.width() + 2);
	} else { // position() == KPanelApplet::pRight
		const int x = pos.x() - m_contextMenu->sizeHint().width() - 2;
		pos.setX(QMAX(0, x));
	}

	m_contextMenu->setFullMenu(true);
	m_contextMenu->exec(pos);
}

void KNewsTicker::positionChange(Position)
{
	delete layout();

	QBoxLayout *layout;

	if (orientation() == Horizontal)
		layout = new QHBoxLayout(this);
	else
		layout = new QVBoxLayout(this);

	if (m_arrowButton) {
		layout->addWidget(m_arrowButton);
		setupArrowButton();
	}

	layout->addWidget(m_scroller);
}

void KNewsTicker::slotContextMenuAboutToHide()
{
	if (m_arrowButton)
		m_arrowButton->setDown(false);
}

void KNewsTicker::slotNotifyOfFailures()
{
	KNotifyClient::Instance instance(m_instance);
	QString notification = QString::null;

	if (m_failedNewsUpdates.count() == 1)
		notification = i18n("<qt>Could not update news site '%1'.<br>"
					"The supplied resource file is probably invalid or"
					" broken.</qt>").arg(m_failedNewsUpdates.first());
	else if (m_failedNewsUpdates.count() > 1 && m_failedNewsUpdates.count() < 8) {
		notification = i18n("<qt>The following news sites had problems. Their"
				" resource files are probably invalid or broken.<ul>");
		QStringList::ConstIterator it = m_failedNewsUpdates.begin();
		QStringList::ConstIterator end = m_failedNewsUpdates.end();
		for (; it != end; ++it)
			notification += QString::fromLatin1("<li>%1</li>").arg(*it);
		notification += QString::fromLatin1("</ul></qt>");
	} else
		notification = i18n("Failed to update several news"
					" sites. The Internet connection might be cut.");

	KNotifyClient::event(winId(), QString::fromLatin1("InvalidRDF"), notification);
}

void KNewsTicker::setInterval(const uint interval)
{
	m_cfg->setInterval(interval);
	if ( interval > 4 )
		m_newsTimer->changeInterval(interval * 60 * 1000);
}

void KNewsTicker::setScrollingSpeed(const uint scrollingSpeed)
{
	m_cfg->setScrollingSpeed(scrollingSpeed);
	m_scroller->reset(true);
}

void KNewsTicker::setMouseWheelSpeed(const uint mouseWheelSpeed)
{
	m_cfg->setMouseWheelSpeed(mouseWheelSpeed);
}

void KNewsTicker::setScrollingDirection(const uint scrollingDirection)
{
	m_cfg->setScrollingDirection(scrollingDirection);
	m_scroller->reset(true);
}

void KNewsTicker::setCustomNames(bool customNames)
{
	m_cfg->setCustomNames(customNames);
}

void KNewsTicker::setScrollMostRecentOnly(bool scrollMostRecentOnly)
{
	m_cfg->setScrollMostRecentOnly(scrollMostRecentOnly);
	m_scroller->reset(true);
}

void KNewsTicker::setOfflineMode(bool offlineMode)
{
	if (offlineMode)
		m_newsTimer->stop();
	else
		if ( m_cfg->interval() > 4 )
			m_newsTimer->start(m_cfg->interval() * 1000 * 60);

	m_cfg->setOfflineMode(offlineMode);
}

void KNewsTicker::setUnderlineHighlighted(bool underlineHighlighted)
{
	m_cfg->setUnderlineHighlighted(underlineHighlighted);
	m_scroller->reset(true);
}

void KNewsTicker::setShowIcons(bool showIcons)
{
	m_cfg->setShowIcons(showIcons);
	m_scroller->reset(true);
}

void KNewsTicker::setSlowedScrolling(bool slowedScrolling)
{
	m_cfg->setSlowedScrolling(slowedScrolling);
}

void KNewsTicker::setForegroundColor(const QColor &foregroundColor)
{
	m_cfg->setForegroundColor(foregroundColor);
	m_scroller->reset(false);
}

void KNewsTicker::setBackgroundColor(const QColor &backgroundColor)
{
	m_cfg->setBackgroundColor(backgroundColor);
	m_scroller->reset(false);
}

void KNewsTicker::setHighlightedColor(const QColor &highlightedColor)
{
	m_cfg->setHighlightedColor(highlightedColor);
	m_scroller->reset(false);
}

void KNewsTicker::setupArrowButton()
{
	ArrowType at;

	if (orientation() == Horizontal) {
		m_arrowButton->setFixedWidth(12);
		m_arrowButton->setMaximumHeight(128);
		at = (position() == KPanelApplet::pTop ? DownArrow : UpArrow);
	} else {
		m_arrowButton->setMaximumWidth(128);
		m_arrowButton->setFixedHeight(12);
		at = (position() == KPanelApplet::pLeft ? RightArrow : LeftArrow);
	}
	m_arrowButton->setArrowType(at);
}

KNewsTickerMenu::KNewsTickerMenu(KNewsTicker *parent, const char *name)
  : KPopupMenu(parent, name),
	m_parent(parent),
	m_fullMenu(false)
{
	populateMenu();
}

void KNewsTickerMenu::populateMenu()
{
	clear();

	/*
	 * Perhaps this hardcoded stuff should be replaced by some kind of
	 * themeing functionality?
	 */
	const QPixmap lookIcon = SmallIcon(QString::fromLatin1("viewmag"));
	const QPixmap newArticleIcon = SmallIcon(QString::fromLatin1("info"));
	const QPixmap oldArticleIcon = SmallIcon(QString::fromLatin1("mime_empty"));
	const QPixmap noArticlesIcon = SmallIcon(QString::fromLatin1("remove"));

	unsigned int articleIdx = 0;
	const NewsSourceBase::List sources = m_parent->m_newsSources;
	NewsSourceBase::List::ConstIterator nIt = sources.begin();
	for (; nIt != sources.end(); ++nIt) {
		NewsSourceBase::Ptr ns = *nIt;

		KPopupMenu *submenu = new KPopupMenu;
		int checkNewsId = submenu->insertItem(lookIcon, i18n("Check News"), this, SLOT(slotCheckNews(int)), 0, sources.findIndex(ns) + 1000);
		setItemParameter(checkNewsId, sources.findIndex(ns));

		submenu->insertSeparator();

		if (m_parent->m_pendingNewsUpdates.contains(ns->newsSourceName())) {
			submenu->insertItem(noArticlesIcon, i18n("Currently Being Updated, No Articles Available"));
		} else if (!ns->articles().isEmpty()) {
			const Article::List articles = ns->articles();
			Article::List::ConstIterator artIt = articles.begin();
			for (; artIt != articles.end(); ++artIt) {
				Article::Ptr a = *artIt;
				QString headline = a->headline().replace('&', "&&");
				int id;
				if ( a->read() )
					id = submenu->insertItem(oldArticleIcon, headline, this, SLOT(slotOpenArticle(int)), 0, articleIdx+2000);
				else
					id = submenu->insertItem(newArticleIcon, headline, this, SLOT(slotOpenArticle(int)), 0, articleIdx+2000);
				kdDebug( 5005 ) << "Setting articles index for " << a->headline() << " to " << articleIdx << endl;
				setItemParameter( id, articleIdx++ );
			}
		} else {
			submenu->insertItem(noArticlesIcon, i18n("No Articles Available"));
		}

		insertItem(ns->icon(), ns->newsSourceName().replace('&', "&&"), submenu);
	}

	if (!m_parent->m_cfg->newsSources().isEmpty())
		insertSeparator();

	insertItem(lookIcon, i18n("Check News"), m_parent, SLOT(slotUpdateNews()));
	int i = insertItem(i18n("Offline Mode"), this, SLOT(slotToggleOfflineMode()), 0, 4711 );
	setItemChecked(i, m_parent->m_cfg->offlineMode());

	if (m_fullMenu) {
		insertSeparator();

		const QPixmap logoIcon = SmallIcon(QString::fromLatin1("knewsticker"));
		const QPixmap helpIcon = SmallIcon(QString::fromLatin1("help"));
		const QPixmap confIcon = SmallIcon(QString::fromLatin1("configure"));

		insertTitle(logoIcon, i18n("KNewsTicker"), 0, 0);

		insertItem(helpIcon, i18n("Help"), this, SLOT(slotShowHelp()));
		insertItem(logoIcon, i18n("About KNewsTicker"), this, SLOT(slotShowAbout()));
		insertSeparator();
		insertItem(confIcon, i18n("Configure KNewsTicker..."), this, SLOT(slotConfigure()));
	}
}

void KNewsTickerMenu::slotShowHelp()
{
	m_parent->help();
}

void KNewsTickerMenu::slotShowAbout()
{
	m_parent->about();
}

void KNewsTickerMenu::slotConfigure()
{
	m_parent->preferences();
}

void KNewsTickerMenu::slotToggleOfflineMode()
{
	m_parent->setOfflineMode(!m_parent->m_cfg->offlineMode());
	setItemChecked( indexOf( 4711 ), !m_parent->m_cfg->offlineMode() );
}

void KNewsTickerMenu::slotCheckNews(int idx)
{
	m_parent->m_newsSources[ idx - 1000 ]->retrieveNews();
}

void KNewsTickerMenu::slotOpenArticle(int idx)
{
	unsigned int i = idx - 2000;
	const NewsSourceBase::List sources = m_parent->m_newsSources;
	NewsSourceBase::List::ConstIterator it = sources.begin();
	while ( it != sources.end() ) {
		if ( ( *it )->articles().isEmpty() ) {
			++it;
			continue;
		}

		if ( i <= ( *it )->articles().count() - 1 )
			break;

		i -= ( *it )->articles().count();

		++it;
	}

	if ( it == sources.end() )
		return;

	( *it )->articles()[ i ]->open();
}

extern "C"
{
	KDE_EXPORT KPanelApplet* init(QWidget *parent, const QString &configFile)
	{
		KGlobal::locale()->insertCatalogue(QString::fromLatin1("knewsticker"));
		return new KNewsTicker(configFile, KPanelApplet::Stretch,
				KPanelApplet::Preferences | KPanelApplet::About |
                KPanelApplet::Help | KPanelApplet::ReportBug,
			    parent, "knewsticker");
	}
}

#include "knewsticker.moc"
