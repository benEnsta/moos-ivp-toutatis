/************************************************************/
/*    FILE: Saucisse.cpp
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "Saucisse.h"

using namespace std;

//---------------------------------------------------------
// Constructor

Saucisse::Saucisse()
{

}

//---------------------------------------------------------
// Denstructor

Saucisse::~Saucisse()
{
  delete m_pololu;
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool Saucisse::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);

  MOOSMSG_LIST::iterator p;
  for(p = NewMail.begin() ; p != NewMail.end() ; p++)
  {
    CMOOSMsg &msg = *p;
    string key    = msg.GetKey();

    #if 0 // Keep these around just for template
      string comm  = msg.GetCommunity();
      double dval  = msg.GetDouble();
      string sval  = msg.GetString(); 
      string msrc  = msg.GetSource();
      double mtime = msg.GetTime();
      bool   mdbl  = msg.IsDouble();
      bool   mstr  = msg.IsString();
    #endif

    /*if(key == "POWER_CAMERA1")
    {
      int success = m_pololu->turnOnBistableRelay(2, 3, (int)msg.GetDouble() == 1);
      Notify("POWERED_CAMERA1", success >= 0 ? (int)msg.GetDouble() : -1);
    }

    else if(key == "POWER_CAMERA2")
    {
      int success = m_pololu->turnOnBistableRelay(4, 5, (int)msg.GetDouble() == 1);
      Notify("POWERED_CAMERA2", success >= 0 ? (int)msg.GetDouble() : -1);
    }

    else */if(key == "POWER_MODEM")
    {
      int success = m_pololu->turnOnBistableRelay(6, 7, (int)msg.GetDouble() == 1);
      Notify("POWERED_MODEM", success >= 0 ? (int)msg.GetDouble() : -1);
    }

    else if(key == "POWER_MODEM_EA")
    {
      int success = m_pololu->turnOnBistableRelay(8, 9, (int)msg.GetDouble() == 1);
      Notify("POWERED_MODEM_EA", success >= 0 ? (int)msg.GetDouble() : -1);
    }

    else if(key == "POWER_SONAR")
    {
      int success = m_pololu->turnOnBistableRelay(10, 11, (int)msg.GetDouble() == 1);
      Notify("POWERED_SONAR", success >= 0 ? (int)msg.GetDouble() : -1);
    }

    else if(key == "POWER_ECHOSOUNDER")
    {
      int success = m_pololu->turnOnBistableRelay(12, 13, (int)msg.GetDouble() == 1);
      Notify("POWERED_ECHOSOUNDER", success >= 0 ? (int)msg.GetDouble() : -1);
    }

    /*else if(key == "POWER_GPS")
    {
      int success = m_pololu->turnOnBistableRelay(14, 15, (int)msg.GetDouble() == 1);
      Notify("POWERED_GPS", success >= 0 ? (int)msg.GetDouble() : -1);
    }*/

    else if(key != "APPCAST_REQ") // handle by AppCastingMOOSApp
      reportRunWarning("Unhandled Mail: " + key);
  }

  return true;
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool Saucisse::OnConnectToServer()
{
  registerVariables();
  return true;
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool Saucisse::Iterate()
{
  AppCastingMOOSApp::Iterate();

  string error_message;
  bool pololu_ok = m_pololu->isReady(error_message);
  Notify("SAUCISSE_POLOLU_STATUS", pololu_ok ? "ok" : error_message);

  AppCastingMOOSApp::PostReport();
  return true;
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool Saucisse::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp();

  STRING_LIST sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  if(!m_MissionReader.GetConfiguration(GetAppName(), sParams))
    reportConfigWarning("No config block found for " + GetAppName());

  STRING_LIST::iterator p;
  sParams.reverse();
  for(p = sParams.begin() ; p != sParams.end() ; p++)
  {
    string orig  = *p;
    string line  = *p;
    string param = toupper(biteStringX(line, '='));
    string value = line;
    bool handled = false;

    if(param == "DEVICE_NAME")
    {
      m_device_name = value;
      handled = true;
    }

    if(!handled)
      reportUnhandledConfigWarning(orig);
  }

  m_pololu = new Pololu(m_device_name);
  registerVariables();  
  return true;
}

//---------------------------------------------------------
// Procedure: registerVariables

void Saucisse::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  Register("POWER_*", "*", 0);
}

//------------------------------------------------------------
// Procedure: buildReport()

bool Saucisse::buildReport() 
{
  m_msgs << "============================================ \n";
  m_msgs << "iRosen:                                      \n";
  m_msgs << "============================================ \n";
  m_msgs << "\n";
  
  string error_message;
  bool pololu_ok = m_pololu->isReady(error_message);
  m_msgs << "Status: " << (pololu_ok ? "ok" : error_message);

  return true;
}
