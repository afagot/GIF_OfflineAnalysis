#ifndef __MSGSVC_H_
#define __MSGSVC_H_

//***************************************************************
// *    GIF OFFLINE TOOL v7
// *
// *    Program developped to extract from the raw data files
// *    the rates, currents and DIP parameters.
// *
// *    MsgSvc.cc
// *
// *    Macro to print log messages into the log file.
// *    This file was originally developped for the GIF_DAQ.
// *    and was an improvement of an existing file created by
// *    Nir Amram on 21/06/2010
// *
// *    Developped by : Alexis Fagot & Salvador Carillo
// *    07/06/2017
//***************************************************************

#include <string>

using namespace std;

//Function that returns the UNIX time for the log messages
string GetLogTimeStamp();

//Different codes returned by the messaging function
const int FATAL   =-3;
const int ERROR   =-2;
const int WARNING =-1;
const int INFO    = 0;
const int DEBUG   = 1;
const int VERBOSE = 2;
const int ALWAYS  = 3;
const int INIT    = 4;

//Generic messaging function
int MSG(string message, int level);

//Different messaging functions calling the generic
//with the assiaciated level
void MSG_FATAL(string message);
void MSG_ERROR(string message);
void MSG_WARNING(string message);
void MSG_INFO(string message);
void MSG_DEBUG(string message);
void MSG_VERBOSE(string message);
void MSG_ALWAYS(string message);

int MSG_INIT();

#endif

