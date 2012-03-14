/* -------------------------------------------------------------

   actions.cpp (part of The KDE Dictionary Client)

   Copyright (C) 2000-2001 Christian Gebauer <gebauer@kde.org>

   This file is distributed under the Artistic License.
   See LICENSE for details.

   -------------------------------------------------------------

   DictComboAction,            special KAction subclasses used
   DictLabelAction,            in the toolbar
   DictButtonAction

 ------------------------------------------------------------- */

#include "actions.h"

#include <qlabel.h>
#include <qpushbutton.h>

#include <kcombobox.h>
#include <ktoolbar.h>


DictComboAction::DictComboAction( const QString &text, QObject *parent, const char *name,
                                  bool editable, bool autoSized )
  : KAction( text, 0, parent, name ), m_editable(editable), m_autoSized(autoSized), m_compMode(KGlobalSettings::completionMode())
{
}


DictComboAction::~DictComboAction()
{
}


int DictComboAction::plug( QWidget *widget, int index )
{
  if ( widget->inherits( "KToolBar" ) )
  {
    KToolBar* bar = static_cast<KToolBar*>( widget );
    int id_ = KAction::getToolButtonID();

    m_combo = new KComboBox(m_editable,bar);
    m_combo->setCompletionMode(m_compMode);

    bar->insertWidget( id_, m_combo->sizeHint().width(), m_combo, index );
    bar->setItemAutoSized(id_,m_autoSized);

    if ( m_combo ) {
      connect(bar->getCombo(id_), SIGNAL(activated(const QString&)), SLOT(slotComboActivated(const QString&)));
      connect(bar->getCombo(id_), SIGNAL(activated(int)), SLOT(slotComboActivated(int)));

      if (m_editable)
        m_combo->setInsertionPolicy( QComboBox::NoInsertion );
    }

    addContainer( bar, id_ );
    connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );
    return containerCount() - 1;
  }

  return -1;
}


void DictComboAction::unplug( QWidget *widget )
{
  if ( widget->inherits( "KToolBar" ) )
  {
    KToolBar *bar = (KToolBar *)widget;

    int idx = findContainer( bar );

    if ( idx != -1 )
    {
      bar->removeItem( itemId( idx ) );
      removeContainer( idx );
    }

    return;
  }
}


QWidget* DictComboAction::widget()
{
  return m_combo;
}


void DictComboAction::setFocus()
{
  if (m_combo)
   m_combo->setFocus();
}


QString DictComboAction::currentText() const
{
  if (m_combo)
    return m_combo->currentText();
  else
    return QString::null;
}

void DictComboAction::selectAll()
{
  if (m_combo)
  {
    m_combo->lineEdit()->selectAll();
    m_combo->lineEdit()->setFocus();
  }
}


void DictComboAction::setEditText(const QString &s)
{
  if (m_combo && m_editable)
    m_combo->setEditText(s);
}


void DictComboAction::setCurrentItem(int index)
{
  if (m_combo)
    m_combo->setCurrentItem(index);
}


void DictComboAction::clearEdit()
{
  if (m_combo && m_editable)
    m_combo->clearEdit();
}


void DictComboAction::clear()
{
  if (m_combo) {
    m_combo->clear();
    if (m_editable && m_combo->completionObject())
      m_combo->completionObject()->clear();
  }
}


void DictComboAction::setList(QStringList items)
{
  if (m_combo) {
    m_combo->clear();
    m_combo->insertStringList(items);
    if (m_editable && m_combo->completionObject())
      m_combo->completionObject()->setItems(items);
    if (!m_autoSized)
      m_combo->setFixedWidth(m_combo->sizeHint().width());
  }
}


KGlobalSettings::Completion DictComboAction::completionMode()
{
  if (m_combo)
    return m_combo->completionMode();
  else
    return m_compMode;
 }


void DictComboAction::setCompletionMode(KGlobalSettings::Completion mode)
{
  if (m_combo)
    m_combo->setCompletionMode(mode);
  else
    m_compMode = mode;
}


void DictComboAction::slotComboActivated(int i)
{
  emit(activated(i));
}


void DictComboAction::slotComboActivated(const QString &s)
{
  emit(activated(s));
}


//*********************************************************************************


DictLabelAction::DictLabelAction( const QString &text, QObject *parent, const char *name )
  : KAction( text, 0, parent, name )
{
}


DictLabelAction::~DictLabelAction()
{
}


int DictLabelAction::plug( QWidget *widget, int index )
{
  if ( widget->inherits( "KToolBar" ) )
  {
    KToolBar *tb = (KToolBar *)widget;

    int id = KAction::getToolButtonID();

    QLabel *label = new QLabel( text(), widget,  "kde toolbar widget" );
    label->setMinimumWidth(label->sizeHint().width());
    label->setBackgroundMode( Qt::PaletteButton );
    label->setAlignment(AlignCenter | AlignVCenter);
    label->adjustSize();

    tb->insertWidget( id, label->width(), label, index );

    addContainer( tb, id );

    connect( tb, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    m_label = label;

    return containerCount() - 1;
  }

  return -1;
}


void DictLabelAction::unplug( QWidget *widget )
{
  if ( widget->inherits( "KToolBar" ) )
  {
    KToolBar *bar = (KToolBar *)widget;

    int idx = findContainer( bar );

    if ( idx != -1 )
    {
      bar->removeItem( itemId( idx ) );
      removeContainer( idx );
    }

    return;
  }
}


void DictLabelAction::setBuddy(QWidget *buddy)
{
  if (m_label && buddy)
    m_label->setBuddy(buddy);
}


//*********************************************************************************


DictButtonAction::DictButtonAction( const QString& text, QObject* receiver,
                                    const char* slot, QObject* parent, const char* name )
  : KAction( text, 0, receiver, slot, parent, name )
{
}


DictButtonAction::~DictButtonAction()
{
}


int DictButtonAction::plug( QWidget *widget, int index )
{
  if ( widget->inherits( "KToolBar" ) )
  {
    KToolBar *tb = (KToolBar *)widget;

    int id = KAction::getToolButtonID();

    QPushButton *button = new QPushButton( text(), widget );
    button->adjustSize();
    connect(button,SIGNAL(clicked()),this,SLOT(activate()));
    tb->insertWidget( id, button->width(), button, index );

    addContainer( tb, id );

    connect( tb, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    m_button = button;

    return containerCount() - 1;
  }

  return -1;
}


void DictButtonAction::unplug( QWidget *widget )
{
  if ( widget->inherits( "KToolBar" ) )
  {
    KToolBar *bar = (KToolBar *)widget;

    int idx = findContainer( bar );

    if ( idx != -1 )
    {
      bar->removeItem( itemId( idx ) );
      removeContainer( idx );
    }
  }
}


int DictButtonAction::widthHint()
{
  if (m_button)
    return m_button->sizeHint().width();
  else
    return 0;
}


void DictButtonAction::setWidth(int width)
{
  if (m_button)
    m_button->setFixedWidth(width);
}

#include "actions.moc"
