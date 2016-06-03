#include "../include/Current.h"
#include "../include/utils.h"
#include "../include/MsgSvc.h"

#include "TFile.h"
#include "TH1.h"

#include <vector>
#include <cmath>
#include <cstdlib>

using namespace std;

void GetCurrent(string fName, string caenName){

    //****************** DAQ ROOT FILE *******************************

    //input ROOT data file containing the RAWData TTree that we'll
    //link to our RAWData structure
    TFile dataFile(fName.c_str());

    //Then get the HVstep number from the ID histogram
    TH1D* ID = (TH1D*)dataFile.Get("ID");
    string HVstep = floatTostring(ID->GetBinContent(1));

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
    string csvName = caenName.substr(0,caenName.find_last_of("/")) + "/Online-Current.csv";
    ofstream outputCSV(csvName.c_str(),ios::app);
    //Print the HV step as first column
    outputCSV << HVstep << '\t';

    for (unsigned int t = 0; t < GIFInfra.nTrolleys; t++){
        unsigned int nSlotsTrolley = GIFInfra.Trolleys[t].nSlots;

        for (unsigned int s = 0; s < nSlotsTrolley; s++){
            unsigned int nGapsRPC = GIFInfra.Trolleys[t].RPCs[s].nGaps;

            for(unsigned int g = 0; g < nGapsRPC; g++){
                string gapID = GIFInfra.Trolleys[t].RPCs[s].gaps[g];
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
                    MSG_ERROR("[Current] Histogram " + HVeffHisto + " does not exist");
                }

                //Save the corresponding gap currents
                if(caenFile.GetListOfKeys()->Contains(ImonHisto.c_str())){
                    TH1F* Imon = (TH1F*)caenFile.Get(ImonHisto.c_str());
                    float current = Imon->GetMean();
                    float currentErr = Imon->GetRMS()/sqrt(Imon->GetEntries());

                    outputCSV << current << '\t' << currentErr << '\t';
                } else {
                    MSG_ERROR("[Current] Histogram " + ImonHisto + " does not exist");
                }
            }
        }
    }

    outputCSV << '\n';
    outputCSV.close();

    dataFile.Close();
    caenFile.Close();

    //Finally give the permission to the DCS to delete the file if necessary
    string GivePermission = "chmod 775 " + csvName;
    system(GivePermission.c_str());
}
