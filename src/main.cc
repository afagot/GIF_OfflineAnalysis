//***************************************************************
// *    GIF OFFLINE TOOL v3
// *
// *    Program developped to extract from the raw data files
// *    the rates, currents and DIP parameters.
// *
// *    main.cc
// *
// *    Developped by : Alexis Fagot
// *    22/04/2016
//***************************************************************


#include "../include/NoiseRate.h"
#include "../include/Current.h"
#include "../include/DIP.h"
#include "../include/MsgSvc.h"

#include <sstream>

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
        if(existFiles(baseName)) {
            GetNoiseRate(baseName);
            GetCurrent(baseName);
            GetDIP(baseName);
        } else {
            MSG_ERROR("[Offline] a data file is missing for run " + baseName);
        }

        return 0;
    }
}
