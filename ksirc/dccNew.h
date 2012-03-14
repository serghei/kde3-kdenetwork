/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation version 2 of the License.                *
 *                                                                         *
 ***************************************************************************/

#ifndef DCC_NEW_H
#define DCC_NEW_H

#include "dccNewbase.h"

class dccNew : public dccNewBase
{
Q_OBJECT

public:
    dccNew( QWidget *parent = 0,
	    const char *name = 0,
	    int type = -1,
	    QString nick = QString::null );
    ~dccNew();

    enum { Chat, Send };

    QString getFile();
    QString getNick();
    int getType(); 


signals:
    void modified();
    void accepted(int type, QString nick, QString file);

protected slots:
    virtual void chatClicked();
    virtual void fileSendClicked();
    virtual void sendClicked();
    virtual void fileClicked();
    virtual void reject();
};

#endif
