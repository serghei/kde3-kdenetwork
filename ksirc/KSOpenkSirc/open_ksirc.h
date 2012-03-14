/**********************************************************************

	--- Qt Architect generated file ---

	File: open_ksirc.h
	Last generated: Wed Jul 29 16:41:26 1998

 *********************************************************************/

#ifndef open_ksirc_included
#define open_ksirc_included

#include "open_ksircData.h"

#include <qdict.h>

class KSircServer;

class open_ksirc : public open_ksircData
{
    Q_OBJECT

public:

    open_ksirc
    (
        QWidget* parent = NULL,
        const char* name = NULL
    );

    virtual ~open_ksirc();

protected slots:
  void setGroup( const QString& );
  void setServer( const QString& );
  void clickConnect();
  void clickCancel();
  void clickEdit();
  void passwordChanged( const QString& );

private:
  void insertGroupList();
  void insertServerList( const char * );
  void setServerDesc( QString );
  QString encryptPassword( const QString & );
  QString decryptPassword( const QString & );

signals:
//  void open_ksircprocess( const char *, int, const char * );
  void open_ksircprocess( KSircServer & );

};
#endif // open_ksirc_included
