/*
 * knewstickerconfig.cpp
 *
 * Copyright (c) 2000, 2001 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#include "knewstickerconfig.h"
#include "configaccess.h"
#include "newsengine.h"
#include "newsiconmgr.h"
#include "newssourcedlgimpl.h"

#include <kapplication.h>
#include <kcolorbutton.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <kfontdialog.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klistview.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kpopupmenu.h>

#include <qcursor.h>
#include <qcheckbox.h>
#include <qdragobject.h>
#include <qlabel.h>
#include <qslider.h>
#include <qregexp.h>

CategoryItem::CategoryItem(QListView *parent, const QString &text)
	: QListViewItem(parent, text)
{
	setOpen(true);
}

void CategoryItem::setOpen(bool open)
{
	if (open)
		setPixmap(0, SmallIcon(QString::fromLatin1("folder_open")));
	else
		setPixmap(0, SmallIcon(QString::fromLatin1("folder")));

	QListViewItem::setOpen(open);
}

NewsSourceItem::NewsSourceItem(KNewsTickerConfig *kcm, CategoryItem *parent,
		const NewsSourceBase::Data &nsd)
	: QCheckListItem(parent, QString::null, QCheckListItem::CheckBox),
	m_parent(parent),
	m_kcm(kcm)
{
	setData(nsd);
}

NewsSourceBase::Data NewsSourceItem::data() const
{
	NewsSourceBase::Data nsd;
	nsd.enabled = isOn();
	nsd.name = text(0);
	nsd.sourceFile = text(1);
	nsd.maxArticles = text(2).toUInt();
	nsd.icon = m_icon;
	nsd.isProgram = m_isProgram;
	nsd.subject = m_subject;
	return nsd;
}

void NewsSourceItem::setData(const NewsSourceBase::Data &nsd)
{
	setOn(nsd.enabled);
	setText(0, nsd.name);
	setText(1, nsd.sourceFile);
	setText(2, QString::number(nsd.maxArticles));

	m_icon = nsd.icon;
	m_isProgram = nsd.isProgram;
	m_subject = nsd.subject;
	m_kcm->getNewsIcon(this, KURL( m_icon ));
}

void NewsSourceItem::setIcon(const QPixmap &pixmap)
{
	setPixmap(0, pixmap);
}

KNewsTickerConfig::KNewsTickerConfig(ConfigAccess *cfg, QWidget *parent,  const char *name)
	: KDialogBase(parent, name, true, i18n("Configuration"), Ok|Close),
	m_cfg(cfg),
	m_child(new KNewsTickerConfigWidget(this)),
	m_newsIconMgr(NewsIconMgr::self())
{
	setMainWidget(m_child);

	// Change various properties of the view which the Qt Designer cannot
	// set at design time yet.
	m_child->niInterval->setLabel(i18n("News query interval:"));
	m_child->niInterval->setRange(4, 180);

	m_child->lvNewsSources->setShowSortIndicator(true);
	m_child->lvNewsSources->setSelectionModeExt(KListView::Extended);
	m_child->lvNewsSources->setAcceptDrops(true);
	m_child->lvNewsSources->viewport()->setAcceptDrops(true);
	m_child->lvNewsSources->viewport()->installEventFilter(this);
	m_child->lvNewsSources->installEventFilter(this);

	connect(m_newsIconMgr, SIGNAL(gotIcon(const KURL &, const QPixmap &)), SLOT(slotGotNewsIcon(const KURL &, const QPixmap &)));
	connect(m_child->bChooseFont, SIGNAL(clicked()), SLOT(slotChooseFont()));

	connect(m_child->lvNewsSources, SIGNAL(contextMenu(KListView *, QListViewItem *, const QPoint &)),
			SLOT(slotNewsSourceContextMenu(KListView *, QListViewItem *, const QPoint &)));
	connect(m_child->lvNewsSources, SIGNAL(selectionChanged()),
			SLOT(slotNewsSourceSelectionChanged()));
	connect(m_child->lvNewsSources, SIGNAL(doubleClicked(QListViewItem *, const QPoint &, int)),
			SLOT(slotModifyNewsSource(QListViewItem *, const QPoint &, int)));
	connect(m_child->bAddNewsSource, SIGNAL(clicked()), SLOT(slotAddNewsSource()));
	connect(m_child->bRemoveNewsSource, SIGNAL(clicked()), SLOT(slotRemoveNewsSource()));
	connect(m_child->bModifyNewsSource, SIGNAL(clicked()), SLOT(slotModifyNewsSource()));

	connect(m_child->lvFilters, SIGNAL(selectionChanged(QListViewItem *)),
			SLOT(slotFilterSelectionChanged(QListViewItem *)));
	connect(m_child->comboFilterAction, SIGNAL(activated(const QString &)),
			SLOT(slotFilterActionChanged(const QString &)));
	connect(m_child->comboFilterNewsSource, SIGNAL(activated(const QString &)),
			SLOT(slotFilterNewsSourceChanged(const QString &)));
	connect(m_child->comboFilterCondition, SIGNAL(activated(const QString &)),
			SLOT(slotFilterConditionChanged(const QString &)));
	connect(m_child->leFilterExpression, SIGNAL(textChanged(const QString &)),
			SLOT(slotFilterExpressionChanged(const QString &)));
	connect(m_child->bAddFilter, SIGNAL(clicked()), SLOT(slotAddFilter()));
	connect(m_child->bRemoveFilter, SIGNAL(clicked()), SLOT(slotRemoveFilter()));

	load();

	m_child->show();
}

void KNewsTickerConfig::load()
{
	m_child->comboFilterNewsSource->clear();
	m_child->comboFilterNewsSource->insertItem(i18n("All News Sources"));

	m_child->niInterval->setValue(m_cfg->interval());
	m_child->sliderMouseWheelSpeed->setValue(m_cfg->mouseWheelSpeed());
	m_child->cbCustomNames->setChecked(m_cfg->customNames());
	m_child->cbScrollMostRecentOnly->setChecked(m_cfg->scrollMostRecentOnly());
	m_child->cbSlowedScrolling->setChecked(m_cfg->slowedScrolling());

	m_child->sliderScrollSpeed->setValue(m_cfg->scrollingSpeed());
	m_child->comboDirection->setCurrentItem(m_cfg->scrollingDirection());

	m_font = m_cfg->font();
	m_child->colorForeground->setColor(m_cfg->foregroundColor());
	m_child->colorBackground->setColor(m_cfg->backgroundColor());
	m_child->colorHighlighted->setColor(m_cfg->highlightedColor());
	m_child->cbUnderlineHighlighted->setChecked(m_cfg->underlineHighlighted());
	m_child->cbShowIcons->setChecked(m_cfg->showIcons());

	m_child->lvNewsSources->clear();
	QStringList nsList = m_cfg->newsSources();
	for (QStringList::Iterator it = nsList.begin(); it != nsList.end(); ++it)
		addNewsSource(m_cfg->newsSource((*it))->data());

	ArticleFilter::List filterList = m_cfg->filters();
	ArticleFilter::List::ConstIterator it = filterList.begin();
	ArticleFilter::List::ConstIterator end = filterList.end();
	for (; it != end; ++it)
		addFilter(m_cfg->filter((*it)));

	slotNewsSourceSelectionChanged();
}

void KNewsTickerConfig::save()
{
	m_cfg->setInterval(m_child->niInterval->value());
	m_cfg->setMouseWheelSpeed(m_child->sliderMouseWheelSpeed->value());
	m_cfg->setCustomNames(m_child->cbCustomNames->isChecked());
	m_cfg->setScrollMostRecentOnly(m_child->cbScrollMostRecentOnly->isChecked());
	m_cfg->setSlowedScrolling(m_child->cbSlowedScrolling->isChecked());

	m_cfg->setScrollingSpeed(m_child->sliderScrollSpeed->value());
	m_cfg->setScrollingDirection(static_cast<ConfigAccess::Direction>(m_child->comboDirection->currentItem()));

	m_cfg->setFont(m_font);
	m_cfg->setForegroundColor(m_child->colorForeground->color());
	m_cfg->setBackgroundColor(m_child->colorBackground->color());
	m_cfg->setHighlightedColor(m_child->colorHighlighted->color());
	m_cfg->setUnderlineHighlighted(m_child->cbUnderlineHighlighted->isChecked());
	m_cfg->setShowIcons(m_child->cbShowIcons->isChecked());

	QStringList newsSources;
	for (QListViewItemIterator it(m_child->lvNewsSources); it.current(); it++)
		if (NewsSourceItem *item = dynamic_cast<NewsSourceItem *>(it.current())) {
			newsSources += item->data().name;
			m_cfg->setNewsSource(item->data());
		}
	m_cfg->setNewsSources(newsSources);

	ArticleFilter::List filters;
	ArticleFilter f;
	unsigned int i = 0;
	for (QListViewItemIterator it(m_child->lvFilters); it.current(); it++)
		if (QCheckListItem *item = dynamic_cast<QCheckListItem *>(it.current())) {
			filters.append(i);
			f.setAction(item->text(0));
			f.setNewsSource(item->text(2));
			f.setCondition(item->text(4));
			f.setExpression(item->text(5));
			f.setEnabled(item->isOn());
			f.setId(i++);
			m_cfg->setFilter(f);
		}
	m_cfg->setFilters(filters);
}

bool KNewsTickerConfig::eventFilter(QObject *o, QEvent *e)
{
	//<HACK>
	// 	"if ( e->type() == QEvent::DragEnter ) {" shoult normaly be enough. but there must be a bug somewhere in KListView.
	if ( e->type() == QEvent::DragMove ) {
	//</HACK>
		QDragEnterEvent *d = (QDragEnterEvent*)e;
		d->accept(QTextDrag::canDecode(d));
		return true;                        // eat event
	}
	else if ( e->type() == QEvent::Drop) {
		QDropEvent *d = (QDropEvent*)e;
		QString newSourceUrl;
		if ( QTextDrag::decode(d, newSourceUrl) ) {
			// <HACK>
			// This is just for http://www.webreference.com/services/news/
			newSourceUrl = newSourceUrl.replace( QRegExp("^view-source:http%3A//"), "http://" );
			// </HACK>
			newSourceUrl = newSourceUrl.stripWhiteSpace();

			//look for a new Name of Source:
			QString name = i18n("Unknown");
			bool validName = false;
			for (QListViewItemIterator it(m_child->lvNewsSources); it.current(); it++) {
				if (it.current()->text(0) == name) {
					validName = false;
					break;
				} else {
					validName = true;
				}
			}
			int i = 0;
			while (validName == false) {
				name = i18n("Unknown %1").arg(i);
				for (QListViewItemIterator it(m_child->lvNewsSources); it.current(); it++) {
					if (it.current()->text(0) == name) {
						i++;
						validName = false;
						break;
					} else {
						validName = true;
					}
				}
			}

			NewsSourceBase::Data nsd(name, newSourceUrl, "" , NewsSourceBase::Computers , 10, true, false);
			NewsSourceDlgImpl nsDlg(this, 0L, true);
			connect(&nsDlg, SIGNAL(newsSource(const NewsSourceBase::Data &)),
			SLOT(slotAddNewsSource(const NewsSourceBase::Data &)));
			nsDlg.setup(nsd, false);
			nsDlg.exec();
		}
		return true;                        // eat event
	}
	return QWidget::eventFilter( o, e );
}

void KNewsTickerConfig::resizeEvent(QResizeEvent *)
{
	m_child->resize(width(), height());
}

void KNewsTickerConfig::addNewsSource(const NewsSourceBase::Data &nsd,
		bool select)
{
	CategoryItem *catItem = 0L;

	for (QListViewItemIterator it(m_child->lvNewsSources); it.current(); it++) {
		if (it.current()->text(0) == NewsSourceBase::subjectText(nsd.subject)) {
			catItem = static_cast<CategoryItem *>(it.current());
			break;
		}
	}

	if (!catItem)
		catItem = new CategoryItem(m_child->lvNewsSources,
				NewsSourceBase::subjectText(nsd.subject));

	NewsSourceItem *item = new NewsSourceItem(this, catItem, nsd);
	if (select)
		m_child->lvNewsSources->setCurrentItem(item);

	m_child->comboFilterNewsSource->insertItem(item->data().name);
}

void KNewsTickerConfig::addFilter(const ArticleFilter &fd)
{
	QCheckListItem *item = new QCheckListItem(m_child->lvFilters, fd.action(), QCheckListItem::CheckBox);
	item->setOn(fd.enabled());
	item->setText(1, m_child->lArticles->text());
	item->setText(2, fd.newsSource());
	item->setText(3, m_child->lHeadlines->text());
	item->setText(4, fd.condition());
	item->setText(5, fd.expression());
}

void KNewsTickerConfig::removeNewsSource()
{
	if (KMessageBox::warningContinueCancel(this, i18n("<p>Do you really want to remove %n news"
							" source?</p>",
						 "<p>Do you really want to remove these %n news"
							" sources?</p>",
						 m_child->lvNewsSources->selectedItems().count()), QString::null, KStdGuiItem::del()) == KMessageBox::Continue) {
		int itemCount = m_child->lvNewsSources->selectedItems().count();
		for (int j = 0; j < itemCount; j++) {
			if (m_child->lvNewsSources->selectedItems().isEmpty()) { break; }
			QListViewItem *item = m_child->lvNewsSources->selectedItems().take(0);
			for (int i = 0; i < m_child->comboFilterNewsSource->count(); i++)
				if (m_child->comboFilterNewsSource->text(i) == item->text(0)) {
					m_child->comboFilterNewsSource->removeItem(i);
					break;
				}
			if (dynamic_cast<NewsSourceItem *>(item) && item->parent()->childCount() == 1)
				delete item->parent();
			else
				delete item;
		}
		m_child->bRemoveNewsSource->setEnabled(false);
		m_child->bModifyNewsSource->setEnabled(false);
	}
}

void KNewsTickerConfig::removeFilter(QListViewItem *item)
{
	if (KMessageBox::warningContinueCancel(this, i18n("<p>Do you really want to remove the selected"
					" filter?</p>"), QString::null, KStdGuiItem::del()) == KMessageBox::Continue) {
		delete item;
		m_child->bRemoveFilter->setEnabled(false);
	}
}

void KNewsTickerConfig::slotNewsSourceContextMenu(KListView *, QListViewItem *item, const QPoint &)
{
	if (!dynamic_cast<NewsSourceItem *>(item))
		return;

	KPopupMenu *menu = new KPopupMenu();

	QPixmap addIcon = SmallIcon(QString::fromLatin1("news_subscribe"));
	QPixmap modifyIcon = SmallIcon(QString::fromLatin1("edit"));
	QPixmap removeIcon = SmallIcon(QString::fromLatin1("news_unsubscribe"));
	QPixmap logoIcon = SmallIcon(QString::fromLatin1("knewsticker"));

	menu->insertTitle(logoIcon, i18n("Edit News Source"));
	menu->insertItem(addIcon, i18n("&Add News Source"), 0);
	if (item) {
		menu->insertItem(modifyIcon, i18n("&Modify '%1'").arg(item->text(0)), 1);
		if (m_child->lvNewsSources->selectedItems().count() == 1) {
			menu->insertItem(removeIcon, i18n("&Remove '%1'").arg(item->text(0)), 2);
		} else {
			menu->insertItem(removeIcon, i18n("&Remove News Sources"), 2);
		}
	} else {
		menu->insertItem(modifyIcon, i18n("&Modify News Source"), 1);
		menu->insertItem(removeIcon, i18n("&Remove News Source"), 2);
		menu->setItemEnabled(1, false);
		menu->setItemEnabled(2, false);
	}

	switch (menu->exec(QCursor::pos())) {
		case 0: slotAddNewsSource(); break;
		case 1: modifyNewsSource(item); break;
		case 2: removeNewsSource(); break;
	}

	delete menu;
}

void KNewsTickerConfig::slotChooseFont()
{
	KFontDialog fd(this, "Font Dialog", false, true);

	fd.setFont(m_font);

	if (fd.exec() == KFontDialog::Accepted) {
		if (m_font != fd.font()) {
			m_font = fd.font();
		}
	}
}

void KNewsTickerConfig::slotAddNewsSource()
{
	NewsSourceDlgImpl nsDlg(this, 0L, true);
	connect(&nsDlg, SIGNAL(newsSource(const NewsSourceBase::Data &)),
			SLOT(slotAddNewsSource(const NewsSourceBase::Data &)));
	nsDlg.exec();
}

void KNewsTickerConfig::slotAddNewsSource(const NewsSourceBase::Data &nsd)
{
	addNewsSource(nsd);
}

void KNewsTickerConfig::slotModifyNewsSource()
{
	if ((m_modifyItem = dynamic_cast<NewsSourceItem *>(m_child->lvNewsSources->selectedItems().take(0))))
		openModifyDialog();
}

void KNewsTickerConfig::slotModifyNewsSource(const NewsSourceBase::Data &nsd)
{
	if (m_modifyItem->data().subject != nsd.subject) {
		QListViewItem *parentItem = m_modifyItem->parentItem();
		parentItem->takeItem(m_modifyItem);
		if (parentItem->childCount() == 0)
			delete parentItem;

		CategoryItem *catItem = 0L;

		for (QListViewItemIterator it(m_child->lvNewsSources); it.current(); it++) {
			if (it.current()->text(0) == NewsSourceBase::subjectText(nsd.subject)) {
				catItem = static_cast<CategoryItem *>(it.current());
				break;
			}
		}

		if (!catItem)
			catItem = new CategoryItem(m_child->lvNewsSources,
			                           NewsSourceBase::subjectText(nsd.subject));

		catItem->insertItem(m_modifyItem);
	}

	m_modifyItem->setData(nsd);
}

void KNewsTickerConfig::slotModifyNewsSource(QListViewItem *item, const QPoint &, int)
{
	if (dynamic_cast<NewsSourceItem *>(item))
		modifyNewsSource(item);
}

void KNewsTickerConfig::modifyNewsSource(QListViewItem *item)
{
	if ((m_modifyItem = dynamic_cast<NewsSourceItem *>(item)))
		openModifyDialog();
}

void KNewsTickerConfig::openModifyDialog()
{
	NewsSourceDlgImpl nsDlg(this, 0L, true);
	connect(&nsDlg, SIGNAL(newsSource(const NewsSourceBase::Data &)),
			SLOT(slotModifyNewsSource(const NewsSourceBase::Data &)));
	nsDlg.setup(m_modifyItem->data(), true);
	nsDlg.exec();
}

void KNewsTickerConfig::slotAddFilter()
{
	ArticleFilter fd;
	fd.setAction(m_child->comboFilterAction->currentText());
	fd.setNewsSource(m_child->comboFilterNewsSource->currentText());
	fd.setCondition(m_child->comboFilterCondition->currentText());
	fd.setExpression(m_child->leFilterExpression->text());
	fd.setEnabled(true);
	addFilter(fd);
}

void KNewsTickerConfig::slotRemoveNewsSource()
{
    removeNewsSource();
}

void KNewsTickerConfig::slotRemoveFilter()
{
    QListViewItem *item=m_child->lvFilters->selectedItem();
    if(!item)
        return;
    removeFilter(item);
}

void KNewsTickerConfig::slotNewsSourceSelectionChanged()
{
	m_child->bRemoveNewsSource->setEnabled(! m_child->lvNewsSources->selectedItems().isEmpty());
	m_child->bModifyNewsSource->setEnabled(m_child->lvNewsSources->selectedItems().count() == 1);
}

void KNewsTickerConfig::slotFilterSelectionChanged(QListViewItem *item)
{
	for (int i = 0; i < m_child->comboFilterAction->count(); i++)
		if (m_child->comboFilterAction->text(i) == item->text(0)) {
			m_child->comboFilterAction->setCurrentItem(i);
			break;
		}

	for (int i = 0; i < m_child->comboFilterNewsSource->count(); i++)
		if (m_child->comboFilterNewsSource->text(i) == item->text(2)) {
			m_child->comboFilterNewsSource->setCurrentItem(i);
			break;
		}

	for (int i = 0; i < m_child->comboFilterCondition->count(); i++)
		if (m_child->comboFilterCondition->text(i) == item->text(4)) {
			m_child->comboFilterCondition->setCurrentItem(i);
			break;
		}

	m_child->leFilterExpression->setText(item->text(5));

	m_child->bRemoveFilter->setEnabled(item);
}

void KNewsTickerConfig::slotFilterActionChanged(const QString &action)
{
	QListViewItem *item = m_child->lvFilters->selectedItem();

	if (item) {
		item->setText(0, action);
	}
}

void KNewsTickerConfig::slotFilterNewsSourceChanged(const QString &newsSource)
{
	QListViewItem *item = m_child->lvFilters->selectedItem();

	if (item) {
		item->setText(2, newsSource);
	}
}

void KNewsTickerConfig::slotFilterConditionChanged(const QString &condition)
{
	QListViewItem *item = m_child->lvFilters->selectedItem();

	if (item) {
		item->setText(4, condition);
	}
}

void KNewsTickerConfig::slotFilterExpressionChanged(const QString &expression)
{
	QListViewItem *item = m_child->lvFilters->selectedItem();

	if (item) {
		item->setText(5, expression);
	}
}

void KNewsTickerConfig::getNewsIcon(NewsSourceItem *item, const KURL &url)
{
	m_itemIconMap[url.url()] = item;
	m_newsIconMgr->getIcon(url);
}

void KNewsTickerConfig::slotGotNewsIcon(const KURL &url, const QPixmap &pixmap)
{
	if (m_itemIconMap.find(url.url()) == m_itemIconMap.end()) {
		kdDebug(5005) << "Got unknown icon for URL " << url << endl;
		return;
	}
	m_itemIconMap[url.url()]->setIcon(pixmap);
	m_itemIconMap.remove(url.url());
}

void KNewsTickerConfig::slotOk()
{
	save();
	accept();
}

#include "knewstickerconfig.moc"
