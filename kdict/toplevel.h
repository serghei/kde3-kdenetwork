/* -------------------------------------------------------------

   toplevel.h (part of The KDE Dictionary Client)

   Copyright (C) 2000-2001 Christian Gebauer <gebauer@kde.org>
   (C) by Matthias Hölzer 1998

   This file is distributed under the Artistic License.
   See LICENSE for details.

   -------------------------------------------------------------

   TopLevel   The toplevel widget of Kdict.

 ------------------------------------------------------------- */

#ifndef _TOPLEVEL_H_
#define _TOPLEVEL_H_

#include <qtimer.h>
#include <kmainwindow.h>
#include "dcopinterface.h"

class QSplitter;

class KToggleAction;
class KToolBarPopupAction;

class DictLabelAction;
class DictComboAction;
class DictButtonAction;
class MatchView;
class QueryView;
class OptionsDialog;
class DbSetsDialog;


class TopLevel : public KMainWindow, virtual public KDictIface
{
  Q_OBJECT

  friend class QueryView;

public:

  TopLevel(QWidget* parent = 0, const char* name = 0);
  ~TopLevel();

  void normalStartup();             // called when started without commandline parameters

  // DCOP-Interface...
  void quit();
  void makeActiveWindow();
  void definePhrase(QString phrase);
  void matchPhrase(QString phrase);
  void defineClipboardContent();
  void matchClipboardContent();
  QStringList getDatabases();
  QString currentDatabase();
  QStringList getStrategies();
  QString currentStrategy();
  bool setDatabase(QString db);
  bool setStrategy(QString strategy);
  bool historyGoBack();
  bool historyGoForward();

public slots:

  void define(const QString &query);
  void defineClipboard();

  void match(const QString &query);
  void matchClipboard();

protected:
	bool queryClose();

private:

  void setupActions();
  void setupStatusBar();
  void recreateGUI();
  void raiseWindow();

  void addCurrentInputToHistory();  // add text in the query-combobox to the history

private slots:
  void clearInput();                // erase text in query-combobox

  void doDefine();                  // define text in the combobox
  void doMatch();                   // match text in the combobox

  void stopClients();

  void buildHistMenu();
  void queryHistMenu();             // process a query via the history menu
  void clearQueryHistory();

  void stratDbChanged();
  void dbInfoMenuClicked();
  void databaseSelected(int num);

  void enableCopy(bool selected);
  void enablePrintSave();

  void clientStarted(const QString &message);
  void clientStopped(const QString &message);
  void resetStatusbar();
  void renderingStarted();
  void renderingStopped();

  void newCaption(const QString&);

  void toggleMatchListShow();
  void saveMatchViewSize();
  void adjustMatchViewSize();

  void slotConfToolbar();
  void slotNewToolbarConfig();

  void showSetsDialog();
  void hideSetsDialog();
  void setsChanged();

  void showOptionsDialog();
  void hideOptionsDialog();
  void optionsChanged();

private:

  KAction *actSave, *actPrint, *actStartQuery, *actStopQuery, *actCopy;
  KToggleAction *actShowMatchList;
  DictLabelAction *actQueryLabel, *actDbLabel;
  DictComboAction *actQueryCombo, *actDbCombo;
  DictButtonAction *actDefineBtn, *actMatchBtn;
  QPtrList<KAction> historyActionList, dbActionList;
  KToolBarPopupAction *actBack, *actForward;

  QSplitter *splitter;              // widgets....
  QueryView *queryView;
  MatchView *matchView;
  OptionsDialog *optDlg;
  DbSetsDialog *setsDlg;

  QTimer resetStatusbarTimer;
  int stopRef;                      // remember how many "clients" are running
};

#endif
