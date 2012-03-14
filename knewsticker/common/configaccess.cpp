/*
 * configaccess.cpp
 *
 * Copyright (c) 2001 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#include "configaccess.h"
#include "newsengine.h"

#include <qregexp.h>

#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>

static NewsSourceBase::Data NewsSourceDefault[DEFAULT_NEWSSOURCES] = {
	// Arts ---------------
		NewsSourceBase::Data(
		QString::fromLatin1("Bureau 42"),
		QString::fromLatin1("http://www.bureau42.com/rdf/"),
		QString::fromLatin1("http://www.bureau42.com/favicon.ico"),
		NewsSourceBase::Arts, 5, false, false),
		NewsSourceBase::Data(
		QString::fromLatin1("eFilmCritic"),
		QString::fromLatin1("http://efilmcritic.com/fo.rdf"),
		QString::fromLatin1("http://efilmcritic.com/favicon.ico"),
		NewsSourceBase::Arts, 3, false, false),
		NewsSourceBase::Data(
		QString::fromLatin1("superhits.ch"),
		QString::fromLatin1("http://www.superhits.ch/cgi-bin/superhits.cgi?page=rdf"),
		QString::fromLatin1("http://www.superhits.ch/favicon.ico"),
		NewsSourceBase::Arts, 10, false, false, QString::fromLatin1("de")),
	// Business -----------
		NewsSourceBase::Data(
		QString::fromLatin1("Internet.com Business"),
		QString::fromLatin1("http://headlines.internet.com/internetnews/bus-news/news.rss"),
		QString::null,
		NewsSourceBase::Business, 10, false, false),
		NewsSourceBase::Data(
		QString::fromLatin1("TradeSims"),
		QString::fromLatin1("http://www.tradesims.com/AEX.rdf"),
		QString::null,
		NewsSourceBase::Business, 10, false, false),
	// Computers ----------
		NewsSourceBase::Data(
		QString::fromLatin1( "linuxartist.org" ),
		QString::fromLatin1( "http://www.linuxartist.org/backend.php"),
		QString::fromLatin1( "http://www.linuxartist.org/favicon.ico"),
		NewsSourceBase::Computers, 10, false, false ),
		NewsSourceBase::Data(
		QString::fromLatin1("KDE Deutschland"),
		QString::fromLatin1("http://www.kde.de/nachrichten/nachrichten.rdf"),
		QString::fromLatin1("http://www.kde.de/favicon.ico"),
		NewsSourceBase::Computers, 10, true, false, QString::fromLatin1("de")),
		NewsSourceBase::Data(
		QString::fromLatin1("KDE France"),
		QString::fromLatin1("http://www.kde-france.org/backend-breves.php3"),
		QString::null,
		NewsSourceBase::Computers, 10, true, false, QString::fromLatin1("fr")),
		NewsSourceBase::Data(
		QString::fromLatin1("FreeBSD Project News"),
		QString::fromLatin1("http://www.freebsd.org/news/news.rdf"),
		QString::fromLatin1("http://www.freebsd.org/favicon.ico"),
		NewsSourceBase::Computers, 10, false, false),
		NewsSourceBase::Data(
		QString::fromLatin1("dot.kde.org"),
		QString::fromLatin1("http://www.kde.org/dotkdeorg.rdf"),
		QString::fromLatin1("http://www.kde.org/favicon.ico"),
		NewsSourceBase::Computers, 10, true, false),
		NewsSourceBase::Data(
		QString::fromLatin1("GNOME News"),
		QString::fromLatin1("http://www.gnomedesktop.org/backend.php"),
		QString::null,
		NewsSourceBase::Computers, 10, false, false),
		NewsSourceBase::Data(
		QString::fromLatin1("Slashdot"),
		QString::fromLatin1("http://slashdot.org/slashdot.rdf"),
		QString::fromLatin1("http://slashdot.org/favicon.ico"),
		NewsSourceBase::Computers, 10, true, false),
		NewsSourceBase::Data(
		QString::fromLatin1("Ask Slashdot"),
		QString::fromLatin1("http://slashdot.org/askslashdot.rdf"),
		QString::fromLatin1("http://slashdot.org/favicon.ico"),
		NewsSourceBase::Computers, 10, false, false),
		NewsSourceBase::Data(
		QString::fromLatin1("Slashdot: Features"),
		QString::fromLatin1("http://slashdot.org/features.rdf"),
		QString::fromLatin1("http://slashdot.org/favicon.ico"),
		NewsSourceBase::Computers, 10, false, false),
		NewsSourceBase::Data(
		QString::fromLatin1("Slashdot: Apache"),
		QString::fromLatin1("http://slashdot.org/apache.rdf"),
		QString::fromLatin1("http://slashdot.org/favicon.ico"),
		NewsSourceBase::Computers, 10, false, false),
		NewsSourceBase::Data(
		QString::fromLatin1("Slashdot: Books"),
		QString::fromLatin1("http://slashdot.org/books.rdf"),
		QString::fromLatin1("http://slashdot.org/favicon.ico"),
		NewsSourceBase::Computers, 10, false, false),
		NewsSourceBase::Data(
		QString::fromLatin1("Jabber News"),
		QString::fromLatin1("http://www.jabber.org/news/rss.xml"),
		QString::null,
		NewsSourceBase::Computers, 10, false, false),
		NewsSourceBase::Data(
		QString::fromLatin1("Freshmeat"),
		QString::fromLatin1("http://freshmeat.net/backend/fm.rdf"),
		QString::fromLatin1("http://freshmeat.net/favicon.ico"),
		NewsSourceBase::Computers, 10, false, false),
		NewsSourceBase::Data(
		QString::fromLatin1("Linux Weekly News"),
		QString::fromLatin1("http://www.lwn.net/headlines/rss"),
		QString::fromLatin1("http://www.lwn.net/favicon.ico"),
		NewsSourceBase::Computers, 10, true, false),
		NewsSourceBase::Data(
		QString::fromLatin1("heise online news"),
		QString::fromLatin1("http://www.heise.de/newsticker/heise.rdf"),
		QString::fromLatin1("http://www.heise.de/favicon.ico"),
		NewsSourceBase::Computers, 10, true, false, QString::fromLatin1("de")),
		NewsSourceBase::Data(
		QString::fromLatin1("RUS-CERT Ticker"),
		QString::fromLatin1("http://cert.uni-stuttgart.de/ticker/rus-cert.rdf"),
		QString::fromLatin1("http://cert.uni-stuttgart.de/favicon.ico"),
		NewsSourceBase::Computers, 10, true, false, QString::fromLatin1("de")),
		NewsSourceBase::Data(
		QString::fromLatin1("RUS-CERT Elsewhere"),
		QString::fromLatin1("http://cert.uni-stuttgart.de/ticker/rus-cert-elsewhere.rdf"),
		QString::fromLatin1("http://cert.uni-stuttgart.de/favicon.ico"),
		NewsSourceBase::Computers, 10, false, false, QString::fromLatin1("de")),
		NewsSourceBase::Data(
		QString::fromLatin1("Kuro5hin"),
		QString::fromLatin1("http://kuro5hin.org/backend.rdf"),
		QString::fromLatin1("http://kuro5hin.org/favicon.ico"),
		NewsSourceBase::Computers, 10, true, false),
		NewsSourceBase::Data(
		QString::fromLatin1("Prolinux"),
		QString::fromLatin1("http://www.pl-forum.de/backend/pro-linux.rdf"),
		QString::fromLatin1("http://www.prolinux.de/favicon.ico"),
		NewsSourceBase::Computers, 10, false, false, QString::fromLatin1("de")),
		NewsSourceBase::Data(
		QString::fromLatin1("Linuxde.org"),
		QString::fromLatin1("http://www.linuxde.org/backends/news.rdf"),
		QString::fromLatin1("http://www.linuxde.org/favicon.ico"),
		NewsSourceBase::Computers, 10, false, false, QString::fromLatin1("de")),
		NewsSourceBase::Data(
		QString::fromLatin1("LinuxSecurity.com"),
		QString::fromLatin1("http://www.linuxsecurity.com/linuxsecurity_hybrid.rdf"),
		QString::null,
		NewsSourceBase::Computers, 10, false, false),
		NewsSourceBase::Data(
		QString::fromLatin1("Linux Game Tome"),
		QString::fromLatin1("http://happypenguin.org/html/news.rdf"),
		QString::null,
		NewsSourceBase::Computers, 10, false, false),
		NewsSourceBase::Data(
		QString::fromLatin1("Telefragged"),
		QString::fromLatin1("http://www.telefragged.com/cgi-bin/rdf.pl"),
		QString::null,
		NewsSourceBase::Computers, 10, false, false),
/*		NewsSourceBase::Data(
		QString::fromLatin1("Gimp News"),
		QString::fromLatin1("http://www.xach.com/gimp/news/channel.rdf"),
		QString::null,
		NewsSourceBase::Computers, 10, false, false),*/
		NewsSourceBase::Data(
		QString::fromLatin1("Mozilla"),
		QString::fromLatin1("http://www.mozilla.org/news.rdf"),
		QString::fromLatin1("http://www.mozillazine.org/favicon.ico"),
		NewsSourceBase::Computers, 10, false, false),
		NewsSourceBase::Data(
		QString::fromLatin1("MozillaZine"),
		QString::fromLatin1("http://www.mozillazine.org/contents.rdf"),
		QString::fromLatin1("http://www.mozillazine.org/favicon.ico"),
		NewsSourceBase::Computers, 10, false, false),
		NewsSourceBase::Data(
		QString::fromLatin1("Daemon News"),
		QString::fromLatin1("http://daily.daemonnews.org/ddn.rdf.php3"),
		QString::null,
		NewsSourceBase::Computers, 10, false, false),
		NewsSourceBase::Data(
		QString::fromLatin1("use Perl;"),
		QString::fromLatin1("http://use.perl.org/useperl.rdf"),
		QString::null,
		NewsSourceBase::Computers, 10, false, false),
		NewsSourceBase::Data(
		QString::fromLatin1("desktopian.org"),
		QString::fromLatin1("http://www.desktopian.org/includes/headlines.xml"),
		QString::fromLatin1("http://www.desktopian.org/favicon.ico"),
		NewsSourceBase::Computers, 10, false, false),
		NewsSourceBase::Data(
		QString::fromLatin1("Root prompt"),
		QString::fromLatin1("http://www.rootprompt.org/rss/"),
		QString::fromLatin1("http://www.rootprompt.org/favicon.ico"),
		NewsSourceBase::Computers, 10, false, false),
		NewsSourceBase::Data(
		QString::fromLatin1("SecurityFocus"),
		QString::fromLatin1("http://www.securityfocus.com/rss/news.xml"),
		QString::fromLatin1("http://www.securityfocus.com/favicon.ico"),
		NewsSourceBase::Computers, 10, false, false),
		NewsSourceBase::Data(
		QString::fromLatin1("LinuxNewbie"),
		QString::fromLatin1("http://www.linuxnewbie.org/news.cdf"),
		QString::fromLatin1("http://www.linuxnewbie.org/favicon.ico"),
		NewsSourceBase::Computers, 5, false, false),
		NewsSourceBase::Data(
		QString::fromLatin1("Arstechnica"),
		QString::fromLatin1("http://arstechnica.com/etc/rdf/ars.rdf"),
		QString::fromLatin1("http://arstechnica.com/favicon.ico"),
		NewsSourceBase::Computers, 10, false, false),
		NewsSourceBase::Data(
		QString::fromLatin1("amiga-news.de - deutschsprachige Amiga Nachrichten"),
		QString::fromLatin1("http://www.amiga-news.de/de/backends/news/index.rss"),
		QString::fromLatin1("http://www.amiga-news.de/favicon.ico"),
		NewsSourceBase::Computers, 10, false, false, QString::fromLatin1("de")),
		NewsSourceBase::Data(
		QString::fromLatin1("amiga-news.de - english Amiga news"),
		QString::fromLatin1("http://www.amiga-news.de/en/backends/news/index.rss"),
		QString::fromLatin1("http://www.amiga-news.de/favicon.ico"),
		NewsSourceBase::Computers, 10, false, false),
		NewsSourceBase::Data(
		QString::fromLatin1("RadioTux)"),
		QString::fromLatin1("http://blog.radiotux.de/feed/"),
		QString::null,
		NewsSourceBase::Computers, 10, false, false, QString::fromLatin1("de")),
		NewsSourceBase::Data(
		QString::fromLatin1("kdenews.unixcode.org"),
		QString::fromLatin1("http://kdenews.unixcode.org/?node=news&action=rss"),
		QString::null,
		NewsSourceBase::Computers, 10, false, false),
		NewsSourceBase::Data(
		QString::fromLatin1("FreshPorts - the place for ports"),
		QString::fromLatin1("http://www.freshports.org/news.php"),
		QString::fromLatin1("http://www.freshports.org/favicon.ico"),
		NewsSourceBase::Computers, 20, false, false),
		NewsSourceBase::Data(
		QString::fromLatin1("NetPhoenix"),
		QString::fromLatin1("http://www.netphoenix.at/rss/netphoenix.php"),
		QString::fromLatin1("http://www.netphoenix.at/favicon.ico"),
		NewsSourceBase::Computers, 10, false, false, QString::fromLatin1("de")),
		NewsSourceBase::Data(
		QString::fromLatin1("ShortNews - by www.netphoenix.at"),
		QString::fromLatin1("http://www.netphoenix.at/rss/shortnews.php"),
		QString::fromLatin1("http://www.netphoenix.at/favicon.ico"),
		NewsSourceBase::Computers, 10, false, false, QString::fromLatin1("de")),
		NewsSourceBase::Data(
		QString::fromLatin1("zez.org - about code "),
		QString::fromLatin1("http://zez.org/article/rssheadlines"),
		QString::null,
		NewsSourceBase::Computers, 10, false, false),
		NewsSourceBase::Data(
		QString::fromLatin1("BSDatwork.com"),
		QString::fromLatin1("http://BSDatwork.com/backend.php"),
		QString::fromLatin1("http://BSDatwork.com/favicon.ico"),
		NewsSourceBase::Computers, 10, false, false),
		NewsSourceBase::Data(
		QString::fromLatin1("FreshSource - the place for source"),
		QString::fromLatin1("http://www.freshsource.org/news.php"),
		QString::fromLatin1("http://www.freshsource.org/favicon.ico"),
		NewsSourceBase::Computers, 20, false, false),
		NewsSourceBase::Data(
		QString::fromLatin1("The FreeBSD Diary"),
		QString::fromLatin1("http://www.freebsddiary.org/news.php"),
		QString::fromLatin1("http://www.freebsddiary.org/favicon.ico"),
		NewsSourceBase::Computers, 10, false, false),
		NewsSourceBase::Data(
		QString::fromLatin1("MaximumBSD"),
		QString::fromLatin1("http://www.maximumbsd.com/backend/mb.rdf"),
		QString::fromLatin1("http://www.maximumbsd.com/favicon.ico"),
		NewsSourceBase::Computers, 10, false, false),
		NewsSourceBase::Data(
		QString::fromLatin1("BR-Linux.org"),
		QString::fromLatin1("http://br-linux.org/noticias/index.rdf"),
		QString::fromLatin1("http://br-linux.org/noticias/favicon.ico"),
		NewsSourceBase::Computers, 10, false, false),
		NewsSourceBase::Data(
		QString::fromLatin1("OSNews.com"),
		QString::fromLatin1("http://www.osnews.com/files/recent.rdf"),
		QString::fromLatin1("http://www.osnews.com/favicon.ico"),
		NewsSourceBase::Computers, 10, false, false),
	// Miscellaneous ------
		NewsSourceBase::Data(
		QString::fromLatin1("tagesschau.de"),
		QString::fromLatin1("http://www.tagesschau.de/newsticker.rdf"),
		QString::fromLatin1("http://www.tagesschau.de/favicon.ico"),
		NewsSourceBase::Misc, 10, true, false, QString::fromLatin1("de")),
		NewsSourceBase::Data(
		QString::fromLatin1("N24.de"),
		QString::fromLatin1("http://www.n24.de/rss/?rubrik=home"),
		QString::fromLatin1("http://www.n24.de/favicon.ico"),
		NewsSourceBase::Misc, 10, true, false, QString::fromLatin1("de")),
