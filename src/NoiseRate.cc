//***************************************************************
// *    904 OFFLINE TOOL v4
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

using namespace std;

//*******************************************************************************

void GetNoiseRate(string baseName){

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

        float PeakMeanTime[NSLOTS][NPARTITIONS] = {{0.}};
        float PeakSpread[NSLOTS][NPARTITIONS] = {{0.}};

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

        TH1I *BeamProf_H[NSLOTS][NPARTITIONS];
        TH1I *NoiseProf_H[NSLOTS][NPARTITIONS];
        TH1F *TimeProfile_H[NSLOTS][NPARTITIONS];
        TH1I *HitMultiplicity_H[NSLOTS][NPARTITIONS];
        TH1I *StripHitProf_H[NSLOTS][NPARTITIONS];
        TH1F *StripMeanNoiseProf_H[NSLOTS][NPARTITIONS];
        TH1F *StripActivity_H[NSLOTS][NPARTITIONS];
        TH1F *StripHomogeneity_H[NSLOTS][NPARTITIONS];
        TH1I *ChipHitProf_H[NSLOTS][NPARTITIONS];
        TH1F *ChipMeanNoiseProf_H[NSLOTS][NPARTITIONS];
        TH1F *ChipActivity_H[NSLOTS][NPARTITIONS];
        TH1F *ChipHomogeneity_H[NSLOTS][NPARTITIONS];

        char hisname[50];  //ID name of the histogram
        char histitle[50]; //Title of the histogram

        unsigned int nSlots = GIFInfra.nSlots;

        for (unsigned int s = 0; s < nSlots; s++){
            unsigned int nPartRPC = GIFInfra.RPCs[s].nPartitions;
            unsigned int slot = CharToInt(GIFInfra.SlotsID[s]) - 1;

            //Get the chamber ID in terms of slot position SX
            string rpcID = GIFInfra.RPCs[s].name;

            for (unsigned int p = 0; p < nPartRPC; p++){
                //Set bining
                unsigned int nStrips = GIFInfra.RPCs[s].strips;
                float low_s = nStrips*p + 0.5;
                float high_s = nStrips*(p+1) + 0.5;

                unsigned int nBinsMult = 101;
                float lowBin = -0.5;
                float highBin = (float)nBinsMult + lowBin;

                //Noise rate bin size depending on the strip surface
                float timeWidth = 1.;

                if(RunType->CompareTo("efficiency") == 0) timeWidth = BMTDCWINDOW;
                else timeWidth = RDMTDCWINDOW;

                //Initialisation of the histograms

                //***************************************** General histograms

                //Beam profile
                SetTitleName(rpcID,p,hisname,histitle,"Beam_Profile","Beam profile");
                BeamProf_H[slot][p] = new TH1I( hisname, histitle, nStrips, low_s, high_s);
                SetTH1(BeamProf_H[slot][p],"Strip","Number of hits");

                //Noise profile
                SetTitleName(rpcID,p,hisname,histitle,"Noise_Profile","Noise profile");
                NoiseProf_H[slot][p] = new TH1I( hisname, histitle, nStrips, low_s, high_s);
                SetTH1(NoiseProf_H[slot][p],"Strip","Number of hits");

                //Time profile
                SetTitleName(rpcID,p,hisname,histitle,"Time_Profile","Time profile");
                TimeProfile_H[slot][p] = new TH1F( hisname, histitle, (int)timeWidth/TIMEBIN, 0., timeWidth);
                SetTH1(TimeProfile_H[slot][p],"Time [ns]","Number of hits");

                //Hit multiplicity
                SetTitleName(rpcID,p,hisname,histitle,"Hit_Multiplicity","Hit multiplicity");
                HitMultiplicity_H[slot][p] = new TH1I( hisname, histitle, nBinsMult, lowBin, highBin);
                SetTH1(HitMultiplicity_H[slot][p],"Multiplicity","Number of events");

                //****************************************** Strip granularuty level histograms

                //Hit profile
                SetTitleName(rpcID,p,hisname,histitle,"Strip_Hit_Profile","Strip hit profile");
                StripHitProf_H[slot][p] = new TH1I( hisname, histitle, nStrips, low_s, high_s);
                SetTH1(StripHitProf_H[slot][p],"Strip","Number of hits");

                //Mean noise rate profile
                SetTitleName(rpcID,p,hisname,histitle,"Strip_Mean_Noise","Strip mean noise rate");
                StripMeanNoiseProf_H[slot][p] = new TH1F( hisname, histitle, nStrips, low_s, high_s);
                SetTH1(StripMeanNoiseProf_H[slot][p],"Strip","Rate [Hz/cm^{2}]");

                //Strip activity
                SetTitleName(rpcID,p,hisname,histitle,"Strip_Activity","Strip activity");
                StripActivity_H[slot][p] = new TH1F( hisname, histitle, nStrips, low_s, high_s);
                SetTH1(StripActivity_H[slot][p],"Strip","Activity (normalized strip profil)");

                //Noise homogeneity
                SetTitleName(rpcID,p,hisname,histitle,"Strip_Homogeneity","Strip homogeneity");
                StripHomogeneity_H[slot][p] = new TH1F( hisname, histitle, 1, 0, 1);
                SetTH1(StripHomogeneity_H[slot][p],"","Homogeneity");

                //****************************************** Chip granularuty level histograms

                //Hit profile
                SetTitleName(rpcID,p,hisname,histitle,"Chip_Hit_Profile","Chip hit profile");
                ChipHitProf_H[slot][p] = new TH1I( hisname, histitle, nStrips/8, low_s, high_s);
                SetTH1(ChipHitProf_H[slot][p],"Chip","Number of hits");

                //Mean noise rate profile
                SetTitleName(rpcID,p,hisname,histitle,"Chip_Mean_Noise","Chip mean noise rate");
                ChipMeanNoiseProf_H[slot][p] = new TH1F( hisname, histitle, nStrips/8, low_s, high_s);
                SetTH1(ChipMeanNoiseProf_H[slot][p],"Chip","Rate [Hz/cm^{2}]");

                //Strip activity
                SetTitleName(rpcID,p,hisname,histitle,"Chip_Activity","Chip activity");
                ChipActivity_H[slot][p] = new TH1F( hisname, histitle, nStrips/8, low_s, high_s);
                SetTH1(ChipActivity_H[slot][p],"Chip","Activity (normalized chip profil)");

                //Noise homogeneity
                SetTitleName(rpcID,p,hisname,histitle,"Chip_Homogeneity","Chip homogeneity");
                ChipHomogeneity_H[slot][p] = new TH1F( hisname, histitle, 1, 0, 1);
                SetTH1(ChipHomogeneity_H[slot][p],"","Homogeneity");
            }
        }

        //****************** MACRO ***************************************

        //Tabel to count the hits in every chamber partitions - used to
        //compute the noise rate
        int Multiplicity[NSLOTS][NPARTITIONS] = { {0} };

        unsigned int nEntries = dataTree->GetEntries();

        for(unsigned int i = 0; i < nEntries; i++){
            dataTree->GetEntry(i);

            //Loop over the TDC hits
            for(int h = 0; h < data.TDCNHits; h++){
                RPCHit hit;

                //Get rid of the noise hits outside of the connected channels
                if(data.TDCCh->at(h) < 3000 || data.TDCCh->at(h) > 3127) continue;
                if(RPCChMap[data.TDCCh->at(h)] == 0) continue;

                SetRPCHit(hit, RPCChMap[data.TDCCh->at(h)], data.TDCTS->at(h), GIFInfra);

                if(RunType->CompareTo("efficiency") == 0){
                    //First define the accepted peak time range
                    float lowlimit = PeakMeanTime[hit.Station-1][hit.Partition-1]
                            - PeakSpread[hit.Station-1][hit.Partition-1];
                    float highlimit = PeakMeanTime[hit.Station-1][hit.Partition-1]
                            + PeakSpread[hit.Station-1][hit.Partition-1];

                    bool peakrange = (hit.TimeStamp >= lowlimit && hit.TimeStamp < highlimit);

                    //Fill the hits inside of the defined noise range
                    if(peakrange)
                        BeamProf_H[hit.Station-1][hit.Partition-1]->Fill(hit.Strip);
                    //Reject the 100 first ns due to inhomogeneity of data
                    else if(hit.TimeStamp >= TIMEREJECT)
                        NoiseProf_H[hit.Station-1][hit.Partition-1]->Fill(hit.Strip);
                } else if(RunType->CompareTo("efficiency") != 0){
                    //Reject the 100 first ns due to inhomogeneity of data
                    if(hit.TimeStamp >= TIMEREJECT)
                        NoiseProf_H[hit.Station-1][hit.Partition-1]->Fill(hit.Strip);
                }

                //Fill the profiles
                StripHitProf_H[hit.Station-1][hit.Partition-1]->Fill(hit.Strip);
                ChipHitProf_H[hit.Station-1][hit.Partition-1]->Fill(hit.Strip);
                TimeProfile_H[hit.Station-1][hit.Partition-1]->Fill(hit.TimeStamp);
                Multiplicity[hit.Station-1][hit.Partition-1]++;
            }

            //********** MULTIPLICITY ************************************

            for(unsigned int sl=0; sl<nSlots; sl++){
                unsigned int nPartRPC = GIFInfra.RPCs[sl].nPartitions;
                unsigned int slot = CharToInt(GIFInfra.SlotsID[sl]) - 1;

                for (unsigned int p = 0; p < nPartRPC; p++){
                    HitMultiplicity_H[slot][p]->Fill(Multiplicity[slot][p]);
                    Multiplicity[slot][p] = 0;
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

        for (unsigned int sl = 0; sl < nSlots; sl++){
            unsigned int nPartRPC = GIFInfra.RPCs[sl].nPartitions;
            unsigned int slot = CharToInt(GIFInfra.SlotsID[sl]) - 1;

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
                        << GIFInfra.RPCs[sl].name
                        << "-" << partID[p]
                        << "\t";

                //Get the mean noise on the strips and chips using the noise hit
                //profile. Normalise the number of hits in each bin by the integrated
                //time and the strip sruface (counts/s/cm2)
                float normalisation = 0.;

                //Get the number of noise hits
                int nNoise = NoiseProf_H[slot][p]->GetEntries();

                //Get the strip geometry
                float stripArea = GIFInfra.RPCs[sl].stripGeo[p];

                if(RunType->CompareTo("efficiency") == 0){
                    float noiseWindow = BMTDCWINDOW - TIMEREJECT - 2*PeakSpread[slot][p];
                    normalisation = nEntries*noiseWindow*1e-9*stripArea;
                } else if(RunType->CompareTo("efficiency") != 0)
                    normalisation = nEntries*RDMNOISEWDW*1e-9*stripArea;

                //Get the average number of hits per strip to normalise the activity
                //histogram (this number is the same for both Strip and Chip histos).
                unsigned int nStripsPart = GIFInfra.RPCs[sl].strips;
                float averageNhit = (nNoise>0) ? (float)(nNoise/nStripsPart) : 1.;

                for(unsigned int st = 1; st <= nStripsPart; st++){
                    //Fill noise rates
                    float stripRate = NoiseProf_H[slot][p]->GetBinContent(st)/normalisation;

                    StripMeanNoiseProf_H[slot][p]->Fill(p*nStripsPart+st,stripRate);

                    //The chip rate only is incremented by a rate that is
                    //normalised to the number of strip per chip
                    ChipMeanNoiseProf_H[slot][p]->Fill(p*nStripsPart+st,stripRate/NSTRIPSCHIP);

                    //Fill activities
                    float stripAct = NoiseProf_H[slot][p]->GetBinContent(st)/averageNhit;

                    StripActivity_H[slot][p]->Fill(p*nStripsPart+st,stripAct);

                    //The chip activity only is incremented by an activity
                    //that is normalised to the number of strip per chip
                    ChipActivity_H[slot][p]->Fill(p*nStripsPart+st,stripAct/NSTRIPSCHIP);

                    //Get profit of the loop over strips to subtract the
                    //average background from the beam profile. This average
                    //calculated strip by strip is obtained using a proportionnality
                    //rule on the number of hits measured during the noise
                    //window and the time width of the peak
                    if(RunType->CompareTo("efficiency") == 0){
                        int nNoiseHits = NoiseProf_H[slot][p]->GetBinContent(st);
                        float noiseWindow = BMTDCWINDOW - TIMEREJECT - 2*PeakSpread[slot][p];
                        float peakWindow = 2*PeakSpread[sl][p];
                        float nNoisePeak = nNoiseHits*peakWindow/noiseWindow;

                        int nPeakHits = BeamProf_H[slot][p]->GetBinContent(st);

                        float correctedContent = (nPeakHits<nNoisePeak) ? 0. : (float)nPeakHits-nNoisePeak;
                        BeamProf_H[slot][p]->SetBinContent(st,correctedContent);
                    }
                }

                //Write in the output file the mean noise rate per
                //partition
                float MeanPartRate = GetTH1Mean(StripMeanNoiseProf_H[slot][p]);
                outputCSV << MeanPartRate << '\t';

                //Get the partition homogeneity defined as exp(RMS(noise)/MEAN(noise))
                //The closer the homogeneity is to 1 the more homogeneus, the closer
                //the homogeneity is to 0 the less homogeneous.
                //This gives idea about noisy strips and dead strips.
                float MeanPartSDev = GetTH1StdDev(StripMeanNoiseProf_H[slot][p]);
                float strip_homog = exp(-MeanPartSDev/MeanPartRate);
                StripHomogeneity_H[slot][p]->Fill(0.,strip_homog);

                //Same thing for the chip level - need to get the RMS at the chip level, the mean stays the same
                float ChipStDevMean = GetTH1StdDev(ChipMeanNoiseProf_H[slot][p]);

                float chip_homog = exp(-ChipStDevMean/MeanPartRate);
                ChipHomogeneity_H[slot][p]->Fill(0.,chip_homog);

                //Push the partition results into the chamber level
                nStripsRPC  += nStripsPart;
                RPCarea     += stripArea * nStripsPart;
                MeanRPCRate += MeanPartRate * stripArea * nStripsPart;
                MeanRPCSDev += MeanPartSDev * stripArea * nStripsPart;

                //Draw and write the histograms into the output ROOT file

                //********************************* General histograms

                BeamProf_H[slot][p]->Write();
                NoiseProf_H[slot][p]->Write();
                TimeProfile_H[slot][p]->Write();
                HitMultiplicity_H[slot][p]->Write();

                //******************************* Strip granularity histograms

                StripHitProf_H[slot][p]->Write();
                StripMeanNoiseProf_H[slot][p]->Write();
                StripActivity_H[slot][p]->Write();

                StripHomogeneity_H[slot][p]->GetYaxis()->SetRangeUser(0.,1.);
                StripHomogeneity_H[slot][p]->Write();

                //******************************* Chip granularity histograms

                ChipHitProf_H[slot][p]->Write();
                ChipMeanNoiseProf_H[slot][p]->Write();
                ChipActivity_H[slot][p]->Write();

                ChipHomogeneity_H[slot][p]->GetYaxis()->SetRangeUser(0.,1.);
                ChipHomogeneity_H[slot][p]->Write();
            }

            //Finalise the calculation of the chamber rate
            MeanRPCRate /= RPCarea;
            MeanRPCSDev /= RPCarea;

            //Write the header file
            listCSV << "Rate-"
                    << GIFInfra.RPCs[sl].name
                    << "-TOT\t";

            //Write the output file
            outputCSV << MeanRPCRate << '\t';
        }
        listCSV << '\n';
        listCSV.close();

        outputCSV << '\n';
        outputCSV.close();

        outputfile.Close();
        dataFile.Close();
    } else {
        MSG_INFO("[Offline-Rate] File " + daqName + " could not be opened");
        MSG_INFO("[Offline-Rate] Skipping rate analysis");
    }
}
