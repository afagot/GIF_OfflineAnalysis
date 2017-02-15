//***************************************************************
// *    GIF OFFLINE TOOL v4
// *
// *    Program developped to extract from the raw data files
// *    the rates, currents and DIP parameters.
// *
// *    NoiseRate.cc
// *
// *    Extraction of the rates from Scan_00XXXX_HVX_DAQ.root
// *    files + monitoring of strip activities and noise
// *    homogeneity.
// *
// *    Developped by : Alexis Fagot
// *    07/03/2017
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

#include "../include/NoiseRate.h"
#include "../include/IniFile.h"
#include "../include/MsgSvc.h"
#include "../include/utils.h"

//*******************************************************************************

string GetSavePath(string baseName, string stepID){
    string path;
    path = baseName.substr(0,baseName.find_last_of("/")) + "/HV" + stepID + "/DAQ/";
    MSG_INFO("[Offline] DQM files in " + path);
    return path;
}

//*******************************************************************************

map<int,int> TDCMapping(string baseName){
    //# RPC Channel (TS000 to TS127 : T = trolleys, S = slots, up to 127 strips)
    int RPCCh;

    //# TDC Channel (M000 to M127 : M modules (from 0), 128 channels)
    int TDCCh;

    //2D Map of the TDC Channels and their corresponding RPC strips
    map<int,int> Map;

    //File that contains the path to the mapping file located
    //in the scan directory
    string mapping = baseName.substr(0,baseName.find_last_of("/")) + "/ChannelsMapping.csv";

    //Open mapping file
    ifstream mappingfile(mapping.c_str(), ios::in);
    if(mappingfile){
        while (mappingfile.good()) { //Fill the map with RPC and TDC channels
            mappingfile >> RPCCh >> TDCCh;
            if ( TDCCh != -1 ) Map[TDCCh] = RPCCh;
        }
        mappingfile.close();
    } else {
        MSG_ERROR("[Offline] Couldn't open file " + mapping);
        exit(EXIT_FAILURE);
    }

    return Map;
}

//*******************************************************************************

