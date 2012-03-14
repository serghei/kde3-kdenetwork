/* ascii table for ksirc - Robbie Ward <linuxphreak@gmx.co.uk>*/

#include "charSelector.h"
#include "charSelector.moc"

#include <qlayout.h>
#include <qpushbutton.h>

#include <klocale.h>

charSelector::charSelector(QWidget *parent, const char* name)
           : KDialog(parent, name, false)
{
  testLayout = new QVBoxLayout(this);
  testLayout->setSpacing( spacingHint() );
  testLayout->setMargin( marginHint() );

  charSelect = new KCharSelect(this, QCString(name) + "_kcharselector", "", 0);
  testLayout->addWidget(charSelect);
  charSelect->installEventFilter(this);

  connect(charSelect, SIGNAL(doubleClicked()), SLOT(insertText()));

  QHBoxLayout *buttonLayout = new QHBoxLayout;
  buttonLayout->setSpacing( spacingHint() );

  insertButton = new QPushButton(i18n("&Insert Char"), this);
  connect(insertButton, SIGNAL(clicked()), SLOT(insertText()));
  buttonLayout->addWidget(insertButton);

  QSpacerItem *spacer = new QSpacerItem(50, 20, QSizePolicy::Expanding, QSizePolicy::Expanding);
  buttonLayout->addItem(spacer);

  testLayout->addLayout(buttonLayout);
}

charSelector::~charSelector()
{
    delete charSelect;
    charSelect = 0x0;
}

void charSelector::insertText()
{
  emit clicked();
}

void charSelector::setFont( const QString &font )
{
    charSelect->setFont(font);
}

void charSelector::reject()
{
    KDialog::reject();
    close();
}

void charSelector::keyPressEvent(QKeyEvent *e)
{
    KDialog::keyPressEvent(e);
}

bool charSelector::eventFilter ( QObject *, QEvent * e )
{
    if ( e->type() == QEvent::AccelOverride ) {
	// special processing for key press
	QKeyEvent *k = (QKeyEvent *)e;
	if(k->key() == Key_Escape){
            keyPressEvent(k);
	    return TRUE; // eat event
	}
    }
    // standard event processing
    return FALSE;
}

