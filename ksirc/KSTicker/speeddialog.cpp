/**********************************************************************

	--- Qt Architect generated file ---

	File: speeddialog.cpp
	Last generated: Sun Dec 21 08:52:31 1997

 *********************************************************************/

#include <klocale.h>

#include "speeddialog.h"

SpeedDialog::SpeedDialog
(
 int tick,
 int step,
 QWidget* parent,
 const char* name
)
  : speeddialogData( parent, name )
{
  setCaption(i18n( "Speed Setup") );
  connect(sliderTick, SIGNAL(valueChanged(int)),
	  lcdTick, SLOT(display(int)));
  connect(sliderStep, SIGNAL(valueChanged(int)),
	  lcdStep, SLOT(display(int)));
  lcdTick->display(tick);
  sliderTick->setValue(tick);
  lcdStep->display(step);
  sliderStep->setValue(step);
}


SpeedDialog::~SpeedDialog()
{
}


void SpeedDialog::updateTick(int tick)
{
  emit stateChange(tick, sliderStep->value());
}

void SpeedDialog::updateStep(int step)
{
  emit stateChange(sliderTick->value(), step);
}

void SpeedDialog::terminate()
{
  delete this;
}

void SpeedDialog::setLimit(int tmin, int tmax, int smin, int smax)
{
  sliderTick->setRange(tmin, tmax);
  sliderStep->setRange(smin, smax);
}

#include "speeddialog.moc"
