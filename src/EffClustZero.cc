//***************************************************************
// *    904 OFFLINE TOOL v4
// *
// *    Program developped to extract from the raw data files
// *    the rates, currents and DIP parameters.
// *
// *    efficiency.cc
// *
// *    Extraction of the efficiency from Scan_00XXXX_HVX_DAQ.root
// *    files + monitoring of cosmics cluster size and multiplicity.
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

#include "../include/EffClustZero.h"
#include "../include/NoiseRate.h"
#include "../include/IniFile.h"
#include "../include/MsgSvc.h"
#include "../include/utils.h"

using namespace std;

void GetEffClustZero(string baseName){
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

            float PeakMeanTime[NSLOTS][NPARTITIONS] = {{0.}};
            float PeakSpread[NSLOTS][NPARTITIONS] = {{0.}};

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

            TH1I *Efficiency0_H[NSLOTS][NPARTITIONS];
            TH1I *ClusterSize0_H[NSLOTS][NPARTITIONS];
            TH1I *ClusterMult0_H[NSLOTS][NPARTITIONS];

            char hisname[50];                    //ID name of the histogram
            char histitle[50];                   //Title of the histogram

            unsigned int nSlots = GIFInfra.nSlots;

            for (unsigned int s = 0; s < nSlots; s++){
                unsigned int nPartRPC = GIFInfra.RPCs[s].nPartitions;
                unsigned int slot = CharToInt(GIFInfra.SlotsID[s]) - 1;

                //Get the chamber ID in terms of slot position SX
                string rpcID = GIFInfra.RPCs[s].name;

                for (unsigned int p = 0; p < nPartRPC; p++){
                    //Set bining
                    unsigned int nStrips = GIFInfra.RPCs[s].strips;

                    //Initialisation of the histograms

                    //Efficiency
                    SetTitleName(rpcID,p,hisname,histitle,"L0_Efficiency","L0 efficiency");
                    Efficiency0_H[slot][p] = new TH1I( hisname, histitle, 2, -0.5, 1.5);

                    //Cluster size
                    SetTitleName(rpcID,p,hisname,histitle,"L0_Cluster_Size","L0 cluster size");
                    ClusterSize0_H[slot][p] = new TH1I( hisname, histitle, nStrips, 0.5, nStrips+0.5);

                    //Cluster multiplicity
                    SetTitleName(rpcID,p,hisname,histitle,"L0_Cluster_Mult","L0 cluster multiplicity");
                    ClusterMult0_H[slot][p] = new TH1I( hisname, histitle, nStrips, 0.5, nStrips+0.5);
                }
            }

            //****************** MACRO ***************************************

            unsigned int nEntries = dataTree->GetEntries();

            for(unsigned int i = 0; i < nEntries; i++){
                dataTree->GetEntry(i);

                //Vectors to store the hits and reconstruct clusters
                vector<RPCHit> RPCHits[NSLOTS][NPARTITIONS];
                vector<RPCCluster> RPCClusters[NSLOTS][NPARTITIONS];

                //Loop over TDC hits
                for(int h = 0; h < data.TDCNHits; h++){
                    RPCHit hit;

                    //Get rid of the noise hits outside of the connected channels
                    if(data.TDCCh->at(h) < 3000 || data.TDCCh->at(h) > 3127) continue;
                    if(RPCChMap[data.TDCCh->at(h)] == 0) continue;

                    SetRPCHit(hit, RPCChMap[data.TDCCh->at(h)], data.TDCTS->at(h), GIFInfra);

                    //First define the accepted peak time range
                    float lowlimit = PeakMeanTime[hit.Station-1][hit.Partition-1]
                            - PeakSpread[hit.Station-1][hit.Partition-1];
                    float highlimit = PeakMeanTime[hit.Station-1][hit.Partition-1]
                            + PeakSpread[hit.Station-1][hit.Partition-1];

                    bool peakrange = (hit.TimeStamp >= lowlimit && hit.TimeStamp < highlimit);

                    if(peakrange)
                        RPCHits[hit.Station-1][hit.Partition-1].push_back(hit);
                }

                //Get effiency and cluster size
                for(unsigned int sl=0; sl<nSlots; sl++){
                    unsigned int nPartRPC = GIFInfra.RPCs[sl].nPartitions;
                    unsigned int slot = CharToInt(GIFInfra.SlotsID[sl]) - 1;

                    for (unsigned int p = 0; p < nPartRPC; p++){
                        if(RPCHits[sl][p].size() > 0){
                            sort(RPCHits[sl][p].begin(),RPCHits[sl][p].end(),SortStrips);

                            RPCCluster tmpCluster;
                            tmpCluster.Station = slot;
                            tmpCluster.Partition = p;

                            //Default previous strip value
                            unsigned int previousstrip = 0;

                            //Loop over the hits
                            for(vector<RPCHit>::iterator it = RPCHits[sl][p].begin(); it != RPCHits[sl][p].end(); it++){
                                //If a hit is present in the next strip
                                if((*it).Strip == (previousstrip+1)) tmpCluster.Size++;

                                //Initialisation or reinitialisation of the tempcluster condition.
                                //In case of a reinitialisation (end of a cluster before the end
                                //of the hit list), save the cluster
                                else{
                                    //if you start a new cluster, save size and push to list
                                    if(previousstrip != 0){
                                        RPCClusters[sl][p].push_back(tmpCluster);
                                        ClusterSize0_H[sl][p]->Fill(tmpCluster.Size);
                                    }
                                    tmpCluster.Size = 1;
                                    tmpCluster.FirstStrip = (*it).Strip;
                                }
                                tmpCluster.LastStrip = (*it).Strip;
                                tmpCluster.Center = .5*(tmpCluster.LastStrip + tmpCluster.FirstStrip);
                                previousstrip = tmpCluster.LastStrip;
                            }

                            //Save the last cluster if any
                            if(previousstrip != 0){
                                RPCClusters[sl][p].push_back(tmpCluster);
                                ClusterSize0_H[sl][p]->Fill(tmpCluster.Size);
                            }

                            //Save multiplicity
                            if(RPCClusters[sl][p].size() > 0){
                                ClusterMult0_H[sl][p]->Fill(RPCClusters[sl][p].size());
                                Efficiency0_H[sl][p]->Fill(1);
                            } else
                                Efficiency0_H[sl][p]->Fill(0);
                        }
                    }
                }
            }

            //************** OUTPUT FILES *********************************
            //create a ROOT output file to save the histograms
            string fNameROOT = baseName + "_DAQ-L0_EffCl.root";
            TFile outputfile(fNameROOT.c_str(), "recreate");

            //Write histograms into ROOT file
            for (unsigned int s = 0; s < nSlots; s++){
                unsigned int nPartRPC = GIFInfra.RPCs[s].nPartitions;
                unsigned int slot = CharToInt(GIFInfra.SlotsID[s]) - 1;

                for (unsigned int p = 0; p < nPartRPC; p++){
                    Efficiency0_H[slot][p]->Write();
                    ClusterSize0_H[slot][p]->Write();
                    ClusterMult0_H[slot][p]->Write();
                }
            }
            outputfile.Close();
        }
        dataFile.Close();
    } else {
        MSG_INFO("[Offline-Efficiency] File " + daqName + " could not be opened");
        MSG_INFO("[Offline-Efficiency] Skipping efficiency analysis");
    }
}
