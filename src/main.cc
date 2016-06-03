#include "../include/NoiseRate.h"
#include "../include/Current.h"
#include "../include/MsgSvc.h"

#include <sstream>

using namespace std;

int main(int argc ,char *argv[]){
    stringstream converter;
    converter << argv[0];
    string program;
    converter >> program;
    converter.clear();

    if(argc != 3){
        MSG_WARNING("[Online] USAGE is : " + program + " DAQfilename CAENfilename");
        return -1;
    } else {
        converter << argv[1];
        string fName;
        converter >> fName;
        converter.clear();

        converter << argv[2];
        string caenName;
        converter >> caenName;
        converter.clear();

        GetNoiseRate(fName, caenName);
        GetCurrent(fName, caenName);

        MSG_INFO("[Online] Analysis complete");
        return 0;
    }
}
