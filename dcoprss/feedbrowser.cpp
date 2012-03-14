#include "feedbrowser.h"

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <klistview.h>
#include <klocale.h>
#include <dcopclient.h>

#include <qcstring.h>
#include <qdatastream.h>
#include <qvbox.h>

CategoryItem::CategoryItem( KListView *parent, const QString &category )
	: KListViewItem( parent ),
	m_category( category )
{
	init();
}

CategoryItem::CategoryItem( KListViewItem *parent, const QString &category )
	: KListViewItem( parent ),
	m_category( category )
{
	init();
}

void CategoryItem::init()
{
	m_populated = false;
	m_dcopIface = 0;

	setText( 0, m_category.mid( m_category.findRev( '/' ) + 1 ).replace( '_', ' ' ) );
}

void CategoryItem::setOpen( bool open )
{
	if ( open && !m_populated ) {
		populate();
		m_populated = true;
	}
	KListViewItem::setOpen( open );
}

void CategoryItem::populate()
{
	m_dcopIface = new DCOPRSSIface( this, "m_dcopIface" );
	connect( m_dcopIface, SIGNAL( gotCategories( const QStringList & ) ),
	         this, SLOT( gotCategories( const QStringList & ) ) );
	m_dcopIface->getCategories( m_category );
}

void CategoryItem::gotCategories( const QStringList &categories )
{
	delete m_dcopIface;
	m_dcopIface = 0;

	QStringList::ConstIterator it = categories.begin();
	QStringList::ConstIterator end = categories.end();
	for ( ; it != end; ++it )
		new CategoryItem( this, *it );

	if ( !categories.isEmpty() )
		KListViewItem::setOpen( true );
}

DCOPRSSIface::DCOPRSSIface( QObject *parent, const char *name ) :
	QObject( parent, name ), DCOPObject( "FeedBrowser" )
{
	connectDCOPSignal( "rssservice", "RSSQuery", "gotCategories(QStringList)",
	                   "slotGotCategories(QStringList)", false );
}

void DCOPRSSIface::getCategories( const QString &cat )
{
	QByteArray data;
	QDataStream stream( data, IO_WriteOnly );
	stream << cat;
	kapp->dcopClient()->send( "rssservice", "RSSQuery",
	                          "getCategories(QString)", data );
}

void DCOPRSSIface::slotGotCategories( const QStringList &categories )
{
	emit gotCategories( categories );
}

FeedBrowserDlg::FeedBrowserDlg( QWidget *parent, const char *name )
	: KDialogBase( parent, name, true, i18n( "DCOPRSS Feed Browser" ),
	               Close, Close, true )
{
	m_dcopIface = new DCOPRSSIface( this, "m_dcopIface" );
	connect( m_dcopIface, SIGNAL( gotCategories( const QStringList & ) ),
	         this, SLOT( gotTopCategories( const QStringList & ) ) );

	QVBox *mainWidget = makeVBoxMainWidget();

	m_feedList = new KListView( mainWidget, "m_feedList" );
	m_feedList->setAllColumnsShowFocus( true );
	m_feedList->setRootIsDecorated( true );
	m_feedList->addColumn( i18n( "Name" ) );
	connect( m_feedList, SIGNAL( executed( QListViewItem * ) ),
	         this, SLOT( itemSelected( QListViewItem * ) ) );
	connect( m_feedList, SIGNAL( returnPressed( QListViewItem * ) ),
	         this, SLOT( itemSelected( QListViewItem * ) ) );

	resize( 500, 400 );

	getTopCategories();
}

void FeedBrowserDlg::getTopCategories()
{
	m_dcopIface->getCategories( "Top" );
}

void FeedBrowserDlg::gotTopCategories( const QStringList &categories )
{
	QStringList::ConstIterator it = categories.begin();
	QStringList::ConstIterator end = categories.end();
	for ( ; it != end; ++it )
		new CategoryItem( m_feedList, *it );
}

void FeedBrowserDlg::itemSelected( QListViewItem *item )
{
	item->setOpen( !item->isOpen() );
}

int main( int argc, char **argv )
{
	KGlobal::locale()->setMainCatalogue( "dcoprss" );
	KAboutData aboutData( "feedbrowser", I18N_NOOP( "Feed Browser" ), "0.1" );
	KCmdLineArgs::init( argc, argv, &aboutData );
	KApplication app;
	FeedBrowserDlg *dlg = new FeedBrowserDlg( 0 );
	app.setMainWidget( dlg );
	dlg->show();
	return app.exec();
}

#include "feedbrowser.moc"
