//***************************************************************
// *    GIF++ OFFLINE TOOL v4
// *
// *    Program developped to extract from the raw data files
// *    the rates, currents and DIP parameters.
// *
// *    efficiency.cc
// *
// *    Extraction of the efficiency from Scan_00XXXX_HVX_DAQ.root
// *    files.
// *
// *    Developped by : Alexis Fagot
// *    30/03/2017
//***************************************************************

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <cmath>

#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TH1F.h"
#include "TH1I.h"
#include "TH2F.h"
#include "TProfile.h"

#include "../include/EffLevelZero.h"
#include "../include/NoiseRate.h"
#include "../include/IniFile.h"
#include "../include/MsgSvc.h"
#include "../include/utils.h"

using namespace std;

void GetEffLevelZero(string baseName){
    string daqName = baseName + "_DAQ.root";

    //****************** DAQ ROOT FILE *******************************

    //input ROOT data file containing the RAWData TTree that we'll
    //link to our RAWData structure
    TFile   dataFile(daqName.c_str());

    if(dataFile.IsOpen()){
        TTree*  dataTree = (TTree*)dataFile.Get("RAWData");

        //Then get the HVstep number from the ID histogram
        string HVstep = baseName.substr(baseName.find_last_of("_HV")+1);

        //****************** GEOMETRY ************************************

        //Get the chambers geometry and the GIF infrastructure details
        string dimpath = daqName.substr(0,daqName.find_last_of("/")) + "/Dimensions.ini";
        IniFile* Dimensions = new IniFile(dimpath.c_str());
        Dimensions->Read();

        Infrastructure GIFInfra;
        SetInfrastructure(GIFInfra,Dimensions);

        //****************** MAPPING *************************************

        map<int,int> RPCChMap = TDCMapping(daqName);

        //****************** PEAK TIME ***********************************

        //First open the RunParameters TTree from the dataFile
        //Then link a string to the branch corresponding to the
        //run type
        //Convention : efficiency = cosmic trigger , noise_reference = random trigger
        TTree* RunParameters = (TTree*)dataFile.Get("RunParameters");
        TString* RunType = new TString();
        RunParameters->SetBranchAddress("RunType",&RunType);
        RunParameters->GetEntry(0);

        if(RunType->CompareTo("efficiency") == 0) {

            float PeakMeanTime[NTROLLEYS][NSLOTS][NPARTITIONS] = {{{0.}}};
            float PeakSpread[NTROLLEYS][NSLOTS][NPARTITIONS] = {{{0.}}};

            SetBeamWindow(PeakMeanTime,PeakSpread,dataTree,RPCChMap,GIFInfra);

            //****************** LINK RAW DATA *******************************

            RAWData data;

            data.TDCCh = new vector<unsigned int>;
            data.TDCTS = new vector<float>;
            data.TDCCh->clear();
            data.TDCTS->clear();

            dataTree->SetBranchAddress("EventNumber",    &data.iEvent);
            dataTree->SetBranchAddress("number_of_hits", &data.TDCNHits);
            dataTree->SetBranchAddress("TDC_channel",    &data.TDCCh);
            dataTree->SetBranchAddress("TDC_TimeStamp",  &data.TDCTS);

            //****************** HISTOGRAMS & CANVAS *************************

            TH1I *Efficiency0_H[NTROLLEYS][NSLOTS][NPARTITIONS];

            char hisname[50];                    //ID name of the histogram
            char histitle[50];                   //Title of the histogram


            for(unsigned int tr = 0; tr < GIFInfra.nTrolleys; tr++){
                unsigned int nSlotsTrolley = GIFInfra.Trolleys[tr].nSlots;
                unsigned int trolley = CharToInt(GIFInfra.TrolleysID[tr]);

                for(unsigned int sl = 0; sl < nSlotsTrolley; sl++){
                    unsigned int nPartRPC = GIFInfra.Trolleys[tr].RPCs[sl].nPartitions;
                    unsigned int slot = CharToInt(GIFInfra.Trolleys[tr].SlotsID[sl]) - 1;

                    //Get the chamber ID in terms of slot position SX
                    string rpcID = GIFInfra.Trolleys[tr].RPCs[sl].name;

                    for (unsigned int p = 0; p < nPartRPC; p++){

                        //Initialisation of the histogram

                        //Efficiency
                        SetTitleName(rpcID,p,hisname,histitle,"L0_Efficiency","L0 efficiency");
                        Efficiency0_H[trolley][slot][p] = new TH1I( hisname, histitle, 2, -0.5, 1.5);
                        SetTH1(Efficiency0_H[trolley][slot][p],"Is efficient?","Number of events");
                    }
                }
            }

            //****************** MACRO ***************************************

            //Table to keep track of the number of in time hits.
            //Will be used later to estimated the proportion of
            //noise hits within the peak time range and correct
            //the efficiency accordingly.
            int inTimeHits[NTROLLEYS][NSLOTS][NPARTITIONS] = {{0}};
            int noiseHits[NTROLLEYS][NSLOTS][NPARTITIONS] = {{0}};

            unsigned int nEntries = dataTree->GetEntries();

            for(unsigned int i = 0; i < nEntries; i++){
                dataTree->GetEntry(i);

                //Vectors to store the hits and reconstruct clusters
                vector<RPCHit> RPCHits[NTROLLEYS][NSLOTS][NPARTITIONS];

                //Loop over TDC hits
                for(int h = 0; h < data.TDCNHits; h++){
                    RPCHit hit;

                    //Get rid of the noise hits outside of the connected channels
                    if(RPCChMap[data.TDCCh->at(h)] == 0) continue;

                    SetRPCHit(hit, RPCChMap[data.TDCCh->at(h)], data.TDCTS->at(h), GIFInfra);

                    //First define the accepted peak time range
                    float lowlimit = PeakMeanTime[hit.Trolley][hit.Station-1][hit.Partition-1]
                            - PeakSpread[hit.Trolley][hit.Station-1][hit.Partition-1];
                    float highlimit = PeakMeanTime[hit.Trolley][hit.Station-1][hit.Partition-1]
                            + PeakSpread[hit.Trolley][hit.Station-1][hit.Partition-1];

                    bool peakrange = (hit.TimeStamp >= lowlimit && hit.TimeStamp < highlimit);

                    if(peakrange){
                        RPCHits[hit.Trolley][hit.Station-1][hit.Partition-1].push_back(hit);
                        inTimeHits[hit.Trolley][hit.Station-1][hit.Partition-1]++;
                    } else if(hit.TimeStamp >= TIMEREJECT)
                        noiseHits[hit.Trolley][hit.Station-1][hit.Partition-1]++;
                }

                //Get effiency and cluster size
                for(unsigned int tr = 0; tr < GIFInfra.nTrolleys; tr++){
                    unsigned int nSlotsTrolley = GIFInfra.Trolleys[tr].nSlots;
                    unsigned int trolley = CharToInt(GIFInfra.TrolleysID[tr]);

                    for(unsigned int sl=0; sl<nSlotsTrolley; sl++){
                        unsigned int nPartRPC = GIFInfra.Trolleys[tr].RPCs[sl].nPartitions;
                        unsigned int slot = CharToInt(GIFInfra.Trolleys[tr].SlotsID[sl]) - 1;

                        for (unsigned int p = 0; p < nPartRPC; p++){
                            if(RPCHits[trolley][slot][p].size() > 0)
                                Efficiency0_H[trolley][slot][p]->Fill(1);
                            else
                                Efficiency0_H[trolley][slot][p]->Fill(0);
                        }
                    }
                }
            }

            //************** OUTPUT FILES *********************************
            //create a ROOT output file to save the histograms
            string fNameROOT = baseName + "_DAQ-L0_EffCl.root";
            TFile outputfile(fNameROOT.c_str(), "recreate");

            //output csv file to save the list of parameters saved into the
            //Offline-Rate.csv file - it represents the header of that file
            string listName = baseName.substr(0,baseName.find_last_of("/")) + "/Offline-L0-EffCl-Header.csv";
            ofstream listCSV(listName.c_str(),ios::out);
            listCSV << "HVstep\t";

            //output csv file
            string csvName = baseName.substr(0,baseName.find_last_of("/")) + "/Offline-L0-EffCl.csv";
            ofstream outputCSV(csvName.c_str(),ios::app);
            //Print the HV step as first column
            outputCSV << HVstep << '\t';

            //Write histograms into ROOT file
            for(unsigned int tr = 0; tr < GIFInfra.nTrolleys; tr++){
                unsigned int nSlotsTrolley = GIFInfra.Trolleys[tr].nSlots;
                unsigned int trolley = CharToInt(GIFInfra.TrolleysID[tr]);

                for(unsigned int sl=0; sl<nSlotsTrolley; sl++){
                    unsigned int nPartRPC = GIFInfra.Trolleys[tr].RPCs[sl].nPartitions;
                    unsigned int slot = CharToInt(GIFInfra.Trolleys[tr].SlotsID[sl]) - 1;

                    for (unsigned int p = 0; p < nPartRPC; p++){
                        string partID = "ABCD";
                        string partName = GIFInfra.Trolleys[tr].RPCs[sl].name + "-" + partID[p];
                        //Write the header file
                        listCSV << "Peak-" << partName << '\t'
                                << "Peak-" << partName << "_RMS\t"
                                << "Noise-" << partName << "\t"
                                << "DataRatio-" << partName << "\t"
                                << "Eff-" << partName << '\t'
                                << "Eff-" << partName << "_Err\t";

                        //For each cases, evaluate the proportion of noise
                        //with respect to the actual muon data. The efficiency
                        //will then be corrected using this factor to "substract"
                        //the fake efficiency caused by the noise
                        float meanNoiseHitPerns = (float)noiseHits[trolley][slot][p]/(BMTDCWINDOW-TIMEREJECT-2*PeakSpread[trolley][slot][p]);
                        float integralNoise = 2*PeakSpread[trolley][slot][p]*meanNoiseHitPerns;
                        float integralPeak = (float)inTimeHits[trolley][slot][p];

                        float DataNoiseRatio = 1.;
                        if(integralNoise != 0.) DataNoiseRatio = (integralPeak-integralNoise)/integralPeak;
                        if(DataNoiseRatio < 0.) DataNoiseRatio = 0.;

                        //Get efficiency, cluster size and multiplicity
                        //and evaluate the streamer probability (cls > 5)
                        float peak = PeakMeanTime[trolley][slot][p];
                        float peakRMS = PeakSpread[trolley][slot][p];
                        float noise = meanNoiseHitPerns*TIMEBIN;
                        float eff = Efficiency0_H[trolley][slot][p]->GetMean()*DataNoiseRatio;
                        float effErr = sqrt(eff*(1.-eff)/nEntries);

                        //Write in the output CSV file
                        outputCSV << peak << '\t' << peakRMS << '\t'
                                  << noise << '\t' << DataNoiseRatio << '\t'
                                  << eff << '\t' << effErr << '\t';

                        Efficiency0_H[trolley][slot][p]->Write();
                    }
                }
            }
            listCSV << '\n';
            listCSV.close();

            outputCSV << '\n';
            outputCSV.close();

            outputfile.Close();
        }
        dataFile.Close();
    } else {
        MSG_INFO("[Offline-L0-Eff] File " + daqName + " could not be opened");
        MSG_INFO("[Offline-L0-Eff] Skipping L0 analysis");
    }
}