/*
		NewsSourceBase::Data(
		QString::fromLatin1("CNN"),
		QString::fromLatin1("http://www.cnn.com/cnn.rss"),
		QString::fromLatin1("http://www.cnn.com/favicon.ico"),
		NewsSourceBase::Misc, 10, false, false),
		NewsSourceBase::Data(
		QString::fromLatin1("CNN Europe"),
		QString::fromLatin1("http://europe.cnn.com/cnn.rss"),
		QString::fromLatin1("http://europe.cnn.com/favicon.ico"),
		NewsSourceBase::Misc, 10, false, false),
*/
		NewsSourceBase::Data(
		QString::fromLatin1("HotWired"),
		QString::fromLatin1("http://www.hotwired.com/webmonkey/meta/headlines.rdf"),
		QString::fromLatin1("http://www.hotwired.com/favicon.ico"),
		NewsSourceBase::Misc, 10, false, false),
		NewsSourceBase::Data(
		QString::fromLatin1("The Register"),
		QString::fromLatin1("http://www.theregister.co.uk/tonys/slashdot.rdf"),
		QString::fromLatin1("http://www.theregister.co.uk/favicon.ico"),
		NewsSourceBase::Misc, 10, false, false),
		NewsSourceBase::Data(
		QString::fromLatin1( "Christian Science Monitor" ),
		QString::fromLatin1( "http://www.csmonitor.com/rss/csm.rss"),
		QString::fromLatin1( "http://www.csmonitor.com/favicon.ico"),
		NewsSourceBase::Misc, 10, false, false ),
	// Magazines ------
		NewsSourceBase::Data(
		QString::fromLatin1("Revista do Linux"),
		QString::fromLatin1("http://www.revistadolinux.com.br/noticias/arquivo/noticias.rdf"),
		QString::fromLatin1("http://www.revistadolinux.com.br/favicon.ico"),
		NewsSourceBase::Magazines, 10, false, false /*, QString::fromLatin1("br")*/ ),
		NewsSourceBase::Data(
		QString::fromLatin1("Spiegel.de"),
		QString::fromLatin1("http://www.spiegel.de/schlagzeilen/rss/0,5291,,00.xml"),
		QString::fromLatin1("http://www.spiegel.de/favicon.ico"),
		NewsSourceBase::Magazines, 10, true, false, QString::fromLatin1("de")),
		NewsSourceBase::Data(
		QString::fromLatin1("FAZ.de"),
		QString::fromLatin1("http://www.faz.net/s/Rub/Tpl~Epartner~SRss_.xml"),
		QString::fromLatin1("http://www.faz.net/favicon.ico"),
		NewsSourceBase::Magazines, 10, true, false, QString::fromLatin1("de")),
	// Recreation
		NewsSourceBase::Data(
		QString::fromLatin1("Segfault"),
		QString::fromLatin1("http://segfault.org/stories.xml"),
		QString::fromLatin1("http://segfault.org/favicon.ico"),
		NewsSourceBase::Recreation, 10, false, false),
	// Society
		NewsSourceBase::Data(
		QString::fromLatin1("nippon.it"),
		QString::fromLatin1("http://www.nippon.it/backend.it.php"),
		QString::fromLatin1("http://www.nippon.it/favicon.ico"),
		NewsSourceBase::Society, 10, false, false, QString::fromLatin1("it")),
		NewsSourceBase::Data( 
		QString::fromLatin1( "gflash" ),
		QString::fromLatin1( "http://www.gflash.de/backend.php"),
		QString::fromLatin1( "http://www.gflash.de/favicon.ico"),
		NewsSourceBase::Society, 10, false, false, QString::fromLatin1( "de" ) ),
		NewsSourceBase::Data( 
		QString::fromLatin1( "Quintessenz" ),
		QString::fromLatin1( "http://quintessenz.at/cgi-bin/rdf"),
		QString::fromLatin1( "http://quintessenz.at/favicon.ico"),
		NewsSourceBase::Society, 9, false, false, QString::fromLatin1( "de" ) )
};

