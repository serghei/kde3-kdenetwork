/* This file is part of the KDE project
   Copyright (C) 2001 Simon Hausmann <hausmann@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the Artistic License.
*/
#ifndef __topic_h__
#define __topic_h__

#include <kactivelabel.h>
#include <qtextedit.h>
#include <qlineedit.h>
#include <qpopupmenu.h>
#include <qguardedptr.h>

class KSircTopicEditor;

class KSircTopic : public KActiveLabel
{
    Q_OBJECT
public:
    KSircTopic( QWidget *parent, const char *name = 0 );

public slots:
    virtual void setText( const QString &);

signals:
    void topicChange( const QString &newTopic );

protected:
    virtual void contentsMouseDoubleClickEvent( QMouseEvent * );
    virtual void contentsMouseReleaseEvent( QMouseEvent * );
    virtual void fontChange(QFont &);

private slots:
    void setNewTopic();
    void slotEditResized();
    void doResize();

private:


    QGuardedPtr<KSircTopicEditor> m_editor;
    bool m_doEdit;
    int m_height;
    QString m_text;
};

class KSircTopicEditor : public QTextEdit
{
    Q_OBJECT
public:
    KSircTopicEditor( QWidget *parent, const char *name = 0 );

signals:
    void resized();

public slots:
    virtual void slotMaybeResize();

protected:
    virtual void keyPressEvent( QKeyEvent *ev );
    virtual void focusOutEvent( QFocusEvent * );
    virtual QPopupMenu *createPopupMenu( const QPoint &pos );

private:
    QGuardedPtr<QPopupMenu> m_popup;
};

#endif
