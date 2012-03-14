#ifndef FEEDBROWSER_H
#define FEEDBROWSER_H

#include <qobject.h>
#include <dcopobject.h>
#include <kdialogbase.h>
#include <klistview.h>

class DCOPRSSIface : public QObject, public DCOPObject
{
	K_DCOP
	Q_OBJECT
	public:
		DCOPRSSIface( QObject *parent, const char *name = 0 );

	k_dcop:
		void slotGotCategories( const QStringList &categories );

	public slots:
		void getCategories( const QString &cat = "Top" );

	signals:
		void gotCategories( const QStringList &categories );
};

class CategoryItem : public QObject, public KListViewItem
{
	Q_OBJECT
	public:
		CategoryItem( KListView *parent, const QString &category );
		CategoryItem( KListViewItem *parent, const QString &category );

		virtual void setOpen( bool open );

	private slots:
		void gotCategories( const QStringList &categories );

	private:
		void populate();
		void init();

		QString m_category;
		bool m_populated;
		DCOPRSSIface *m_dcopIface;
};

class FeedBrowserDlg : public KDialogBase
{
	Q_OBJECT
	friend class CategoryItem;
	public:
		FeedBrowserDlg( QWidget *parent, const char *name = 0 );

	private slots:
		void itemSelected( QListViewItem *item );
		void gotTopCategories( const QStringList &categories );

	private:
		void getTopCategories();

		DCOPRSSIface *m_dcopIface;
		KListView *m_feedList;
};

#endif // FEEDBROWSER_H
// vim:ts=4:sw=4:noet
