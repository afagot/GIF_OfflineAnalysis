//***************************************************************
// *    GIF OFFLINE TOOL v6
// *
// *    Program developped to extract from the raw data files
// *    the rates, currents and DIP parameters.
// *
// *    OfflineAnalysis.cc
// *
// *    Extraction of the data from Scan_00XXXX_HVX_DAQ.root
// *    files :
// *    + Noise or gamma rate estimation
// *    + Channel activity monitoring
// *    + First rude estimation of efficiency (L0)
// *    + Noise or gamma cluster size estimation
// *
// *    Developped by : Alexis Fagot & Salvador Carillo
// *    07/06/2017
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
#include "TMath.h"
#include "TF1.h"
#include "TLatex.h"

#include "../include/OfflineAnalysis.h"
#include "../include/IniFile.h"
#include "../include/MsgSvc.h"
#include "../include/Mapping.h"
#include "../include/Infrastructure.h"
#include "../include/Cluster.h"
#include "../include/RPCHit.h"
#include "../include/types.h"
#include "../include/utils.h"

using namespace std;

//*******************************************************************************

void OfflineAnalysis(string baseName){

    string daqName = baseName + "_DAQ.root";

    //****************** DAQ ROOT FILE *******************************

    //input ROOT data file containing the RAWData TTree that we'll
    //link to our RAWData structure
    TFile   dataFile(daqName.c_str());

    if(dataFile.IsOpen()){
        TTree*  dataTree = (TTree*)dataFile.Get("RAWData");

        //Check if we are about to analyse an old format file or a new one
        //The newest files contain an extra branch called "Quality_flag"
        //that helps on rejecting any corrupted data.
        bool isNewFormat = dataTree->GetListOfBranches()->Contains("Quality_flag");

        //Then get the HVstep number from the ID histogram
        string HVstep = baseName.substr(baseName.find_last_of("_HV")+1);

        //****************** GEOMETRY ************************************

        //Get the chambers geometry and the GIF infrastructure details
        string dimpath = daqName.substr(0,daqName.find_last_of("/")) + __dimension;
        IniFile* Dimensions = new IniFile(dimpath);
        Dimensions->Read();

        Infrastructure* GIFInfra = new Infrastructure(Dimensions);

        //****************** MAPPING *************************************

        //Get the channels mapping as well as the mask
        string mappath = daqName.substr(0,daqName.find_last_of("/")) + __mapping;
        Mapping* RPCChMap = new Mapping(mappath);
        RPCChMap->Read();

        //****************** PEAK TIME ***********************************

        //First open the RunParameters TTree from the dataFile
        //Then link a string to the branch corresponding to the beam
        //status and get the entry
        //Convention : ON = beam trigger , OFF = Random trigger
        TTree* RunParameters = (TTree*)dataFile.Get("RunParameters");
        TString* RunType = new TString();
        RunParameters->SetBranchAddress("RunType",&RunType);
        RunParameters->GetEntry(0);

        muonPeak PeakTime = {{0.}};
        muonPeak PeakWidth = {{0.}};

        if(IsEfficiencyRun(RunType)){
            MSG_INFO("[Analysis] Run is efficiency type - steting beam window");
            SetBeamWindow(PeakTime,PeakWidth,dataTree,RPCChMap,GIFInfra);
        } else {
            MSG_INFO("[Analysis] Run is test, rate or noise type - using full window for rate calculation");
        }

        //****************** LINK RAW DATA *******************************

        RAWData data;

        data.TDCCh = new vector<Uint>;
        data.TDCTS = new vector<float>;
        data.TDCCh->clear();
        data.TDCTS->clear();

        dataTree->SetBranchAddress("EventNumber",    &data.iEvent);
        dataTree->SetBranchAddress("number_of_hits", &data.TDCNHits);
        dataTree->SetBranchAddress("TDC_channel",    &data.TDCCh);
        dataTree->SetBranchAddress("TDC_TimeStamp",  &data.TDCTS);

        if(isNewFormat){
            MSG_INFO("[Analysis] Run uses Quality flag");
            dataTree->SetBranchAddress("Quality_flag", &data.QFlag);
        }

        //****************** HISTOGRAMS & CANVAS *************************

        GIFH1Array TimeProfile_H;
        GIFH1Array HitProfile_H;
        GIFH1Array HitMultiplicity_H;

        GIFH1Array StripNoiseProfile_H;
        GIFH1Array StripActivity_H;
        GIFH1Array StripHomogeneity_H;
        GIFH1Array MaskNoiseProfile_H;
        GIFH1Array MaskActivity_H;
        GIFH1Array NoiseCSize_H;
        GIFH1Array NoiseCMult_H;

        GIFH1Array ChipMeanNoiseProf_H;
        GIFH1Array ChipActivity_H;
        GIFH1Array ChipHomogeneity_H;

        GIFH1Array BeamProfile_H;
        GIFH1Array Efficiency0_H;
        GIFH1Array MuonCSize_H;
        GIFH1Array MuonCMult_H;

        char hisname[50];  //ID name of the histogram
        char histitle[50]; //Title of the histogram

        //Set a table to get the ranges of different multiplicity
        //histograms in an almost dynamical way (range is adapted
        //every time the multiplicity value goes beyond the actual
        //range). This variable will also be used to later know the
        //fitting range of multiplicity histograms.
        GIFnBinsMult nBinsMult;

        for (Uint sl = 0; sl < GIFInfra->GetNSlots(); sl++){
            Uint S = GIFInfra->GetSlotID(sl) - 1;

            //Get the chamber ID name
            string rpcID = GIFInfra->GetName(sl);

            for (Uint p = 0; p < GIFInfra->GetNPartitions(sl); p++){
                //Set bining
                Uint nStrips = GIFInfra->GetNStrips(sl);
                float low_s = nStrips*p + 0.5;
                float high_s = nStrips*(p+1) + 0.5;

                //Initialise the number of bins to 10 for multiplicity histograms
                nBinsMult.rpc[S][p] = 10;
                float lowBin = -0.5;
                float highBin = (float)nBinsMult.rpc[S][p] + lowBin;

                //Time profile binning
                float timeWidth = 1.;

                if(IsEfficiencyRun(RunType))
                    timeWidth = BMTDCWINDOW;
                else
                    timeWidth = RDMTDCWINDOW;

                //Initialisation of the histograms

                //****************************************** General histograms

                //Time profile
                SetTitleName(rpcID,p,hisname,histitle,"Time_Profile","Time profile");
                TimeProfile_H.rpc[S][p] = new TH1F(hisname, histitle, (int)timeWidth/TIMEBIN, 0., timeWidth);
                SetTH1(TimeProfile_H.rpc[S][p],"Time (ns)","Number of hits");

                //Hit profile
                SetTitleName(rpcID,p,hisname,histitle,"Hit_Profile","Hit profile");
                HitProfile_H.rpc[S][p] = new TH1I(hisname, histitle, nStrips, low_s, high_s);
                SetTH1(HitProfile_H.rpc[S][p],"Strip","Number of events");

                //Hit multiplicity
                SetTitleName(rpcID,p,hisname,histitle,"Hit_Multiplicity","Hit multiplicity");
                HitMultiplicity_H.rpc[S][p] = new TH1I(hisname, histitle, nBinsMult.rpc[S][p], lowBin, highBin);
                SetTH1(HitMultiplicity_H.rpc[S][p],"Multiplicity","Number of events");

                //****************************************** Strip granularuty level histograms

                //Mean noise/gamma rate profile
                SetTitleName(rpcID,p,hisname,histitle,"Strip_Mean_Noise","Strip mean noise rate");
                StripNoiseProfile_H.rpc[S][p] = new TH1F(hisname, histitle, nStrips, low_s, high_s);
                SetTH1(StripNoiseProfile_H.rpc[S][p],"Strip","Rate (Hz/cm^{2})");

                //Strip activity
                SetTitleName(rpcID,p,hisname,histitle,"Strip_Activity","Strip activity");
                StripActivity_H.rpc[S][p] = new TH1F(hisname, histitle, nStrips, low_s, high_s);
                SetTH1(StripActivity_H.rpc[S][p],"Strip","Activity (normalized strip profil)");

                //Noise/gamma homogeneity
                SetTitleName(rpcID,p,hisname,histitle,"Strip_Homogeneity","Strip homogeneity");
                StripHomogeneity_H.rpc[S][p] = new TH1F(hisname, histitle, 1, 0, 1);
                StripHomogeneity_H.rpc[S][p]->SetOption("TEXT");
                SetTH1(StripHomogeneity_H.rpc[S][p],"","Homogeneity");

                //Masked strip mean noise/gamma rate profile
                SetTitleName(rpcID,p,hisname,histitle,"mask_Strip_Mean_Noise","Masked strip mean noise rate");
                MaskNoiseProfile_H.rpc[S][p] = new TH1F(hisname, histitle, nStrips, low_s, high_s);
                SetTH1(MaskNoiseProfile_H.rpc[S][p],"Strip","Rate (Hz/cm^{2})");

                //Masked strip activity
                SetTitleName(rpcID,p,hisname,histitle,"mask_Strip_Activity","Masked strip activity");
                MaskActivity_H.rpc[S][p] = new TH1F(hisname, histitle, nStrips, low_s, high_s);
                SetTH1(MaskActivity_H.rpc[S][p],"Strip","Activity (normalized strip profil)");

                //Noise/gamma cluster size
                SetTitleName(rpcID,p,hisname,histitle,"NoiseCSize_H","Noise/gamma cluster size");
                NoiseCSize_H.rpc[S][p] = new TH1I(hisname, histitle, nStrips, 0.5, nStrips+0.5);
                SetTH1(NoiseCSize_H.rpc[S][p],"Cluster size","Number of events");

                //Noise/gamma cluster multiplicity
                SetTitleName(rpcID,p,hisname,histitle,"NoiseCMult_H","Noise/gamma cluster multiplicity");
                NoiseCMult_H.rpc[S][p] = new TH1I(hisname, histitle,  nBinsMult.rpc[S][p], lowBin, highBin);
                SetTH1(NoiseCMult_H.rpc[S][p],"Cluster multiplicity","Number of events");

                //****************************************** Chip granularuty level histograms

                //Mean noise rate profile
                SetTitleName(rpcID,p,hisname,histitle,"Chip_Mean_Noise","Chip mean noise rate");
                ChipMeanNoiseProf_H.rpc[S][p] = new TH1F(hisname, histitle, nStrips/8, low_s, high_s);
                SetTH1(ChipMeanNoiseProf_H.rpc[S][p],"Chip","Rate (Hz/cm^{2})");

                //Strip activity
                SetTitleName(rpcID,p,hisname,histitle,"Chip_Activity","Chip activity");
                ChipActivity_H.rpc[S][p] = new TH1F(hisname, histitle, nStrips/8, low_s, high_s);
                SetTH1(ChipActivity_H.rpc[S][p],"Chip","Activity (normalized chip profil)");

                //Noise homogeneity
                SetTitleName(rpcID,p,hisname,histitle,"Chip_Homogeneity","Chip homogeneity");
                ChipHomogeneity_H.rpc[S][p] = new TH1F(hisname, histitle, 1, 0, 1);
                ChipHomogeneity_H.rpc[S][p]->SetOption("TEXT");
                SetTH1(ChipHomogeneity_H.rpc[S][p],"","Homogeneity");

                //****************************************** Muon histogram

                //Beam profile
                SetTitleName(rpcID,p,hisname,histitle,"Beam_Profile","Beam profile");
                BeamProfile_H.rpc[S][p] = new TH1I(hisname, histitle, nStrips, low_s, high_s);
                SetTH1(BeamProfile_H.rpc[S][p],"Strip","Number of hits");

                //Efficiency
                SetTitleName(rpcID,p,hisname,histitle,"L0_Efficiency","L0 efficiency");
                Efficiency0_H.rpc[S][p] = new TH1I(hisname, histitle, 2, -0.5, 1.5);
                SetTH1(Efficiency0_H.rpc[S][p],"Is efficient?","Number of events");

                //Muon cluster Size
                SetTitleName(rpcID,p,hisname,histitle,"MuonCSize_H","Muon cluster size");
                MuonCSize_H.rpc[S][p] = new TH1I(hisname, histitle, nStrips, 0.5, nStrips+0.5);
                SetTH1(MuonCSize_H.rpc[S][p],"Cluster size","Number of events");

                //Muon cluster multiplicity
                SetTitleName(rpcID,p,hisname,histitle,"MuonCMult_H","Muon cluster multiplicity");
                MuonCMult_H.rpc[S][p] = new TH1I(hisname, histitle, nBinsMult.rpc[S][p], lowBin, highBin);
                SetTH1(MuonCMult_H.rpc[S][p],"Cluster multiplicity","Number of events");
            }
        }

        //****************** MACRO ***************************************

        //Tabel to count the hits in every chamber partitions - used to
        //compute the noise rate
        GIFintArray Multiplicity = {{0}};

        Uint nEntries = dataTree->GetEntries();

        MSG_INFO("[Analysis] Starting loop over entries...");
        for(Uint i = 0; i < nEntries; i++){

            //********** LOOP THROUGH HIT LIST ***************************

            dataTree->GetEntry(i);

            //Vectors to store the hits and reconstruct clusters:
            //for muons
            GIFHitList MuonHitList;
            //and for noise/gammas
            GIFHitList NoiseHitList;

            //Get quality flag in case of new format file
            //and discard events with corrupted data.
            if(isNewFormat && IsCorruptedEvent(data.QFlag)) continue;

            //Loop over the TDC hits
            for(int h = 0; h < data.TDCCh->size(); h++){
                Uint tdcchannel = data.TDCCh->at(h);
                Uint rpcchannel = RPCChMap->GetLink(tdcchannel);
                float timestamp = data.TDCTS->at(h);

                //Get rid of the noise hits outside of the connected channels
                if(tdcchannel > MAXTDCCHANNEL) continue;

                //Get rid of the hits in channels not considered in the mapping
                if(rpcchannel == NORPCCHANNEL) continue;

                RPCHit hit(rpcchannel, timestamp, GIFInfra);
                Uint S = hit.GetStation()-1;
                Uint P = hit.GetPartition()-1;

                //Fill the profiles
                TimeProfile_H.rpc[S][P]->Fill(hit.GetTime());
                HitProfile_H.rpc[S][P]->Fill(hit.GetStrip());

                //Reject the 100 first ns due to inhomogeneity of data
                if(hit.GetTime() >= TIMEREJECT){
                    Multiplicity.rpc[S][P]++;

                    if(IsEfficiencyRun(RunType)){
                        //First define the accepted peak time range
                        float lowlimit = PeakTime.rpc[S][P] - PeakWidth.rpc[S][P];
                        float highlimit = PeakTime.rpc[S][P] + PeakWidth.rpc[S][P];

                        bool peakrange = (hit.GetTime() >= lowlimit && hit.GetTime() < highlimit);

                        //Fill the hits inside of the defined noise range
                        if(peakrange){
                            BeamProfile_H.rpc[S][P]->Fill(hit.GetStrip());
                            MuonHitList.rpc[S][P].push_back(hit);
                        } else {
                            StripNoiseProfile_H.rpc[S][P]->Fill(hit.GetStrip());
                            NoiseHitList.rpc[S][P].push_back(hit);
                        }
                    } else {
                        StripNoiseProfile_H.rpc[S][P]->Fill(hit.GetStrip());
                        NoiseHitList.rpc[S][P].push_back(hit);
                    }
                }
            }

            //********** MULTIPLICITY AND CLUSTERS ***********************

            for(Uint sl = 0; sl < GIFInfra->GetNSlots(); sl++){
                Uint S = GIFInfra->GetSlotID(sl) - 1;
                string rpcID = GIFInfra->GetName(sl);

                for (Uint p = 0; p < GIFInfra->GetNPartitions(sl); p++){
                    //In case the value of the multiplicity is beyond the actual
                    //range, create a new histo with a wider range to store the data.
                    //Do this work for all 3 multiplicity histograms. To make sure to
                    //avoid repeating this operation too often, the range is chosen to
                    //be the value that exceeds the range + 10.
                    if(Multiplicity.rpc[S][p] > nBinsMult.rpc[S][p]){
                        nBinsMult.rpc[S][p] = Multiplicity.rpc[S][p] + 10;

                        //Hit multiplicity
                        TList *listHM = new TList;
                        listHM->Add(HitMultiplicity_H.rpc[S][p]);

                        TH1* newHitMultiplicity_H = new TH1I("", "", nBinsMult.rpc[S][p], -0.5, nBinsMult.rpc[S][p]-0.5);
                        newHitMultiplicity_H->Merge(listHM);

                        delete HitMultiplicity_H.rpc[S][p];
                        delete listHM;
                        HitMultiplicity_H.rpc[S][p] = newHitMultiplicity_H;

                        SetTitleName(rpcID,p,hisname,histitle,"Hit_Multiplicity","Hit Multiplicity");
                        HitMultiplicity_H.rpc[S][p]->SetNameTitle(hisname,histitle);
                        SetTH1(HitMultiplicity_H.rpc[S][p],"Multiplicity","Number of events");

                        //Noise/gamma cluster multiplicity
                        TList *listNCM = new TList;
                        listNCM->Add(NoiseCMult_H.rpc[S][p]);

                        TH1* newNoiseCMult_H = new TH1I("", "", nBinsMult.rpc[S][p], -0.5, nBinsMult.rpc[S][p]-0.5);
                        newNoiseCMult_H->Merge(listNCM);

                        delete NoiseCMult_H.rpc[S][p];
                        delete listNCM;
                        NoiseCMult_H.rpc[S][p] = newNoiseCMult_H;

                        SetTitleName(rpcID,p,hisname,histitle,"NoiseCMult_H","Noise/gamma cluster multiplicity");
                        NoiseCMult_H.rpc[S][p]->SetNameTitle(hisname,histitle);
                        SetTH1(NoiseCMult_H.rpc[S][p],"Cluster multiplicity","Number of events");

                        //Muon cluster multiplicity if effiency run
                        if(IsEfficiencyRun(RunType)){
                            TList *listMCM = new TList;
                            listMCM->Add(MuonCMult_H.rpc[S][p]);

                            TH1* newMuonCMult_H = new TH1I("", "", nBinsMult.rpc[S][p], -0.5, nBinsMult.rpc[S][p]-0.5);
                            newMuonCMult_H->Merge(listMCM);

                            delete MuonCMult_H.rpc[S][p];
                            delete listMCM;
                            MuonCMult_H.rpc[S][p] = newMuonCMult_H;

                            SetTitleName(rpcID,p,hisname,histitle,"MuonCMult_H","Muon cluster multiplicity");
                            MuonCMult_H.rpc[S][p]->SetNameTitle(hisname,histitle);
                            SetTH1(MuonCMult_H.rpc[S][p],"Cluster multiplicity","Number of events");
                        }
                    }

                    //Clusterize noise/gamma data
                    sort(NoiseHitList.rpc[S][p].begin(),NoiseHitList.rpc[S][p].end(),SortHitbyTime);
                    Clusterization(NoiseHitList.rpc[S][p],NoiseCSize_H.rpc[S][p],NoiseCMult_H.rpc[S][p]);

                    //Clusterize muon data and fill efficiency if efficiency run
                    if(IsEfficiencyRun(RunType)){
                        sort(MuonHitList.rpc[S][p].begin(),MuonHitList.rpc[S][p].end(),SortHitbyTime);
                        Clusterization(MuonHitList.rpc[S][p],MuonCSize_H.rpc[S][p],MuonCMult_H.rpc[S][p]);

                        if(MuonHitList.rpc[S][p].size() > 0)
                            Efficiency0_H.rpc[S][p]->Fill(DETECTED);
                        else
                            Efficiency0_H.rpc[S][p]->Fill(MISSED);
                    }

                    //Save and reinitialise the hit multiplicity
                    HitMultiplicity_H.rpc[S][p]->Fill(Multiplicity.rpc[S][p]);
                    Multiplicity.rpc[S][p] = 0;
                }
            }
        }

        //************** OUTPUT FILES ***********************************

        //create a ROOT output file to save the histograms
        string fNameROOT = baseName + "_Offline.root";
        TFile outputfile(fNameROOT.c_str(), "recreate");

        //********************************* Rate
        //output csv file to save the list of parameters saved into the
        //Offline-Rate.csv file - it represents the header of that file
        string headNameRate = baseName.substr(0,baseName.find_last_of("/")) + "/Offline-Rate-Header.csv";
        ofstream headRateCSV(headNameRate.c_str(),ios::out);
        headRateCSV << "HVstep\t";

        //output Rate csv file
        string csvNameRate = baseName.substr(0,baseName.find_last_of("/")) + "/Offline-Rate.csv";
        ofstream outputRateCSV(csvNameRate.c_str(),ios::app);
        //Print the HV step as first column
        outputRateCSV << HVstep << '\t';

        //********************************* Corrupted data
        //output csv file to save the percentage of corrupted data
        //Offline-Corrupted-Header.csv
        string headNameCorr = baseName.substr(0,baseName.find_last_of("/")) + "/Offline-Corrupted-Header.csv";
        ofstream headCorrCSV(headNameCorr.c_str(),ios::out);
        headCorrCSV << "HVstep\t";

        //Offline-Corrupted.csv
        string csvNameCorr = baseName.substr(0,baseName.find_last_of("/")) + "/Offline-Corrupted.csv";
        ofstream outputCorrCSV(csvNameCorr.c_str(),ios::app);
        //Print the HV step as first column
        outputCorrCSV << HVstep << '\t';

        //********************************* Efficiency, muon cluster
        //output csv file to save the list of parameters saved into the
        //Offline-L0-EffCl.csv file - it represents the header of that file
        string headNameEff = baseName.substr(0,baseName.find_last_of("/")) + "/Offline-L0-EffCl-Header.csv";
        ofstream headEffCSV(headNameEff.c_str(),ios::out);
        headEffCSV << "HVstep\t";

        //output csv file
        string csvNameEff = baseName.substr(0,baseName.find_last_of("/")) + "/Offline-L0-EffCl.csv";
        ofstream outputEffCSV(csvNameEff.c_str(),ios::app);
        //Print the HV step as first column
        outputEffCSV << HVstep << '\t';

        //************** DATA ANALYSIS **********************************

        MSG_INFO("[Analysis] Loop over entries done - entering analysis loop");
        //Loop over slots
        for (Uint sl = 0; sl < GIFInfra->GetNSlots(); sl++){
            Uint S = GIFInfra->GetSlotID(sl) - 1;

            //Get the total chamber rate
            //we need to now the total chamber surface (sum active areas)
            Uint  nStripsPart   = GIFInfra->GetNStrips(sl);
            float RPCarea       = 0.;
            float MeanNoiseRate = 0.;
            float ClusterRate   = 0.;
            float ClusterSDev   = 0.;

            for (Uint p = 0; p < GIFInfra->GetNPartitions(sl); p++){
                string partID = "ABCD";
                string partName = GIFInfra->GetName(sl) + "-" + partID[p];

                //**************** CORRUPTED DATA ESTIMATION **********************************

                //In case of old format files (no quality flag), it is need to estimate
                //the amount of corrupted data via a fit as the corrupted data will
                //always fill events with a fake "0 multiplicity". Indeed, at first,
                //as this problem was believed to be small and negligible, no good
                //was put in trying to reject it directly using a quality flag. 2017
                //data showed us otherwise.
                int nEmptyEvent = 0;
                int nPhysics = 0;

                //Write the rate header file as well as the corrupted header file
                //This file will still be writen even after the new file format
                //has been used.
                headRateCSV << "Rate-" << partName << "\t"
                            << "ClS-" << partName << "\t"
                            << "ClS-" << partName << "_Err\t"
                            << "ClM-" << partName << "\t"
                            << "ClM-" << partName << "_Err\t"
                            << "ClRate-" << partName << "\t"
                            << "ClRate-" << partName << "_Err\t";

                headCorrCSV << "Corr-" << partName << "\t";

                if(!isNewFormat){

                    //First of all, we will fit the multiplicity using first a gaussian
                    //that is used to get parameter initialisation for a skew fit (it
                    //works better to first fit with a gaussian).
                    //BUT! the fit will hardly work in the case the mean of the distribution
                    //is low and close to 0. To check that the fit worked, we will compare
                    //the value given by the fit for multiplicity 1 (x=1) and ask for a
                    //variation of less than 1% with respect to the data.

                    //Start by getting the x range on wich we should perform the fit
                    //We will re-use the same definition than for the multiplicity
                    //histogram range
                    double Xmax = (double)nBinsMult.rpc[S][p];

                    //Then fit : Gauss then Skew (gauss divided by sigmoid to introduce
                    //an asymmetry)
                    TF1* GaussFit = new TF1("gaussfit","[0]*exp(-0.5*((x-[1])/[2])**2)",0,Xmax);
                    GaussFit->SetParameter(0,100);
                    GaussFit->SetParameter(1,10);
                    GaussFit->SetParameter(2,1);
                    HitMultiplicity_H.rpc[S][p]->Fit(GaussFit,"LIQR","",0.5,Xmax);

                    TF1* SkewFit = new TF1("skewfit","[0]*exp(-0.5*((x-[1])/[2])**2) / (1 + exp(-[3]*(x-[4])))",0,Xmax);
                    SkewFit->SetParameter(0,GaussFit->GetParameter(0));
                    SkewFit->SetParameter(1,GaussFit->GetParameter(1));
                    SkewFit->SetParameter(2,GaussFit->GetParameter(2));
                    SkewFit->SetParameter(3,1);
                    SkewFit->SetParameter(4,1);
                    HitMultiplicity_H.rpc[S][p]->Fit(SkewFit,"LIQR","",0.5,Xmax);

                    //Check that the fit worked:
                    //  - make sure fit gives a value close enough to multiplicity = 1 bin
                    //(variation of less than 1% with respect to the data)
                    //  - make sure there is enough statistics (most of the data is not
                    //contained in multiplicity 0 bin)
                    //Then, if the fit is good but the value of the fit for multiplicity
                    //0 is higher than the content of the data bin, keep the number of empty
                    //events to 0 to make sure it does not turn negative.

                    double fitValue = SkewFit->Eval(1,0,0,0);
                    double dataValue = (double)HitMultiplicity_H.rpc[S][p]->GetBinContent(2);
                    double difference = TMath::Abs(dataValue - fitValue);
                    double fitTOdataVSentries_ratio = difference / (double)nEntries;
                    bool isFitGOOD = fitTOdataVSentries_ratio < 0.01;

                    double nSinglehit = (double)HitMultiplicity_H.rpc[S][p]->GetBinContent(1);
                    double lowMultRatio = nSinglehit / (double)nEntries;
                    bool isMultLOW = lowMultRatio > 0.4;

                    if(isFitGOOD && !isMultLOW){
                        nEmptyEvent = HitMultiplicity_H.rpc[S][p]->GetBinContent(1);
                        nPhysics = (int)SkewFit->Eval(0,0,0,0);
                        if(nPhysics < nEmptyEvent)
                            nEmptyEvent = nEmptyEvent-nPhysics;
                    }
                }

                //Print the percentage of corrupted data and the corresponding header
                double corrupt_ratio = 100.*(double)nEmptyEvent / (double)nEntries;
                outputCorrCSV << corrupt_ratio << '\t';

                //**************** RATE CALCULATION / NOISE HISTO RESCALING *******************

                //Get the mean noise on the strips and chips using the noise hit
                //profile. Normalise the number of hits in each bin by the integrated
                //time and the strip sruface (counts/s/cm2).
                float rate_norm = 0.;

                //Now we can proceed with getting the number of noise/gamma hits
                //and convert it into a noise/gamma rate per unit area.
                //Get the number of noise hits
                int nNoise = StripNoiseProfile_H.rpc[S][p]->GetEntries();

                //Get the strip geometry
                float stripArea = GIFInfra->GetStripGeo(sl,p);

                if(IsEfficiencyRun(RunType)){
                    float noiseWindow = BMTDCWINDOW - TIMEREJECT - 2*PeakWidth.rpc[S][p];
                    rate_norm = (nEntries-nEmptyEvent)*noiseWindow*NANOSEC*stripArea;
                } else
                    rate_norm = (nEntries-nEmptyEvent)*RDMNOISEWDW*NANOSEC*stripArea;

                //Get the average number of hits per strip to normalise the activity
                //histogram (this number is the same for both Strip and Chip histos).
                float averageNhit = (nNoise>0) ? (float)(nNoise/nStripsPart) : 1.;

                for(Uint st = 1; st <= nStripsPart; st++){
                    //Get profit of the loop over strips to subtract the
                    //average background from the beam profile. This average
                    //calculated strip by strip is obtained using a proportionnality
                    //rule on the number of hits measured during the noise
                    //window and the time width of the peak
                    if(IsEfficiencyRun(RunType)){
                        int nNoiseHits = StripNoiseProfile_H.rpc[S][p]->GetBinContent(st);
                        float noiseWindow = BMTDCWINDOW - TIMEREJECT - 2*PeakWidth.rpc[S][p];
                        float peakWindow = 2*PeakWidth.rpc[S][p];
                        float nNoisePeak = nNoiseHits*peakWindow/noiseWindow;

                        int nPeakHits = BeamProfile_H.rpc[S][p]->GetBinContent(st);

                        float correctedContent = (nPeakHits<nNoisePeak) ? 0. : (float)nPeakHits-nNoisePeak;
                        BeamProfile_H.rpc[S][p]->SetBinContent(st,correctedContent);
                    }

                    //Get full RPCCh info usinf format TSCCC
                    Uint RPCCh = (S+1)*1e3 + st + p*nStripsPart;

                    //Fill noise rates and activities, and apply mask
                    float stripRate = StripNoiseProfile_H.rpc[S][p]->GetBinContent(st)/rate_norm;
                    float stripAct = StripNoiseProfile_H.rpc[S][p]->GetBinContent(st)/averageNhit;

                    if(RPCChMap->GetMask(RPCCh) == ACTIVE){
                        StripNoiseProfile_H.rpc[S][p]->SetBinContent(st,stripRate);
                        StripActivity_H.rpc[S][p]->SetBinContent(st,stripAct);
                    } else if (RPCChMap->GetMask(RPCCh) == MASKED){
                        StripNoiseProfile_H.rpc[S][p]->SetBinContent(st,0.);
                        StripActivity_H.rpc[S][p]->SetBinContent(st,0.);
                        MaskNoiseProfile_H.rpc[S][p]->SetBinContent(st,stripRate);
                        MaskActivity_H.rpc[S][p]->SetBinContent(st,stripAct);
                    }
                }

                for(Uint ch = 0; ch < (nStripsPart/NSTRIPSCHIP); ch++){
                    //The chip rate and activity only iare incremented by a rate
                    //that is normalised to the number of active strip per chip
                    ChipMeanNoiseProf_H.rpc[S][p]->SetBinContent(ch+1,GetChipBin(StripNoiseProfile_H.rpc[S][p],ch));
                    ChipActivity_H.rpc[S][p]->SetBinContent(ch+1,GetChipBin(StripActivity_H.rpc[S][p],ch));
                }

                //Write in the output file the mean noise rate per
                //partition
                float MeanPartRate = GetTH1Mean(StripNoiseProfile_H.rpc[S][p]);
                float cSizePart = NoiseCSize_H.rpc[S][p]->GetMean();
                float cSizePartErr = (NoiseCSize_H.rpc[S][p]->GetEntries() == 0)
                        ? 0.
                        : 2*NoiseCSize_H.rpc[S][p]->GetStdDev()/sqrt(NoiseCSize_H.rpc[S][p]->GetEntries());
                float cMultPart = NoiseCMult_H.rpc[S][p]->GetMean();
                float cMultPartErr = (NoiseCMult_H.rpc[S][p]->GetEntries() == 0)
                        ? 0.
                        : 2*NoiseCMult_H.rpc[S][p]->GetStdDev()/sqrt(NoiseCMult_H.rpc[S][p]->GetEntries());
                float ClustPartRate = (cSizePart==0)
                        ? 0.
                        : MeanPartRate/cSizePart;
                float ClustPartRateErr = (cSizePart==0)
                        ? 0.
                        : ClustPartRate * cSizePartErr/cSizePart;

                outputRateCSV << MeanPartRate << '\t'
                              << cSizePart << '\t' << cSizePartErr << '\t'
                              << cMultPart << '\t' << cMultPartErr << '\t'
                              << ClustPartRate << '\t' << ClustPartRateErr << '\t';

                //Get the partition homogeneity defined as exp(RMS(noise)/MEAN(noise))
                //The closer the homogeneity is to 1 the more homogeneus, the closer
                //the homogeneity is to 0 the less homogeneous.
                //This gives idea about noisy strips and dead strips.
                float MeanPartSDev = GetTH1StdDev(StripNoiseProfile_H.rpc[S][p]);
                float strip_homog = (MeanPartRate==0)
                        ? 0.
                        : exp(-MeanPartSDev/MeanPartRate);
                StripHomogeneity_H.rpc[S][p]->Fill("exp -#left(#frac{#sigma_{Strip Rate}}{#mu_{Strip Rate}}#right)",strip_homog);
                StripHomogeneity_H.rpc[S][p]->GetYaxis()->SetRangeUser(0.,1.);

                //Same thing for the chip level - need to get the RMS at the chip level, the mean stays the same
                float ChipStDevMean = GetTH1StdDev(ChipMeanNoiseProf_H.rpc[S][p]);

                float chip_homog = (MeanPartRate==0)
                        ? 0.
                        : exp(-ChipStDevMean/MeanPartRate);
                ChipHomogeneity_H.rpc[S][p]->Fill("exp -#left(#frac{#sigma_{Chip Rate}}{#mu_{Chip Rate}}#right)",chip_homog);
                ChipHomogeneity_H.rpc[S][p]->GetYaxis()->SetRangeUser(0.,1.);

                //Push the partition results into the chamber level
                RPCarea       += stripArea * nStripsPart;
                MeanNoiseRate += MeanPartRate * stripArea * nStripsPart;
                ClusterRate   += ClustPartRate * stripArea * nStripsPart;
                ClusterSDev   += (cSizePart==0)
                        ? 0.
                        : ClusterRate*cSizePartErr/cSizePart;

                //Draw and write the histograms into the output ROOT file
                //******************************* General histograms

                TimeProfile_H.rpc[S][p]->Write();
                HitProfile_H.rpc[S][p]->Write();
                HitMultiplicity_H.rpc[S][p]->Write();

                //******************************* Strip granularity histograms

                StripNoiseProfile_H.rpc[S][p]->Write();
                StripActivity_H.rpc[S][p]->Write();
                StripHomogeneity_H.rpc[S][p]->Write();
                MaskNoiseProfile_H.rpc[S][p]->Write();
                MaskActivity_H.rpc[S][p]->Write();
                NoiseCSize_H.rpc[S][p]->Write();
                NoiseCMult_H.rpc[S][p]->Write();

                //******************************* Chip granularity histograms

                ChipMeanNoiseProf_H.rpc[S][p]->Write();
                ChipActivity_H.rpc[S][p]->Write();
                ChipHomogeneity_H.rpc[S][p]->Write();

                //***************************************************************************

                //Write the efficiency/cluster header file
                headEffCSV << "Eff-" << partName << '\t'
                           << "Eff-" << partName << "_Err\t"
                           << "ClS-" << partName << '\t'
                           << "ClS-" << partName << "_Err\t"
                           << "ClM-" << partName << '\t'
                           << "ClM-" << partName << "_Err\t";

                //For each cases, evaluate the proportion of noise
                //with respect to the actual muon data. The efficiency
                //will then be corrected using this factor to "subtract"
                //the fake efficiency caused by the noise.

                //Get the ratio between the noise calculation window (ns)
                //and the peak window used for muon clustering (ns)
                float noiseWindow = BMTDCWINDOW - TIMEREJECT - 2*PeakWidth.rpc[S][p];
                float peakWindow = 2*PeakWidth.rpc[S][p];
                float windowRatio = peakWindow/noiseWindow;

                //Define the average noise cluster multiplicity in the peak window,
                //the noise mean cluster size, the mean (noise+muon) multiplicity
                //in the peak and the (noise+muon) cluster size and calculate the
                //mean multiplicity of muons alone and the associated cluster size.
                //Get the errors as well.
                float PeakCM = MuonCMult_H.rpc[S][p]->GetMean();
                float PeakCS = MuonCSize_H.rpc[S][p]->GetMean();
                float NoiseCM = NoiseCMult_H.rpc[S][p]->GetMean()*windowRatio;
                float NoiseCS = NoiseCSize_H.rpc[S][p]->GetMean();
                float MuonCM = (PeakCM<NoiseCM)
                        ? 0.
                        : PeakCM-NoiseCM;
                float MuonCS = (MuonCM==0 || PeakCM*PeakCS<NoiseCM*NoiseCS)
                        ? 0.
                        : (PeakCM*PeakCS-NoiseCM*NoiseCS)/MuonCM;

                float PeakCM_err = (MuonCMult_H.rpc[S][p]->GetEntries()==0.)
                        ? 0.
                        : 2*MuonCMult_H.rpc[S][p]->GetStdDev()/sqrt(MuonCMult_H.rpc[S][p]->GetEntries());
                float PeakCS_err = (MuonCSize_H.rpc[S][p]->GetEntries()==0.)
                        ? 0.
                        : 2*MuonCSize_H.rpc[S][p]->GetStdDev()/sqrt(MuonCSize_H.rpc[S][p]->GetEntries());
                float NoiseCM_err = (NoiseCMult_H.rpc[S][p]->GetEntries()==0.)
                        ? 0.
                        : windowRatio*2*NoiseCMult_H.rpc[S][p]->GetStdDev()/sqrt(NoiseCMult_H.rpc[S][p]->GetEntries());
                float NoiseCS_err = (NoiseCSize_H.rpc[S][p]->GetEntries()==0.)
                        ? 0.
                        : 2*NoiseCSize_H.rpc[S][p]->GetStdDev()/sqrt(NoiseCSize_H.rpc[S][p]->GetEntries());
                float MuonCM_err = (MuonCM==0)
                        ? 0.
                        : PeakCM_err+NoiseCM_err;
                float MuonCS_err = (MuonCS==0 || MuonCM==0)
                        ? 0.
                        : (PeakCS*PeakCM_err+PeakCM*PeakCS_err+NoiseCS*NoiseCM_err+NoiseCM*NoiseCS_err+MuonCS*MuonCM_err)/MuonCM;

                //Evaluate the data/(data+noise) ratio by comparing the multiplicities
                float DataRatio = MuonCM/PeakCM;

                //Get corrected efficiency
                float eff = DataRatio*Efficiency0_H.rpc[S][p]->GetMean();
                float eff_err = sqrt(eff*(1-eff)/Efficiency0_H.rpc[S][p]->GetEntries());

                //Write in the output CSV file
                outputEffCSV << eff << '\t' << eff_err << '\t'
                             << MuonCS << '\t' << MuonCS_err << '\t'
                             << MuonCM << '\t' << MuonCM_err << '\t';

                //******************************* muon histograms

                BeamProfile_H.rpc[S][p]->Write();
                Efficiency0_H.rpc[S][p]->Write();
                MuonCSize_H.rpc[S][p]->Write();
                MuonCMult_H.rpc[S][p]->Write();
            }

            //Finalise the calculation of the chamber rate
            MeanNoiseRate /= RPCarea;
            ClusterRate   /= RPCarea;
            ClusterSDev   /= RPCarea;

            //Write the header file
            headRateCSV << "Rate-" << GIFInfra->GetName(sl) << "-TOT\t"
                        << "ClRate-" << GIFInfra->GetName(sl) << "-TOT\t"
                        << "ClRate-" << GIFInfra->GetName(sl) << "-TOT_Err\t";

            //Write the output file
            outputRateCSV << MeanNoiseRate << '\t'
                          << ClusterRate << '\t' << ClusterSDev << '\t';
        }

        headRateCSV << '\n';
        headRateCSV.close();

        outputRateCSV << '\n';
        outputRateCSV.close();

        headCorrCSV << '\n';
        headCorrCSV.close();

        outputCorrCSV << '\n';
        outputCorrCSV.close();

        headEffCSV << '\n';
        headEffCSV.close();

        outputEffCSV << '\n';
        outputEffCSV.close();

        outputfile.Close();
        dataFile.Close();
        MSG_INFO("[Analysis] Offline analysis performed with success!");
    } else {
        MSG_INFO("[Analysis] File " + daqName + " could not be opened");
        MSG_INFO("[Analysis] Skipping offline analysis");
    }
}
