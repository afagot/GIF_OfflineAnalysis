#include "../include/Current.h"
#include "../include/utils.h"
#include "../include/MsgSvc.h"

#include "TFile.h"
#include "TH1.h"

#include <vector>
#include <cmath>
#include <cstdlib>

using namespace std;

void GetCurrent(string caenName){

    //****************** HVSTEP **************************************

    //Get the HVstep number from the file name
    string HVstep = caenName.substr(caenName.find_last_of("_")-1,1);

    //****************** CAEN ROOT FILE ******************************

    //input CAEN ROOT data file containing the values of the HV eff for
    //every HV step
    TFile caenFile(caenName.c_str());

    //****************** GEOMETRY ************************************

    //Get the chamber geometry
    IniFile* Dimensions = new IniFile(__dimensions.c_str());
    Dimensions->Read();

    Infrastructure GIFInfra;
    SetInfrastructure(GIFInfra,Dimensions);


    //****************** OUPUT FILE **********************************

    //output csv file
    string csvName = caenName.substr(0,caenName.find_last_of("/")) + "/Offline-Current.csv";
    ofstream outputCSV(csvName.c_str(),ios::app);
    //Print the HV step as first column
    outputCSV << HVstep << '\t';

    for (unsigned int t = 0; t < GIFInfra.nTrolleys; t++){
        unsigned int nSlotsTrolley = GIFInfra.Trolleys[t].nSlots;

        for (unsigned int s = 0; s < nSlotsTrolley; s++){
            unsigned int nGapsRPC = GIFInfra.Trolleys[t].RPCs[s].nGaps;

            for(unsigned int g = 0; g < nGapsRPC; g++){
                string gapID = GIFInfra.Trolleys[t].RPCs[s].gaps[g];
                float areagap = GIFInfra.Trolleys[t].RPCs[s].gapGeo[g];
                string ImonHisto, HVeffHisto;

                if(gapID == "empty"){
                    HVeffHisto = "HVeff_" + GIFInfra.Trolleys[t].RPCs[s].name;
                    ImonHisto = "Imon_" + GIFInfra.Trolleys[t].RPCs[s].name;
                } else {
                    HVeffHisto = "HVeff_" + GIFInfra.Trolleys[t].RPCs[s].name + "-" + gapID;
                    ImonHisto = "Imon_" + GIFInfra.Trolleys[t].RPCs[s].name + "-" + gapID;
                }

                //Save the effective voltages
                if(caenFile.GetListOfKeys()->Contains(HVeffHisto.c_str())){
                    TH1F* HVeff = (TH1F*)caenFile.Get(HVeffHisto.c_str());
                    float voltage = HVeff->GetMean();
                    outputCSV << voltage << '\t';
                } else {
                    float voltage = 0.;
                    outputCSV << voltage << '\t';
                }

                //Save the corresponding gap currents
                if(caenFile.GetListOfKeys()->Contains(ImonHisto.c_str())){
                    TH1F* Imon = (TH1F*)caenFile.Get(ImonHisto.c_str());
                    float current = Imon->GetMean()/areagap;
                    float currentErr = Imon->GetRMS()/sqrt(Imon->GetEntries())/areagap;
                    outputCSV << current << '\t' << currentErr << '\t';
                } else {
                    float current = 0.;
                    float currentErr = 0.;
                    outputCSV << current << '\t' << currentErr << '\t';
                }
            }
        }
    }

    outputCSV << '\n';
    outputCSV.close();

    caenFile.Close();

    //Finally give the permission to the DCS to delete the file if necessary
    string GivePermission = "chmod 775 " + csvName;
    system(GivePermission.c_str());
}
