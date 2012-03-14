/**********************************************************************

 The IO Discarder

 $$Id: ioDiscard.cpp 146382 2002-04-01 08:25:23Z hausmann $$

 Simple rule, junk EVERYTHING!!!

**********************************************************************/

#include "ioDiscard.h"

KSircIODiscard::~KSircIODiscard()
{
}

void KSircIODiscard::sirc_receive(QCString, bool)
{
}

void KSircIODiscard::control_message(int, QString)
{
}
