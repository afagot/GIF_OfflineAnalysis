#ifndef _MSGSVC_H_
#define _MSGSVC_H_

//***************************************************************
// *    GIF OFFLINE TOOL v3
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
// *    Developped by : Alexis Fagot
// *    29/01/2015
//***************************************************************

#include <string>

using namespace std;

#define FATAL   -3
#define ERROR   -2
#define WARNING -1
#define INFO     0
#define DEBUG    1
#define VERBOSE  2
#define ALWAYS   3


int MSG(string message, int level);

void MSG_FATAL(string message);
void MSG_ERROR(string message);
void MSG_WARNING(string message);
void MSG_INFO(string message);
void MSG_DEBUG(string message);
void MSG_VERBOSE(string message);
void MSG_ALWAYS(string message);

#endif

