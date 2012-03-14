/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _DCCMANAGER_H
#define _DCCMANAGER_H

#include <time.h>

#include <qobject.h>

#include "dccManagerbase.h"
#include "klistview.h"


class dccManager;
class QSignal;
class QObject;

class dccItem : public QObject,
                public KListViewItem
{
Q_OBJECT
public:
    enum dccStatus {
        dccRecving,
	dccOpen,
	dccWaitOnResume,
	dccResumed,
	dccSentOffer,
	dccGotOffer,
        dccSending,
        dccDone,
        dccCancel,
        dccError
    };

    enum dccType {
        dccGet,
        dccChat,
        dccSend
    };

    dccItem( KListView *, dccManager *, enum dccType, const QString &file, const QString &who, enum dccStatus, unsigned int size );
    dccItem( KListViewItem *, dccManager *, enum dccType, const QString &file, const QString &who, enum dccStatus, unsigned int size );
    virtual ~dccItem();

    void changeStatus(enum dccStatus);
    void setReceivedBytes(int bytes);

    void setWhoPostfix(const QString &post);
    void changeFilename(const QString &file);
    void changeWho(const QString &who);

    const QString who() { return m_who; }
    const QString file() { return m_file; }
    enum dccStatus status() { return m_status; }
    enum dccType type() { return m_type; }
    double getPercent() { return m_percent; }

    void doRename();

signals:
    void statusChanged(QListViewItem *);
    void itemRenamed(dccItem *, QString oldNick, QString oldFile);

protected:
    QString enumToStatus(enum dccStatus);

    virtual void okRename ( int col );
    virtual void cancelRename ( int col );

private:

    dccManager *m_manager;
    QString m_who;
    QString m_file;
    QString m_post;
    int m_percent;
    unsigned int m_size;
    enum dccStatus m_status;
    const enum dccType m_type;
    time_t m_stime;
    time_t m_lasttime;
};


class dccNew;

class dccManager : public dccManagerbase
{
Q_OBJECT

public:
    dccManager( QWidget *parent = 0, const char *name = 0 );
    ~dccManager();

    dccItem *newSendItem(QString file, QString who, enum dccItem::dccStatus, unsigned int size);
    dccItem *newGetItem(QString file, QString who, enum dccItem::dccStatus, unsigned int size);
    dccItem *newChatItem(QString who, enum dccItem::dccStatus);

    void doChanged() { emit changed(false, QString("dcc activity")); };

public slots:
    virtual void kpbNew_clicked();
    virtual void kpbConnect_clicked();
    virtual void kpbResume_clicked();
    virtual void kpbRename_clicked();
    virtual void kpbAbort_clicked();

signals:
    void outputLine(QCString);
    void changed(bool, QString);
    virtual void dccConnectClicked(dccItem *);
    virtual void dccResumeClicked(dccItem *);
    virtual void dccRenameClicked(dccItem *);
    virtual void dccAbortClicked(dccItem *);

protected slots:
    virtual void getSelChange(QListViewItem *);
    virtual void sendSelChange(QListViewItem *);
    virtual void dccNewAccepted(int type, QString nick, QString file);

private:

    KListViewItem *m_getit;
    KListViewItem *m_sendit;
    KListViewItem *m_chatit;

    dccNew *dccNewDialog;
};

#endif
