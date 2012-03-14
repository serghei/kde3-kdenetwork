/* -------------------------------------------------------------

   actions.h (part of The KDE Dictionary Client)

   Copyright (C) 2000-2001 Christian Gebauer <gebauer@kde.org>

   This file is distributed under the Artistic License.
   See LICENSE for details.

   -------------------------------------------------------------

   DictComboAction,            special KAction subclasses used
   DictLabelAction,            in the toolbar
   DictButtonAction

 ------------------------------------------------------------- */

#ifndef _ACTIONS_H_
#define _ACTIONS_H_

#include <qguardedptr.h>
#include <qptrlist.h>
#include <kaction.h>
#include <kglobalsettings.h>

class KComboBox;
class QLabel;
class QPushButton;


class DictComboAction : public KAction
{
  Q_OBJECT

  public:
    DictComboAction( const QString& text, QObject* parent,
                     const char* name, bool editable, bool autoSized );
    ~DictComboAction();

    virtual int plug( QWidget *w, int index = -1 );
    virtual void unplug( QWidget *w );

    QWidget* widget();
    void setFocus();

    QString currentText() const;
    void selectAll();
    void setEditText(const QString &s);
    void setCurrentItem(int index);
    void clearEdit();

    void clear();
    void setList(QStringList items);

    KGlobalSettings::Completion completionMode();
    void setCompletionMode(KGlobalSettings::Completion mode);

  signals:
    void activated(int);
    void activated(const QString&);

  private slots:
    void slotComboActivated(int);
    void slotComboActivated(const QString&);

  private:
    QGuardedPtr<KComboBox> m_combo;
    bool m_editable, m_autoSized;
    KGlobalSettings::Completion m_compMode;
};


class DictLabelAction : public KAction
{
  Q_OBJECT

  public:
    DictLabelAction( const QString &text, QObject *parent = 0, const char *name = 0 );
    ~DictLabelAction();

    virtual int plug( QWidget *widget, int index = -1 );
    virtual void unplug( QWidget *widget );

    void setBuddy(QWidget *buddy);

  private:
    QGuardedPtr<QLabel> m_label;

};


class DictButtonAction : public KAction
{
  Q_OBJECT

  public:
    DictButtonAction( const QString& text, QObject* receiver,
                     const char* slot, QObject* parent, const char* name );
    ~DictButtonAction();

    virtual int plug( QWidget *w, int index = -1 );
    virtual void unplug( QWidget *w );

    int widthHint();
    void setWidth(int width);

  private:
    QGuardedPtr<QPushButton> m_button;
};

#endif
