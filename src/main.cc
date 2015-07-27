#include "../include/NoiseRate.h"

#include <sstream>

using namespace std;

int main(int argc ,char *argv[]){
    if(argc != 3){
        cout<<"[OfflineAnalysis] USAGE is :"<< argv[0] <<" <filename> <RPCchambertype>\n";
    } else {
        stringstream converter;

        converter << argv[1] << " ";
        string fName;
        converter >> fName;

        converter << argv[2] << " ";
        string chamberType;
        converter >> chamberType;

        string fNameCSV = "AnalysedData/Summary_runs.csv";

        ifstream checkifexists(fNameCSV.c_str(), ios::in);
        if(!checkifexists) MakeHeader(fNameCSV);
        GetNoiseRate(fName,chamberType);

        return 0;
    }
}
