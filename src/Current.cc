//***************************************************************
// *    904 OFFLINE TOOL v4
// *
// *    Program developped to extract from the raw data files
// *    the rates, currents and DIP parameters.
// *
// *    Current.cc
// *
// *    Current extraction from Scan_00XXXX_HVX_CAEN.root files
// *
// *    Developped by : Alexis Fagot
// *    07/03/2017
//***************************************************************

#include <fstream>

#include "TFile.h"
#include "TH1F.h"

#include "../include/MsgSvc.h"
#include "../include/IniFile.h"
#include "../include/Current.h"
#include "../include/utils.h"

using namespace std;

void GetCurrent(string baseName){

    string caenName = baseName + "_CAEN.root";

    //****************** HVSTEP **************************************

    //Get the HVstep number from the file name
    unsigned int length = caenName.rfind("_") - caenName.rfind("HV") - 2;
    string HVstep = caenName.substr(caenName.find_last_of("_")-length,length);

    //****************** CAEN ROOT FILE ******************************

    //input CAEN ROOT data file containing the values of the HV eff for
    //every HV step
    TFile caenFile(caenName.c_str());

    if(caenFile.IsOpen()){

        //****************** GEOMETRY ************************************

        //Get the chamber geometry
        string dimpath = caenName.substr(0,caenName.find_last_of("/")) + "/Dimensions.ini";
        IniFile* Dimensions = new IniFile(dimpath.c_str());
        Dimensions->Read();

        Infrastructure GIFInfra;
        SetInfrastructure(GIFInfra,Dimensions);


        //****************** OUPUT FILE **********************************

        //output csv file
        string csvName = caenName.substr(0,caenName.find_last_of("/")) + "/Offline-Current.csv";
        ofstream outputCSV(csvName.c_str(),ios::app);

        //Print the HV step as first column
        outputCSV << HVstep << '\t';

        //output csv file to save the list of parameters saved into the
        //Offline-Current.csv file - it represents the header of that file
        string listName = caenName.substr(0,caenName.find_last_of("/")) + "/Offline-Current-Header.csv";
        ofstream listCSV(listName.c_str(),ios::out);
        listCSV << "HVstep\t";

        unsigned int nSlots = GIFInfra.nSlots;

        for (unsigned int s = 0; s < nSlots; s++){
            unsigned int nGapsRPC = GIFInfra.RPCs[s].nGaps;

            for(unsigned int g = 0; g < nGapsRPC; g++){
                string gapID = GIFInfra.RPCs[s].gaps[g];
                float areagap = GIFInfra.RPCs[s].gapGeo[g];
                string ImonHisto, HVeffHisto, ADCHisto;

                //Histogram names
                if(gapID == "empty"){
                    HVeffHisto = "HVeff_" + GIFInfra.RPCs[s].name;
                    ImonHisto = "Imon_" + GIFInfra.RPCs[s].name;
                    ADCHisto = "ADC_" + GIFInfra.RPCs[s].name;
                } else {
                    HVeffHisto = "HVeff_" + GIFInfra.RPCs[s].name + "-" + gapID;
                    ImonHisto = "Imon_" + GIFInfra.RPCs[s].name + "-" + gapID;
                    ADCHisto = "ADC_" + GIFInfra.RPCs[s].name + "-" + gapID;
                }

                listCSV << HVeffHisto << '\t'
                        << ImonHisto << '\t' << ImonHisto << "_err\t"
                        << ADCHisto << '\t' << ADCHisto << "_err\t";

                //Save the effective voltages
                if(caenFile.GetListOfKeys()->Contains(HVeffHisto.c_str())){
                    TH1F* HVeff = (TH1F*)caenFile.Get(HVeffHisto.c_str());
                    float voltage = HVeff->GetMean();
                    outputCSV << voltage << '\t';
                } else {
                    float voltage = 0.;
                    outputCSV << voltage << '\t';
                }

                //Save the corresponding gap imons
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

                //Save the corresponding gap ADC currents
                if(caenFile.GetListOfKeys()->Contains(ADCHisto.c_str())){
                    TH1F* ADC = (TH1F*)caenFile.Get(ADCHisto.c_str());
                    float ADCcur = 0.;
                    float ADCcurErr = 0.;

                    if(ADC->GetEntries() > 0){
                        ADCcur = ADC->GetMean()/areagap;
                        ADCcurErr = ADC->GetRMS()/sqrt(ADC->GetEntries())/areagap;
                    }
                    outputCSV << ADCcur << '\t' << ADCcurErr << '\t';
                } else {
                    float ADCcur = 0.;
                    float ADCcurErr = 0.;
                    outputCSV << ADCcur << '\t' << ADCcurErr << '\t';
                }
            }
        }
        listCSV << '\n';
        listCSV.close();

        outputCSV << '\n';
        outputCSV.close();

        caenFile.Close();
    } else {
        MSG_INFO("[Offline-Current] File " + caenName + " could not be opened");
        MSG_INFO("[Offline-Current] Skipping current analysis");
    }
}
