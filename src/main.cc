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

    if(argc != 2 && argc != 3){
        MSG_WARNING("[Offline] expects to have 2 or 3 parameters");
        MSG_WARNING("[Offline] USAGE is : " + program + " CAENfilename");
        MSG_WARNING("[Offline] or : " + program + " DAQfilename CAENfilename");
        return -1;
    } else if(argc == 2){
        converter << argv[1];
        string caenName;
        converter >> caenName;
        converter.clear();

        GetCurrent(caenName);
        GetDIP(caenName);

        if(!IsReRunning(caenName)) MSG_INFO("[Offline] Current HVScan Analysis complete");
        return 0;
    } else if(argc == 3){
        converter << argv[1];
        string fName;
        converter >> fName;
        converter.clear();

        converter << argv[2];
        string caenName;
        converter >> caenName;
        converter.clear();

        GetNoiseRate(fName, caenName);
        GetCurrent(caenName);
        GetDIP(caenName);

        if(!IsReRunning(caenName)) MSG_INFO("[Offline] DAQ HVScan Analysis complete");
        return 0;
    }
}
