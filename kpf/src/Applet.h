/*
  KPF - Public fileserver for KDE

  Copyright 2001 Rik Hemsley (rikkus) <rik@kde.org>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to
  deal in the Software without restriction, including without limitation the
  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
  sell copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef KPF_APPLET_H
#define KPF_APPLET_H

#include <qptrlist.h>
#include <kpanelapplet.h>

class QPopupMenu;
class QPainter;
class DCOPClient;

namespace KPF
{
  class AppletItem;
  class ServerWizard;
  class WebServer;

  /**
   * Main `application' class, providing an implementation of KPanelApplet
   * and managing AppletItem objects. Also provides a popup (context) menu of
   * its own, to allow the user to add WebServer (and therefore AppletItem)
   * objects.
   */
  class Applet : public KPanelApplet
  {
    Q_OBJECT

    public:

      Applet
        (
         const QString  & configFile,
         Type         = Normal,
         int          = 0,
         QWidget    * = 0,
         const char * = 0
        );

      ~Applet();

      /**
       * Overridden to give correct sizing according to orientation and number
       * of contains AppletItem objects.
       */
      virtual int widthForHeight(int h) const;

      /**
       * Overridden to give correct sizing according to orientation and number
       * of contains AppletItem objects.
       */
      virtual int heightForWidth(int w) const;

    protected slots:

      /**
       * Called to create a new server when the path to the server is already
       * known.
       */
      void slotNewServerAtLocation(const QString &);

      /**
       * Called to create a new server when the path to the server is unknown.
       */
      void slotNewServer();

      /**
       * Called when a ServerWizard is about to close.
       */
      void slotWizardDying(ServerWizard *);

      /**
       * Called when a WebServer object has been created. Creates an
       * AppletItem, associates it with the former, and updates the layout.
       */
      void slotServerCreated(WebServer *);

      /**
       * Called when a WebServer object has been disabled. 
       * Deletes the associated AppletItem and updates the layout.
       */
      void slotServerDisabled(WebServer *);

      /**
       * Called when user asks for quit (via popup menu).
       */
      void slotQuit();

    protected:

      /**
       * Overridden to display help window
       */
      virtual void help();

      /**
       * Overridden to provide an `about' dialog.
       */
      virtual void about();

      /**
       * Overridden to keep track of orientation change and update layout
       * accordingly.
       */
      virtual void orientationChange(Orientation);

      /**
       * Overridden to update layout accordingly.
       */
      virtual void moveEvent(QMoveEvent *);
      virtual void resizeEvent(QResizeEvent *);

      /**
       * Overridden to provide a context menu.
       */
      virtual void mousePressEvent(QMouseEvent *);

      /**
       * Updates the layout, moving AppletItem objects into proper positions.
       */
      virtual void resetLayout();

      /**
       * Overridden to provide something other than a blank display when there
       * are no existing AppletItem objects contained.
       */
      virtual void drawContents(QPainter *);

      /**
       * Overridden to allow testing whether the dragged object points to a
       * local directory.
       */
      virtual void dragEnterEvent(QDragEnterEvent *);

      /**
       * Overridden to allow creating a new WebServer when the dropped object
       * points to a local directory.
       */
      virtual void dropEvent(QDropEvent *);

    private:

      enum
      {
        NewServer,
        Quit
      };

      ServerWizard      * wizard_;
      QPopupMenu        * popup_;
      DCOPClient        * dcopClient_;

      QPtrList<AppletItem>   itemList_;
  };
}

#endif // KPF_APPLET_H

// vim:ts=2:sw=2:tw=78:et
