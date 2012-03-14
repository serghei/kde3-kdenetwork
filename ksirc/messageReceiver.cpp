#include "messageReceiver.h"

#include <qtextcodec.h>
#include <kcharsets.h>
#include <kglobal.h>
#include <kdebug.h>
#include <assert.h>

KSircMessageReceiver::KSircMessageReceiver( KSircProcess * _proc )
{
    proc = _proc;
    broadcast = TRUE;
}

KSircMessageReceiver::~KSircMessageReceiver()
{
}

void KSircMessageReceiver::setBroadcast(bool bd)
{
  broadcast = bd;
}

bool KSircMessageReceiver::getBroadcast()
{
  return broadcast;
}

filterRuleList *KSircMessageReceiver::defaultRules()
{
  return new filterRuleList();
}

UnicodeMessageReceiver::UnicodeMessageReceiver( KSircProcess *process )
    : KSircMessageReceiver( process ), m_encoder( 0 )
{
}

void UnicodeMessageReceiver::sirc_receive( QCString str, bool broadcast )
{
    assert( encoder() );
    //kdDebug(5008) << "Encoder: " << encoder()->name() << endl;
    //kdDebug(5008) << "It gives us: " << encoder()->toUnicode( str ).utf8() << endl;
    sirc_receive( encoder()->toUnicode( str ), broadcast );
}

void UnicodeMessageReceiver::setEncoding( const QString &encoding )
{
    m_encoding = encoding;
    m_encoder = 0;
}

QString UnicodeMessageReceiver::encoding() const
{
    if ( m_encoding.isEmpty() )
	return QTextCodec::codecForLocale()->mimeName();
    return m_encoding;
}

QTextCodec *UnicodeMessageReceiver::encoder() const
{
    if ( m_encoding.isEmpty() ) {
	return QTextCodec::codecForLocale();
    }

    if ( !m_encoder ) {
	const_cast<UnicodeMessageReceiver *>( this )->m_encoder = KGlobal::charsets()->codecForName( m_encoding );
	assert( m_encoder );
    }

    return m_encoder;
}

