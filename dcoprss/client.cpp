/* $Id: client.cpp 214186 2003-03-16 01:36:37Z geiseri $ */
#include <kapplication.h>
#include <dcopclient.h>
#include <dcopref.h>
#include <qdatastream.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qcstring.h>
#include <kdebug.h>
#include <stdlib.h>
/*
class rssIface : virtual public DCOPObject
{
	K_DCOP
public:

	rssIface( KApplication *app)
	{
		// get our DCOP client and attach so that we may use it
		DCOPClient *client = app->dcopClient();
		client->attach();
		QString error;
		QCString appID;
		kdDebug() << "Looking for rss service..." << endl;
		if (!client->isApplicationRegistered("rssservice"))
		{
			kdDebug() << "Could not find service so I am starting it..." << endl;
			if(KApplication::startServiceByName("rssservice",QStringList(), &error, &appID ))
			{
				kdDebug() << "Starting rssservice failed with message: " << error << endl;
				exit(0);
			}
		}
		kdDebug ()<< "Accessing rssservice..." << endl;

		if (!connectDCOPSignal(0,0, "documentUpdated(DCOPRef)",
			"refresh(DCOPRef)",false))
			kdDebug() << "Could not attach signal..." << endl;
		else
			kdDebug() << "attached dcop signals..." << endl;

		QString url("http://freshmeat.net/backend/fm.rdf");
		DCOPRef m_rssservice("rssservice","RSSService");
		m_rssservice.call("load(QString)", url);
		QStringList returnList = m_rssservice.call("list()");
		DCOPRef doc = m_rssservice.call("document(QString)", returnList[0]);
		QString title = doc.call("title()");
		QString link = doc.call("link()");
		QString description = doc.call("description()");
		kdDebug() << title << endl;
		kdDebug() << link << endl;
		kdDebug() << description << endl;
	}

	k_dcop:
	virtual void refresh(DCOPRef doc)
	{
		QString title = doc.call("title()");
		QString link = doc.call("link()");
		QString description = doc.call("description()");
		kdDebug() << title << endl;
		kdDebug() << link << endl;
		kdDebug() << description << endl;
	}

	private:

};
*/
int main(int argc, char **argv)
{
	KApplication *app = new KApplication(argc, argv, "client", false);

	app->exec();
}
