/* -------------------------------------------------------------

   options.h (part of The KDE Dictionary Client)

   Copyright (C) 2000-2001 Christian Gebauer <gebauer@kde.org>

   This file is distributed under the Artistic License.
   See LICENSE for details.

   -------------------------------------------------------------

   GlobalData      manages all global data of Kdict
   DialgoListBox   a list box which ignores Enter, usefull for dialogs
   OptionsDialog   the "Preferences" dialog

 ------------------------------------------------------------- */

#ifndef _KDICT_OPTIONS_H_
#define _KDICT_OPTIONS_H_

#include <qlistbox.h>
#include <kdialogbase.h>
#include <kglobalsettings.h>

class QLineEdit;
class QCheckBox;
class QComboBox;
class QRadioButton;

class KColorButton;
class KLineEdit;
class KIntSpinBox;


//********* GlobalData ******************************************

#define COL_CNT 6
#define FNT_CNT 2

class GlobalData
{

public:

  enum ColorIndex   { Ctext=0, Cbackground=1, CheadingsText=2, CheadingsBackground=3, Clinks=4, CvisitedLinks=5  };
  enum FontIndex    { Ftext=0, Fheadings=1 };

  void read();
  void write();

  // colors...
  const QColor& color(int i)           { return c_olors[i]; }
  const QString& colorName(int i)      { return c_olorNames[i]; }
  int colorCount() const                    { return COL_CNT; }
  QColor defaultColor(int i);
  bool useCustomColors;
  QColor textColor();
  QColor backgroundColor();
  QColor headingsTextColor();
  QColor headingsBackgroundColor();
  QColor linksColor();
  QColor visitedLinksColor();

  // fonts...
  const QFont& font(int i)             { return f_onts[i]; }
  const QString& fontName(int i)       { return f_ontNames[i]; }
  int fontCount() const                     { return FNT_CNT; }
  QFont defaultFont(int);
  bool useCustomFonts;
  QFont textFont();
  QFont headingsFont();

  QString encryptStr(const QString& aStr);

  bool defineClipboard;           // define clipboard content on startup?

  QSize optSize,setsSize,matchSize;      // window geometry
  bool showMatchList;
  QValueList<int> splitterSizes;

  KGlobalSettings::Completion queryComboCompletionMode;

  QStringList queryHistory;
  bool saveHistory;               // save query history to disk on exit?
  unsigned int maxHistEntrys, maxBrowseListEntrys, maxDefinitions;
  int headLayout;

  QString server;                 // network client...
  int port,timeout,pipeSize,idleHold;
  QString encoding;
  bool authEnabled;
  QString user, secret;
  QStringList serverDatabases, databases, strategies;
  QPtrList<QStringList> databaseSets;
  unsigned int currentDatabase, currentStrategy;

  QColor  c_olors[COL_CNT];
  QString c_olorNames[COL_CNT];
  QFont   f_onts[FNT_CNT];
  QString f_ontNames[FNT_CNT];

  QWidget *topLevel;
};

extern GlobalData *global;


//*********  OptionsDialog  ******************************************


class OptionsDialog : public KDialogBase
{
  Q_OBJECT

public:

  OptionsDialog(QWidget *parent=0, const char *name=0);
  ~OptionsDialog();

  //===================================================================================

  class DialogListBox : public QListBox {

    public:
      // alwaysIgnore==false: enter is ignored when the widget isn't visible/out of focus
      DialogListBox(bool alwaysIgnore=false, QWidget * parent=0, const char * name=0);
      ~DialogListBox();

    protected:
      void keyPressEvent( QKeyEvent *e );

      bool a_lwaysIgnore;
  };

  //===================================================================================

  class ColorListItem : public QListBoxText {

    public:
      ColorListItem( const QString &text, const QColor &color=Qt::black );
      ~ColorListItem();
      const QColor& color()                     { return mColor; }
      void  setColor( const QColor &color )     { mColor = color; }

    protected:
      virtual void paint( QPainter * );
      virtual int height( const QListBox * ) const;
      virtual int width( const QListBox * ) const;

    private:
      QColor mColor;
  };

  //===================================================================================

  class FontListItem : public QListBoxText {

    public:
      FontListItem( const QString &name, const QFont & );
      ~FontListItem();
      const QFont& font()                     { return f_ont; }
      void setFont( const QFont &);
    protected:
      virtual void paint( QPainter * );
      virtual int width( const QListBox * ) const;

    private:
      QFont f_ont;
      QString fontInfo;
  };

  //===================================================================================

signals:

  void optionsChanged();

protected slots:
  void slotApply();
  void slotOk();
  void slotDefault();
  void slotChanged();

  //server
  void slotAuthRequiredToggled( bool );

  //colors
  void slotColCheckBoxToggled(bool b);
  void slotColItemSelected(QListBoxItem *);   // show color dialog for the entry
  void slotColDefaultBtnClicked();
  void slotColChangeBtnClicked();
  void slotColSelectionChanged();

  //fonts
  void slotFontCheckBoxToggled(bool b);
  void slotFontItemSelected(QListBoxItem *);  // show font dialog for the entry
  void slotFontDefaultBtnClicked();
  void slotFontChangeBtnClicked();
  void slotFontSelectionChanged();

private:

  QFrame *serverTab;
  QLabel *l_user, *l_secret;
  KLineEdit *w_server, *w_user, *w_secret, *w_port;
  QComboBox *w_encoding;
  QCheckBox *w_auth;
  KIntSpinBox *w_idleHold,*w_timeout,*w_pipesize;

  QFrame *appTab;
  DialogListBox *c_List,
                *f_List;
  QCheckBox *c_olorCB,
            *f_ontCB;
  QPushButton *c_olDefBtn,
              *c_olChngBtn,
              *f_ntDefBtn,
              *f_ntChngBtn;

  QFrame *layoutTab;
  QRadioButton *w_layout[3];

  QFrame *otherTab;
  QCheckBox *w_Clipboard, *w_Savehist;
  KIntSpinBox *w_Maxhist, *w_Maxbrowse, *w_MaxDefinitions;
  bool configChanged;
};

#endif
