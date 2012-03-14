#ifndef KSIRCSERVER_H
#define KSIRCSERVER_H

#include <qptrlist.h>
#include <qstring.h>

class KSircServer
{

public:
    KSircServer( const QString &server,
		 const QString &port,
		 const QString &script = QString::null,
		 const QString &password  = QString::null,
		 bool ssl = false ) :
        m_server(server), m_port(port), m_script(script),
	m_password(password), m_ssl(ssl) {}

  const QString server() const     { return m_server; }
  const QString port() const      { return m_port; }
  const QString script() const     { return m_script; }
  const QString password() const   { return m_password; }
  bool    usessl() const     { return m_ssl; }

private:
  const QString     m_server;
  const QString     m_port;
  const QString     m_script;
  const QString     m_password;
  const bool        m_ssl;

};

#endif
