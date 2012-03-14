
#include <qguardedptr.h>

#include <qtabbar.h>

#include <kapplication.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kpopupmenu.h>
#include <klocale.h>
#include <kaction.h>

#include "mditoplevel.h"
#include "toplevel.h"
#include "servercontroller.h"

#define ID_CLOSE 5

void KSTabWidget::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == RightButton){
        QPoint p = tabBar()->mapFromParent(e->pos());
	QTab *tab = tabBar()->selectTab(p);
	if(tab){
	    int id = tab->identifier();
            emit showContexMenu(page(id), tabBar()->mapToGlobal(p));
	}
    }
}

MDITopLevel::MDITopLevel(QWidget *parent, const char *name)
    : KMainWindow(parent, name)
{
    m_closing = false;

    m_tab = new KSTabWidget( this );
    m_tab->setTabPosition( QTabWidget::Bottom );

    setCentralWidget( m_tab );

    connect( m_tab, SIGNAL( currentChanged( QWidget * ) ),
             this, SLOT( slotCurrentChanged( QWidget * ) ) );

    connect( m_tab, SIGNAL( showContexMenu(QWidget *, const QPoint &) ),
             this, SLOT( slotShowContexMenu(QWidget *, const QPoint &) ) );

    KConfig *config = kapp->config();
    config->setGroup("MDI");
    QSize s( 600, 360 );
    resize(config->readSizeEntry( "TopLevelSize", &s ));

    m_dirtyIcon = UserIcon( "star" );
    m_addressedIcon = UserIcon( "info" );

    m_pop = new KPopupMenu(m_tab, "");
    m_pop->insertItem( SmallIcon("fileclose"), i18n("Close"), this, SLOT( slotCloseLastWid() ));

}

MDITopLevel::~MDITopLevel()
{
  kdDebug(5008) << "~MDITopLevel in" << endl;

    KConfig *config = kapp->config();
    config->setGroup( "MDI" );
    config->writeEntry( "TopLevelSize", this->size() );
    config->sync();

    QPtrListIterator<QWidget> it( m_tabWidgets );
    for (; it.current(); ++it )
        it.current()->disconnect( this, 0 );
  kdDebug(5008) << "~MDITopLevel out" << endl;

}

void MDITopLevel::addWidget( QWidget *widget, bool show )
{
    if ( m_tabWidgets.containsRef( widget ) )
        return;

    kdDebug(5008) << "In add widget" << endl;

    widget->reparent( m_tab, 0, QPoint( 0, 0 ), show );

    if(show == TRUE){
        showWidget(widget);
    }

    m_tabWidgets.append( widget );

    connect( widget, SIGNAL( destroyed() ) ,
             this, SLOT( slotWidgetDestroyed() ) );

    connect( widget, SIGNAL( changeChannel( const QString &, const QString & ) ),
             this, SLOT( slotChangeChannelName( const QString &, const QString & ) ) );

    widget->installEventFilter( this );

    connect( widget, SIGNAL( changed( bool, QString ) ),
             this, SLOT( slotMarkPageDirty( bool ) ) );
}

void MDITopLevel::removeWidget( QWidget *widget )
{
    if (m_closing)
        return;
    m_tabWidgets.removeRef( widget );
    removeFromAddressedList( widget );
    m_tab->removePage( widget );
    widget->removeEventFilter( this );
    widget->disconnect( this, 0 );
}

void MDITopLevel::hideWidget( QWidget *widget )
{
    m_tab->removePage( widget );
    widget->hide();
}

void MDITopLevel::showWidget( QWidget *widget )
{
    if(m_tab->indexOf(widget) == -1){
        int space = widget->caption().find(" ");
        QString cap = space < 1 ? widget->caption():widget->caption().left(space);
        m_tab->addTab( widget,  cap);
        m_tab->showPage( widget );
        m_tab->setCurrentPage( m_tab->indexOf(widget) );
    }
}

void MDITopLevel::next()
{
	if (m_tab->currentPageIndex() < m_tab->count() - 1)
		m_tab->setCurrentPage(m_tab->currentPageIndex() + 1);
	else
		m_tab->setCurrentPage(0);
}

void MDITopLevel::previous()
{
	if (m_tab->currentPageIndex() > 0)
		m_tab->setCurrentPage(m_tab->currentPageIndex() - 1);
	else
		m_tab->setCurrentPage(m_tab->count() - 1);
}

void MDITopLevel::closeEvent( QCloseEvent *ev )
{
    m_closing = true;
    // Don't use iterators on a list while deleting elements
    // from it (Antonio)
    int i = 0;
    kdDebug(5008) << "Mdi got close event " << endl;
    while ( m_tabWidgets.count() && (i++ < 100000)) {
        kdDebug(5008) << "MDITopLevel trying to close: " << m_tabWidgets.first()->name() << endl;
        QGuardedPtr<QWidget> w = m_tabWidgets.take(0);
        w->show();
        w->close( false );
        if(w)
            delete (QWidget *)w;
    }

    KMainWindow::closeEvent( ev );
    m_closing = false;
}

void MDITopLevel::slotWidgetDestroyed()
{
    const QWidget *widget = static_cast<const QWidget *>( sender() );

    m_tabWidgets.removeRef( widget );
    removeFromAddressedList( widget );
}

bool MDITopLevel::eventFilter( QObject *obj, QEvent *ev )
{
    if ( ev->type() != QEvent::CaptionChange )
        return false;

    QWidget *widget = dynamic_cast<QWidget *>( obj );

    if ( !widget || !m_tabWidgets.containsRef( widget ) )
        return false;

    if ( m_tab->currentPage() == widget )
        setPlainCaption( widget->caption() );

    return false;
}

void MDITopLevel::slotCurrentChanged( QWidget *page )
{

    m_tab->setTabIconSet( page, QIconSet() );
    removeFromAddressedList( page );

    setPlainCaption( page->QWidget::caption() );

    KSircTopLevel *kst = dynamic_cast<KSircTopLevel *>( page );
    if ( !kst )
        return;
    kst->lineEdit()->setFocus();
}

void MDITopLevel::slotMarkPageDirty( bool addressed )
{
    // This is called when a line appeared in this channel.
    // addressed is true if it was addressed to the user
    KMainWindow *window = dynamic_cast<KMainWindow *>( const_cast<QObject *>( sender() ) );

    if ( !window )
        return;

    if ( window != m_tab->currentPage() )
    {
        if ( m_addressed.containsRef( window ) )
            addressed = true;
        else if ( addressed ) {
            m_addressed.append( window );
        }
        m_tab->setTabIconSet( window, addressed ? m_addressedIcon : m_dirtyIcon );
    }
}

void MDITopLevel::slotChangeChannelName( const QString &, const QString &channelName )
{
    KMainWindow *window = dynamic_cast<KMainWindow *>( const_cast<QObject *>( sender() ) );

    if ( !window )
        return;

    QString esc = channelName;
    esc.replace("&", "&&");
    m_tab->setTabLabel( window, esc );
    removeFromAddressedList( window );
}

void MDITopLevel::removeFromAddressedList( const QWidget* w )
{
    // If this tab was showing a "you're being talked to" icon, remove it
    // and tell the servercontroller (so that it can update the docked icon).
    m_addressed.removeRef( w );
}

void MDITopLevel::slotShowContexMenu(QWidget *w, const QPoint &p)
{
    m_last_pop_wid = w;
    m_pop->popup(p);
}

void MDITopLevel::slotCloseLastWid()
{
	m_last_pop_wid->close();

}

#include "mditoplevel.moc"

// vim: ts=4 sw=4 et
