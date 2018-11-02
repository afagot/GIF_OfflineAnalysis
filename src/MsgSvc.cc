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

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <iomanip>
#include <fstream>
#include <sstream>

#include "../include/MsgSvc.h"
#include "../include/types.h"

using namespace std;

// ****************************************************************************************************
// *    string GetLogTimeStamp()
//
//  Function that gets the system time. The output format of this function has been optimised to be
//  used in the log file. For each log message, the line starts with this time stamp.
// ****************************************************************************************************

string GetLogTimeStamp(){
    stringstream stream;

    //Get time information
    time_t t = time(0);
    struct tm *Time = localtime(&t);
    int Y = Time->tm_year + 1900;
    int M = Time->tm_mon + 1;
    int D = Time->tm_mday;
    int h = Time->tm_hour;
    int m = Time->tm_min;
    int s = Time->tm_sec;

    //Set the Date
    //Format is YYYY-MM-DD.hh:mm:ss.
    string Date;

    stream << setfill('0') << setw(4) << Y << "-"
           << setfill('0') << setw(2) << M << "-"
           << setfill('0') << setw(2) << D << "."
           << setfill('0') << setw(2) << h << ":"
           << setfill('0') << setw(2) << m << ":"
           << setfill('0') << setw(2) << s << ".";

    stream >> Date;
    stream.clear();

    return Date;
}

// ****************************************************************************************************
// *    int MSG(string message, int level)
//
//  Generic messaging function. It prints log messages into the scan log file.
// ****************************************************************************************************

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