ArticleFilter::ArticleFilter(const QString &action, const QString &newsSource,
		const QString &condition, const QString &expression, bool enabled)
	: m_action(action),
	m_newsSource(newsSource),
	m_condition(condition),
	m_expression(expression),
	m_enabled(enabled)
{
}

bool ArticleFilter::matches(Article::Ptr a) const
{
	if (!enabled() ||
	    (a->newsSource()->data().name != newsSource() &&
	    newsSource() != i18n("all news sources")))
		return false;
	
	bool matches;

	if (condition() == i18n("contain"))
		matches = a->headline().contains(expression());
	else if (condition() == i18n("do not contain"))
		matches = !a->headline().contains(expression());
	else if (condition() == i18n("equal"))
		matches = (a->headline() == expression());
	else if (condition() == i18n("do not equal"))
		matches = (a->headline() != expression());
	else { // condition() == i18n("match")
		QRegExp regexp = QRegExp(expression());
		matches = regexp.exactMatch(a->headline());
	}
	
	if (action() == i18n("Show"))
		matches = !matches;

	return matches;
}
	
ConfigAccess::ConfigAccess()
	: m_defaultCfg(new KConfig(QString::null, true, false))
{
	m_cfg = m_defaultCfg;
}

ConfigAccess::ConfigAccess(KConfig *config)
	: m_cfg(config), m_defaultCfg(0L)
{
	m_cfg->setGroup("KNewsTicker");
}

