#ifndef KSIRCCHANNEL_H
#define KSIRCCHANNEL_H

#include <qstring.h>

class KSircChannel
{

public:
  KSircChannel( const QString server,
		const QString channel,
		const QString key = QString::null,
		  const QString encoding = QString::null
		) :
  m_server(server), m_channel(channel),
  m_key(key), m_encoding(encoding) {}

  const QString server() const   { return m_server; }
  const QString channel() const  { return m_channel; }
  const QString key() const      { return m_key; }
  const QString encoding() const { return m_encoding; }

  void setChannel(QString channel) { m_channel = channel; }
  void setKey(QString key) { m_key = key; }
  void setEncoding(QString encoding) { m_encoding = encoding; }

private:
  const QString m_server; /* channels can never change server */
  QString     m_channel;
  QString     m_key;
  QString     m_encoding;

};

#endif
