#include "../include/NoiseRate.h"

#include <sstream>

using namespace std;

int main(int argc ,char *argv[]){
    string triggertype[2] = {"random","beam"};

    if(argc != 3){
        cout<<"[OfflineAnalysis]: USAGE is :"<< argv[0] <<" filename triggertype\n";
        cout<<"[OfflineAnalysis]: triggertype = " << triggertype[0] << " or " << triggertype[1] << endl;
        return -1;
    } else {// if(argv[2] == triggertype[0].c_str() || argv[2] == triggertype[1].c_str()){
        stringstream converter;

        converter << argv[1];
        string fName;
        converter >> fName;
        converter.clear();

        converter << argv[2];
        string trigger;
        converter >> trigger;

//        cout << fName << " " << trigger << endl;

        GetNoiseRate(fName,trigger);

        return 0;
    }/* else {
        cout<<"[OfflineAnalysis]: Wrong trigger type.\n";
        cout<<"[OfflineAnalysis]: USAGE is :"<< argv[0] <<" filename triggertype\n";
        cout<<"[OfflineAnalysis]: triggertype = " << triggertype[0] << " or " << triggertype[1] << endl;
        return -2;
    }*/
}
