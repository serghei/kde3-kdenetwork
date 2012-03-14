#ifndef PAGEAUTOCONNECT_H
#define PAGEAUTOCONNECT_H
#include "page_autoconnectbase.h"

#include "ksopts.h"

class PageAutoConnect : public PageAutoConnectBase
{
    Q_OBJECT

public:
    PageAutoConnect( QWidget* parent = 0, const char* name = 0);
    ~PageAutoConnect();

    void saveConfig();
    void defaultConfig();
    void readConfig();

signals:
    void modified();

public slots:
    virtual void changed();

protected slots:
    virtual void item_changed();

    virtual void add_pressed();
    virtual void new_pressed();
    virtual void delete_pressed();

    virtual void kvl_clicked(QListViewItem *);

};

#endif // PAGEAUTOCONNECT_H
