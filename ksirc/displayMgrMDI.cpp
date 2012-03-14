#include "displayMgrMDI.h"
#include "toplevel.h"

#include <unistd.h>

#include <assert.h>
#include <qpopupmenu.h>
#include <kwin.h>
#include <kaccel.h>
#include <kaction.h>
#include <kapplication.h>
#include <klocale.h>
#include <kdebug.h>

#include <kmenubar.h>

DisplayMgrMDI::DisplayMgrMDI()
{
}

DisplayMgrMDI::~DisplayMgrMDI()
{
  kdDebug(5008) << "~DisplayMgrMDI in" << endl;
    if ( m_topLevel )
      delete static_cast<MDITopLevel *>( m_topLevel );
  kdDebug(5008) << "~DisplayMgrMDI out" << endl;
}

#define DMM_MDI_ID 2351
#define DMM_DEATCH_ID 50
#define DMM_MOVEL_ID 51
#define DMM_MOVER_ID 52

void DisplayMgrMDI::newTopLevel( QWidget *w, bool show )
{
    topLevel()->addWidget( w, show );
    if(w->inherits("KSircTopLevel")){
        KSircTopLevel *t = static_cast<KSircTopLevel *>(w);
        connect(m_topLevel->tabWidget(), SIGNAL(currentChanged(QWidget *)), t, SLOT(focusChange(QWidget *)));
    }
    if(w->inherits("KMainWindow")){
        KMainWindow *t = static_cast<KMainWindow *>(w);

        QMenuBar *cmenu = t->menuBar();
        if(cmenu){
            QPopupMenu *m = new QPopupMenu(t, QCString(t->name()) + "_popup_MDI");
            m->setCheckable(true);
            m->insertItem(i18n("Detach Window"), this, SLOT(reparentReq()), 0, DMM_DEATCH_ID, 0);
            m->insertSeparator(0);
            m->insertItem(i18n("Move Tab Left"), this, SLOT(moveWindowLeft()), ALT+SHIFT+Key_Left, DMM_MOVEL_ID, 0);
	    m->insertItem(i18n("Move Tab Right"), this, SLOT(moveWindowRight()), ALT+SHIFT+Key_Right, DMM_MOVER_ID, 0);
	    /*
	     * By using an posisiton of 4 this works for KSircTopLevel
	     * and DCCTopLevel but will cause problems when we have
             * new top level windows with different menus.
	     * BEWARE THIS IS BAD, we should have a better way of doing this
             */
            cmenu->insertItem(i18n("&Window"), m, DMM_MDI_ID, 4);
            cmenu->setAccel(Key_M, DMM_MDI_ID);

            QPopupMenu *sm = new QPopupMenu(t, "settings" );
	    
	    KToggleAction *showmenu = KStdAction::showMenubar( 0, 0, t->actionCollection() );
	    showmenu->plug( sm );
	    connect( showmenu, SIGNAL(toggled(bool)), cmenu, SLOT(setShown(bool)) );

	    KSelectAction *selectTabbar = new KSelectAction(i18n("&Tab Bar"), 0, this, "tabbar" );
	    QStringList tabbaritems;
	    tabbaritems << i18n("&Top") << i18n("&Bottom");
	    selectTabbar->setItems(tabbaritems);
	    selectTabbar->setCurrentItem(1);
	    selectTabbar->plug( sm );
	    connect( selectTabbar, SIGNAL(activated(int)), this, SLOT(setTabPosition(int)) );

	    KToggleAction *showfull = KStdAction::fullScreen( 0, 0, t->actionCollection(), t );
	    showfull->plug( sm );
	    connect( showfull, SIGNAL(toggled(bool)), this, SLOT(setFullScreen(bool)) );

	    cmenu->insertItem( i18n("&Settings"), sm, -1, 5 );
        }
    }
    topLevel()->show();
}

void DisplayMgrMDI::removeTopLevel(QWidget *w )
{
    if ( !m_topLevel )
        return;

    assert( w );

    kdDebug(5008) << "DisplayMgrMDI: got removeToplevel" << endl;

    m_topLevel->removeWidget( w );

    if ( m_topLevel->widgets().count() == 0 ) {
      kdDebug(5008) << "DisplayMgrMDI: count == 0 nuking all" << endl;
      if ( !m_topLevel->closing() ) {
	kdDebug(5008) << "DisplayMgrMDI: delete m_topLevel" << endl;
	delete static_cast<MDITopLevel *>( m_topLevel );
      }
      m_topLevel = 0;
    }
    else if(m_topLevel->tabWidget()->count() == 0){
        m_topLevel->hide();
    }
}

void DisplayMgrMDI::show(QWidget *w)
{
    if ( !m_topLevel )
        return;

    m_topLevel->showWidget(w);
    m_topLevel->show();
}

void DisplayMgrMDI::hide(QWidget *w)
{
    if ( !m_topLevel )
        return;

    m_topLevel->hideWidget(w);
    if(m_topLevel->tabWidget()->count() == 0){
        m_topLevel->hide();
    }
}