ConfigAccess::~ConfigAccess()
{
	delete m_defaultCfg;
}

unsigned int ConfigAccess::interval() const
{
	return m_cfg->readNumEntry("Update interval", 30);
}

void ConfigAccess::setInterval(const unsigned int interval)
{
	m_cfg->writeEntry("Update interval", interval);
	m_cfg->sync();
}

unsigned int ConfigAccess::mouseWheelSpeed() const
{
	return m_cfg->readNumEntry("Mouse wheel speed", 5);
}

void ConfigAccess::setMouseWheelSpeed(const unsigned int mouseWheelSpeed)
{
	m_cfg->writeEntry("Mouse wheel speed", mouseWheelSpeed);
	m_cfg->sync();
}

QFont ConfigAccess::font() const
{
	QFont font = KGlobalSettings::fixedFont();
	return m_cfg->readFontEntry("Font", &font);
}

void ConfigAccess::setFont(const QFont &font)
{
	m_cfg->writeEntry("Font", font);
	m_cfg->sync();
}

bool ConfigAccess::customNames() const
{
	return m_cfg->readBoolEntry("Custom names", false);
}

void ConfigAccess::setCustomNames(bool customNames)
{
	m_cfg->writeEntry("Custom names", customNames);
	m_cfg->sync();
}

