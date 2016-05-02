#include "../include/NoiseRate.h"

#include <sstream>

using namespace std;

int main(int argc ,char *argv[]){
    if(argc != 2){
        cout<<"[OfflineAnalysis]: USAGE is :"<< argv[0] <<" filename triggertype\n";
        return -1;
    } else {
        stringstream converter;

        converter << argv[1];
        string fName;
        converter >> fName;
        converter.clear();

        GetNoiseRate(fName);

        return 0;
    }
}
