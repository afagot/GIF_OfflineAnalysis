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

        Mapping RPCChMap = TDCMapping(daqName);

        //****************** PEAK TIME ***********************************

        //First open the RunParameters TTree from the dataFile
        //Then link a string to the branch corresponding to the beam
        //status and get the entry
        //Convention : ON = beam trigger , OFF = Random trigger
        TTree* RunParameters = (TTree*)dataFile.Get("RunParameters");
        TString* RunType = new TString();
        RunParameters->SetBranchAddress("RunType",&RunType);
        RunParameters->GetEntry(0);

        muonPeak PeakMeanTime;
        muonPeak PeakSpread;

        if(RunType->CompareTo("efficiency") == 0) SetBeamWindow(PeakMeanTime,PeakSpread,dataTree,RPCChMap,GIFInfra);

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

        //****************** HISTOGRAMS & CANVAS *************************

        TH1I *BeamProf_H[NTROLLEYS][NSLOTS][NPARTITIONS];
        TH1I *NoiseProf_H[NTROLLEYS][NSLOTS][NPARTITIONS];
        TH1F *TimeProfile_H[NTROLLEYS][NSLOTS][NPARTITIONS];
        TH1I *HitMultiplicity_H[NTROLLEYS][NSLOTS][NPARTITIONS];
        TH1I *StripHitProf_H[NTROLLEYS][NSLOTS][NPARTITIONS];
        TH1F *StripMeanNoiseProf_H[NTROLLEYS][NSLOTS][NPARTITIONS];
        TH1F *StripActivity_H[NTROLLEYS][NSLOTS][NPARTITIONS];
        TH1F *MaskMeanNoiseProf_H[NTROLLEYS][NSLOTS][NPARTITIONS];
        TH1F *MaskActivity_H[NTROLLEYS][NSLOTS][NPARTITIONS];
        TH1F *StripHomogeneity_H[NTROLLEYS][NSLOTS][NPARTITIONS];
        TH1I *ChipHitProf_H[NTROLLEYS][NSLOTS][NPARTITIONS];
        TH1F *ChipMeanNoiseProf_H[NTROLLEYS][NSLOTS][NPARTITIONS];
        TH1F *ChipActivity_H[NTROLLEYS][NSLOTS][NPARTITIONS];
        TH1F *ChipHomogeneity_H[NTROLLEYS][NSLOTS][NPARTITIONS];

        char hisname[50];  //ID name of the histogram
        char histitle[50]; //Title of the histogram

        for (Uint t = 0; t < GIFInfra.nTrolleys; t++){
            Uint nSlotsTrolley = GIFInfra.Trolleys[t].nSlots;
            Uint trolley = CharToInt(GIFInfra.TrolleysID[t]);

            for (Uint s = 0; s < nSlotsTrolley; s++){
                Uint nPartRPC = GIFInfra.Trolleys[t].RPCs[s].nPartitions;
                Uint slot = CharToInt(GIFInfra.Trolleys[t].SlotsID[s]) - 1;

                //Get the chamber ID name
                string rpcID = GIFInfra.Trolleys[t].RPCs[s].name;

                for (Uint p = 0; p < nPartRPC; p++){
                    //Set bining
                    Uint nStrips = GIFInfra.Trolleys[t].RPCs[s].strips;
                    float low_s = nStrips*p + 0.5;
                    float high_s = nStrips*(p+1) + 0.5;

                    Uint nBinsMult = 101;
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
                    SetTH1(BeamProf_H[trolley][slot][p],"Strip","Number of hits");

                    //Noise profile
                    SetTitleName(rpcID,p,hisname,histitle,"Noise_Profile","Noise profile");
                    NoiseProf_H[trolley][slot][p] = new TH1I( hisname, histitle, nStrips, low_s, high_s);
                    SetTH1(NoiseProf_H[trolley][slot][p],"Strip","Number of hits");

                    //Time profile
                    SetTitleName(rpcID,p,hisname,histitle,"Time_Profile","Time profile");
                    TimeProfile_H[trolley][slot][p] = new TH1F( hisname, histitle, (int)timeWidth/TIMEBIN, 0., timeWidth);
                    SetTH1(TimeProfile_H[trolley][slot][p],"Time (ns)","Number of hits");

                    //Hit multiplicity
                    SetTitleName(rpcID,p,hisname,histitle,"Hit_Multiplicity","Hit multiplicity");
                    HitMultiplicity_H[trolley][slot][p] = new TH1I( hisname, histitle, nBinsMult, lowBin, highBin);
                    SetTH1(HitMultiplicity_H[trolley][slot][p],"Multiplicity","Number of events");

                    //****************************************** Strip granularuty level histograms

                    //Hit profile
                    SetTitleName(rpcID,p,hisname,histitle,"Strip_Hit_Profile","Strip hit profile");
                    StripHitProf_H[trolley][slot][p] = new TH1I( hisname, histitle, nStrips, low_s, high_s);
                    SetTH1(StripHitProf_H[trolley][slot][p],"Strip","Number of hits");

                    //Mean noise rate profile
                    SetTitleName(rpcID,p,hisname,histitle,"Strip_Mean_Noise","Strip mean noise rate");
                    StripMeanNoiseProf_H[trolley][slot][p] = new TH1F( hisname, histitle, nStrips, low_s, high_s);
                    SetTH1(StripMeanNoiseProf_H[trolley][slot][p],"Strip","Rate (Hz/cm^{2})");

                    //Strip activity
                    SetTitleName(rpcID,p,hisname,histitle,"Strip_Activity","Strip activity");
                    StripActivity_H[trolley][slot][p] = new TH1F( hisname, histitle, nStrips, low_s, high_s);
                    SetTH1(StripActivity_H[trolley][slot][p],"Strip","Activity (normalized strip profil)");

                    //Masked strip mean noise rate profile
                    SetTitleName(rpcID,p,hisname,histitle,"mask_Strip_Mean_Noise","Masked strip mean noise rate");
                    MaskMeanNoiseProf_H[trolley][slot][p] = new TH1F( hisname, histitle, nStrips, low_s, high_s);
                    SetTH1(MaskMeanNoiseProf_H[trolley][slot][p],"Strip","Rate (Hz/cm^{2})");

                    //Masked strip activity
                    SetTitleName(rpcID,p,hisname,histitle,"mask_Strip_Activity","Masked strip activity");
                    MaskActivity_H[trolley][slot][p] = new TH1F( hisname, histitle, nStrips, low_s, high_s);
                    SetTH1(MaskActivity_H[trolley][slot][p],"Strip","Activity (normalized strip profil)");

                    //Noise homogeneity
                    SetTitleName(rpcID,p,hisname,histitle,"Strip_Homogeneity","Strip homogeneity");
                    StripHomogeneity_H[trolley][slot][p] = new TH1F( hisname, histitle, 1, 0, 1);
                    SetTH1(StripHomogeneity_H[trolley][slot][p],"","Homogeneity");

                    //****************************************** Chip granularuty level histograms

                    //Hit profile
                    SetTitleName(rpcID,p,hisname,histitle,"Chip_Hit_Profile","Chip hit profile");
                    ChipHitProf_H[trolley][slot][p] = new TH1I( hisname, histitle, nStrips/8, low_s, high_s);
                    SetTH1(ChipHitProf_H[trolley][slot][p],"Chip","Number of hits");

                    //Mean noise rate profile
                    SetTitleName(rpcID,p,hisname,histitle,"Chip_Mean_Noise","Chip mean noise rate");
                    ChipMeanNoiseProf_H[trolley][slot][p] = new TH1F( hisname, histitle, nStrips/8, low_s, high_s);
                    SetTH1(ChipMeanNoiseProf_H[trolley][slot][p],"Chip","Rate (Hz/cm^{2})");

                    //Strip activity
                    SetTitleName(rpcID,p,hisname,histitle,"Chip_Activity","Chip activity");
                    ChipActivity_H[trolley][slot][p] = new TH1F( hisname, histitle, nStrips/8, low_s, high_s);
                    SetTH1(ChipActivity_H[trolley][slot][p],"Chip","Activity (normalized chip profil)");

                    //Noise homogeneity
                    SetTitleName(rpcID,p,hisname,histitle,"Chip_Homogeneity","Chip homogeneity");
                    ChipHomogeneity_H[trolley][slot][p] = new TH1F( hisname, histitle, 1, 0, 1);
                    SetTH1(ChipHomogeneity_H[trolley][slot][p],"","Homogeneity");
                }
            }
        }

        //****************** MACRO ***************************************

        //Tabel to count the hits in every chamber partitions - used to
        //compute the noise rate
        int Multiplicity[NTROLLEYS][NSLOTS][NPARTITIONS] = { {0} };

        Uint nEntries = dataTree->GetEntries();

        for(Uint i = 0; i < nEntries; i++){
            dataTree->GetEntry(i);

            //Loop over the TDC hits
            for(int h = 0; h < data.TDCNHits; h++){
                RPCHit hit;

                //Get rid of the noise hits outside of the connected channels
                if(data.TDCCh->at(h) > 5127) continue;
                if(RPCChMap.link[data.TDCCh->at(h)] == 0) continue;

                SetRPCHit(hit, RPCChMap.link[data.TDCCh->at(h)], data.TDCTS->at(h), GIFInfra);
                Uint T = hit.Trolley;
                Uint S = hit.Station-1;
                Uint P = hit.Partition-1;

                if(RunType->CompareTo("efficiency") == 0){
                    //First define the accepted peak time range
                    float lowlimit = PeakMeanTime.rpc[T][S][P] - PeakSpread.rpc[T][S][P];
                    float highlimit = PeakMeanTime.rpc[T][S][P] + PeakSpread.rpc[T][S][P];

                    bool peakrange = (hit.TimeStamp >= lowlimit && hit.TimeStamp < highlimit);

                    //Fill the hits inside of the defined noise range
                    if(peakrange)
                        BeamProf_H[T][S][P]->Fill(hit.Strip);
                    //Reject the 100 first ns due to inhomogeneity of data
                    else if(hit.TimeStamp >= TIMEREJECT)
                        NoiseProf_H[T][S][P]->Fill(hit.Strip);
                } else if(RunType->CompareTo("efficiency") != 0){
                    //Reject the 100 first ns due to inhomogeneity of data
                    if(hit.TimeStamp >= TIMEREJECT)
                        NoiseProf_H[T][S][P]->Fill(hit.Strip);
                }

                //Fill the profiles
                StripHitProf_H[T][S][P]->Fill(hit.Strip);
                ChipHitProf_H[T][S][P]->Fill(hit.Strip);
                TimeProfile_H[T][S][P]->Fill(hit.TimeStamp);
                Multiplicity[T][S][P]++;
            }

            //********** MULTIPLICITY ************************************

            for(Uint t=0; t<GIFInfra.nTrolleys; t++){
                Uint nSlotsTrolley = GIFInfra.Trolleys[t].nSlots;
                Uint trolley = CharToInt(GIFInfra.TrolleysID[t]);

                for(Uint sl=0; sl<nSlotsTrolley; sl++){
                    Uint nPartRPC = GIFInfra.Trolleys[t].RPCs[sl].nPartitions;
                    Uint slot = CharToInt(GIFInfra.Trolleys[t].SlotsID[sl]) - 1;

                    for (Uint p = 0; p < nPartRPC; p++){
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
        for (Uint t = 0; t < GIFInfra.nTrolleys; t++){
            Uint nSlotsTrolley = GIFInfra.Trolleys[t].nSlots;
            Uint trolley = CharToInt(GIFInfra.TrolleysID[t]);

            for (Uint sl = 0; sl < nSlotsTrolley; sl++){
                Uint nPartRPC = GIFInfra.Trolleys[t].RPCs[sl].nPartitions;
                Uint slot = CharToInt(GIFInfra.Trolleys[t].SlotsID[sl]) - 1;

                //Get the total chamber rate
                //we need to now the total chamber surface (sum active areas)
                Uint nStripsRPC  = 0;
                float        RPCarea     = 0.;
                float        MeanRPCRate = 0.;
                float        MeanRPCSDev = 0.;

                for (Uint p = 0; p < nPartRPC; p++){
                    string partID = "ABCD";
                    //Write the header file
                    listCSV << "Rate-"
                            << GIFInfra.Trolleys[t].RPCs[sl].name
                            << "-" << partID[p]
                            << "\t";

                    //Get the mean noise on the strips and chips using the noise hit
                    //profile. Normalise the number of hits in each bin by the integrated
                    //time and the strip sruface (counts/s/cm2)
                    float normalisation = 0.;

                    //Get the number of noise hits
                    int nNoise = NoiseProf_H[trolley][slot][p]->GetEntries();

                    //Get the strip geometry
                    float stripArea = GIFInfra.Trolleys[t].RPCs[sl].stripGeo[p];

                    if(RunType->CompareTo("efficiency") == 0){
                        float noiseWindow = BMTDCWINDOW - TIMEREJECT - 2*PeakSpread.rpc[trolley][slot][p];
                        normalisation = nEntries*noiseWindow*1e-9*stripArea;
                    } else if(RunType->CompareTo("efficiency") != 0)
                        normalisation = nEntries*RDMNOISEWDW*1e-9*stripArea;

                    //Get the average number of hits per strip to normalise the activity
                    //histogram (this number is the same for both Strip and Chip histos).
                    Uint nStripsPart = GIFInfra.Trolleys[t].RPCs[sl].strips;
                    float averageNhit = (nNoise>0) ? (float)(nNoise/nStripsPart) : 1.;

                    for(Uint st = 1; st <= nStripsPart; st++){
                        //Get full RPCCh info usinf format TSCCC
                        Uint RPCCh = trolley*1e4 + (slot+1)*1e3 + st + p*nStripsPart;

                        //Fill noise rates
                        float stripRate = NoiseProf_H[trolley][slot][p]->GetBinContent(st)/normalisation;

                        if(RPCChMap.mask[RPCCh] == 1)
                            StripMeanNoiseProf_H[trolley][slot][p]->Fill(p*nStripsPart+st,stripRate);
                        else if (RPCChMap.mask[RPCCh] == 0)
                            MaskMeanNoiseProf_H[trolley][slot][p]->Fill(p*nStripsPart+st,stripRate);

                        //Fill activities
                        float stripAct = NoiseProf_H[trolley][slot][p]->GetBinContent(st)/averageNhit;

                        if(RPCChMap.mask[RPCCh] == 1)
                            StripActivity_H[trolley][slot][p]->Fill(p*nStripsPart+st,stripAct);
                        else if (RPCChMap.mask[RPCCh] == 0)
                            MaskActivity_H[trolley][slot][p]->Fill(p*nStripsPart+st,stripAct);

                        //Get profit of the loop over strips to subtract the
                        //average background from the beam profile. This average
                        //calculated strip by strip is obtained using a proportionnality
                        //rule on the number of hits measured during the noise
                        //window and the time width of the peak
                        if(RunType->CompareTo("efficiency") == 0){
                            int nNoiseHits = NoiseProf_H[trolley][slot][p]->GetBinContent(st);
                            float noiseWindow = BMTDCWINDOW - TIMEREJECT - 2*PeakSpread.rpc[trolley][slot][p];
                            float peakWindow = 2*PeakSpread.rpc[trolley][slot][p];
                            float nNoisePeak = nNoiseHits*peakWindow/noiseWindow;

                            int nPeakHits = BeamProf_H[trolley][slot][p]->GetBinContent(st);

                            float correctedContent = (nPeakHits<nNoisePeak) ? 0. : (float)nPeakHits-nNoisePeak;
                            BeamProf_H[trolley][slot][p]->SetBinContent(st,correctedContent);
                        }
                    }

                    for(Uint ch = 0; ch < (nStripsPart/NSTRIPSCHIP); ch++){
                        //The chip rate and activity only iare incremented by a rate
                        //that is normalised to the number of active strip per chip
                        ChipMeanNoiseProf_H[trolley][slot][p]->SetBinContent(ch+1,GetChipBin(StripMeanNoiseProf_H[trolley][slot][p],ch));
                        ChipActivity_H[trolley][slot][p]->SetBinContent(ch+1,GetChipBin(StripActivity_H[trolley][slot][p],ch));
                    }

                    //Write in the output file the mean noise rate per
                    //partition
                    float MeanPartRate = GetTH1Mean(StripMeanNoiseProf_H[trolley][slot][p]);
                    outputCSV << MeanPartRate << '\t';

                    //Get the partition homogeneity defined as exp(RMS(noise)/MEAN(noise))
                    //The closer the homogeneity is to 1 the more homogeneus, the closer
                    //the homogeneity is to 0 the less homogeneous.
                    //This gives idea about noisy strips and dead strips.
                    float MeanPartSDev = GetTH1StdDev(StripMeanNoiseProf_H[trolley][slot][p]);
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
                    MaskMeanNoiseProf_H[trolley][slot][p]->Write();
                    MaskActivity_H[trolley][slot][p]->Write();

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
                        << "-TOT\t";

                //Write the output file
                outputCSV << MeanRPCRate << '\t';
            }
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