bool ConfigAccess::scrollMostRecentOnly() const
{
	return m_cfg->readBoolEntry("Scroll most recent headlines only", false);
}

void ConfigAccess::setScrollMostRecentOnly(bool scrollMostRecentOnly)
{
	m_cfg->writeEntry("Scroll most recent headlines only", scrollMostRecentOnly);
	m_cfg->sync();
}

bool ConfigAccess::offlineMode() const
{
	return m_cfg->readBoolEntry("Offline mode", false);
}

void ConfigAccess::setOfflineMode(bool offlineMode)
{
	m_cfg->writeEntry("Offline mode", offlineMode);
	m_cfg->sync();
}

QStringList ConfigAccess::newsSources() const
{
	QStringList tempList = m_cfg->readListEntry("News sources");
	if (tempList.isEmpty())
		for (unsigned int i = 0; i < DEFAULT_NEWSSOURCES; i++)
			tempList << NewsSourceDefault[i].name;
	return tempList;
}

ArticleFilter::List ConfigAccess::filters() const
{
	return m_cfg->readIntListEntry("Filters");
}

void ConfigAccess::setNewsSources(const QStringList &newsSources)
{
	m_cfg->writeEntry("News sources", newsSources);
	m_cfg->sync();
}

void ConfigAccess::setFilters(const ArticleFilter::List &filters)
{
	m_cfg->writeEntry("Filters", filters);
	m_cfg->sync();
}

