
#ifndef _DCCTOPLEVEL_H_
#define _DCCTOPLEVEL_H_

#include <kmainwindow.h>

class dccManager;


class dccTopLevel : public KMainWindow
{
Q_OBJECT
public:

    dccTopLevel(QWidget *parent = 0, const char *name = 0);
    virtual ~dccTopLevel();

    dccManager *mgr() { return m_mgr; }

signals:
    void changeChannel(const QString&,const QString&);
    void changed(bool, QString);


public slots:
    void close(void);

private:
    dccManager *m_mgr;
};

#endif

