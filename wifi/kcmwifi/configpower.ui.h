/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

void ConfigPower::load( const IfConfig &ifconfig)
{
  sb_sleepTimeout->setValue(ifconfig.m_sleepTimeout);
  sb_wakeupPeriod->setValue(ifconfig.m_wakeupPeriod);
  bg_packets->setButton(ifconfig.m_pmMode);
}

void ConfigPower::save( IfConfig &ifconfig)
{
  ifconfig.m_sleepTimeout = sb_sleepTimeout->value();
  ifconfig.m_wakeupPeriod = sb_wakeupPeriod->value();
  ifconfig.m_pmMode = (IfConfig::PowerMode)bg_packets->selectedId();
}