unsigned int ConfigAccess::scrollingSpeed() const
{
	return m_cfg->readNumEntry("Scrolling speed", 20);
}

void ConfigAccess::setScrollingSpeed(const unsigned int scrollingSpeed)
{
	m_cfg->writeEntry("Scrolling speed", scrollingSpeed);
	m_cfg->sync();
}

unsigned int ConfigAccess::scrollingDirection() const
{
	return m_cfg->readNumEntry("Scrolling direction", 0);
}

void ConfigAccess::setScrollingDirection(const unsigned int scrollingDirection)
{
	m_cfg->writeEntry("Scrolling direction", scrollingDirection);
	m_cfg->sync();
}

QColor ConfigAccess::foregroundColor() const
{
	return m_cfg->readColorEntry("Foreground color", &Qt::black);
}

void ConfigAccess::setForegroundColor(const QColor &foregroundColor)
{
	m_cfg->writeEntry("Foreground color", foregroundColor);
	m_cfg->sync();
}

QColor ConfigAccess::backgroundColor() const
{
	return m_cfg->readColorEntry("Background color", &Qt::white);
}

void ConfigAccess::setBackgroundColor(const QColor &backgroundColor)
{
	m_cfg->writeEntry("Background color", backgroundColor);
	m_cfg->sync();
}

