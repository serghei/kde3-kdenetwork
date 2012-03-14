/**********************************************************************

	--- Qt Architect generated file ---

	File: open_ksirc.cpp
	Last generated: Wed Jul 29 16:41:26 1998

 *********************************************************************/

#include "open_ksirc.h"
#include "serverDataType.h"
#include "serverFileParser.h"
#include "enter_combo.h"
#include "../ksircserver.h"
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qdict.h>
#include <stdlib.h>
#include <unistd.h>

#include <kdebug.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kmdcodec.h>

QPtrList<Server> Groups;

#undef Inherited
#define Inherited open_ksircData

open_ksirc::open_ksirc
(
	QWidget* parent,
	const char* name
)
	:
	Inherited( parent, name, true )
{
  setCaption( i18n("Connect to Server") );

  // check for existance of ~/.kde/share/apps/ksirc/servers.txt
  // if it don't exist use $KDEDIR/share/apps/ksirc/servers.txt
  // changes are written to ~/.kde/share/apps/ksirc/servers.txt

  QString filename = locate("appdata", "servers.txt");
  serverFileParser::readDatafile( filename );

  Groups.setAutoDelete(TRUE);

  // TODO add "Recent" to global listing servers here..
  // Now we read in the Recent group from the config file
  // remove all recent servers first

  for(Server *s = Groups.first(); s != 0x0; s = Groups.next()){
    if(s->group() == i18n("Recent")){
      Groups.remove();
    }
  }

  // Add current ones
  KConfig *conf = kapp->config();
  conf->setGroup("ServerList");
  CheckB_StorePassword->setChecked( conf->readBoolEntry("StorePasswords") );
  QStringList recent = conf->readListEntry("RecentServers");
  for(QStringList::ConstIterator it = recent.begin(); it != recent.end(); ++it){
      if(conf->hasGroup("RecentServer-" + *it)){
          conf->setGroup("RecentServer-" + *it);
	  QPtrList<port> rp;
          rp.inSort(new port(conf->readEntry("Port", "6667")));
          QString password = decryptPassword(conf->readEntry("Password"));
          bool ssl = conf->readBoolEntry("SSL");
          Groups.insert(0, new Server(i18n("Recent"), *it, rp,
                                      i18n("Recent Server"), "", password,
                                      ssl));
      }
      else {
          QStringList info = QStringList::split(":", *it);
          if (info.isEmpty())
              continue;
          QString name = info[0];
          QPtrList<port> rp;
          if (info.count() > 1)
              rp.inSort(new port(info[1]));
          else
              rp.inSort(new port("6667"));
          QString password;
          if (info.count() > 2)
              password = decryptPassword(info[2]);

          conf->setGroup("RecentServer-" + name);
          conf->writeEntry("Port", rp.first()->portnum());
          conf->writeEntry("Password", encryptPassword(password));
          conf->writeEntry("SSL", false);

          Groups.insert(0, new Server(i18n("Recent"), name, rp,
                                      i18n("Recent Server"), "", password));
      }
  }

  ComboB_ServerName->setAutoCompletion( TRUE );
  ComboB_ServerPort->setAutoCompletion( TRUE );

  insertGroupList();

  QString blah = i18n("Recent");
  setGroup(blah);

  connect(ComboB_ServerGroup, SIGNAL(activated( const QString& )),
          this, SLOT(setGroup( const QString& )));
  connect(ComboB_ServerName,  SIGNAL(activated( const QString& )),
          this, SLOT(setServer( const QString& )));

  connect(PB_Connect, SIGNAL(clicked()), this, SLOT(clickConnect()));
  connect(PB_Edit, SIGNAL(clicked()), this, SLOT(clickEdit()));
  connect(PB_Cancel, SIGNAL(clicked()), this, SLOT(clickCancel()));

  PB_Connect->setDefault(TRUE);
  PB_Connect->setAutoDefault(TRUE);
  PB_Edit->setEnabled(false); // Not yet implemented.

  ComboB_ServerName->setFocus();
  connect(ComboB_ServerName, SIGNAL(enterPressed()), this, SLOT(clickConnect()));
}

