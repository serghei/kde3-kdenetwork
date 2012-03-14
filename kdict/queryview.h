/* -------------------------------------------------------------

   queryview.h (part of The KDE Dictionary Client)

   Copyright (C) 2000-2001 Christian Gebauer <gebauer@kde.org>

   This file is distributed under the Artistic License.
   See LICENSE for details.

    -------------------------------------------------------------

    SaveHelper    network transparent file saving
    BrowseData    data structure used for caching definitions
    DictHTMLPart  handling of middle mouse button clicks
    QueryView     widget that displays the definitions

 ------------------------------------------------------------- */

#ifndef _QUERYVIEW_H_
#define _QUERYVIEW_H_

#include <qvbox.h>
#include <khtml_part.h>

class QFile;
class KTempFile;
class KPopupMenu;
class DictComboAction;


QString getShortString(QString str,unsigned int length);


//********  SaveHelper  *******************************************


class SaveHelper {

public:

  SaveHelper(const QString &saveName, const QString &filter, QWidget *parent);
  ~SaveHelper();

  // returns a file open for writing
  QFile* getFile(const QString &dialogTitle);

private:

  QWidget *p_arent;
  QString s_aveName, f_ilter;
  KURL url;
  QFile* file;
  KTempFile* tmpFile;
  static QString lastPath;

};


//********  BrowseData  ******************************************


class BrowseData
{

public:

  BrowseData(const QString &Nhtml, const QString &NqueryText);

  QString html;
  QString queryText;
  int xPos,yPos;
};


//*********  DictHTMLPart  ***************************************

class DictHTMLPart : public KHTMLPart
{
  Q_OBJECT

public:

  DictHTMLPart(QWidget *parentWidget = 0, const char *widgetname = 0);
  ~DictHTMLPart();

signals:
  void middleButtonClicked();

protected:

  virtual void khtmlMouseReleaseEvent(khtml::MouseReleaseEvent *event);

};

//*********  QueryView  ******************************************


class QueryView : public QVBox
{
  Q_OBJECT

public:

  QueryView(QWidget *_parent = 0L);
  ~QueryView();

  void setActions(KToolBarPopupAction* NactBack, KToolBarPopupAction* NactForward, DictComboAction* NactQueryCombo);

  bool browseBackPossible() const;
  bool browseForwardPossible() const;

  void stop();

public slots:
  void optionsChanged();
  void printQuery();
  void saveQuery();
  void browseBack();
  void browseForward();
  void selectAll();
  void copySelection();
  void showFindDialog();

signals:

  void defineRequested(const QString &query);
  void matchRequested(const QString &query);
  void clipboardRequested();
  void enableCopy(bool selected);   // emited when the user selects/deselects text
  void enablePrintSave();
  void renderingStarted();
  void renderingStopped();
  void newCaption(const QString&);

protected:

  void paletteChange ( const QPalette & );
  void fontChange ( const QFont & );

  void saveCurrentResultPos();
  void showResult();

protected slots:

  void resultReady(const QString &result, const QString &query);
  void partCompleted();
  void slotURLRequest (const KURL &url, const KParts::URLArgs &args);
  void middleButtonClicked();
  void buildPopupMenu(const QString &url, const QPoint &point);
  void popupDefineLink();
  void popupMatchLink();
  void popupOpenLink();
  void popupDefineSelect();
  void popupMatchSelect();
  void popupDbInfo();
  void enableAction(const char *, bool);
  void browseBack(int);
  void browseForward(int);
  void updateBrowseActions();

private:

  DictHTMLPart *part;               // Widgets

  KToolBarPopupAction *actBack, *actForward;
  DictComboAction *actQueryCombo;

  KPopupMenu *rightBtnMenu;
  QString popupLink,popupSelect;       // needed for rightbtn-popup menu

  QPtrList<BrowseData> browseList;
  unsigned int browsePos;              // position in browseList
  QString currentHTMLHeader;

  bool isRendering;
};

#endif
