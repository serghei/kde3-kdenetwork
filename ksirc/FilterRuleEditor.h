/**********************************************************************

	--- Qt Architect generated file ---

	File: FilterRuleEditor.h
	Last generated: Mon Dec 15 18:14:27 1997

 *********************************************************************/

#ifndef FilterRuleEditor_h
#define FilterRuleEditor_h

#include <qstring.h>

#include <kdialogbase.h>

#include "FilterRuleWidget.h"

class FilterRuleEditor : public KDialogBase
{
    Q_OBJECT

public:

    FilterRuleEditor
    (
        QWidget* parent = NULL,
        const char* name = NULL
    );

    virtual ~FilterRuleEditor();

protected slots:

     virtual void newRule();
     virtual void OkPressed();
     virtual void deleteRule();
     virtual void newHighlight(int);
     virtual void raiseRule();
     virtual void lowerRule();

protected:
     void updateListBox(int citem = 0);
     void moveRule(int, int);
     QString convertSpecial(QString);
     QString convertSpecialBack(QString);

    FilterRuleWidget *filter;
};
#endif // FilterRuleEditor_included

// vim:tabstop=2:shiftwidth=2:expandtab:cinoptions=(s,U1,m1