// insert a sorted list of groups into ComboB_ServerGroup, note that
// we want to get the recent servers in first so we insert "Recent"
// in first, then we want Random.

void open_ksirc::insertGroupList()
{
  QStrList tempgroups;
  Server *serv;

  for ( serv=Groups.first(); serv != 0; serv=Groups.next() ) {
      if (tempgroups.find(serv->group()) == -1)
          tempgroups.inSort( serv->group() );
  }

  ComboB_ServerGroup->insertItem(i18n( "Recent") );
  ComboB_ServerGroup->insertItem(i18n( "Random") );
  for (const char* t = tempgroups.first(); t; t = tempgroups.next()) {
      ComboB_ServerGroup->insertItem( t );
  }
}

// insert a sorted list of servers from the group passed as an arg
// into ComboB_ServerName, if a list is already there delete it.
// note this does not check for multiple occurrances of the same server

void open_ksirc::insertServerList( const char * group )
{
  QListBox *newListBox = new QListBox();
  Server *serv;

  for ( serv=Groups.first(); serv != 0; serv=Groups.next() ) {
      if ( !qstrcmp(serv->group(), group) ) {
          newListBox->insertItem( serv->server(), 0 );
      }
  }

  ComboB_ServerName->setListBox(newListBox);
//  ComboB_ServerName->setCurrentItem( 0 ); // this don't work =(
  if (newListBox->count() > 0)
      ComboB_ServerName->setEditText( newListBox->text( 0 ) );
}

// insert a sorted list of ports from the server passed as an arg
// into ComboB_ServerPort, if a list is already there delete it.
// note that this only takes the first occurrance if there is two
// entiies with the same server.

void open_ksirc::setServer( const QString &serveraddress )
{
  QListBox *newListBox  = new QListBox();
  Server *serv;
  QPtrList<port> portlist;
  port *p;
  bool defaultport = FALSE;

  for ( serv=Groups.first(); serv != 0; serv=Groups.next() ) {
    if (serv->server() == serveraddress) {
      setServerDesc( serv->serverdesc() );
      portlist = serv->ports();
      for ( p=portlist.last(); p != 0; p=portlist.prev() ) {
        newListBox->insertItem( p->portnum() );
        if (strcmp(p->portnum(), "6667") == 0)
          defaultport = TRUE;
      }
      LineE_Password->setText( serv->password() );
      CheckB_StorePassword->setEnabled( !serv->password().isEmpty() );
      CheckB_UseSSL->setChecked(serv->usessl());
      break;
    }
  }
  ComboB_ServerPort->setListBox(newListBox);
//  ComboB_ServerPort->setCurrentItem( 0 ); // doesn't work
  if (defaultport) {
    ComboB_ServerPort->setEditText("6667");
  } else {
    if (newListBox->count() > 0)
      ComboB_ServerPort->setEditText( newListBox->text( 0 ) );
  }
}

// Sets the server description if the isn't one set it to "Not Available"

void open_ksirc::setServerDesc( QString description )
{
  if (description.isNull() || description.isEmpty()) {
    Label_ServerDesc->setText( i18n("Not available"));
  } else {
    Label_ServerDesc->setText( description );
  }
}

// This has got nothing to do with real encryption, it just scrambles
// the password a little bit for saving it into the config file.
// A random string of the same length as the password in UTF-8 is generated
// and then each byte of the UTF-8 password is xored with the corresponding
// byte of the random string. The returned value is a base64 encoding of
// that random string followed by the scrambled password.
QString open_ksirc::encryptPassword( const QString &password )
{
  QCString utf8 = password.utf8();
  // Can contain NULL bytes after XORing
  unsigned int utf8Length(utf8.length());
  QByteArray result(utf8Length << 1);
  memcpy(result.data(), kapp->randomString(utf8Length).latin1(), utf8Length);
  for (unsigned int i = 0; i < utf8Length; ++i)
    result[i + utf8Length] = utf8[i] ^ result[i];
  return QString::fromLatin1(KCodecs::base64Encode(result));
}

