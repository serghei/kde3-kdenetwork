/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/
#include <kdebug.h>
#include "wificonfig.h"

void ConfigCrypto::load( const IfConfig &ifconfig)
{
  cmb_activeKey->setCurrentItem( ifconfig.m_activeKey-1 );
  bg_cryptoMode->setButton( ifconfig.m_cryptoMode );
  
  le_key1->setText( ifconfig.m_keys[0].key() );
  le_key2->setText( ifconfig.m_keys[1].key() );
  le_key3->setText( ifconfig.m_keys[2].key() );
  le_key4->setText( ifconfig.m_keys[3].key() );
}

void ConfigCrypto::save( IfConfig &ifconfig )
{
  ifconfig.m_activeKey = cmb_activeKey->currentItem()+1;
  ifconfig.m_cryptoMode = (IfConfig::CryptoMode)bg_cryptoMode->selectedId();

  ifconfig.m_keys[0].setKey( le_key1->text() );
  ifconfig.m_keys[1].setKey( le_key2->text() );
  ifconfig.m_keys[2].setKey( le_key3->text() );
  ifconfig.m_keys[3].setKey( le_key4->text() );
}


void ConfigCrypto::slotUpdateKey1Status(const QString& key)
{
    KeyStates status = ::Key::isValid( key );
    switch (status) {
    case INVALID:
	format_1->setText("<font color=\"#ff0000\">unrecognised</font>");
	break;
    case EMPTY:
	format_1->setText("<font color=\"#000000\">slot empty</font>");
	break;
    case HEX_64:
	format_1->setText("<font color=\"#00b000\">WEP 64-Bit hex</font>");
	break;
    case HEX_128:
	format_1->setText("<font color=\"#00b000\">WEP 128-Bit hex</font>");
	break;
    case HEX_256:
	format_1->setText("<font color=\"#00b000\">WEP 256-Bit hex</font>");
	break;
    case STRING_64:
	format_1->setText("<font color=\"#00b000\">WEP 64-Bit string</font>");
	break;
    case STRING_128:
	format_1->setText("<font color=\"#00b000\">WEP 128-Bit string</font>");
	break;
    case STRING_256:
	format_1->setText("<font color=\"#00b000\">WEP 256-Bit string</font>");
	break;
	}
}

void ConfigCrypto::slotUpdateKey2Status(const QString& key)
{
    KeyStates status = ::Key::isValid( key );
    switch (status) {
    case INVALID:
	format_2->setText("<font color=\"#ff0000\">unrecognised</font>");
	break;
    case EMPTY:
	format_2->setText("<font color=\"#000000\">slot empty</font>");
	break;
    case HEX_64:
	format_2->setText("<font color=\"#00b000\">WEP 64-Bit hex</font>");
	break;
    case HEX_128:
	format_2->setText("<font color=\"#00b000\">WEP 128-Bit hex</font>");
	break;
    case HEX_256:
	format_2->setText("<font color=\"#00b000\">WEP 256-Bit hex</font>");
	break;
    case STRING_64:
	format_2->setText("<font color=\"#00b000\">WEP 64-Bit string</font>");
	break;
    case STRING_128:
	format_2->setText("<font color=\"#00b000\">WEP 128-Bit string</font>");
	break;
    case STRING_256:
	format_2->setText("<font color=\"#00b000\">WEP 256-Bit string</font>");
	break;
	}
}

void ConfigCrypto::slotUpdateKey3Status(const QString& key)
{
    KeyStates status = ::Key::isValid( key );
    switch (status) {
    case INVALID:
	format_3->setText("<font color=\"#ff0000\">unrecognised</font>");
	break;
    case EMPTY:
	format_3->setText("<font color=\"#000000\">slot empty</font>");
	break;
    case HEX_64:
	format_3->setText("<font color=\"#00b000\">WEP 64-Bit hex</font>");
	break;
    case HEX_128:
	format_3->setText("<font color=\"#00b000\">WEP 128-Bit hex</font>");
	break;
    case HEX_256:
	format_3->setText("<font color=\"#00b000\">WEP 256-Bit hex</font>");
	break;
    case STRING_64:
	format_3->setText("<font color=\"#00b000\">WEP 64-Bit string</font>");
	break;
    case STRING_128:
	format_3->setText("<font color=\"#00b000\">WEP 128-Bit string</font>");
	break;
    case STRING_256:
	format_3->setText("<font color=\"#00b000\">WEP 256-Bit string</font>");
	break;
	}
}

void ConfigCrypto::slotUpdateKey4Status(const QString& key)
{
    KeyStates status = ::Key::isValid( key );
    switch (status) {
    case INVALID:
	format_4->setText("<font color=\"#ff0000\">unrecognised</font>");
	break;
    case EMPTY:
	format_4->setText("<font color=\"#000000\">slot empty</font>");
	break;
    case HEX_64:
	format_4->setText("<font color=\"#00b000\">WEP 64-Bit hex</font>");
	break;
    case HEX_128:
	format_4->setText("<font color=\"#00b000\">WEP 128-Bit hex</font>");
	break;
    case HEX_256:
	format_4->setText("<font color=\"#00b000\">WEP 256-Bit hex</font>");
	break;
    case STRING_64:
	format_4->setText("<font color=\"#00b000\">WEP 64-Bit string</font>");
	break;
    case STRING_128:
	format_4->setText("<font color=\"#00b000\">WEP 128-Bit string</font>");
	break;
    case STRING_256:
	format_4->setText("<font color=\"#00b000\">WEP 256-Bit string</font>");
	break;
	}
}
