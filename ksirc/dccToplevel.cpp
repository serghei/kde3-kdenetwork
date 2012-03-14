
#include <qvbox.h>
#include <qpopupmenu.h>

#include <kaction.h>
#include <kstdaction.h>
#include <klocale.h>
#include <kmenubar.h>

#include "dccManager.h"
#include "dccToplevel.h"
#include "displayMgr.h"

extern DisplayMgr *displayMgr;

#define DTL_WINDOW_ID 10

dccTopLevel::dccTopLevel(QWidget *parent, const char *name)
    : KMainWindow(parent, name, Qt::WDestructiveClose)
{
    m_mgr = new dccManager(this, QCString(QObject::name()) + "_dccManager");
//    m_mgr->show();
    setCentralWidget(m_mgr);

    connect(m_mgr, SIGNAL(changed(bool, QString)), this, SIGNAL(changed(bool, QString)));

    QPopupMenu *win = new QPopupMenu(this, QCString(QObject::name()) + "_popup_window");
    KAction *act = KStdAction::close(this, SLOT( close() ), actionCollection() );
    act->plug(win);

    menuBar()->insertItem(i18n("&File"), win, DTL_WINDOW_ID, -1);
    menuBar()->setAccel(Key_F, DTL_WINDOW_ID);


}

dccTopLevel::~dccTopLevel()
{
}

void dccTopLevel::close()
{

    displayMgr->hide(this);

}

#include "dccToplevel.moc"

