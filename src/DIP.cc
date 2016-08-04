
#include "../include/DIP.h"
#include "../include/utils.h"
#include "../include/MsgSvc.h"

#include "TFile.h"
#include "TH1.h"

#include <vector>
#include <cmath>
#include <cstdlib>

using namespace std;

void GetDIP(string dipName){

    //****************** HVSTEP **************************************

    //Get the HVstep number from the file name
    unsigned int length = dipName.rfind("_") - dipName.rfind("HV") - 2;
    string HVstep = dipName.substr(dipName.find_last_of("_")-length,length);

    //*************** DIP ROOT FILE **********************************

    //input DIP ROOT data file containing the values of DIP parameters
    //for every HV step
    TFile dipFile(dipName.c_str());


    //****************** OUPUT FILE **********************************


    //output csv file
    string csvName = dipName.substr(0,dipName.find_last_of("/")) + "/Offline-DIP.csv";
    ofstream outputCSV(csvName.c_str(),ios::app);
    //Print the HV step as first column
    outputCSV << HVstep << '\t';

    //output csv file to save the list of parameters saved into the
    //Offline-DIP.csv file - it represents the header of that file
    string listName = dipName.substr(0,dipName.find_last_of("/")) + "/Offline-DIP-Header.csv";
    ofstream listCSV(listName.c_str(),ios::out);
    listCSV << "HVstep\t";

    for(int k = 0; k < dipFile.GetListOfKeys()->GetSize(); k++){
        TH1F* Parameter = (TH1F*)dipFile.Get(dipFile.GetListOfKeys()->At(k)->GetName());
        float value = Parameter->GetMean();
        float error = Parameter->GetRMS()/sqrt(Parameter->GetEntries());

        outputCSV << value << '\t' << error << '\t';
        listCSV << dipFile.GetListOfKeys()->At(k)->GetName() << '\t'
                << dipFile.GetListOfKeys()->At(k)->GetName() << "_err\t";
    }
    listCSV.close();

    outputCSV << '\n';
    outputCSV.close();

    dipFile.Close();

    //Finally give the permission to the DCS to delete the file if necessary
    string GivePermission = "chmod 775 " + csvName;
    system(GivePermission.c_str());
}