void GetNoiseRate(string baseName){

    string daqName = baseName + "_DAQ.root";
    string caenName = baseName + "_CAEN.root";

    //****************** CAEN ROOT FILE ******************************

    //input CAEN ROOT data file containing the values of the HV eff for
    //every HV step
    TFile caenFile(caenName.c_str());
    TH1F *HVeff[NTROLLEYS][NSLOTS];

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
        //Then link a string to the branch corresponding to the beam
        //status and get the entry
        //Convention : ON = beam trigger , OFF = Random trigger
        TTree* RunParameters = (TTree*)dataFile.Get("RunParameters");
        TString* RunType = new TString();
        RunParameters->SetBranchAddress("RunType",&RunType);
        RunParameters->GetEntry(0);

        float PeakMeanTime[NTROLLEYS][NSLOTS][NPARTITIONS] = {{{0.}}};
        float PeakSpread[NTROLLEYS][NSLOTS][NPARTITIONS] = {{{0.}}};

        if(RunType->CompareTo("efficiency") == 0) SetBeamWindow(PeakMeanTime,PeakSpread,dataTree,RPCChMap,GIFInfra);

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

        TH1I *BeamProf_H[NTROLLEYS][NSLOTS][NPARTITIONS];
        TH1I *NoiseProf_H[NTROLLEYS][NSLOTS][NPARTITIONS];
        TH1F *TimeProfile_H[NTROLLEYS][NSLOTS][NPARTITIONS];
        TH1I *HitMultiplicity_H[NTROLLEYS][NSLOTS][NPARTITIONS];
        TH1I *StripHitProf_H[NTROLLEYS][NSLOTS][NPARTITIONS];
        TH1F *StripMeanNoiseProf_H[NTROLLEYS][NSLOTS][NPARTITIONS];
        TH1F *StripActivity_H[NTROLLEYS][NSLOTS][NPARTITIONS];
        TH1F *StripHomogeneity_H[NTROLLEYS][NSLOTS][NPARTITIONS];
        TH1I *ChipHitProf_H[NTROLLEYS][NSLOTS][NPARTITIONS];
        TH1F *ChipMeanNoiseProf_H[NTROLLEYS][NSLOTS][NPARTITIONS];
        TH1F *ChipActivity_H[NTROLLEYS][NSLOTS][NPARTITIONS];
        TH1F *ChipHomogeneity_H[NTROLLEYS][NSLOTS][NPARTITIONS];

        char hisname[50];                    //ID name of the histogram
        char histitle[50];                   //Title of the histogram

        for (unsigned int t = 0; t < GIFInfra.nTrolleys; t++){
            unsigned int nSlotsTrolley = GIFInfra.Trolleys[t].nSlots;
            unsigned int trolley = CharToInt(GIFInfra.TrolleysID[t]);

            for (unsigned int s = 0; s < nSlotsTrolley; s++){
                unsigned int nPartRPC = GIFInfra.Trolleys[t].RPCs[s].nPartitions;
                unsigned int slot = CharToInt(GIFInfra.Trolleys[t].SlotsID[s]) - 1;
                unsigned int nGaps = GIFInfra.Trolleys[t].RPCs[s].nGaps;

                //Get the HVeff histogram by having the highest gap HVeff
                string HVeffHisto = "";
                float HVmax = 0.;

                for(unsigned int g=0; g<nGaps; g++){
                    string rpc = GIFInfra.Trolleys[t].RPCs[s].name;
                    string tmpHisto;

                    if(GIFInfra.Trolleys[t].RPCs[s].gaps[g] == "empty")
                        tmpHisto = "HVeff_" + rpc;
                    else
                        tmpHisto = "HVeff_" + rpc + "-" + GIFInfra.Trolleys[t].RPCs[s].gaps[g];

                    if(caenFile.GetListOfKeys()->Contains(tmpHisto.c_str())){
                        float tmpHVeff = ((TH1F*)caenFile.Get(tmpHisto.c_str()))->GetMean();
                        if(tmpHVeff > HVmax){
                            HVmax = tmpHVeff;
                            HVeffHisto = tmpHisto;
                        }
                    }
                }

                if(HVeffHisto != "")
                    HVeff[trolley][slot] = (TH1F*)caenFile.Get(HVeffHisto.c_str());

                //Get the chamber ID in terms of trolley + slot position TXSX
                string rpcID = "T"+ CharToString(GIFInfra.TrolleysID[t]) +
                        "S" + CharToString(GIFInfra.Trolleys[t].SlotsID[s]);

                for (unsigned int p = 0; p < nPartRPC; p++){
                    //Set bining
                    unsigned int nStrips = GIFInfra.Trolleys[t].RPCs[s].strips;
                    float low_s = nStrips*p + 0.5;
                    float high_s = nStrips*(p+1) + 0.5;

                    unsigned int nBinsMult = 101;
                    float lowBin = -0.5;
                    float highBin = (float)nBinsMult + lowBin;

                    //Time profile binning
                    float timeWidth = 1.;

                    if(RunType->CompareTo("efficiency") == 0)
                        timeWidth = BMTDCWINDOW;
                    else if(RunType->CompareTo("efficiency") != 0)
                        timeWidth = RDMTDCWINDOW;

                    //Initialisation of the histograms

                    //***************************************** General histograms

                    //Beam profile
                    SetTitleName(rpcID,p,hisname,histitle,"Beam_Profile","Beam profile");
                    BeamProf_H[trolley][slot][p] = new TH1I( hisname, histitle, nStrips, low_s, high_s);

                    //Noise profile
                    SetTitleName(rpcID,p,hisname,histitle,"Noise_Profile","Noise profile");
                    NoiseProf_H[trolley][slot][p] = new TH1I( hisname, histitle, nStrips, low_s, high_s);

                    //Time profile
                    SetTitleName(rpcID,p,hisname,histitle,"Time_Profile","Time profile");
                    TimeProfile_H[trolley][slot][p] = new TH1F( hisname, histitle, (int)timeWidth/20, 0., timeWidth);

                    //Hit multiplicity
                    SetTitleName(rpcID,p,hisname,histitle,"Hit_Multiplicity","Hit multiplicity");
                    HitMultiplicity_H[trolley][slot][p] = new TH1I( hisname, histitle, nBinsMult, lowBin, highBin);

                    //****************************************** Strip granularuty level histograms

                    //Hit profile
                    SetTitleName(rpcID,p,hisname,histitle,"Strip_Hit_Profile","Strip hit profile");
                    StripHitProf_H[trolley][slot][p] = new TH1I( hisname, histitle, nStrips, low_s, high_s);

                    //Mean noise rate profile
                    SetTitleName(rpcID,p,hisname,histitle,"Strip_Mean_Noise","Strip mean noise rate");
                    StripMeanNoiseProf_H[trolley][slot][p] = new TH1F( hisname, histitle, nStrips, low_s, high_s);

                    //Strip activity
                    SetTitleName(rpcID,p,hisname,histitle,"Strip_Activity","Strip activity");
                    StripActivity_H[trolley][slot][p] = new TH1F( hisname, histitle, nStrips, low_s, high_s);

                    //Noise homogeneity
                    SetTitleName(rpcID,p,hisname,histitle,"Strip_Homogeneity","Strip homogeneity");
                    StripHomogeneity_H[trolley][slot][p] = new TH1F( hisname, histitle, 1, 0, 1);

                    //****************************************** Chip granularuty level histograms

                    //Hit profile
                    SetTitleName(rpcID,p,hisname,histitle,"Chip_Hit_Profile","Chip hit profile");
                    ChipHitProf_H[trolley][slot][p] = new TH1I( hisname, histitle, nStrips/8, low_s, high_s);

                    //Mean noise rate profile
                    SetTitleName(rpcID,p,hisname,histitle,"Chip_Mean_Noise","Chip mean noise rate");
                    ChipMeanNoiseProf_H[trolley][slot][p] = new TH1F( hisname, histitle, nStrips/8, low_s, high_s);

                    //Strip activity
                    SetTitleName(rpcID,p,hisname,histitle,"Chip_Activity","Chip activity");
                    ChipActivity_H[trolley][slot][p] = new TH1F( hisname, histitle, nStrips/8, low_s, high_s);

                    //Noise homogeneity
                    SetTitleName(rpcID,p,hisname,histitle,"Chip_Homogeneity","Chip homogeneity");
                    ChipHomogeneity_H[trolley][slot][p] = new TH1F( hisname, histitle, 1, 0, 1);
                }
            }
        }

        //****************** MACRO ***************************************

        //Tabel to count the hits in every chamber partitions - used to
        //compute the noise rate
        int Multiplicity[NTROLLEYS][NSLOTS][NPARTITIONS] = { {0} };

        unsigned int nEntries = dataTree->GetEntries();

        for(unsigned int i = 0; i < nEntries; i++){
            dataTree->GetEntry(i);

            //Loop over the TDC hits
            for(int h = 0; h < data.TDCNHits; h++){
                RPCHit hit;

                //Get rid of the noise hits outside of the connected channels
                if(data.TDCCh->at(h) > 5127) continue;
                if(RPCChMap[data.TDCCh->at(h)] == 0) continue;

                //Get rid of the noise hits in the ground channels of KODEL chambers

                SetRPCHit(hit, RPCChMap[data.TDCCh->at(h)], data.TDCTS->at(h), GIFInfra);

                if(RunType->CompareTo("efficiency") == 0){
                    //First define the accepted peak time range
                    float lowlimit = PeakMeanTime[hit.Trolley][hit.Station-1][hit.Partition-1]
                            - PeakSpread[hit.Trolley][hit.Station-1][hit.Partition-1];
                    float highlimit = PeakMeanTime[hit.Trolley][hit.Station-1][hit.Partition-1]
                            + PeakSpread[hit.Trolley][hit.Station-1][hit.Partition-1];

                    bool peakrange          = (hit.TimeStamp >= lowlimit && hit.TimeStamp < highlimit);

                    //Define also the ranges used for the noise calculation
                    bool earlynoiserange    = (hit.TimeStamp >= 100. && hit.TimeStamp < 200.);
                    bool latenoiserange     = (hit.TimeStamp >= 350. && hit.TimeStamp < 550.);

                    //Fill the hits inside of the defined noise range
                    if(earlynoiserange || latenoiserange){
                        NoiseProf_H[hit.Trolley][hit.Station-1][hit.Partition-1]->Fill(hit.Strip);
                        StripActivity_H[hit.Trolley][hit.Station-1][hit.Partition-1]->Fill(hit.Strip);
                        ChipActivity_H[hit.Trolley][hit.Station-1][hit.Partition-1]->Fill(hit.Strip);
                    } else if(peakrange)
                        BeamProf_H[hit.Trolley][hit.Station-1][hit.Partition-1]->Fill(hit.Strip);
                } else if(RunType->CompareTo("efficiency") != 0){
                    //Reject the 100 first and last ns due to inhomogeneity of data
                    bool rejected = (hit.TimeStamp < 100. || hit.TimeStamp > RDMTDCWINDOW-100.);

                    if(!rejected)
                        NoiseProf_H[hit.Trolley][hit.Station-1][hit.Partition-1]->Fill(hit.Strip);
                }

                //Fill the profiles
                StripHitProf_H[hit.Trolley][hit.Station-1][hit.Partition-1]->Fill(hit.Strip);
                ChipHitProf_H[hit.Trolley][hit.Station-1][hit.Partition-1]->Fill(hit.Strip);
                TimeProfile_H[hit.Trolley][hit.Station-1][hit.Partition-1]->Fill(hit.TimeStamp);
                Multiplicity[hit.Trolley][hit.Station-1][hit.Partition-1]++;
            }

            //********** MULTIPLICITY ************************************

            for(unsigned int t=0; t<GIFInfra.nTrolleys; t++){
                unsigned int nSlotsTrolley = GIFInfra.Trolleys[t].nSlots;
                unsigned int trolley = CharToInt(GIFInfra.TrolleysID[t]);

                for(unsigned int sl=0; sl<nSlotsTrolley; sl++){
                    unsigned int nPartRPC = GIFInfra.Trolleys[t].RPCs[sl].nPartitions;
                    unsigned int slot = CharToInt(GIFInfra.Trolleys[t].SlotsID[sl]) - 1;

                    for (unsigned int p = 0; p < nPartRPC; p++){
                        HitMultiplicity_H[trolley][slot][p]->Fill(Multiplicity[trolley][slot][p]);
                        Multiplicity[trolley][slot][p] = 0;
                    }
                }
            }
        }

        //************** MEAN NOISE RATE *********************************
        //create a ROOT output file to save the histograms
        string fNameROOT = baseName + "_DAQ-Rate.root";
        TFile outputfile(fNameROOT.c_str(), "recreate");

        //output csv file
        string csvName = baseName.substr(0,baseName.find_last_of("/")) + "/Offline-Rate.csv";
        ofstream outputCSV(csvName.c_str(),ios::app);
        //Print the HV step as first column
        outputCSV << HVstep << '\t';

        //output csv file to save the list of parameters saved into the
        //Offline-Rate.csv file - it represents the header of that file
        string listName = baseName.substr(0,baseName.find_last_of("/")) + "/Offline-Rate-Header.csv";
        ofstream listCSV(listName.c_str(),ios::out);
        listCSV << "HVstep\t";

        //Loop over trolleys
        for (unsigned int t = 0; t < GIFInfra.nTrolleys; t++){
            unsigned int nSlotsTrolley = GIFInfra.Trolleys[t].nSlots;
            unsigned int trolley = CharToInt(GIFInfra.TrolleysID[t]);

            for (unsigned int sl = 0; sl < nSlotsTrolley; sl++){
                unsigned int nPartRPC = GIFInfra.Trolleys[t].RPCs[sl].nPartitions;
                unsigned int slot = CharToInt(GIFInfra.Trolleys[t].SlotsID[sl]) - 1;

                float HighVoltage = HVeff[trolley][slot]->GetMean();
                outputCSV << HighVoltage << '\t';

                //Write the header file
                listCSV << "HVeff-" << GIFInfra.Trolleys[t].RPCs[sl].name << '\t';

                //Get the total chamber rate
                //we need to now the total chamber surface (sum active areas)
                unsigned int nStripsRPC  = 0;
                float        RPCarea     = 0.;
                float        MeanRPCRate = 0.;
                float        MeanRPCSDev = 0.;

                for (unsigned int p = 0; p < nPartRPC; p++){
                    string partID = "ABCD";
                    //Write the header file
                    listCSV << "Rate-"
                            << GIFInfra.Trolleys[t].RPCs[sl].name
                            << "-" << partID[p]
                            << "\tRate-"
                            << GIFInfra.Trolleys[t].RPCs[sl].name
                            << "-" << partID[p]
                            << "_err\t";

                    //Get the mean noise on the strips and chips using the noise hit
                    //profile. Normalise the number of hits in each bin by the integrated
                    //time and the strip sruface (counts/s/cm2)
                    float normalisation = 0.;

                    //Get the strip geometry
                    float stripArea = GIFInfra.Trolleys[t].RPCs[sl].stripGeo[p];

                    if(RunType->CompareTo("efficiency") == 0)
                        normalisation = nEntries*BMNOISEWDW*1e-9*stripArea;
                    else if(RunType->CompareTo("efficiency") != 0)
                        normalisation = nEntries*RDMNOISEWDW*1e-9*stripArea;

                    unsigned int nStripsPart = GIFInfra.Trolleys[t].RPCs[sl].strips;

                    for(unsigned int st = 1; st <= nStripsPart; st++){
                        float stripRate = NoiseProf_H[trolley][slot][p]->GetBinContent(st)/normalisation;

                        StripMeanNoiseProf_H[trolley][slot][p]->Fill(p*nStripsPart+st,stripRate);

                        //The chip rate only is incremented by a rate that is
                        //normalised to the number of strip per chip
                        ChipMeanNoiseProf_H[trolley][slot][p]->Fill(p*nStripsPart+st,stripRate/NSTRIPSCHIP);
                    }

                    //Write in the output file the mean noise rate per
                    //partition and its error defined as twice the RMS
                    //over the sqrt of the number of events
                    float MeanPartRate = GetTH1Mean(StripMeanNoiseProf_H[trolley][slot][p]);
                    float MeanPartSDev = GetTH1StdDev(StripMeanNoiseProf_H[trolley][slot][p]);
                    outputCSV << MeanPartRate << '\t' << MeanPartSDev << '\t';

                    //Get the partition homogeneity defined as exp(RMS(noise)/MEAN(noise))
                    //The closer the homogeneity is to 1 the more homogeneus, the closer
                    //the homogeneity is to 0 the less homogeneous.
                    //This gives idea about noisy strips and dead strips.
                    float strip_homog = exp(-MeanPartSDev/MeanPartRate);
                    StripHomogeneity_H[trolley][slot][p]->Fill(0.,strip_homog);

                    //Same thing for the chip level - need to get the RMS at the chip level, the mean stays the same
                    float ChipStDevMean = GetTH1StdDev(ChipMeanNoiseProf_H[trolley][slot][p]);

                    float chip_homog = exp(-ChipStDevMean/MeanPartRate);
                    ChipHomogeneity_H[trolley][slot][p]->Fill(0.,chip_homog);

                    //Push the partition results into the chamber level
                    nStripsRPC  += nStripsPart;
                    RPCarea     += stripArea * nStripsPart;
                    MeanRPCRate += MeanPartRate * stripArea * nStripsPart;
                    MeanRPCSDev += MeanPartSDev * stripArea * nStripsPart;

                    //Draw and write the histograms into the output ROOT file
                    //********************************* General histograms

                    BeamProf_H[trolley][slot][p]->Write();

                    NoiseProf_H[trolley][slot][p]->Write();

                    TimeProfile_H[trolley][slot][p]->Write();

                    HitMultiplicity_H[trolley][slot][p]->Write();

                    //******************************* Strip granularity histograms

                    StripHitProf_H[trolley][slot][p]->Write();

                    StripMeanNoiseProf_H[trolley][slot][p]->Write();

                    StripActivity_H[trolley][slot][p]->Write();

                    StripHomogeneity_H[trolley][slot][p]->GetYaxis()->SetRangeUser(0.,1.);
                    StripHomogeneity_H[trolley][slot][p]->Write();

                    //******************************* Chip granularity histograms

                    ChipHitProf_H[trolley][slot][p]->Write();

                    ChipMeanNoiseProf_H[trolley][slot][p]->Write();

                    ChipActivity_H[trolley][slot][p]->Write();

                    ChipHomogeneity_H[trolley][slot][p]->GetYaxis()->SetRangeUser(0.,1.);
                    ChipHomogeneity_H[trolley][slot][p]->Write();
                }

                //Finalise the calculation of the chamber rate
                MeanRPCRate /= RPCarea;
                MeanRPCSDev /= RPCarea;

                //Write the header file
                listCSV << "Rate-"
                        << GIFInfra.Trolleys[t].RPCs[sl].name
                        << "-TOT\tRate-"
                        << GIFInfra.Trolleys[t].RPCs[sl].name
                        << "-TOT_err\t";

                //Write the output file
                outputCSV << MeanRPCRate << '\t' << MeanRPCSDev << '\t';
            }
        }
        listCSV << '\n';
        listCSV.close();

        outputCSV << '\n';
        outputCSV.close();

        outputfile.Close();
        caenFile.Close();
        dataFile.Close();
    } else {
        MSG_INFO("[Offline-Rate] File " + daqName + " could not be opened");
        MSG_INFO("[Offline-Rate] Skipping rate analysis");
    }
}
