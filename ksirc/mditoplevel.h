#ifndef __mditoplevel_h__
#define __mditoplevel_h__

#include <qptrlist.h>
#include <qtabwidget.h>

#include <kmainwindow.h>

class KSTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    KSTabWidget ( QWidget * parent = 0, const char * name = 0, WFlags f = 0 )
	: QTabWidget(parent, name, f)
    {
    }

protected:
    virtual void mousePressEvent(QMouseEvent *e);

signals:
    void showContexMenu(QWidget *, const QPoint &);

};

class QLabel;
class KPopupMenu;

class MDITopLevel : public KMainWindow
{
    Q_OBJECT
public:
    MDITopLevel(QWidget *parent = 0, const char *name = 0);
    virtual ~MDITopLevel();

    void addWidget( QWidget *widget, bool show );

    void removeWidget( QWidget *widget );

    QTabWidget *tabWidget() const { return m_tab; }
	void next();
	void previous();

    QPtrList<QWidget> &widgets() { return m_tabWidgets; }

    void hideWidget( QWidget *widget );
    void showWidget( QWidget *widget );

    virtual bool eventFilter( QObject *obj, QEvent *ev );
    bool closing() const { return m_closing; }

protected:
    void removeFromAddressedList( const QWidget* w );
    virtual void closeEvent( QCloseEvent *ev );

private slots:
    void slotWidgetDestroyed();
    void slotCurrentChanged( QWidget *page );
    void slotMarkPageDirty( bool );
    void slotChangeChannelName( const QString &, const QString &newName );
    void slotShowContexMenu(QWidget *, const QPoint &);
    void slotCloseLastWid();

private:
    KSTabWidget *m_tab;
    KPopupMenu *m_pop;
    QWidget *m_last_pop_wid;
    QPtrList<QWidget> m_tabWidgets;
    QPtrList<QWidget> m_addressed;
    QPixmap m_dirtyIcon;
    QPixmap m_addressedIcon;
    bool m_closing;
};

#endif
