#include "../include/NoiseRate.h"

#include <sstream>

using namespace std;

int main(int argc ,char *argv[]){
    if(argc != 3){
        cout<<"[OfflineAnalysis]: USAGE is :"<< argv[0] <<" DAQfilename CAENfilename \n";
        return -1;
    } else {
        stringstream converter;

        converter << argv[1];
        string fName;
        converter >> fName;
        converter.clear();

        converter << argv[2];
        string caenName;
        converter >> caenName;
        converter.clear();

        GetNoiseRate(fName, caenName);

        return 0;
    }
}
