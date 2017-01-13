#ifndef _MSGSVC_CXX_
#define _MSGSVC_CXX_

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

#include "../include/MsgSvc.h"
#include "../include/utils.h"
#include <iostream>
#include <fstream>

using namespace std;

int MSG(string message, int level){
    //First we need to get the log file path into the log life
    //in the RUN directory, then we will know where to write the
    //logs.
    string logpath;

    ifstream logpathfile(__logpath.c_str(), ios::in);
    if(logpathfile){
        logpathfile >> logpath;
        logpathfile.close();

        ofstream logfile(logpath.c_str(), ios::app);
        if(logfile){
            logfile << GetLogTimeStamp() << message << endl;
            logfile.close();
            return level;
        } else {
            cout << "File not found " << logpath << endl;
            exit(EXIT_FAILURE);
        }
    } else {
        cout << "File not found " << __logpath << endl;
        exit(EXIT_FAILURE);
    }
}

void MSG_FATAL(string message)  {MSG(message,FATAL);}
void MSG_ERROR(string message)  {MSG(message,ERROR);}
void MSG_WARNING(string message){MSG(message,WARNING);}
void MSG_INFO(string message)   {MSG(message,INFO);}
void MSG_DEBUG(string message)  {MSG(message,DEBUG);}
void MSG_VERBOSE(string message){MSG(message,VERBOSE);}
void MSG_ALWAYS(string message) {MSG(message,ALWAYS);}


#endif

