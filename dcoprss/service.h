/* $Id: service.h 371819 2004-12-19 04:28:15Z geiseri $ */
#ifndef _RSS_SERVICE
#define _RSS_SERVICE

/***************************************************************************
                          service.h  -  A DCOP Service to provide RSS data
                             -------------------
    begin                : Saturday 15 February 2003
    copyright            : (C) 2003 by Ian Reinhart Geiser
    email                : geiseri@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <dcopobject.h>
#include <dcopref.h>
#include <qdict.h>
#include <qptrlist.h>
#include <qstringlist.h>
#include <qstring.h>
#include <qdatetime.h>
#include <qpixmap.h>
#include <librss/global.h>
#include <librss/loader.h>
#include <librss/document.h>
#include <librss/article.h>
#include <librss/image.h>
/**
* This is a DCOP Service do not include this header in anything
*
**/
using namespace RSS;

class RSSDocument;
class RSSArticle;

class RSSService : public DCOPObject
{
	K_DCOP

	private:

		QDict<RSSDocument> m_list;

	public:
		RSSService();
		~RSSService();
		void saveLinks();
		void loadLinks();


	k_dcop_signals:
		/**
		* Emmitted when a new document has been added.  You can then
		* use document(QString) to get the dcop ref for the object.
		* Note: this document may or may not be valid at this
		* point so you should connect your dcop signals and then
		* do a documentValid() on the dcop ref to make sure of its
		* state.
		**/

		void added(QString);
		/**
		* Emmitted when the document has been removed.
		* note at this point the DCOPRef for this object is
		* invalid and you will cannot access it any longer.
		* When in doubt call a refresh on it, since if its in the
		* process of loading the document call will be safely ignored
		* and you will be notified of the updates.
		**/
		void removed(QString);
	k_dcop:
		/**
		* Add a new rdf file resource.  This will return a dcop reference to the resource.  If its a new
		* one it will be added otherwise an existing resource reference will be returned.
		* once this reference has been returned you may connect dcop signals and then call
		* refresh on the RSSDocument.  The document will not be updated until refresh is called.
		**/
		DCOPRef add(QString url);
		/**
		* Return a list of current rss documents
		**/
		QStringList list();
		/**
		* Remove an rss document resource.  NOTE: Be aware that others may be using this
		* resource and if you remove it they may break.  Likewise be aware that someone may
		* decide to remove your resource on you so you should always check to see if the resource
		* is valid before you access it.
		**/
		void remove(QString url);
		/**
		* Return the reference to a requested resource.  If this resource is not present a null dcopref is
		* returned.
		**/
		DCOPRef document(QString url);
		/**
		* Exit the RSSService.  This will clean everything up and exit.
		**/
		void exit();
};

class RSSDocument :  public QObject, public DCOPObject
{
	Q_OBJECT
	K_DCOP

	private:
		bool m_isLoading;
		QString m_Url;
		Document *m_Doc;
		QPixmap m_pix;
		QPtrList<RSSArticle> m_list;
		QMap<QString,int> m_state;
		QDateTime m_Timeout;
		int m_maxAge;

	private slots:
		void pixmapLoaded(const QPixmap&);
		void loadingComplete(Loader *, Document, Status);

	public:
		RSSDocument(const QString& url);
		~RSSDocument();

	k_dcop_signals:
		/**
		* The pixmap is currently loading
		**/
		void pixmapUpdating(DCOPRef);
		/**
		*  The pixmap is ready for viewing
		*  you can then use dcopref->call("pixmap()"); to return it.
		*
		**/
		void pixmapUpdated(DCOPRef);
		/**
		* The document is currently updating
		**/
		void documentUpdating(DCOPRef);
		/**
		* The document is ready for viewing
		* you can then use dcopref->call() to access its data
		**/
		void documentUpdated(DCOPRef);
		/**
		* The document failed to update, with and error...
		* 1 - RSS Parse Error
		* 2 - Could not access file
		* 3 - Unknown error.
		**/
		void documentUpdateError(DCOPRef, int);

	k_dcop:
		/**
		* Return the webmaster information from the RSS::Document
		**/
		QString webMaster();
		/**
		* Return the manageing editor from the RSS::Document
		**/
		QString managingEditor();
		/**
		* Returns the rating of the RSS::Document
		**/
		QString rating();
		/**
		* Returns the last build date from the RSS::Document
		**/
		QDateTime lastBuildDate();
		/**
		* Returns the publication date from the RSS::Document
		**/
		QDateTime pubDate();
		/**
		* Returns the copyright information from the RSS::Document
		**/
		QString copyright();
		/**
		* Returns a list of article titles
		**/
		QStringList articles();
		/**
		* Returns the number of articles
		**/
		int count();
		/**
		* Returns a dcop reference to the article from the index
		**/
		DCOPRef article(int idx);
		/**
		* Returns the link from the  RSS::Document
		**/
		QString link();
		/**
		* Returns the description from the RSS::Document
		**/
		QString description();
		/**
		* Returns the title from the RSS::Document
		**/
		QString title();
		/**
		*  Returns the text version from the RSS::Document
		**/
		QString verbVersion();
		/**
		*  Returns the url for the pixmap from the RSS::Document
		**/
		QString pixmapURL();
		/**
		* Returns the actual pixmap from the RSS::Document's RSS::Image
		**/
		QPixmap pixmap();
		/**
		* Returns if the RSSDocument contains a valid RSS::Document yet.
		**/
		bool documentValid();
		/**
		* Returns if the RSSDocument contains a valid RSS::Image
		**/
		bool pixmapValid();
		/**
		* Refresh the current RSS::Document.
		* This must be called before the document is valid.
		**/
		void refresh();

		/**
		* Return the maximum age of the RSS document (Default is 60 minutes)
		**/
		int maxAge();

		/**
		* Set the maximum age of the RSS document.
		**/
		void setMaxAge(int minutes);
		
		/**
		 * Returns the state of the article
		 * 0 - not present (deleted from the rss service)
		 * 1 - new
		 * 2 - unread
		 * 3 - read
		 */
		int state( const QString &title) const;
		
		/**
		 * Set the article state
		 */
		void setState( const QString &title, int s );
		
		/**
		 * Convience method that will set a title to read.
		 */
		void read( const QString &title);
};

class RSSArticle : public DCOPObject
{
	K_DCOP

	private:
		Article *m_Art;

	public:
		RSSArticle(Article *art);
		~RSSArticle();

	k_dcop:
		/**
		* Return the articles title
		**/
		QString title();
		/**
		* Return the articles description
		**/
		QString description();
		/**
		* Return the link to the article
		**/
		QString link();
};
#endif
