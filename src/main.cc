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

        //Write in the log file of the RUN directory the path to the log file
        //in the HVSCAN directory to know where to write the logs.
        string logpath = baseName.substr(0, baseName.find_last_of("/")+1) + "log.txt";

        ofstream logpathfile(__logpath.c_str(), ios::out);
        logpathfile << logpath;
        logpathfile.close();

        if(existFile(baseName+"_DAQ.root")){
            if(existFile(baseName+"_CAEN.root"))
                GetNoiseRate(baseName+"_DAQ.root",baseName+"_CAEN.root");
            else
                GetNoiseRate(baseName+"_DAQ.root");
        }
        if(existFile(baseName+"_CAEN.root")) GetCurrent(baseName+"_CAEN.root");
        if(existFile(baseName+"_DIP.root")) GetDIP(baseName+"_DIP.root");

        return 0;
    }
}
