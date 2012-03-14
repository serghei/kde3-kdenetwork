#ifndef __displaymgrmdi_h__
#define __displaymgrmdi_h__

#include "displayMgr.h"
#include "mditoplevel.h"

class DisplayMgrMDI : public QObject, public DisplayMgr
{
    Q_OBJECT
public:
    DisplayMgrMDI();
    virtual ~DisplayMgrMDI();

    virtual void newTopLevel(QWidget *, bool show = FALSE);
    virtual void removeTopLevel(QWidget *);

    virtual void show(QWidget *);
    virtual void hide(QWidget *);
    virtual void raise(QWidget *, bool takefocus = false);

    virtual void setCaption(QWidget *, const QString&);

public slots:
    virtual void slotCycleTabsLeft();
    virtual void slotCycleTabsRight();

protected slots:
    void reparentReq();
    void moveWindowLeft();
    void moveWindowRight();
    void setTabPosition(int);
    void setFullScreen(bool);

protected:
    void moveWindow(int step);
    void refreshMenuEntry();

private:
    MDITopLevel *topLevel();

    QGuardedPtr<MDITopLevel> m_topLevel;
    int m_moveRightTab, m_moveLeftTab;
    QPopupMenu *m_popupWindows;
};


#endif

