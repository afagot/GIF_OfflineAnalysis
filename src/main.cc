//***************************************************************
// *    GIF OFFLINE TOOL v7
// *
// *    Program developped to extract from the raw data files
// *    the rates and currents.
// *
// *    main.cc
// *
// *    Developped by : Alexis Fagot & Salvador Carillo
// *    07/06/2017
//***************************************************************

#include <sstream>
#include <string>

#include "../include/OfflineAnalysis.h"
#include "../include/Current.h"
#include "../include/MsgSvc.h"
#include "../include/utils.h"

using namespace std;

int main(int argc ,char *argv[]){
    stringstream converter;
    converter << argv[0];
    string program;
    converter >> program;
    converter.clear();

    if(argc != 2){
        MSG_WARNING("[Offline] expects to have 2 parameters");
        MSG_WARNING("[Offline] USAGE is : " + program + " filebasename");
        return -1;
    } else if(argc == 2){
        converter << argv[1];
        string baseName;
        converter >> baseName;
        converter.clear();

        //Write in the files of the RUN directory the path to the files
        //in the HVSCAN directory to know where to write the logs
        WritePath(baseName);

        //Start the needed analysis tools - check if the ROOT files exist
        string daqName = baseName + "_DAQ.root";
        if(existFile(daqName)) OfflineAnalysis(baseName);
        else MSG_ERROR("[Offline] No DAQ file for run " + baseName);

        string caenName = baseName + "_CAEN.root";
        if(existFile(caenName)) GetCurrent(baseName);
        else  MSG_ERROR("[Offline] No CAEN file for run " + baseName);

        return 0;
    }
}
