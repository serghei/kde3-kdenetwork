#include "serverFileParser.h"
#include "serverDataType.h"
#include <qfile.h>
#include <stdlib.h>

#include <kstandarddirs.h>
#include <kdebug.h>

extern QPtrList<Server> Groups;

//  Opens, reads and parses server information from a server file, 
//  sets the information in the global variable and returns 1 if
//  sucessful, takes a filename as an argument.

int serverFileParser::readDatafile( const char *fileName ) 
{
  Groups.setAutoDelete( TRUE );
  Groups.clear();
  QFile serverFile( fileName );
  if ( !serverFile.open( IO_ReadOnly ) )
    return -1;

  QTextStream fileStream(&serverFile);

  // the file is layed out as follows:
  //  service:servername:serveraddress:ports:script:
  // so we parse it this way

  while( !fileStream.eof() ) {
    QString str = fileStream.readLine();
    const char *strC = str.ascii();
    char *token;
    char groupC[1024], servernameC[1024], serveraddressC[1024], portsC[1024];
    int pos = 0;
    QString group;
    QString servername;
    QString serveraddress;
    QPtrList<port> ports;
    QString script;

    QString buf;
    QString portbuff;

    pos = sscanf(strC, "%1023[^:]:%1024[^:]:%1023[^:]:%1023[^:]:", groupC, servernameC, serveraddressC, portsC);
    if(pos != 4){
      kdWarning() << "Failed to parse servers.txt on line: " << strC << ". Invalid format" << endl;
      return 0;
    }
    group = groupC;
    servername = servernameC;
    serveraddress = serveraddressC;
    token = strtok(portsC, ",");
    while(token != NULL){
      ports.inSort(new port(token));
      token = strtok(NULL, ",");
    }
    
    /*
     for( uint loc = 0; loc <= str.length(); loc++ ) {
      if ( str[loc] == ':' || loc == str.length()) {
        switch(pos) {
          case 0: // service
            group = buf.copy();
            break;
          case 1: // server name
            servername = buf.copy();
            break;
          case 2: // server address
            serveraddress = buf.copy();
            break;
          case 3: // port listing
            for ( uint portloc = 0; portloc <= buf.length(); portloc++ ) {
              if (buf[portloc] == ',' || portloc == buf.length()) {
                if (!portbuff.isEmpty())
                  ports.inSort( new port(portbuff));
                portbuff.truncate( 0 );
              } else {
                portbuff += buf[portloc];
              }
            }
            break;
          default: // script
            script = buf.copy();
        }
        pos++;
        buf.truncate( 0 );
        portbuff.truncate( 0 );
      } else {
        buf += str[loc];
      }
      } // for loop
      */
    

    Groups.inSort( new Server(group, serveraddress, ports, 
                              servername, script) );
 
  } // while loop

  serverFile.close();
  return 1;

}

// Writes the data stored in the global variable to the server file,
// returns 1 if sucessful, takes a filename as an argument, uhh NOT.
// we want to write it ~/.kde/share/apps/ksirc/servers.txt
// 

int writeDataFile() 
{
  QString filename(KGlobal::dirs()->saveLocation("appdata")+"servers.txt");
  return 1;
}