QColor ConfigAccess::highlightedColor() const
{
	return m_cfg->readColorEntry("Highlighted color", &Qt::red);
}

void ConfigAccess::setHighlightedColor(const QColor &highlightedColor)
{
	m_cfg->writeEntry("Highlighted color", highlightedColor);
	m_cfg->sync();
}

bool ConfigAccess::underlineHighlighted() const
{
	return m_cfg->readBoolEntry("Underline highlighted headlines", true);
}

void ConfigAccess::setUnderlineHighlighted(bool underlineHighlighted)
{
	m_cfg->writeEntry("Underline highlighted headlines", underlineHighlighted);
	m_cfg->sync();
}

NewsSourceBase *ConfigAccess::newsSource(const QString &newsSource)
{
	NewsSourceBase::Data nsd;

	if (m_cfg->hasGroup(newsSource)) {
		m_cfg->setGroup(newsSource);
		nsd.name = newsSource;
		nsd.sourceFile = m_cfg->readPathEntry("Source file");
		nsd.isProgram = m_cfg->readBoolEntry("Is program", false);
		nsd.subject = static_cast<NewsSourceBase::Subject>
			(m_cfg->readNumEntry("Subject", NewsSourceBase::Computers));
		nsd.icon = m_cfg->readEntry("Icon");
		nsd.maxArticles = m_cfg->readNumEntry("Max articles", 10);
		nsd.enabled = m_cfg->readBoolEntry("Enabled", true);
		nsd.language = m_cfg->readEntry("Language", QString::fromLatin1("C"));
		m_cfg->setGroup("KNewsTicker");
	} else for (unsigned int i = 0; i < DEFAULT_NEWSSOURCES; i++)
		if (NewsSourceDefault[i].name == newsSource) {
			nsd = NewsSourceDefault[i];
			if (nsd.enabled)
				nsd.enabled = (nsd.language == QString::fromLatin1("C") ||
						KGlobal::locale()->languageList().contains(nsd.language));
			break;
		}
	
	if (nsd.isProgram)
		return new ProgramNewsSource(nsd, this);
	else 
		return new SourceFileNewsSource(nsd, this);
	
	return 0L;
}

