/*
 * kntsrcfilepropsdlg.cpp
 *
 * Copyright (c) 2001, 2002, 2003 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#include "kntsrcfilepropsdlg.h"
#include "kntsrcfilepropsdlgwidget.h"
#include "xmlnewsaccess.h"
#include "newsiconmgr.h"

#include <article.h>
#include <document.h>
#include <loader.h>

#include <kapplication.h>
#include <klistbox.h>
#include <klocale.h>
#include <kurl.h>
#include <kurllabel.h>
#include <kstandarddirs.h>
#include <kglobal.h>

#include <qmultilineedit.h>
#include <qvbox.h>

using namespace RSS;

class ArticleListBoxItem : public QListBoxText
{
	public:
		ArticleListBoxItem( QListBox *listbox, const Article &article );

		const Article &article() const { return m_article; }

	private:
		Article m_article;
};

ArticleListBoxItem::ArticleListBoxItem( QListBox *listbox, const Article &article )
	: QListBoxText( listbox ),
	m_article( article )
{
	setText( article.title() );
}

KntSrcFilePropsDlg::KntSrcFilePropsDlg(KPropertiesDialog *props)
	: KPropsDlgPlugin(props)
{
	m_child = new KntSrcFilePropsDlgWidget(properties->addVBoxPage(i18n("News Resource")));
	connect(m_child->urlName, SIGNAL(leftClickedURL(const QString &)),
	        SLOT(slotOpenURL(const QString &)));
	connect(m_child->lbArticles, SIGNAL(executed(QListBoxItem *)),
	        SLOT(slotClickedArticle(QListBoxItem *)));

	Loader *loader = Loader::create();
	connect(loader, SIGNAL(loadingComplete(Loader *, Document, Status)),
	        SLOT(slotConstructUI(Loader *, Document, Status)));
	loader->loadFrom(props->item()->url(), new FileRetriever);

	connect(NewsIconMgr::self(), SIGNAL(gotIcon(const KURL &, const QPixmap &)),
	        SLOT(slotGotIcon(const KURL &, const QPixmap &)));

	m_child->show();
}

void KntSrcFilePropsDlg::slotConstructUI(Loader *, Document doc, Status status)
{
	if (status != RSS::Success)
		return;

	KURL iconURL = doc.link();
	iconURL.setEncodedPathAndQuery(QString::fromLatin1("/favicon.ico"));
	NewsIconMgr::self()->getIcon(iconURL);

	m_child->urlName->setText(doc.title());
	m_child->urlName->setURL(doc.link().url());

	m_child->mleDescription->setText(doc.description());

	Article::List::ConstIterator it = doc.articles().begin();
	Article::List::ConstIterator end = doc.articles().end();
	for (; it != end; ++it)
		new ArticleListBoxItem(m_child->lbArticles, *it);
}

void KntSrcFilePropsDlg::slotOpenURL(const QString &url)
{
	kapp->invokeBrowser(url);
}

void KntSrcFilePropsDlg::slotGotIcon(const KURL &, const QPixmap &pixmap)
{
	m_child->pixmapIcon->setPixmap(pixmap);
}

void KntSrcFilePropsDlg::slotClickedArticle(QListBoxItem *item)
{
	ArticleListBoxItem *articleItem = static_cast<ArticleListBoxItem *>(item);
	slotOpenURL(articleItem->article().link().url());
}

QObject *KntSrcFilePropsFactory::createObject(QObject *parent, const char *,
		const char *classname, const QStringList &)
{
	if (QString::fromLatin1(classname) == "KPropsDlgPlugin")
	{
		if (!parent->inherits("KPropertiesDialog"))
			return 0L;

		QObject *obj = new KntSrcFilePropsDlg(static_cast<KPropertiesDialog *>(parent));
		return obj;
	}
	return 0L;
}

extern "C"
{
	KDE_EXPORT void *init_libkntsrcfilepropsdlg()
	{
        KGlobal::locale()->insertCatalogue( "knewsticker" );
		return new KntSrcFilePropsFactory();
	}
}

#include "kntsrcfilepropsdlg.moc"