QString open_ksirc::decryptPassword( const QString &scrambled )
{
  QByteArray base64, orig;
  base64.duplicate(scrambled.latin1(), scrambled.length());
  KCodecs::base64Decode(base64, orig);
  QCString result;
  for (unsigned int i = 0; i < (orig.size() >> 1); ++i)
    result += orig[i] ^ orig[i + (orig.size() >> 1)];
  return QString::fromUtf8(result);
}

void open_ksirc::setGroup( const QString &group )
{
  insertServerList( group );
  if (ComboB_ServerName->count() > 0) {
    QString blah = QString(ComboB_ServerName->text( 0 ));
    setServer(blah);
  } else {
    setServerDesc( "" );
    ComboB_ServerPort->setEditText("6667");
    ComboB_ServerPort->insertItem("6667");
  }
  if(ComboB_ServerPort->currentText() == 0x0){
      ComboB_ServerPort->setEditText("6667");
      ComboB_ServerPort->insertItem("6667");
  }
}

void open_ksirc::clickConnect()
{
  if ( ComboB_ServerName->currentText().isEmpty() )
  {
    KMessageBox::information( this, i18n( "Please enter a server name." ) );
    return;
  }

  QString server;
  QString port;
  QString script;
  Server *serv;
  KConfig *conf = kapp->config();

  hide();

  server = ComboB_ServerName->currentText();
  port = ComboB_ServerPort->currentText();

  for ( serv=Groups.first(); serv != 0; serv=Groups.next() ) {
    if (strcmp(serv->server(), server) == 0) {
      script = serv->script();
    }
    break;
  }

  if(server.length() == 0)
    reject();

  if(port.isEmpty())
    port = "6667";

  QString plain, scrambled;
  if (!LineE_Password->text().isEmpty())
  {
      plain = LineE_Password->text();
      if (CheckB_StorePassword->isChecked())
	  scrambled = encryptPassword(LineE_Password->text());
  }

  conf->setGroup("ServerList");
  conf->writeEntry("StorePasswords", CheckB_StorePassword->isChecked());
  QStringList recent = conf->readListEntry("RecentServers");
  if(recent.contains(server)){
      QStringList::Iterator it = recent.find(server);
      recent.remove(it);
  }

  /*
   * This is legacy code only
   */
  //str is now "server:port"
  //plain is now "server:port" or "server:port:pass" if a password was entered
  //scrambled is now "server:port" or "server:port:scrambledpass" if
  //a password was given and "store password" is checked

  for (QStringList::Iterator it = recent.begin(); it != recent.end(); ) {
      if ((*it).startsWith(server)) // ignore password
      {
	  QStringList::Iterator del = it++;
	  recent.remove(del);
      }
      else
	  ++it;
  }

  recent.prepend(server);
  conf->writeEntry("RecentServers", recent);

  conf->setGroup("RecentServer-" + server);
  conf->writeEntry("Port", port);
  conf->writeEntry("Password", scrambled);
  conf->writeEntry("SSL", CheckB_UseSSL->isChecked());

  conf->sync();

  KSircServer kss(server,
		  port,
		  script,
		  plain,
		  CheckB_UseSSL->isChecked());

  //  emit open_ksircprocess( server, port, script );
  emit open_ksircprocess(kss);

  accept();
}

void open_ksirc::clickCancel()
{
  reject();
}

void open_ksirc::clickEdit()
{
  // TODO open new server editor
}

void open_ksirc::passwordChanged( const QString& password )
{
  CheckB_StorePassword->setEnabled( !password.isEmpty() );
}

open_ksirc::~open_ksirc()
{
  Groups.clear();
}

#include "open_ksirc.moc"

// vim: ts=2 sw=2 et