ArticleFilter ConfigAccess::filter(const unsigned int filterNo) const
{
	ArticleFilter f;
	f.setId(filterNo);

	if (m_cfg->hasGroup(QString::fromLatin1("Filter #%1").arg(filterNo))) {
		m_cfg->setGroup(QString::fromLatin1("Filter #%1").arg(filterNo));
		f.setAction(m_cfg->readEntry("Action", i18n("Show")));
		f.setNewsSource(m_cfg->readEntry("News source", i18n("all news sources")));
		f.setCondition(m_cfg->readEntry("Condition", i18n("contain")));
		f.setExpression(m_cfg->readEntry("Expression"));
		f.setEnabled(m_cfg->readBoolEntry("Enabled", true));
		m_cfg->setGroup("KNewsTicker");
	}

	return f;
}

void ConfigAccess::setNewsSource(const NewsSourceBase::Data &ns)
{
	m_cfg->setGroup(ns.name);
	m_cfg->writePathEntry("Source file", ns.sourceFile);
	m_cfg->writeEntry("Is program", ns.isProgram);
	m_cfg->writeEntry("Max articles", ns.maxArticles);
	m_cfg->writeEntry("Subject", ns.subject);
	m_cfg->writeEntry("Icon", ns.icon);
	m_cfg->writeEntry("Enabled", ns.enabled);
	m_cfg->writeEntry("Language", ns.language);
	m_cfg->setGroup("KNewsTicker");
	m_cfg->sync();
}

void ConfigAccess::setFilter(const ArticleFilter &f)
{
	m_cfg->setGroup(QString::fromLatin1("Filter #%1").arg(f.id()));
	m_cfg->writeEntry("Action", f.action());
	m_cfg->writeEntry("News source", f.newsSource());
	m_cfg->writeEntry("Condition", f.condition());
	m_cfg->writeEntry("Expression", f.expression());
	m_cfg->writeEntry("Enabled", f.enabled());
	m_cfg->setGroup("KNewsTicker");
	m_cfg->sync();
}

bool ConfigAccess::showIcons() const
{
	return m_cfg->readBoolEntry("Show icons", true);
}

void ConfigAccess::setShowIcons(bool showIcons)
{
	m_cfg->writeEntry("Show icons", showIcons);
	m_cfg->sync();
}

bool ConfigAccess::slowedScrolling() const
{
	return m_cfg->readBoolEntry("Slowed scrolling", false);
}

void ConfigAccess::setSlowedScrolling(bool slowedScrolling)
{
	m_cfg->writeEntry("Slowed scrolling", slowedScrolling);
	m_cfg->sync();
}