void DisplayMgrMDI::raise(QWidget *w, bool takefocus)
{
    assert( m_topLevel );
    if(takefocus){
	KWin::setCurrentDesktop(KWin::KWin::windowInfo(m_topLevel->winId()).desktop());

	m_topLevel->show();
	m_topLevel->raise();
	m_topLevel->setActiveWindow();
#if KDE_IS_VERSION(3,1,92)
	KWin::activateWindow(m_topLevel->winId());
#else
	KWin::setActiveWindow(m_topLevel->winId());
#endif
    }
    m_topLevel->tabWidget()->showPage( w );
}


void DisplayMgrMDI::setCaption(QWidget *w, const QString& cap)
{
    assert( m_topLevel );

    w->setCaption(cap);

    QString esc = cap;
    esc.replace("&", "&&");
    kdDebug(5008) << "Set caption: " << esc << endl;

    m_topLevel->tabWidget()->setTabLabel( w, esc );
}

void DisplayMgrMDI::slotCycleTabsLeft()
{

    assert( m_topLevel );

    m_topLevel->previous();
}

void DisplayMgrMDI::slotCycleTabsRight()
{

    assert( m_topLevel );

    m_topLevel->next();
}

void DisplayMgrMDI::reparentReq()
{
    kdDebug(5008) << "got reparent" << endl;

    QWidget *o = kapp->focusWidget();
    QWidget *s;

    if(o == NULL){
        kdDebug(5008) << "Kapp says no widget has focus!" << endl;
        o = kapp->activeWindow();

        if(o->inherits("KMainWindow") == false)
            return;

        s = o;
    }
    else {

        kdDebug(5008) << "QWidget is: " << o->className() << " name: " << o->name("none give") << endl;


        s = o;
        while(s->parentWidget()){
            kdDebug(5008) << "Got a: " << s->className() << endl;
            s = s->parentWidget();
            if(s->inherits("KMainWindow"))
                break;

        }
    }


//    QWidget *s = m_topLevel->tabWidget()->currentPage(); can't do this since you can never reattach
    if(s){
        KMainWindow *kst = static_cast<KMainWindow *>(s);
        kdDebug(5008) << "Top is: " << kst->name("none give") <<endl;

        QMenuData *tmenu = kst->menuBar();
        if(tmenu){
            QMenuItem *menui = tmenu->findItem(DMM_MDI_ID);
            if(menui){
                QMenuData *cmenu = menui->popup();
                if(cmenu->findItem(DMM_DEATCH_ID) && cmenu->isItemChecked(DMM_DEATCH_ID)){
                    kst->reparent( topLevel()->tabWidget(), 0, QPoint( 0, 0 ), true );
                    show(kst);
                    cmenu->setItemChecked(DMM_DEATCH_ID, false);
                }
                else {
                    hide(kst);
                    kst->reparent( 0, QPoint(0,0), true );
                    cmenu->setItemChecked(DMM_DEATCH_ID, true);
                }
            }
        }
    }
}

MDITopLevel *DisplayMgrMDI::topLevel()
{
    if ( !m_topLevel )
    {
        m_topLevel = new MDITopLevel(0x0, "MDITopLevel");
        m_topLevel->show();

        KAccel *a = new KAccel( m_topLevel );
        a->insert( "cycle left", i18n("Cycle left"), QString::null, ALT+Key_Left, ALT+Key_Left, this, SLOT(slotCycleTabsLeft()) );
        a->insert( "cycle right", i18n("Cycle right"), QString::null, ALT+Key_Right, ALT+Key_Right, this, SLOT(slotCycleTabsRight()) );
    }

    return m_topLevel;
}

void DisplayMgrMDI::setTabPosition( int idx ) {

    switch ( idx ) {
	case 0:
	    m_topLevel->tabWidget()->setTabPosition(QTabWidget::Top);
	    break;
	case 1:
	    m_topLevel->tabWidget()->setTabPosition(QTabWidget::Bottom);
	    break;
    }
}


void DisplayMgrMDI::setFullScreen( bool full ) {
    if ( full )
	m_topLevel->showFullScreen();
    else
	m_topLevel->showNormal();
}



void DisplayMgrMDI::moveWindowLeft() {
    moveWindow(-1);
}

void DisplayMgrMDI::moveWindowRight() {
    moveWindow(1);
}

void DisplayMgrMDI::moveWindow(int step)
{
    if(m_topLevel->tabWidget()->count() == 0){
        return;
    }

    QWidget *w = m_topLevel->tabWidget()->currentPage();
    if(w != 0x0){
        int index = m_topLevel->tabWidget()->currentPageIndex();
        index += step;
        if(index < 0)
            return;
        if(index >= m_topLevel->tabWidget()->count())
            return;

        m_topLevel->setUpdatesEnabled(false);
        m_topLevel->hideWidget(w);
	int space = w->caption().find(" ");
	QString esc = space < 1 ? w->caption():w->caption().left(space);
	esc.replace("&", "&&");
	kdDebug(5008) << "Insert tab: " << esc << endl;
        m_topLevel->tabWidget()->insertTab( w, esc, index );
        m_topLevel->tabWidget()->showPage( w );
        m_topLevel->setUpdatesEnabled(true);
    }
}

#include "displayMgrMDI.moc"
